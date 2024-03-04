#ifndef AUDIOLEVELCALCULATOR_H
#define AUDIOLEVELCALCULATOR_H

#include <QObject>
#include <QAudioDecoder>
#include <QVector>
#include <QFile>

class AudioLevelCalculator : public QObject {
    Q_OBJECT
public:
    explicit AudioLevelCalculator(QObject *parent = nullptr);
    void calculateLevels(const QString &filePath);

signals:
    void levelsCalculated(const QVector<float> &levels);

private slots:
    void handleBufferReady();
    void handleFinished();

private:
    QAudioDecoder *decoder;
    QVector<double> rmsValues;
    void calculateRMS(const QAudioBuffer &buffer);
};

#endif // AUDIOLEVELCALCULATOR_H
