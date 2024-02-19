// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "audiorecorder.h"
#include "audiolevel.h"

#include <QAudioBuffer>
#include <QAudioDevice>
#include <QAudioInput>
#include <QDir>
#include <QFileDialog>
#include <QImageCapture>
#include <QMediaDevices>
#include <QMediaFormat>
#include <QMediaRecorder>
#include <QMimeType>
#include <QStandardPaths>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QMessageBox>
#include <QApplication>

#if QT_CONFIG(permissions)
  #include <QPermission>
#endif

int AudioRecorder::recordingCount = 0;

AudioRecorder::AudioRecorder()
{
    // audio input initialization
    init();

    m_audioRecorder = new QMediaRecorder(this);
    m_captureSession.setRecorder(m_audioRecorder);
    m_captureSession.setAudioInput(new QAudioInput(this));

    connect(m_audioRecorder, &QMediaRecorder::recorderStateChanged, this, [=](QMediaRecorder::RecorderState state){
        if (state == QMediaRecorder::RecorderState::StoppedState) {
            emit recordingFinished();
        }
    });
}

void AudioRecorder::init()
{
#if QT_CONFIG(permissions)
    QMicrophonePermission microphonePermission;
    switch (qApp->checkPermission(microphonePermission)) {
    case Qt::PermissionStatus::Undetermined:
        qApp->requestPermission(microphonePermission, this, &AudioRecorder::init);
        return;
    case Qt::PermissionStatus::Denied:
        QMessageBox::warning(NULL, "Permission Error", "Microphone permission is not granted!");
        return;
    case Qt::PermissionStatus::Granted:
        break;
    }
#endif
}

void AudioRecorder::toggleRecord()
{
    if (m_audioRecorder->recorderState() == QMediaRecorder::StoppedState) {

        m_captureSession.audioInput()->setDevice(QMediaDevices::defaultAudioInput());

        QString dir = QCoreApplication::applicationDirPath();
        QString tempFilePath = dir + QDir::separator() + "whisper" + QString::number(++recordingCount) + ".wav";
        m_audioRecorder->setOutputLocation(QUrl::fromLocalFile(tempFilePath));

        QMediaFormat format;
        format.setFileFormat(QMediaFormat::Wave);
        format.setAudioCodec(QMediaFormat::AudioCodec::Wave);
        m_audioRecorder->setMediaFormat(format);
        m_audioRecorder->setAudioSampleRate(16000);
        m_audioRecorder->setAudioBitRate(128000);
        m_audioRecorder->setAudioChannelCount(1);
        m_audioRecorder->setQuality(QMediaRecorder::HighQuality);
        m_audioRecorder->setEncodingMode(QMediaRecorder::ConstantBitRateEncoding);

        m_audioRecorder->record();
    } else {
        m_audioRecorder->stop();
    }
}

QUrl AudioRecorder::getOutputLocation()
{
    return m_audioRecorder->outputLocation();
}


void AudioRecorder::togglePause()
{
    if (m_audioRecorder->recorderState() != QMediaRecorder::PausedState)
        m_audioRecorder->pause();
    else
        m_audioRecorder->record();
}


