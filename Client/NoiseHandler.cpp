#include "NoiseHandler.h"
#include <QRandomGenerator>
#include <QDebug>

NoiseHandler::NoiseHandler() : rsEncoder(), rsDecoder() {}

QByteArray NoiseHandler::addNoise(const QByteArray &data, double noiseProbability) {
    QByteArray noisyData = data;
    QRandomGenerator rng(QTime::currentTime().msec());
    for (int i = 0; i < noisyData.size(); ++i) {
        for (int bit = 0; bit < 8; ++bit) {
            if (rng.generateDouble() < noiseProbability) {
                noisyData[i] ^= (1 << bit); // Flip bit
            }
        }
    }
    return noisyData;
}

QByteArray NoiseHandler::encodeReedSolomon(const QByteArray &data) {
    std::vector<unsigned char> input(data.begin(), data.end());
    std::vector<unsigned char> output;
    rsEncoder.encode(input, output);
    return QByteArray(reinterpret_cast<char*>(output.data()), output.size());
}

QByteArray NoiseHandler::decodeReedSolomon(const QByteArray &data, bool &isCorrupt, int &errorCount) {
    std::vector<unsigned char> input(data.begin(), data.end());
    std::vector<unsigned char> output;
    std::vector<int> errorPositions;
    bool success = rsDecoder.decode(input, output, errorPositions);

    errorCount = errorPositions.size();
    isCorrupt = !success || errorCount > FEC_LENGTH / 2; // Cannot correct more than t/2 errors
    if (success) {
        return QByteArray(reinterpret_cast<char*>(output.data()), output.size());
    }
    return QByteArray();
}
