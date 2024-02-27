#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "QtWidgets/qcombobox.h"
#include <QMainWindow>
#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QInputDialog>
#include <QSettings>

class OpenAIRequest;
class AudioRecorder;
class CalendarWidget;

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

    void saveSettings();
    void loadSettings();

    CalendarWidget *getCalendarWidget(){return calendarWidget;}

    static QString currentPath;

    void onApiKeyButtonClicked();
    void onDarkModeButtonClicked();
    void handleThemeChange(bool isDarkMode);

protected:
    void closeEvent(QCloseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    static MainWindow *singleton;

    void updateApiKeyButtonLabel();

    void setDarkMode(bool darkMode);

    QSettings *settings;

    QWidget *centralWidget;
    QVBoxLayout *layout;
    QPushButton *apiKeyButton;
    QPushButton *darkModeButton;

    QString apiKey;
    bool isDarkMode;
    QString voice;

    QComboBox *voiceSelectionComboBox;

    QPushButton *sendChatButton;
    QLineEdit *textInputField;

    QPushButton *recordAudioButton;
    AudioRecorder *audioRecorder;

    OpenAIRequest *chatRequest;
    OpenAIRequest *speechRequest;
    OpenAIRequest *whisperRequest;

    CalendarWidget *calendarWidget;
};

#endif // MAINWINDOW_H
