#ifndef SVGBUTTON_H
#define SVGBUTTON_H

#include <QPushButton>
#include <QSvgRenderer>
#include <QPainter>
#include <QEvent>

class SvgButton : public QPushButton {
    Q_OBJECT

public:
    explicit SvgButton(QWidget *parent = nullptr);
    ~SvgButton();

    void setSvgPath(const QString &path);
    void setIconSize(const QSize &size);

    // convenience function
    void setColors(const QColor &defaultColor, const QColor &disabledColor,
                   const QColor &hoverColor, const QColor &pressedColor);

    QColor defaultColor() const;
    void setDefaultColor(const QColor &color);

    QColor disabledColor() const;
    void setDisabledColor(const QColor &color);

    QColor hoverColor() const;
    void setHoverColor(const QColor &color);

    QColor pressedColor() const;
    void setPressedColor(const QColor &color);

    void startColorOverride(const QColor &newOverrideColor);
    void stopColorOverride();

    static QString modifySvgColor(const QString &svgContent, const QColor &color);
    static QIcon createIconFromSVG(const QString &svgPath, const QColor &color, QSize iconSize = QSize(24, 24));

    bool isUsingAppColors();
    void setUsingAppColors(bool newUsingAppColors);
    static void setAppColors(const QColor &defaultColor, const QColor &disabledColor,
                             const QColor &hoverColor, const QColor &pressedColor);

    void updateColor();

    static QColor appDefaultColor();
    static QColor appDisabledColor();
    static QColor appHoverColor();
    static QColor appPressedColor();

protected:
    bool event(QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    QString svgPath;
    QSize iconSize;

    QColor m_currentColor;

    QColor m_defaultColor;
    QColor m_disabledColor;
    QColor m_hoverColor;
    QColor m_pressedColor;


    // app defaults
    bool usingAppColors;
    static QColor s_defaultColor;
    static QColor s_disabledColor;
    static QColor s_hoverColor;
    static QColor s_pressedColor;

    static QList<SvgButton *> instances;


    QString originalSvgString;
    QMap<QRgb, QString> colorToSvgString;

    // ignore the mouse states
    bool override;
    QColor m_overrideColor;
};

#endif // SVGBUTTON_H
