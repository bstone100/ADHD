#include "mainwindow.h"
#include <QApplication>
#include <QFile>
#include "AI/openai_request.h"
#include "QKeyEvent"
#include "QMenuBar"
#include "QDir"
#include "QComboBox"
#include "QtCore/qjsondocument.h"
#include "audio/audiorecorder.h"
#include "QTimer"
#include "calendarwidget.h"
#include "QJsonObject"
#include "qstandardpaths.h"
#include "sidepanel.h"
#include "QSvgRenderer"
#include "svgbutton.h"
#include "iOS/hapticfeedback.h"
#include "iOS/DarkModeDetector.h"
#include "macOS/MacThemeDetector.h"
#include "QButtonGroup"
#include "audio/audiolevel.h"

MainWindow *MainWindow::singleton = NULL;
QString MainWindow::currentPath;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    if (!singleton) {
        singleton = this;
    }

    settingsLoaded = false;

    qApp->setOrganizationName("BenProductions");
    qApp->setApplicationName("ADHD");

    qApp->installEventFilter(this);


// Preprocessor directives to check the platform
#if defined(Q_OS_IOS)
    currentPath = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation).value(0);
#elif defined(Q_OS_MACOS) || defined(Q_OS_WIN)
    currentPath = QCoreApplication::applicationDirPath();
#else
    // Default to AppDataLocation for other platforms
    currentPath = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation).value(0);
#endif

    QDir dir(currentPath);
    if (!dir.exists()) {
        dir.mkpath(currentPath);
    }

    settings = new QSettings;

    centralWidget = new QWidget(this);
    centralWidget->setFocusPolicy(Qt::StrongFocus);
    setCentralWidget(centralWidget);

    layout = new QVBoxLayout(centralWidget);

    apiKeyButton = new QPushButton(/*this*/);
    updateApiKeyButtonLabel();
    connect(apiKeyButton, &QPushButton::clicked, this, &MainWindow::onApiKeyButtonClicked);


    themeComboBox = new QComboBox();
    QStringList themes = {"Light", "Dark", "Auto"};
    themeComboBox->addItems(themes);
    connect(themeComboBox, &QComboBox::currentTextChanged, this, [=]{
        int index = themeComboBox->currentIndex();
        if (index == 0) {
            isAutoTheme = false;
            handleThemeChange(false);
        } else if (index == 1) {
            isAutoTheme = false;
            handleThemeChange(true);
        } else if (index == 2) {
            isAutoTheme = true;
            handleThemeChange(isSystemDark());
        }
    });


    // Create the dropdown menu for voice selection
    voiceSelectionComboBox = new QComboBox();
    QStringList voices = {"alloy", "echo", "fable", "onyx", "nova", "shimmer"};
    voiceSelectionComboBox->addItems(voices);
    connect(voiceSelectionComboBox, &QComboBox::currentTextChanged, this, [=]{
        voice = voiceSelectionComboBox->currentText();
    });


// fixes mac combo box behavior
#if defined(Q_OS_MACOS)
    voiceSelectionComboBox->setStyleSheet("combobox-popup: 0;");
    themeComboBox->setStyleSheet("combobox-popup: 0;");
#endif


    // Create the text input field
    textInputField = new QLineEdit(this);
    textInputField->setPlaceholderText("Send a message...");
    textInputField->installEventFilter(this);
    textInputField->setFixedHeight(40);

    connect(textInputField, &QLineEdit::textChanged, this, [=](const QString &text){
        sendChatButton->setEnabled(text != "");
    });

    audioRecorder = new AudioRecorder();

    recordAudioButton = new SvgButton(audioRecorder->getLevelWidget());
    recordAudioButton->setSvgPath(":/images/microphone.svg");
    recordAudioButton->setIconSize(QSize(30,30));
    recordAudioButton->setUsingAppColors(true);

    connect(recordAudioButton, &QPushButton::clicked, audioRecorder, &AudioRecorder::toggleRecord);

    // hard code this
    apiKey = "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";

    // STT request
    whisperRequest = new OpenAIRequest();
    whisperRequest->setModel("whisper-1");
    whisperRequest->setAccessToken(apiKey);

    // chat request
    chatRequest = new OpenAIRequest();
    chatRequest->setModel("gpt-3.5-turbo-1106");
    chatRequest->setAccessToken(apiKey);

    QDateTime currentDateTime = QDateTime::currentDateTime();
    QString dateTimeStr = currentDateTime.toString("yyyy-MM-dd ddd HH:mm");

    QJsonObject systemPrompt;

    systemPrompt["prompt"] = "You are part of an app called ADHD Task Manager. The app is an improved task management app "
                             "specifically made for people with ADHD. You are a chatbot built into the app that can "
                             "modify the user's calendar and tell the user about their schedule."
                             "Only use tools that you have been given access to."
                             "Use natural language to describe dates and time.";

    QFile file(":/exampleConversation.json");
    if (file.open(QFile::ReadOnly | QFile::Text)) {
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        QJsonArray convo = doc.array();
        systemPrompt["example conversation"] = convo;
    }

    systemPrompt["current date and time"] = dateTimeStr;

    chatRequest->addMessage(new OpenAIMessage(systemPrompt, OpenAIMessage::Role::System));

    // TTS request
    speechRequest = new OpenAIRequest();
    speechRequest->setModel("tts-1");
    speechRequest->setAccessToken(apiKey);
    speechRequest->setFilePath(currentPath + QDir::separator() + "speech.mp3");
    speechRequest->setResponseFormat("mp3");

    // say the response out loud
    connect(chatRequest, &OpenAIRequest::requestFinished, this, &MainWindow::say);

    // transcribe the user's voice
    connect(audioRecorder, &AudioRecorder::recordingFinished, this, &MainWindow::transcribe);
    connect(whisperRequest, &OpenAIRequest::requestFinished, textInputField, &QLineEdit::setText);


#if defined(Q_OS_IOS)
    connect(textInputField, &QLineEdit::textChanged, this, &prepareHapticFeedback);
#endif


    sendChatButton = new SvgButton(this);
    sendChatButton->setSvgPath(":/images/send.svg");
    sendChatButton->setIconSize(QSize(30,30));
    sendChatButton->setUsingAppColors(true);

    sendChatButton->setEnabled(false);
    connect(sendChatButton, &QPushButton::clicked, this, &MainWindow::sendChat);

    // Calendar widget
    calendarWidget = new CalendarWidget(this);
    calendarWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);


    // today button
    SvgButton *todayButton = new SvgButton(this);
    todayButton->setSvgPath(":/images/today.svg");
    todayButton->setIconSize(QSize(30,30));
    todayButton->setFixedSize(70, 70);
    todayButton->setUsingAppColors(true);

    connect(todayButton, &QPushButton::clicked, this, [=]{
        calendarWidget->setSelectedDate(QDate::currentDate());
    });

    // side panel
    SvgButton *drawerButton = new SvgButton(this);
    drawerButton->setSvgPath(":/images/drawer.svg");
    drawerButton->setIconSize(QSize(30,30));
    drawerButton->setFixedSize(70, 70);
    drawerButton->setUsingAppColors(true);

    connect(SidePanel::self(), &SidePanel::animationStarted, drawerButton, [=]{
        drawerButton->startColorOverride(drawerButton->appDefaultColor());
    });
    connect(SidePanel::self(), &SidePanel::animationFinished, drawerButton, [=]{
        drawerButton->stopColorOverride();
    });

    connect(drawerButton, &QPushButton::clicked, SidePanel::self(), &SidePanel::toggle);

    auto vLayout = SidePanel::self()->verticalLayout();
    vLayout->setSpacing(30);
    vLayout->addWidget(themeComboBox);
    vLayout->addWidget(voiceSelectionComboBox);
    vLayout->addStretch();

    // Top layout for dark mode button, voice selection, and record button
    QHBoxLayout *topRowLayout = new QHBoxLayout();
    topRowLayout->addWidget(drawerButton);
    topRowLayout->addStretch();
    topRowLayout->addWidget(todayButton);
    topRowLayout->setContentsMargins(0,0,0,0);

    // Second row layout for text input and send chat button
    QHBoxLayout *secondRowLayout = new QHBoxLayout();
    secondRowLayout->setSpacing(5);
    secondRowLayout->addWidget(textInputField);
    secondRowLayout->addWidget(audioRecorder->getLevelWidget());
    secondRowLayout->addWidget(sendChatButton);

    // Adding layouts and widgets to the main layout
    layout->addLayout(topRowLayout);
    layout->addWidget(calendarWidget, 1); // Calendar takes most of the space
    layout->addLayout(secondRowLayout);

    auto margins = layout->contentsMargins();
    margins.setTop(0);
    layout->setContentsMargins(margins);

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

#if defined(Q_OS_IOS)
//    generateHapticFeedback();
#endif

    OpenAIMessage *userMessage = new OpenAIMessage("", OpenAIMessage::Role::User);
    userMessage->setUserMessage(textInputField->text());
    textInputField->clear();

    chatRequest->addMessage(userMessage);
    chatRequest->execute();
}

void MainWindow::transcribe()
{
    whisperRequest->setFilePath(audioRecorder->getRecordingLocation());
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

bool MainWindow::isSystemDark()
{
#if defined(Q_OS_IOS)
    return isIOSInDarkMode();
#elif defined(Q_OS_MACOS)
    return isMacInDarkMode();
#else
    // TODO: implement for windows and android
    return this->isDarkMode;
#endif
}

void MainWindow::handleThemeChange(bool isDarkMode)
{
    if (this->isDarkMode == isDarkMode) return;
    if (!settingsLoaded) return;

    this->isDarkMode = isDarkMode;
    saveSettings();
    setDarkMode(isDarkMode);
}

void MainWindow::setDarkMode(bool isDarkMode)
{
    QString path = isDarkMode ? ":/style/darkStyle.qss" : ":/style/lightStyle.qss";

    QFile file(path);
    if (file.open(QFile::ReadOnly | QFile::Text)) {
        QString styleSheet = file.readAll();
        qApp->processEvents();
        qApp->setStyleSheet(styleSheet);
        qApp->processEvents();
    }

//    0xF2E9FF (light) 0x3D315B (dark)

    if (isDarkMode) {
        SvgButton::setAppColors(0xF2E9FF, Qt::lightGray, 0xF2E9FF, Qt::lightGray);
        audioRecorder->getLevelWidget()->setFillColor(0xF2E9FF);
    } else {
        SvgButton::setAppColors(0x3D315B, Qt::lightGray, 0x3D315B, Qt::lightGray);
        audioRecorder->getLevelWidget()->setFillColor(0x3D315B);
    }
}

void MainWindow::saveSettings()
{
    if (!settingsLoaded) return;

    settings->setValue("apiKey", apiKey);
    settings->setValue("isDarkMode", isDarkMode);
    settings->setValue("isAutoTheme", isAutoTheme);
    settings->setValue("voice", voice);

    settings->setValue("mainWindow/geometry", saveGeometry());
    settings->setValue("mainWindow/windowState", saveState());

    // this will be slow with many events
    // TODO: maintain list of changed dates
    calendarWidget->saveSettings();
}

void MainWindow::loadSettings()
{
    apiKey = settings->value("apiKey").toString();
    isDarkMode = settings->value("isDarkMode").toBool();
    isAutoTheme = settings->value("isAutoTheme").toBool();
    voice = settings->value("voice").toString();

    voiceSelectionComboBox->setCurrentText(voice);

    if (isAutoTheme) {
        themeComboBox->setCurrentIndex(2);
    } else if (isDarkMode) {
        themeComboBox->setCurrentIndex(1);
    } else {
        themeComboBox->setCurrentIndex(0);
    }

    if (isAutoTheme) {
        setDarkMode(isSystemDark());
    } else {
        setDarkMode(isDarkMode);
    }

    calendarWidget->loadSettings();

    restoreGeometry(settings->value("mainWindow/geometry").toByteArray());
    restoreState(settings->value("mainWindow/windowState").toByteArray());

    settingsLoaded = true;
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
    // pressing enter sends chat
    if (obj == textInputField && event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) {
            sendChatButton->click();
            return true;
        }
    }

    // this prevents the whole app from being pushed up when the virtual keyboard comes up
    // but it doesn't move the line edit up
//    if (event->type() == QEvent::InputMethodQuery) {
//        QInputMethodQueryEvent *imEvt = static_cast<QInputMethodQueryEvent *>(event);
//        if (imEvt->queries() == Qt::InputMethodQuery::ImCursorRectangle) {
//            imEvt->setValue(Qt::InputMethodQuery::ImCursorRectangle, QRectF());
//            return true;
//        }
//    }



    // macOS: caught main window change:  QEvent(ThemeChange, 0x16f193498)
    // iOS: caught app change:  QEvent(ApplicationPaletteChange, 0x16b590c00)

    if (isAutoTheme) {
#if defined(Q_OS_IOS)
        if (obj == qApp && event->type() == QEvent::ApplicationPaletteChange) {
            handleThemeChange(isSystemDark());
        }
#elif defined(Q_OS_MACOS)
        if (obj == this && event->type() == QEvent::ThemeChange) {
            handleThemeChange(isSystemDark());
        }
#else
        // TODO: test on windows
#endif
    }

    // TODO: use this on windows
    // attempt to catch dark mode light mode change
//    switch (event->type()) {
//    case QEvent::PaletteChange:
//    case QEvent::ApplicationPaletteChange:
//    case QEvent::StyleChange:
//    case QEvent::ThemeChange:
//        if (obj == qApp) {
////            qDebug() << "caught app change: " << event;
//        }
//        if (obj == this) {
////            qDebug() << "caught main window change: " << event;
//        }
//        break;
//    default:
//        break;
//    }

    return QObject::eventFilter(obj, event);
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    if (SidePanel::self()->isVisible()) {
        SidePanel::self()->updateSize();
    }

    return QMainWindow::resizeEvent(event);
}









