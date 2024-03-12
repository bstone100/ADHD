#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "QtWidgets/qcombobox.h"
#include <QMainWindow>
#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QInputDialog>
#include <QSettings>
#include <QRadioButton>
#include <QTouchEvent>
#include "QTextEdit"
#include "QStackedWidget"

class OpenAIRequest;
class AudioRecorder;
class CalendarWidget;
class SvgButton;
class ChatTextEdit;
class AudioLevel;
class EventListWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    static MainWindow *self();

    void say(const QString &text);
    void sendChat();
    void transcribe();
    void setAssistantWidgetText(const QString &text);
    void playAssistantLevel(const QVector<float> &levels, int duration);

    void saveSettings();
    void loadSettings();

    CalendarWidget *getCalendarWidget(){return calendarWidget;}

    static QString currentPath;

    void onApiKeyButtonClicked();

    bool isDarkModeOn(){return isDarkMode;}
    bool isSystemDark();
    void handleThemeChange(bool isDarkMode);

    static QColor lightColor;
    static QColor lightMidColor;
    static QColor darkMidColor;
    static QColor darkColor;

    void updateEventViews();

protected:
    void closeEvent(QCloseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    static MainWindow *singleton;

    bool settingsLoaded;

    void updateApiKeyButtonLabel();

    void setDarkMode(bool isDarkMode);

    QSettings *settings;

    QWidget *centralWidget;
    QVBoxLayout *layout;
    QPushButton *apiKeyButton;

    QString apiKey;
    bool isDarkMode;
    bool isAutoTheme;
    QString voice;

    QComboBox *voiceSelectionComboBox;

    QComboBox *themeComboBox;

    SvgButton *sendChatButton;
    QLineEdit *textInputField;
    ChatTextEdit *assistantTextEdit;

    SvgButton *recordAudioButton;
    AudioRecorder *audioRecorder;

    AudioLevel *assistantLevelWidget;

    OpenAIRequest *chatRequest;
    OpenAIRequest *speechRequest;
    OpenAIRequest *whisperRequest;

    QStackedWidget *stackedWidget;
    CalendarWidget *calendarWidget;
    EventListWidget *eventListWidget;

    void expandEventList(QDate date);
    void collapseEventList();
};

#endif // MAINWINDOW_H
