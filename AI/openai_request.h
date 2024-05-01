#pragma once

#include <QObject>
#include <QNetworkAccessManager>
#include "QtMultimedia/qmediaplayer.h"
#include "openai_message.h"


class AudioLevelCalculator;

class OpenAIRequest : public QObject
{
    Q_OBJECT

public:
    explicit OpenAIRequest(QObject *parent = nullptr);
    virtual ~OpenAIRequest();

    enum class RequestStatus {
        Idle,
        InProgress,
        Error,
        Success
    };

    QString accessToken() const;
    void setAccessToken(const QString& accessToken);

    QString model() const;
    void setModel(const QString& model);

    QString filePath() const;
    void setFilePath(const QString& filePath);

    QString generatedText() const;
    QString errorString() const;
    RequestStatus status() const;

    int maxTokens() const;
    void setMaxTokens(int maxTokens);

    double temperature() const;
    void setTemperature(double temperature);

    double topP() const;
    void setTopP(double topP);

    double frequencyPenalty() const;
    void setFrequencyPenalty(double frequencyPenalty);

    double presencePenalty() const;
    void setPresencePenalty(double presencePenalty);

    QList<OpenAIMessage*> messages() const;
    void setMessages(const QList<OpenAIMessage *> &newMessages);

    void addMessage(OpenAIMessage *newMessage);
    void removeMessage(OpenAIMessage *message);
    void removeAllMessages();

    void removeAllScenegraphs();
    void removeAllInstructions();
    void removeAllTimestamps();

    QString ttsInputText() const;
    void setTtsInputText(const QString &newTtsInputText);

    QString ttsVoice() const;
    void setTtsVoice(const QString &newTtsVoice);

    QString responseFormat() const;
    void setResponseFormat(const QString &newResponseFormat);

    double speed() const;
    void setSpeed(double newSpeed);

    // execute a request
    void execute();

    void saveMessagesToFile() const;

signals:
    // Signal emitted when the request is finished successfully
    void requestFinished(const QString& generatedText);

    // Signal emitted when the request encounters an error
    void requestError(const QString& errorString);

    void generatedTextChanged();
    void errorStringChanged();
    void statusChanged();

private:
    QNetworkAccessManager *m_networkAccessManager;
    QString m_accessToken;
    QString m_model;
    QString m_filePath;
    QString m_generatedText;
    QString m_errorString;
    RequestStatus m_status;
    int m_maxTokens;
    double m_temperature;
    double m_topP;
    double m_frequencyPenalty;
    double m_presencePenalty;
    QList<OpenAIMessage*> m_messages;

    QString m_ttsInputText;
    QString m_ttsVoice;
    QString m_responseFormat;
    double  m_speed;

    void sendChatCompletionsRequest();
    void sendAudioTranscriptionsRequest();
    void sendAudioTranscriptionsRequestLocal();
    void sendAudioSpeechRequest();

    void playAudio(const QByteArray &audioData);

    static const int MAX_SELF_RESPONSE_CALLS = 5;
    int selfResponseCount = 0;
};




