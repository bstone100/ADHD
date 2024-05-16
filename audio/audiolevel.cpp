// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "audiolevel.h"
#include "audiorecorder.h"

#include <QPainter>
#include "QPainterPath"

AudioLevel::AudioLevel(QWidget *parent) : QWidget(parent)
{

    pandaImage = QPixmap(":/images/pandaTask.png");

    audioRecorder = NULL;

    fillColor = QColorConstants::Svg::purple;

    timer.setInterval(25);
    connect(&timer, &QTimer::timeout, this, &AudioLevel::updateOpacity);
}

void AudioLevel::setLevel(qreal level)
{
    if (m_level != level) {
        m_level = level;

        if (!audioRecorder) {
            resizeImage();
        }

        update();
    }
}

void AudioLevel::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    audioRecorder ? paintMic() : paintPandaImage();
}

void AudioLevel::paintMic()
{
    QPainter painter(this);

    if (!isRecording()) {
        painter.fillRect(rect(), Qt::transparent);
        return;
    }


    // these numbers represent coordinates in the microphone svg 512x512 view box
    qreal topLeftX = (198.4/512) * width();
    qreal topLeftY = (42.2/512) * height();

    qreal bottomRightX = (315.6/512) * width();
    qreal bottomRightY = (289.3/512) * height();

    //    qreal micWidth = bottomRightX - topLeftX;
    qreal micHeight = bottomRightY - topLeftY;
    qreal levelHeight = m_level * micHeight;

    qreal actualTopLeftY = topLeftY + (micHeight - levelHeight);



    painter.setRenderHint(QPainter::Antialiasing);

    // Set the dynamic opacity for the painter
    painter.setOpacity(opacity);

    // Set the brush to a solid red color
    painter.setBrush(Qt::red);
    painter.setPen(Qt::NoPen); // No border

    // Calculate the center and size for the circle
    int diameter = 7;
    int x = width() - diameter;
    int y = 0;

    // Draw the circle
    painter.drawEllipse(x, y, diameter, diameter);


    QRectF levelRect(QPointF(topLeftX, actualTopLeftY), QPointF(bottomRightX, bottomRightY));

    painter.setOpacity(1.0);
    painter.fillRect(levelRect, fillColor);
}

void AudioLevel::paintMascot()
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    qreal topLeftX = (50.0/512) * width();
    qreal topLeftY = (93.0/512) * height();

    qreal topRightX = (462.0/512) * width();
//    qreal topRightY = (93.0/512) * height();

    qreal radius = (topRightX - topLeftX) / 2;

    qreal centerX = topLeftX + radius;
    qreal centerY = topLeftY + radius;

    qreal levelRadius = qBound(10.0, radius - 1 + (m_level * 3), (qreal)width() - 10);

    painter.setBrush(fillColor);
    painter.setPen(Qt::NoPen);

    // Draw the circle
    painter.drawEllipse(QPointF(centerX, centerY), levelRadius, levelRadius);
}

void AudioLevel::paintConcaveMascot()
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    qreal topLeftX = (50.0 / 512) * width();
    qreal topLeftY = (93.0 / 512) * height();

    qreal topRightX = (462.0 / 512) * width();
    qreal radius = (topRightX - topLeftX) / 2;

    qreal centerX = topLeftX + radius;
    qreal centerY = topLeftY + radius;

    QPointF center(centerX, centerY);
    QRectF rect(QPointF(topLeftX, topLeftY), QSizeF(radius*2, radius*2));

    painter.setBrush(fillColor);
    painter.setPen(fillColor);

    // slice representing mouth
    // startAngle > endAngle
    qreal startAngle = 300;
    qreal endAngle = 240;

    // arc excluding mouth
    QPainterPath bodyPath;
    bodyPath.moveTo(center);
    bodyPath.arcTo(rect, startAngle, 360 - (startAngle - endAngle));
    bodyPath.closeSubpath();
    painter.drawPath(bodyPath);

    // with level 0 draw circular arc from start to end
    // otherwise use bezier with control point somewhere in quadrant 1
    QPainterPath mouthPath;
    if (m_level <= 0.00) {
        mouthPath.moveTo(center);
        mouthPath.arcTo(rect, endAngle, (startAngle - endAngle));
        mouthPath.closeSubpath();
    } else {
        QPointF startPoint(centerX + radius * qCos(qDegreesToRadians(startAngle)),
                           centerY - radius * qSin(qDegreesToRadians(startAngle)));

        QPointF endPoint(centerX + radius * qCos(qDegreesToRadians(endAngle)),
                         centerY - radius * qSin(qDegreesToRadians(endAngle)));

        // 5 control points, 3 quads

        qreal angle1 = endAngle + (0.70 * (startAngle - endAngle));
        qreal angle2 = endAngle + (0.70 * (startAngle - endAngle));
        qreal angle3 = endAngle + (0.50 * (startAngle - endAngle));
        qreal angle4 = endAngle + (0.30 * (startAngle - endAngle));
        qreal angle5 = endAngle + (0.30 * (startAngle - endAngle));

        qreal levelRadius = m_level * radius;
        qreal midRadius1 = levelRadius + (0.99 * (radius - levelRadius));
        qreal midRadius2 = levelRadius + (0.75 * (radius - levelRadius));
        qreal midRadius3 = levelRadius + (0.60 * (radius - levelRadius));
        qreal midRadius4 = levelRadius + (0.75 * (radius - levelRadius));
        qreal midRadius5 = levelRadius + (0.99 * (radius - levelRadius));

        QPointF controlPoint1(centerX + midRadius1 * qCos(qDegreesToRadians(angle1)),
                              centerY - midRadius1 * qSin(qDegreesToRadians(angle1)));

        QPointF controlPoint2(centerX + midRadius2 * qCos(qDegreesToRadians(angle2)),
                              centerY - midRadius2 * qSin(qDegreesToRadians(angle2)));

        QPointF controlPoint3(centerX + midRadius3 * qCos(qDegreesToRadians(angle3)),
                              centerY - midRadius3 * qSin(qDegreesToRadians(angle3)));

        QPointF controlPoint4(centerX + midRadius4 * qCos(qDegreesToRadians(angle4)),
                              centerY - midRadius4 * qSin(qDegreesToRadians(angle4)));

        QPointF controlPoint5(centerX + midRadius5 * qCos(qDegreesToRadians(angle5)),
                              centerY - midRadius5 * qSin(qDegreesToRadians(angle5)));

        mouthPath.moveTo(center);
        mouthPath.lineTo(startPoint);
        mouthPath.quadTo(controlPoint1, controlPoint2);
        mouthPath.quadTo(controlPoint3, controlPoint4);
        mouthPath.quadTo(controlPoint5, endPoint);
        mouthPath.lineTo(center);
    }
    painter.drawPath(mouthPath);
}

void AudioLevel::paintPandaImage()
{
    QPainter painter(this);
    // Adjust the drawing position based on the device pixel ratio
    qreal ratio = devicePixelRatioF();
    painter.drawPixmap(QRect((width() - currentPixmap.width() / ratio) / 2,
                             (height() - currentPixmap.height() / ratio) / 2,
                             currentPixmap.width() / ratio,
                             currentPixmap.height() / ratio), currentPixmap);
}

void AudioLevel::updateOpacity()
{
    static const qreal minOpacity = 0.0;
    static const qreal maxOpacity = 0.6;
    static const qreal opacityChange = 0.02;
    if (fadingOut) {
        opacity -= opacityChange;
        if (opacity <= minOpacity) {
            opacity = minOpacity;
            fadingOut = false;
        }
    } else {
        opacity += opacityChange;
        if (opacity >= maxOpacity) {
            opacity = maxOpacity;
            fadingOut = true;
        }
    }
}

AudioRecorder *AudioLevel::getAudioRecorder() const
{
    return audioRecorder;
}

void AudioLevel::setAudioRecorder(AudioRecorder *newAudioRecorder)
{
    audioRecorder = newAudioRecorder;
}

bool AudioLevel::isRecording()
{
    if (audioRecorder) {
        return audioRecorder->currentlyRecording();
    }
    return true;
}

void AudioLevel::start()
{
    if (!timer.isActive()) {
        fadingOut = false;
        opacity = 0.0;
        timer.start();
    }
}

void AudioLevel::stop()
{
    timer.stop();
}

void AudioLevel::setFillColor(const QColor &newFillColor)
{
    fillColor = newFillColor;
}

void AudioLevel::resizeImage() {
    int minSize = width() - 20;
    int maxSize = width();
    int size = static_cast<int>(minSize + (maxSize - minSize) * m_level);

    qreal ratio = devicePixelRatioF();
    // Calculate the size considering the device pixel ratio
    currentPixmap = pandaImage.scaled(size * ratio, size * ratio, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    currentPixmap.setDevicePixelRatio(ratio);  // Set the device pixel ratio for pixmap
}




