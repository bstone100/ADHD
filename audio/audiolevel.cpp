// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "audiolevel.h"
#include "audiorecorder.h"

#include <QPainter>

AudioLevel::AudioLevel(QWidget *parent) : QWidget(parent)
{
    setFixedSize(30,30);

    audioRecorder = NULL;

    fillColor = QColorConstants::Svg::purple;

    timer.setInterval(25);
    connect(&timer, &QTimer::timeout, this, &AudioLevel::updateOpacity);
}

void AudioLevel::setLevel(qreal level)
{
    if (m_level != level) {
        m_level = level;
        update();
    }
}

void AudioLevel::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    audioRecorder ? paintMic() : paintMascot();
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
//    qreal topRightY = (93/512) * height();

    qreal radius = (topRightX - topLeftX) / 2;

    qreal centerX = topLeftX + radius;
    qreal centerY = topLeftY + radius;

    qreal levelRadius = qBound(10.0, radius - 1 + (m_level * 3), (qreal)width() - 10);

    painter.setBrush(fillColor);
    painter.setPen(Qt::NoPen);

    // Draw the circle
    painter.drawEllipse(QPointF(centerX, centerY), levelRadius, levelRadius);
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

