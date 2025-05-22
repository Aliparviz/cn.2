#ifndef NOISE_HANDLER_H
#define NOISE_HANDLER_H

#include <QByteArray>
#include <QString>
#include "schifra_reed_solomon.hpp"

class NoiseHandler {
public:
    NoiseHandler();
    QByteArray addNoise(const QByteArray &data, double noiseProbability = 0.01);
    QByteArray encodeReedSolomon(const QByteArray &data);
    QByteArray decodeReedSolomon(const QByteArray &data, bool &isCorrupt, int &errorCount);

private:
    static const int FEC_LENGTH = 4; // t=4, can correct up to t/2=2 errors
    schifra::reed_solomon::encoder<255, FEC_LENGTH> rsEncoder;
    schifra::reed_solomon::decoder<255, FEC_LENGTH> rsDecoder;
};

#endif // NOISE_HANDLER_H
