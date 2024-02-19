// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef AUDIORECORDER_H
#define AUDIORECORDER_H

#include <QMediaCaptureSession>
#include <QMediaRecorder>
#include <QUrl>
#include "QTimer"

QT_BEGIN_NAMESPACE
namespace Ui {
class AudioRecorder;
}
class QAudioBuffer;
QT_END_NAMESPACE

class AudioLevel;

class AudioRecorder : public QObject
{
    Q_OBJECT

public:
    AudioRecorder();
    void init();

    void togglePause();
    void toggleRecord();

    QUrl getOutputLocation();

signals:
    void recordingFinished();

private:
    QMediaCaptureSession m_captureSession;
    QMediaRecorder *m_audioRecorder = nullptr;

    static int recordingCount;
};

#endif // AUDIORECORDER_H


