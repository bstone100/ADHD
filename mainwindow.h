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
#include "QtWidgets/qlabel.h"
#include "qelapsedtimer.h"
#include "qpropertyanimation.h"
#include "QQueue"
#include "QTableView"
#include "QTimer"

class OpenAIRequest;
class AudioRecorder;
class CalendarWidget;
class SvgButton;
class ChatTextEdit;
class AudioLevel;
class EventListWidget;
class ResizingComboBox;

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

    static QString version;

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

    bool isEventListExpanding() const;
    bool isEventListCollapsing() const;

    void expandEventList(QDate date);
    void collapseEventList();

    void animateToNextMonth();
    void animateToPrevMonth();
    void animateToCurrentMonth();

    QWidget *getTopOfStackWidget() const;

    bool getIsTouching() const;

    void fadeInWidget(QWidget *widget, int duration);
    void fadeOutWidget(QWidget *widget, int duration);
    void fadeInWidgets(QList<QWidget *> widgets, int duration);
    void fadeOutWidgets(QList<QWidget *> widgets, int duration);

protected:
    void closeEvent(QCloseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    static MainWindow *singleton;

    bool settingsLoaded = false;

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
    QString model;

    ResizingComboBox *voiceSelectionComboBox;
    ResizingComboBox *themeComboBox;
    ResizingComboBox *modelComboBox;

    SvgButton *sendChatButton;
    QLineEdit *textInputField;
    ChatTextEdit *assistantTextEdit;

    SvgButton *recordAudioButton;
    AudioRecorder *audioRecorder;

    SvgButton *todayButton;

    AudioLevel *assistantLevelWidget;

    OpenAIRequest *chatRequest;
    OpenAIRequest *speechRequest;
    OpenAIRequest *whisperRequest;

    QStackedWidget *stackedWidget;
    CalendarWidget *calendarWidget;
    QTableView *calendarTableView;
    EventListWidget *eventListWidget;
    QLabel *eventListSnapshot;
    QWidget *topOfStackWidget;

    QRect calculateExplosionRect(QDate date);
    bool eventListExpanding = false;
    bool eventListCollapsing = false;

    QPixmap captureWidgetSnapshot(QWidget *widget, QLabel *snapshot);
    void captureNextMonthSnapshot();
    void capturePrevMonthSnapshot();

    void exitEventListTouchEvent(QTouchEvent *event);
    void exitEventListHandleSwipeEnd();

    void swipeMonthTouchEvent(QTouchEvent *event);
    void swipeMonthHandleSwipeEnd();

    bool isAnimatingToNextMonth = false;
    bool isAnimatingToPrevMonth = false;

    bool isSidePanelTouch = false;

    bool isTouching = false;
    QPoint touchStartPoint;
    QPoint previousPoint;

    // v = x/t
    int dx;
    int dt;
    qreal progress;

    QElapsedTimer stopwatch;

    QPropertyAnimation *calendarInterpolator = NULL;
    QPropertyAnimation *eventListInterpolator = NULL;


    // swipe between months logic
    // calendarTableView and calendarSnapshot are a revolving door for this
    QQueue<int> navigateMonthsQueue; // +1 for next month, -1 for previous month, 0 for current month

    void navigateMonths(int direction);
    QPropertyAnimation *currentMonthSwipeAnimation();
    void startNextMonthSwipeAnimation();
    void handleMonthSwipeAnimationFinished();
    void renderSnapshotsToCache(int cacheRange);

    QLabel *currentCalendarSnapshot;
    QLabel *adjacentCalendarSnapshot;

    QMap<QDate, QPixmap> calendarSnapshotCache;

};

#endif // MAINWINDOW_H








