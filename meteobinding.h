#ifndef METEOBINDING_H
#define METEOBINDING_H

#include <QObject>
#include <QQueue>

#include <time.h>
#include <math.h>

#include "meteocollector.h"

class MeteoBinding : public QObject
{
    Q_OBJECT
    Q_PROPERTY(double runway READ getRunway NOTIFY runwayChanged)
    Q_PROPERTY(double windDir READ getWindDir NOTIFY windChanged)
    Q_PROPERTY(double windDirAvg READ getWindDirAvg NOTIFY windChanged)
    Q_PROPERTY(double windVelo READ getWindVelo NOTIFY windChanged)
    Q_PROPERTY(double windVeloPeak READ getWindVeloPeak NOTIFY windChanged)
    Q_PROPERTY(double airTemp READ getAirTemp NOTIFY airTempChanged)
    Q_PROPERTY(double airPress READ getAirPress NOTIFY airPressChanged)
    Q_PROPERTY(QString airPressTrend READ getAirPressTrend NOTIFY airPressChanged)
    Q_PROPERTY(QString time READ getTimeStr NOTIFY timeChanged)
public:
    explicit MeteoBinding(MeteoCollector *collector, double runway, QObject *parent = 0);

    double getRunway() { return runway; }

    double getWindDir() { return windDataOk ? posAngle(windDir) : NAN; }
    double getWindDirAvg() { return windDataOk ? posAngle(collector->getWindDirAvg()) : NAN; }
    double getWindVelo() { return windDataOk ? windVelo : NAN; }
    double getWindVeloPeak() { return windDataOk ? collector->getWindVeloPeak() : NAN; }

    double getAirTemp() { return airTempOk ? collector->getAirTemp() : NAN; }
    double getAirPress() { return airPressOk ? collector->getAirPress() : NAN; }

    QString getAirPressTrend();
    QString getTimeStr();

private:
    MeteoCollector *collector;

    double runway;

    time_t last_ti;

    bool windDataOk;
    double windDirSin;
    double windDirCos;
    double windDir;
    double windVelo;

    bool airTempOk;
    bool airPressOk;

    double posAngle(double a);

protected:
    void timerEvent(QTimerEvent *event);

signals:
    void timeChanged();
    void runwayChanged();
    void windChanged();
    void airTempChanged();
    void airPressChanged();

private slots:
    void windUpdate();
    void airTempUpdate();
    void airPressUpdate();

};

#endif // METEOBINDING_H
