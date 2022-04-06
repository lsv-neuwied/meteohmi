#include "meteobinding.h"

#include <math.h>

#define RECEIVE_TIMEOUT 5000

#define FILTER_PERIOD_MS 30
#define FILTER_DIR_DT 0.8
#define FILTER_VELO_DT 0.5

#define FILTER_DIR_FACTOR ((FILTER_PERIOD_MS * 0.001) / FILTER_DIR_DT)
#define FILTER_VELO_FACTOR ((FILTER_PERIOD_MS * 0.001) / FILTER_VELO_DT)

#define DEG_TO_RAD (M_PI / 180.0)
#define RAD_TO_DEG (180.0 / M_PI)

MeteoBinding::MeteoBinding(MeteoCollector *collector, double runway, QObject *parent) :
    QObject(parent), collector(collector), runway(runway)
{
    connect(collector, SIGNAL(windUpdate()), this, SLOT(windUpdate()));
    connect(collector, SIGNAL(airTempUpdate()), this, SLOT(airTempUpdate()));
    connect(collector, SIGNAL(airPressUpdate()), this, SLOT(airPressUpdate()));

    last_ti = 0;

    windDataOk = false;
    windDirSin = 0.0;
    windDirCos = 0.0;
    windDir = 0.0;
    windVelo = 0.0;

    airTempOk = false;
    airPressOk = false;

    startTimer(FILTER_PERIOD_MS);

    emit runwayChanged();
}

// move to 0-380 deg range for gauge
double MeteoBinding::posAngle(double a) {
    if (a < 0.0) {
        return a + 360.0;
    }

    return a;
}

void MeteoBinding::timerEvent(QTimerEvent *event) {
    Q_UNUSED(event);

    // update time
    time_t ti;
    time(&ti);
    if (ti != last_ti) {
        last_ti = ti;
        emit timeChanged();
    }

    // check timeouts
    qint64 timeout = collector->currentTimestamp() - RECEIVE_TIMEOUT;
    if (collector->getAirTempTimestamp() < timeout) {
        if (airTempOk) {
            airTempOk = false;
            emit airTempChanged();
        }
    }
    if (collector->getAirPressTimestamp() < timeout) {
        if (airPressOk) {
            airPressOk = false;
            emit airPressChanged();
        }
    }
    if (collector->getWindTimestamp() < timeout) {
        if (windDataOk) {
            windDataOk = false;
            emit windChanged();
        }
    }

    // filter wind data
    if (windDataOk) {
        double dir = DEG_TO_RAD * collector->getWindDir();
        double velo = collector->getWindVelo();

        // filter current values
        windDirSin = windDirSin + (sin(dir) - windDirSin) * FILTER_DIR_FACTOR;
        windDirCos = windDirCos + (cos(dir) - windDirCos) * FILTER_DIR_FACTOR;
        windVelo = windVelo + (velo - windVelo) * FILTER_VELO_FACTOR;

        windDir = RAD_TO_DEG * atan2(windDirSin, windDirCos);

        emit windChanged();
    }
}

void MeteoBinding::windUpdate()
{
    // initialize values if data become valid
    if (!windDataOk) {
        double dir = DEG_TO_RAD * collector->getWindDir();
        windDirSin = sin(dir);
        windDirCos = cos(dir);
        windVelo = collector->getWindVelo();
    }

    // just set flag, change event is triggered by filter
    windDataOk = true;
}

void MeteoBinding::airTempUpdate()
{
    airTempOk = true;
    emit airTempChanged();
}

void MeteoBinding::airPressUpdate()
{
    airPressOk = true;
    emit airPressChanged();
}

QString MeteoBinding::getAirPressTrend()
{
    if (!airPressOk) {
        return "";
    }

    switch (collector->getAirPressTrend()) {
    case MeteoCollector::Rising:
        return "+";
    case MeteoCollector::Falling:
        return "-";
    case MeteoCollector::Unsteady:
        return "~";
    default:
        return " ";
    }
}

QString MeteoBinding::getTimeStr() {
    struct tm tm;
    gmtime_r(&last_ti, &tm);
    QString s;
    s.sprintf("%02d:%02d:%02d UTC %02d.%02d.%04d",
                    tm.tm_hour, tm.tm_min, tm.tm_sec,
                    tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900);
    return s;
}

