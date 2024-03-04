// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef AUDIOLEVEL_H
#define AUDIOLEVEL_H

#include "QtCore/qtimer.h"
#include <QWidget>

class AudioRecorder;

class AudioLevel : public QWidget
{
    Q_OBJECT
public:
    explicit AudioLevel(QWidget *parent = nullptr);

    // Using [0; 1.0] range
    void setLevel(qreal level);

    void setFillColor(const QColor &newFillColor);

    AudioRecorder *getAudioRecorder() const;
    void setAudioRecorder(AudioRecorder *newAudioRecorder);

    bool isRecording();

    void start();
    void stop();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void paintMic();
    void paintMascot();

    qreal m_level = 0.0;

    QColor fillColor;

    QTimer timer;
    qreal opacity;
    bool fadingOut;
    void updateOpacity();

    AudioRecorder *audioRecorder;
};

#endif // QAUDIOLEVEL_H
