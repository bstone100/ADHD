#include "AudioLevelCalculator.h"
#include "QtCore/qurl.h"
#include <QAudioBuffer>
#include <QtMath>

AudioLevelCalculator::AudioLevelCalculator(QObject *parent) : QObject(parent), decoder(new QAudioDecoder(this)) {
    connect(decoder, &QAudioDecoder::bufferReady, this, &AudioLevelCalculator::handleBufferReady);
    connect(decoder, &QAudioDecoder::finished, this, &AudioLevelCalculator::handleFinished);
}

void AudioLevelCalculator::calculateLevels(const QString &filePath) {
    decoder->setSource(QUrl::fromLocalFile(filePath));
    rmsValues.clear();
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
    // Assuming 32-bit samples are in float format
    const float* data = buffer.constData<float>();
    qint64 numSamples = buffer.sampleCount();
    double rms = 0.0;

    for (qint64 i = 0; i < numSamples; ++i) {
        double sample = static_cast<double>(data[i]); // Directly use the float sample
        rms += sample * sample;
    }

    rms = qSqrt(rms / numSamples); // Calculate RMS
    rmsValues.append(rms);
}

