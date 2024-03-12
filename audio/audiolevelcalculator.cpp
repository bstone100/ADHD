#include "AudioLevelCalculator.h"
#include "QtCore/qurl.h"
#include <QAudioBuffer>
#include <QtMath>
#include <QDebug>

AudioLevelCalculator::AudioLevelCalculator(QObject *parent) : QObject(parent), decoder(new QAudioDecoder(this)) {
    connect(decoder, &QAudioDecoder::bufferReady, this, &AudioLevelCalculator::handleBufferReady);
    connect(decoder, &QAudioDecoder::finished, this, &AudioLevelCalculator::handleFinished);
}

void AudioLevelCalculator::calculateLevels(const QString &filePath, qint64 durationMs) {
    decoder->setSource(QUrl::fromLocalFile(filePath));
    rmsValues.clear();
    this->duration = durationMs;
    decoder->start();
}

void AudioLevelCalculator::handleBufferReady() {
    QAudioBuffer buffer = decoder->read();
    calculateRMS(buffer);
}

void AudioLevelCalculator::handleFinished() {
    QVector<float> levels;

    // Normalize RMS values to a range of 0 to 1
    double maxRms = *std::max_element(rmsValues.begin(), rmsValues.end());
    for (double rms : rmsValues) {
        levels.append(static_cast<float>(rms / maxRms));
    }

    emit levelsCalculated(levels);
}

void AudioLevelCalculator::calculateRMS(const QAudioBuffer &buffer) {
    if (!buffer.isValid() || buffer.frameCount() == 0) {
        qDebug() << "Invalid buffer or buffer data is null or empty";
        return;
    }

    auto format = buffer.format();
    const int samplesPer20ms = static_cast<int>(format.sampleRate() * 0.02 * format.channelCount());
    qint64 totalSamples = buffer.frameCount() * format.channelCount();
    qint64 numChunks = totalSamples / samplesPer20ms;
    double rms = 0.0;

    // Determine sample format and process accordingly
    switch (format.sampleFormat()) {
    case QAudioFormat::Int16: {
        const qint16* data = buffer.constData<qint16>();
        for (qint64 chunk = 0; chunk < numChunks; ++chunk) {
            rms = 0.0;
            for (int i = 0; i < samplesPer20ms; ++i) {
                double sample = data[chunk * samplesPer20ms + i] / static_cast<double>(INT16_MAX);
                rms += sample * sample;
            }
            rmsValues.append(qSqrt(rms / samplesPer20ms));
        }
        break;
    }
    case QAudioFormat::Float: {
        const float* data = buffer.constData<float>();
        for (qint64 chunk = 0; chunk < numChunks; ++chunk) {
            rms = 0.0;
            for (int i = 0; i < samplesPer20ms; ++i) {
                double sample = static_cast<double>(data[chunk * samplesPer20ms + i]);
                rms += sample * sample;
            }
            rmsValues.append(qSqrt(rms / samplesPer20ms));

        }
        break;
    }
    default:
        break;
    }

    // Calculate the remaining samples for the last partial chunk
    qint64 remainingSamples = totalSamples % samplesPer20ms;
    if (remainingSamples > 0) {
        rms = 0.0; // Reset RMS for the new chunk

        // Handle remaining samples based on sample format
        switch (format.sampleFormat()) {
        case QAudioFormat::Int16: {
            const qint16* data = buffer.constData<qint16>() + (numChunks * samplesPer20ms);
            for (int i = 0; i < remainingSamples; ++i) {
                double sample = data[i] / static_cast<double>(INT16_MAX);
                rms += sample * sample;
            }
            break;
        }
        case QAudioFormat::Float: {
            const float* data = buffer.constData<float>() + (numChunks * samplesPer20ms);
            for (int i = 0; i < remainingSamples; ++i) {
                double sample = static_cast<double>(data[i]);
                rms += sample * sample;
            }
            break;
        }
        default:
            break;
        }

        // Calculate and append the RMS value for the last partial chunk
        rmsValues.append(qSqrt(rms / remainingSamples));
    }
}








