#ifndef METEOCOLLECTOR_H
#define METEOCOLLECTOR_H

#include <QObject>
#include <QQueue>

#include "n2kparser.h"

class MeteoWindAvgItem {
public:
    qint64 timestamp;
    double dirSin;
    double dirCos;
    double velo;
};

class MeteoAirPressTrendItem {
public:
    qint64 timestamp;
    double press;
};

class MeteoCollector : public QObject
{
    Q_OBJECT
public:
    enum AirPressTrend { Steady, Unsteady, Rising, Falling };

    explicit MeteoCollector(const QObject *parser, double windDirOffset, double airPressOffset, QObject *parent = 0);

    double getWindVelo() { return windVelo; }
    double getWindVeloPeak() { return windVeloPeak; }
    double getWindDir() { return windDir; }
    double getWindDirAvg() { return windDirAvg; }
    double getAirTemp() { return airTemp; }
    double getAirPress() { return airPress; }
    enum AirPressTrend getAirPressTrend() { return airPressTrend; }

    qint64 currentTimestamp();

    qint64 getWindTimestamp() { return windTimestamp; }
    qint64 getAirTempTimestamp() { return airTempTimestamp; }
    qint64 getAirPressTimestamp() { return airPressTimestamp; }

    enum AirPressTrend calculateTrend(const QQueue<MeteoAirPressTrendItem> &queue);

private:
    double normalizeAngle(double a);

    double windDirOffset;
    double airPressOffset;

    double windDir;
    double windDirAvg;
    double windVelo;
    double windVeloPeak;
    QQueue<MeteoWindAvgItem> windAvgQueue;

    double airTemp;
    double airPress;
    enum AirPressTrend airPressTrend;
    double airPressTendAcc;
    int airPressTendCnt;
    QQueue<MeteoAirPressTrendItem> airPressTrendQueue;

    qint64 windTimestamp;
    qint64 airTempTimestamp;
    qint64 airPressTimestamp;

signals:
    void windUpdate();
    void airTempUpdate();
    void airPressUpdate();

private slots:
    void receivedWindData(int sid, N2K_WIND_REF_T ref, double velo, double dir);
    void receivedTemperature(int sid, int inst, N2K_TEMP_SRC_T source, double temp, double setp);
    void receivedActualPressure(int sid, int inst, N2K_PRESS_SRC_T source, double press);
};

#endif // METEOCOLLECTOR_H
