#include "mainwindow.h"
#include <QApplication>
#include <QFile>
#include "AI/openai_request.h"
#include "QKeyEvent"
#include "QMenuBar"
#include "QDir"
#include "QComboBox"
#include "audio/audiorecorder.h"
#include "QTimer"
#include "calendarwidget.h"
#include "QJsonObject"

MainWindow *MainWindow::singleton = NULL;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    if (!singleton) {
        singleton = this;
    }

    qApp->setOrganizationName("BenProductions");
    qApp->setApplicationName("ADHD");

    settings = new QSettings;

    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    layout = new QVBoxLayout(centralWidget);

    apiKeyButton = new QPushButton(/*this*/);
    updateApiKeyButtonLabel();
    darkModeButton = new QPushButton(this);

    connect(apiKeyButton, &QPushButton::clicked, this, &MainWindow::onApiKeyButtonClicked);
    connect(darkModeButton, &QPushButton::clicked, this, &MainWindow::onDarkModeButtonClicked);


    // Create the dropdown menu for voice selection
    voiceSelectionComboBox = new QComboBox(this);
    QStringList voices = {"alloy", "echo", "fable", "onyx", "nova", "shimmer"};
    voiceSelectionComboBox->addItems(voices);
    connect(voiceSelectionComboBox, &QComboBox::currentTextChanged, this, [=]{
        voice = voiceSelectionComboBox->currentText();
    });


    // Create the text input field
    textInputField = new QLineEdit(this);
    textInputField->setPlaceholderText("Send a message to ADHD Task Manager...");
    textInputField->installEventFilter(this);

    connect(textInputField, &QLineEdit::textChanged, this, [=](const QString &text){
        sendChatButton->setEnabled(text != "");
    });

    audioRecorder = new AudioRecorder();
    recordAudioButton = new QPushButton("Record Audio", this);

    connect(recordAudioButton, &QPushButton::clicked, audioRecorder, &AudioRecorder::toggleRecord);

    // hard code this
    apiKey = "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";

    // STT request
    whisperRequest = new OpenAIRequest();
    whisperRequest->setModel("whisper-1");
    whisperRequest->setAccessToken(apiKey);

    // chat request
    chatRequest = new OpenAIRequest();
    chatRequest->setModel("gpt-3.5-turbo-16k");
    chatRequest->setAccessToken(apiKey);

    QDateTime currentDateTime = QDateTime::currentDateTime();
    QString dateTimeStr = currentDateTime.toString("yyyy-MM-dd ddd HH:mm");

    QJsonObject systemPrompt;

    systemPrompt["prompt"] = "You are part of an app called ADHD Task Manager. The app is an improved task management app "
                             "specifically made for people with ADHD. You are a chatbot built into the app that can "
                             "modify the user's calendar. You can also tell the user about their schedule.";

    systemPrompt["current date and time"] = dateTimeStr;

    chatRequest->addMessage(new OpenAIMessage(systemPrompt, OpenAIMessage::Role::System));

    // TTS request
    speechRequest = new OpenAIRequest();
    speechRequest->setModel("tts-1");
    speechRequest->setAccessToken(apiKey);
    speechRequest->setFilePath(QCoreApplication::applicationDirPath() + QDir::separator() + "speech.mp3");
    speechRequest->setResponseFormat("mp3");

    // say the response out loud
    connect(chatRequest, &OpenAIRequest::requestFinished, this, &MainWindow::say);

    // transcribe the user's voice
    connect(audioRecorder, &AudioRecorder::recordingFinished, this, &MainWindow::transcribe);
    connect(whisperRequest, &OpenAIRequest::requestFinished, textInputField, &QLineEdit::setText);


    sendChatButton = new QPushButton("Send Chat", this);
    sendChatButton->setEnabled(false);
    connect(sendChatButton, &QPushButton::clicked, this, &MainWindow::sendChat);

    // Adding widgets to the top layout
    QHBoxLayout *topLayout = new QHBoxLayout();
//    topLayout->addWidget(apiKeyButton);
    topLayout->addWidget(darkModeButton);
    topLayout->addWidget(voiceSelectionComboBox);
    topLayout->addWidget(recordAudioButton);
    topLayout->addWidget(textInputField);
    topLayout->addWidget(sendChatButton);

    // Calendar widget
    calendarWidget = new CalendarWidget(this);
    calendarWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // Adding layouts and widgets to the main layout
    layout->addLayout(topLayout); // Add the top layout first
    layout->addWidget(calendarWidget, 1); // Calendar takes most of the space

    loadSettings();
}


MainWindow::~MainWindow()
{

}

MainWindow *MainWindow::self()
{
    if (!singleton) {
        singleton = new MainWindow();
    }
    return singleton;
}

void MainWindow::say(const QString &text)
{
    if (text == "") return;

    speechRequest->setTtsVoice(voice);
    speechRequest->setTtsInputText(text);
    speechRequest->execute();
}

void MainWindow::sendChat()
{
    if (textInputField->text() == "") return;

    OpenAIMessage *userMessage = new OpenAIMessage("", OpenAIMessage::Role::User);
    userMessage->setUserMessage(textInputField->text());
    textInputField->clear();

    chatRequest->addMessage(userMessage);
    chatRequest->execute();
}

void MainWindow::transcribe()
{
    whisperRequest->setFilePath(audioRecorder->getOutputLocation().toLocalFile());
    whisperRequest->execute();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    saveSettings();

    QMainWindow::closeEvent(event);
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    // Check if Command (Meta) key is pressed and the key event is for 'W'
    if (event->modifiers() == Qt::ControlModifier && event->key() == Qt::Key_W)
    {
        close();
        return;
    }

    QMainWindow::keyPressEvent(event);
}

void MainWindow::onApiKeyButtonClicked()
{
    QString prompt = "Enter your OpenAI API Key:";
    QString currentApiKey = apiKey;

    bool ok;
    QString newApiKey = QInputDialog::getText(this, "API Key", prompt, QLineEdit::Normal, currentApiKey, &ok);
    if (!ok) return;

    apiKey = newApiKey;
    saveSettings();

    updateApiKeyButtonLabel();
}

void MainWindow::onDarkModeButtonClicked()
{
    isDarkMode = !isDarkMode; // Toggle dark mode
    saveSettings();
    setDarkMode(isDarkMode); // Apply the selected mode
}

void MainWindow::setDarkMode(bool darkMode)
{
    QString path = darkMode ? ":/style/darkStyle.qss" : ":/style/lightStyle.qss";

    QFile file(path);
    if (file.open(QFile::ReadOnly | QFile::Text)) {
        QString styleSheet = file.readAll();
        qApp->setStyleSheet(styleSheet);
        qApp->processEvents();
    }

    if (darkMode) {
        darkModeButton->setText("Switch to Light Mode");
    } else {
        darkModeButton->setText("Switch to Dark Mode");
    }
}

void MainWindow::saveSettings()
{
    settings->setValue("apiKey", apiKey);
    settings->setValue("isDarkMode", isDarkMode);
    settings->setValue("voice", voice);

    settings->setValue("mainWindow/geometry", saveGeometry());
    settings->setValue("mainWindow/windowState", saveState());

    calendarWidget->saveSettings();
}

void MainWindow::loadSettings()
{
    apiKey = settings->value("apiKey").toString();
    isDarkMode = settings->value("isDarkMode").toBool();
    voice = settings->value("voice").toString();

    setDarkMode(isDarkMode);
    voiceSelectionComboBox->setCurrentText(voice);

    calendarWidget->loadSettings();

    restoreGeometry(settings->value("mainWindow/geometry").toByteArray());
    restoreState(settings->value("mainWindow/windowState").toByteArray());
}

void MainWindow::updateApiKeyButtonLabel()
{
    if (apiKey.isEmpty()) {
        apiKeyButton->setText("Add API Key");
    } else {
        apiKeyButton->setText("Change API Key");
    }
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == textInputField && event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) {
            sendChatButton->click();
            return true;
        }
    }
    return QObject::eventFilter(obj, event);
}











