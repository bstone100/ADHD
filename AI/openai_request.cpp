#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QUrl>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QHttpMultiPart>
#include <QHttpPart>
#include <QFile>

#include "QtMultimedia/qaudiooutput.h"
#include "QtMultimedia/qmediaplayer.h"
#include "openai_request.h"
#include "../api.h"

OpenAIRequest::OpenAIRequest(QObject *parent)
    : QObject(parent)
    , m_networkAccessManager(new QNetworkAccessManager(this))
    , m_accessToken("")
    , m_model("")
    , m_filePath("")
    , m_generatedText("")
    , m_errorString("")
    , m_status(RequestStatus::Idle)
    , m_maxTokens(100)
    , m_temperature(0.3)
    , m_topP(1.0)
    , m_frequencyPenalty(0.0)
    , m_presencePenalty(0.0)
    , m_speed(1.0)
{
    connect(this, &OpenAIRequest::requestFinished, this, [this](const QString& generatedText) {
        m_generatedText = generatedText;
        emit generatedTextChanged();
        m_status = RequestStatus::Success;
        emit statusChanged();

        qDebug() << generatedText;
    });
    connect(this, &OpenAIRequest::requestError, this, [this](const QString& errorString) {
        m_errorString = errorString;
        emit errorStringChanged();
        m_status = RequestStatus::Error;
        emit statusChanged();

        qDebug() << errorString;
    });
}

OpenAIRequest::~OpenAIRequest()
{
    for (OpenAIMessage *message : m_messages) {
        message->deleteLater();
    }
    m_messages.clear();
}

void OpenAIRequest::execute()
{
    qDebug() << "executing " << m_model << " request";
    if (m_model.startsWith("gpt"))
    {
        sendChatCompletionsRequest();
    }
    else if (m_model.startsWith("whisper"))
    {
        sendAudioTranscriptionsRequest();
    }
    else if (m_model.startsWith("tts"))
    {
        sendAudioSpeechRequest();
    }
}

QString OpenAIRequest::ttsInputText() const
{
    return m_ttsInputText;
}

void OpenAIRequest::setTtsInputText(const QString &newTtsInputText)
{
    m_ttsInputText = newTtsInputText;
}

QString OpenAIRequest::ttsVoice() const
{
    return m_ttsVoice;
}

void OpenAIRequest::setTtsVoice(const QString &newTtsVoice)
{
    m_ttsVoice = newTtsVoice;
}

QString OpenAIRequest::responseFormat() const
{
    return m_responseFormat;
}

void OpenAIRequest::setResponseFormat(const QString &newResponseFormat)
{
    m_responseFormat = newResponseFormat;
}

double OpenAIRequest::speed() const
{
    return m_speed;
}

void OpenAIRequest::setSpeed(double newSpeed)
{
    m_speed = newSpeed;
}

// Send a request to the OpenAI API
void OpenAIRequest::sendChatCompletionsRequest()
{
    // Set up the API endpoint URL
    const QUrl endpointUrl("https://api.openai.com/v1/chat/completions");

    // Set up the request headers
    QNetworkRequest request(endpointUrl);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", ("Bearer " + m_accessToken).toUtf8());

    // Set up the request body
    QJsonObject requestBody;
    requestBody.insert("model", m_model);
//    requestBody.insert("max_tokens", m_maxTokens);
    requestBody.insert("temperature", m_temperature);
    requestBody.insert("top_p", m_topP);
    requestBody.insert("frequency_penalty", m_frequencyPenalty);
    requestBody.insert("presence_penalty", m_presencePenalty);

    // only the newest message should have a scenegraph
    removeAllScenegraphs();
    auto newestMessage = m_messages.last();
    if (newestMessage) {
        switch (newestMessage->role()) {
        case OpenAIMessage::Role::User:
            newestMessage->addScenegraph();
            requestBody.insert("tools", API::getToolsJsonArray());
            break;
        default:
            break;
        }
    }

    QJsonArray messageArray;
    foreach (OpenAIMessage *message, m_messages) {
        QJsonObject messageObject;
        messageObject.insert("role", OpenAIMessage::roleToString(message->role()));
        messageObject.insert("content", message->content());

        if (!message->tool_calls().isEmpty()) {
            messageObject.insert("tool_calls", message->tool_calls());
        }
        if (message->tool_call_id() != "") {
            messageObject.insert("tool_call_id", message->tool_call_id());
        }

        messageArray.append(messageObject);
    }
    requestBody.insert("messages", messageArray);

    QJsonDocument requestBodyJson(requestBody);
    QByteArray requestBodyBytes = requestBodyJson.toJson(QJsonDocument::Compact);

    // Send the request
    QNetworkReply *reply = m_networkAccessManager->post(request, requestBodyBytes);
    m_status = RequestStatus::InProgress;
    emit statusChanged();

    // Connect signals and slots to handle the response
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            const QByteArray responseBytes = reply->readAll();
            const QJsonDocument responseJson = QJsonDocument::fromJson(responseBytes);

//            qDebug() << responseJson;

            // Handle the response
            const auto message = responseJson.object().value("choices").toArray().at(0).toObject().value("message").toObject();
            QString content = message.value("content").toString();
            QJsonArray tool_calls = message.value("tool_calls").toArray();

            qDebug() << message;

            OpenAIMessage *assistantMessage = new OpenAIMessage(content, OpenAIMessage::Role::Assistant);
            assistantMessage->setTool_calls(tool_calls);
            addMessage(assistantMessage);

            API::processToolCalls(tool_calls, this);

            emit requestFinished(content);
        } else {
            emit requestError(reply->errorString() + reply->readAll());
        }

        reply->deleteLater();
    });
}

void OpenAIRequest::sendAudioTranscriptionsRequest()
{
    // Set up the API endpoint URL
    const QUrl endpointUrl("https://api.openai.com/v1/audio/transcriptions");

    // Set up the request headers
    QNetworkRequest request(endpointUrl);
    request.setRawHeader("Authorization", ("Bearer " + m_accessToken).toUtf8());

    // Set up the form data
    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
 
    // Add a JSON parameter
    QHttpPart modelPart;
    modelPart.setHeader(QNetworkRequest::ContentTypeHeader, "text/plain");
    modelPart.setHeader(QNetworkRequest::ContentDispositionHeader, "form-data; name=\"model\"");
    modelPart.setBody(m_model.toUtf8());

    QHttpPart filePart;
    filePart.setHeader(QNetworkRequest::ContentTypeHeader, "audio/wav");
    filePart.setHeader(QNetworkRequest::ContentDispositionHeader, "form-data; name=\"file\"; filename=\"audio.wav\"");
    QFile *file = new QFile(m_filePath);
    file->open(QIODevice::ReadOnly);
    filePart.setBodyDevice(file);
    file->setParent(multiPart); // Ownership transfer
    
    multiPart->append(filePart);
    multiPart->append(modelPart);

    // Send the request
    QNetworkReply *reply = m_networkAccessManager->post(request, multiPart);
    multiPart->setParent(reply); // Ownership transfer
    m_status = RequestStatus::InProgress;
    emit statusChanged();

    // Connect signals and slots to handle the response
    connect(reply, &QNetworkReply::finished, this, [this, file, reply]() {
        file->close();
        if (reply->error() == QNetworkReply::NoError) {
            const QByteArray responseBytes = reply->readAll();
            const QJsonDocument responseJson = QJsonDocument::fromJson(responseBytes);

            // Handle the response
            const QString generatedText = responseJson.object().value("text").toString();
            emit requestFinished(generatedText);
        } else {
            qDebug() << reply->errorString();
            emit requestError(reply->errorString());
        }

        reply->deleteLater();
    });
}

void OpenAIRequest::sendAudioTranscriptionsRequestLocal()
{
    // Set up the local server URL, adjust the port if needed
    const QUrl localServerUrl("http://localhost:8080/transcribe");

    // Set up the request headers
    QNetworkRequest request(localServerUrl);
    // No Authorization header needed for local server

    // Set up the form data
    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    QHttpPart filePart;
    filePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("audio/wav")); // Ensure the content type matches your audio file
    filePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"file\"; filename=\"audio.wav\""));
    QFile *file = new QFile(m_filePath);
    if(file->open(QIODevice::ReadOnly)) {
        filePart.setBodyDevice(file);
        file->setParent(multiPart); // Ownership transfer

        multiPart->append(filePart);

        // Send the request
        QNetworkReply *reply = m_networkAccessManager->post(request, multiPart);
        multiPart->setParent(reply); // Ownership transfer
        m_status = RequestStatus::InProgress;
        emit statusChanged();

        // Connect signals and slots to handle the response
        connect(reply, &QNetworkReply::finished, this, [this, file, reply]() {
            file->close();
            if (reply->error() == QNetworkReply::NoError) {
                const QByteArray responseBytes = reply->readAll();
                const QJsonDocument responseJson = QJsonDocument::fromJson(responseBytes);

                // Handle the response
                // Assuming the Flask server returns a JSON with a "transcription" field
                const QString generatedText = responseJson.object().value("transcription").toString();
                emit requestFinished(generatedText);
            } else {
                qDebug() << reply->errorString();
                emit requestError(reply->errorString());
            }

            reply->deleteLater();
        });
    } else {
        qDebug() << "Unable to open file for reading";
        emit requestError("Unable to open file");
    }
}


void OpenAIRequest::sendAudioSpeechRequest()
{
    // Set up the API endpoint URL
    const QUrl endpointUrl("https://api.openai.com/v1/audio/speech");

    // Set up the request headers
    QNetworkRequest request(endpointUrl);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", ("Bearer " + m_accessToken).toUtf8());

    // Set up the request body
    QJsonObject requestBody;
    requestBody.insert("model", m_model); // tts-1 or tts-1-hd
    requestBody.insert("input", m_ttsInputText); // Text to be converted to speech
    requestBody.insert("voice", m_ttsVoice); // Voice selection (alloy, echo, etc.)

    // Optional parameters
    if (m_responseFormat != "") {
        requestBody.insert("response_format", m_responseFormat); // mp3, opus, aac, flac
    }
    if (m_speed != 1.0) {
        requestBody.insert("speed", m_speed); // Speed of the generated audio
    }

    QJsonDocument requestBodyJson(requestBody);
    QByteArray requestBodyBytes = requestBodyJson.toJson();

    // Send the request
    QNetworkReply *reply = m_networkAccessManager->post(request, requestBodyBytes);
    m_status = RequestStatus::InProgress;
    emit statusChanged();

    // Connect signals and slots to handle the response
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            // The response will be the audio file content
            const QByteArray responseBytes = reply->readAll();
            emit requestFinished("speech received");
            playAudio(responseBytes);
        } else {
            emit requestError(reply->errorString());
        }
        reply->deleteLater();
    });
}

void OpenAIRequest::playAudio(const QByteArray &audioData)
{
    QFile file(m_filePath);
    if(file.open(QIODevice::WriteOnly)) {
        file.write(audioData);
        file.close();
    } else {
        return;
    }

    QMediaPlayer *mediaPlayer = new QMediaPlayer(this);
    QAudioOutput *audioOutput = new QAudioOutput(this);

    mediaPlayer->setAudioOutput(audioOutput);
    mediaPlayer->setSource(QUrl::fromLocalFile(m_filePath));
    audioOutput->setVolume(100);
    mediaPlayer->play();
}


QString OpenAIRequest::accessToken() const
{
    return m_accessToken;
}

void OpenAIRequest::setAccessToken(const QString& accessToken)
{
    if (m_accessToken == accessToken)
        return;
    m_accessToken = accessToken;
}

QString OpenAIRequest::model() const
{
    return m_model;
}

void OpenAIRequest::setModel(const QString& model)
{
    if (m_model == model)
        return;
    m_model = model;
}

QString OpenAIRequest::filePath() const
{
    return m_filePath;
}

void OpenAIRequest::setFilePath(const QString& filePath)
{
    if (m_filePath == filePath)
        return;
    m_filePath = filePath;
}

QString OpenAIRequest::generatedText() const
{
    return m_generatedText;
}

QString OpenAIRequest::errorString() const
{
    return m_errorString;
}

OpenAIRequest::RequestStatus OpenAIRequest::status() const
{
    return m_status;
}

int OpenAIRequest::maxTokens() const
{
    return m_maxTokens;
}

void OpenAIRequest::setMaxTokens(int maxTokens)
{
    if (m_maxTokens == maxTokens)
        return;
    m_maxTokens = maxTokens;
}

double OpenAIRequest::temperature() const
{
    return m_temperature;
}

void OpenAIRequest::setTemperature(double temperature)
{
    if (qFuzzyCompare(m_temperature, temperature))
        return;
    m_temperature = temperature;
}

double OpenAIRequest::topP() const
{
    return m_topP;
}

void OpenAIRequest::setTopP(double topP)
{
    if (qFuzzyCompare(m_topP, topP))
        return;
    m_topP = topP;
}

double OpenAIRequest::frequencyPenalty() const
{
    return m_frequencyPenalty;
}

void OpenAIRequest::setFrequencyPenalty(double frequencyPenalty)
{
    if (qFuzzyCompare(m_frequencyPenalty, frequencyPenalty))
        return;
    m_frequencyPenalty = frequencyPenalty;
}

double OpenAIRequest::presencePenalty() const
{
    return m_presencePenalty;
}

void OpenAIRequest::setPresencePenalty(double presencePenalty)
{
    if (qFuzzyCompare(m_presencePenalty, presencePenalty))
        return;
    m_presencePenalty = presencePenalty;
}

QList<OpenAIMessage*> OpenAIRequest::messages() const
{
    return m_messages;
}

void OpenAIRequest::setMessages(const QList<OpenAIMessage *> &newMessages)
{
    if (m_messages == newMessages)
        return;
    m_messages = newMessages;
}

void OpenAIRequest::addMessage(OpenAIMessage *newMessage)
{
    if (!newMessage) return;

    m_messages.append(newMessage);

    saveMessagesToFile();
}

void OpenAIRequest::removeMessage(OpenAIMessage *message)
{
    if (!message) return;

    m_messages.removeOne(message);
}

void OpenAIRequest::removeAllMessages()
{
    qDeleteAll(m_messages);
    m_messages.clear();
}

void OpenAIRequest::removeAllScenegraphs()
{
    foreach (auto message, m_messages) {
        message->removeScenegraph();
    }
}

void OpenAIRequest::removeAllInstructions()
{
    foreach (auto message, m_messages) {
        message->removeInstructions();
    }
}

void OpenAIRequest::saveMessagesToFile() const
{
    QFile file("messages.json");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "Failed to open file for writing:" << file.errorString();
        return;
    }

    QJsonArray messageArray;
    foreach (OpenAIMessage *message, m_messages) {
        QJsonObject messageObject;
        messageObject.insert("role", OpenAIMessage::roleToString(message->role()));
        messageObject.insert("content", message->content());

        if (!message->tool_calls().isEmpty()) {
            messageObject.insert("tool_calls", message->tool_calls());
        }
        if (message->tool_call_id() != "") {
            messageObject.insert("tool_call_id", message->tool_call_id());
        }
//        if (message->instructions() != "") {
//            messageObject.insert("instructions", message->instructions());
//        }

        messageArray.append(messageObject);
    }

    QJsonObject requestBody;
    requestBody.insert("messages", messageArray);
    requestBody.insert("tools", API::getToolsJsonArray());

    QJsonDocument doc(requestBody);
    file.write(doc.toJson());
    file.close();
}


