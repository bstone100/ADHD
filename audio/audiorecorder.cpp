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
#include "../mainwindow.h"
#include "../iOS/AudioSessionHelper.h"
#include "../macOS/AudioRecorderBridge.h"

#if QT_CONFIG(permissions)
  #include <QPermission>
#endif

AudioRecorder::AudioRecorder()
{
    // audio input initialization
    init();

// Code for platforms other than Apple's
#if !defined(Q_OS_DARWIN)
    m_audioRecorder = new QMediaRecorder(this);
    m_captureSession.setRecorder(m_audioRecorder);
    m_captureSession.setAudioInput(new QAudioInput(this));

    connect(m_audioRecorder, &QMediaRecorder::recorderStateChanged, this, [=](QMediaRecorder::RecorderState state){
        if (state == QMediaRecorder::RecorderState::StoppedState) {
            emit recordingFinished();
        }
    });
#endif

    updateLevelTimer.setSingleShot(false);
    updateLevelTimer.setInterval(10);

    connect(&updateLevelTimer, &QTimer::timeout, this, &AudioRecorder::updateLevelWidget);

    recordingLocation = MainWindow::currentPath + QDir::separator() + "latestRecording.wav";
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
#if defined(Q_OS_DARWIN)
    // apple
    if (!isRecording()) {
        setOutputFileName(recordingLocation.toUtf8().data());
        startRecording();

        updateLevelTimer.start();
        getLevelWidget()->start();
    } else {
        updateLevelTimer.stop();
        getLevelWidget()->stop();
        clearLevelWidget();

        stopRecording();
        emit recordingFinished();
    }
#else
    // not apple
    if (m_audioRecorder->recorderState() == QMediaRecorder::StoppedState) {

        m_captureSession.audioInput()->setDevice(QMediaDevices::defaultAudioInput());

        // Check if the file exists and delete it if so
        QFile file(recordingLocation);
        if (file.exists()) {
            file.remove();
        }

        m_audioRecorder->setOutputLocation(QUrl::fromLocalFile(recordingLocation));

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

        updateLevelTimer.start();
        getLevelWidget()->start();
    } else {
        updateLevelTimer.stop();
        getLevelWidget()->stop();
        clearLevelWidget();

        m_audioRecorder->stop();

        //#if defined(Q_OS_IOS)
        //        QTimer::singleShot(500, this, &deactivateAudioSession);
        //#endif
    }
#endif
}

AudioLevel *AudioRecorder::getLevelWidget()
{
    if (!levelWidget) {
        levelWidget = new AudioLevel(MainWindow::self());
        levelWidget->setAudioRecorder(this);
    }
    return levelWidget;
}

void AudioRecorder::updateLevelWidget()
{
#if defined(Q_OS_DARWIN)
    float level = getCurrentLevel();
    level = qBound(0.0, level, 1.0);
    level = qMin(level * 3, 1.0);

    getLevelWidget()->setLevel(level);
#else
    getLevelWidget()->setLevel(1.0);
#endif
}

void AudioRecorder::clearLevelWidget()
{
    getLevelWidget()->setLevel(0.0);
}

QString AudioRecorder::getRecordingLocation() const
{
    return recordingLocation;
}

bool AudioRecorder::currentlyRecording()
{
#if defined(Q_OS_DARWIN)
    return isRecording();
#else
    return m_audioRecorder->recorderState() != QMediaRecorder::StoppedState;
#endif
}








