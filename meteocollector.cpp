#include "meteocollector.h"

#include <math.h>
#include <time.h>

#define AVG_WINDOW (5L * 60L * 1000L)

#define TREND_WINDOW (60L * 60L * 1000L)
#define TREND_INTERVAL (5L * 60L * 1000L)

#define MS_PER_HOUR (60.0 * 60.0 * 1000.0)

#define MTRPERSEC_TO_KNOTS 1.9438445
#define DEG_TO_RAD (M_PI / 180.0)
#define RAD_TO_DEG (180.0 / M_PI)

MeteoCollector::MeteoCollector(const QObject *parser, double windDirOffset, double airPressOffset, QObject *parent)
    : QObject(parent), windDirOffset(windDirOffset * DEG_TO_RAD), airPressOffset(airPressOffset)
{
    connect(parser, SIGNAL(receivedWindData(int, N2K_WIND_REF_T, double, double)), this, SLOT(receivedWindData(int, N2K_WIND_REF_T, double, double)));
    connect(parser, SIGNAL(receivedTemperature(int, int, N2K_TEMP_SRC_T, double, double)), this, SLOT(receivedTemperature(int, int, N2K_TEMP_SRC_T, double, double)));
    connect(parser, SIGNAL(receivedActualPressure(int, int, N2K_PRESS_SRC_T, double)), this, SLOT(receivedActualPressure(int, int, N2K_PRESS_SRC_T, double)));

    windVelo = 0.0;
    windVeloPeak = 0.0;
    windDir = 0.0;
    windDirAvg = 0.0;
    windTimestamp = 0;

    airTemp = 0.0;
    airTempTimestamp = 0;

    airPress = 0.0;
    airPressTrend = Steady;
    airPressTendAcc = 0.0;
    airPressTendCnt = 0;
    airPressTimestamp = 0;
}

qint64 MeteoCollector::currentTimestamp() {
    struct timespec tp;
    clock_gettime(CLOCK_MONOTONIC, &tp);
    return (qint64) tp.tv_sec * 1000LL + ((qint64) tp.tv_nsec / 1000000LL);
}

double MeteoCollector::normalizeAngle(double a) {
    return atan2(sin(a), cos(a));
}

void MeteoCollector::receivedWindData(int sid, N2K_WIND_REF_T ref, double velo, double dir) {
    Q_UNUSED(sid);
    Q_UNUSED(ref);

    dir += windDirOffset;

    MeteoWindAvgItem last;
    last.timestamp = currentTimestamp();
    last.dirSin = sin(dir);
    last.dirCos = cos(dir);
    last.velo = velo;

    // remove old items
    qint64 timeout = last.timestamp - AVG_WINDOW;
    while(!windAvgQueue.isEmpty()) {
        const MeteoWindAvgItem &item = windAvgQueue.head();
        if (item.timestamp > timeout) {
            break;
        }
        windAvgQueue.dequeue();
    }

    // add new item to avg queue
    windAvgQueue.enqueue(last);

    // build avg/max values
    double dirSinSum = 0.0;
    double dirCosSum = 0.0;
    double veloPeak = 0.0;
    QListIterator<MeteoWindAvgItem> iter(windAvgQueue);
    while (iter.hasNext()) {
        const MeteoWindAvgItem &item = iter.next();
        dirSinSum += item.dirSin * item.velo;
        dirCosSum += item.dirCos * item.velo;
        if (item.velo > veloPeak) {
            veloPeak = item.velo;
        }
    }

    windDir = RAD_TO_DEG * atan2(last.dirSin, last.dirCos);
    windDirAvg = RAD_TO_DEG * atan2(dirSinSum, dirCosSum);
    windVelo = MTRPERSEC_TO_KNOTS * last.velo;
    windVeloPeak = MTRPERSEC_TO_KNOTS * veloPeak;
    windTimestamp = last.timestamp;
    emit windUpdate();
}

void MeteoCollector::receivedTemperature(int sid, int inst, N2K_TEMP_SRC_T source, double temp, double setp) {
    Q_UNUSED(sid);
    Q_UNUSED(inst);
    Q_UNUSED(setp);

    if (source != N2K_TEMP_SRC_OUTSIDE) {
        return;
    }

    airTemp = temp;
    airTempTimestamp = currentTimestamp();
    emit airTempUpdate();
}

void MeteoCollector::receivedActualPressure(int sid, int inst, N2K_PRESS_SRC_T source, double press) {
    Q_UNUSED(sid);
    Q_UNUSED(inst);

    if (source != N2K_PRESS_SRC_ATMOSPHERIC) {
        return;
    }

    qint64 timestamp = currentTimestamp();
    press += airPressOffset;

    qint64 timeout = timestamp - TREND_INTERVAL;
    if (airPressTimestamp < timeout) {
        airPressTrendQueue.clear();
        airPressTendAcc = 0.0;
        airPressTendCnt = 0;
    }

    airPressTendAcc += press;
    airPressTendCnt++;

    if (airPressTrendQueue.isEmpty() || airPressTrendQueue.last().timestamp <= timeout) {
        MeteoAirPressTrendItem last;
        last.timestamp = timestamp;
        last.press = airPressTendAcc / ((double) airPressTendCnt);

        airPressTendAcc = 0.0;
        airPressTendCnt = 0;

        // remove old items
        qint64 timeout = timestamp - TREND_WINDOW;
        while(!airPressTrendQueue.isEmpty()) {
            const MeteoAirPressTrendItem &item = airPressTrendQueue.head();
            if (item.timestamp > timeout) {
                break;
            }
            airPressTrendQueue.dequeue();
        }

        // add new item to trend queue
        airPressTrendQueue.enqueue(last);

        airPressTrend = calculateTrend(airPressTrendQueue);
    }

    airPress = press;
    airPressTimestamp = timestamp;
    emit airPressUpdate();
}

enum MeteoCollector::AirPressTrend MeteoCollector::calculateTrend(const QQueue<MeteoAirPressTrendItem> &queue)
{
    int count = 0;
    double sum = 0.0;
    double min = 0.0;
    double max = 0.0;
    double trendAcc = 0.0;
    const MeteoAirPressTrendItem *next = NULL;

    QListIterator<MeteoAirPressTrendItem> iter(queue);
    iter.toBack();
    while (iter.hasPrevious()) {
        const MeteoAirPressTrendItem &item = iter.previous();

        if (next != NULL && !isnan(trendAcc)) {
            double delta = next->press - item.press;
            qint64 dt = next->timestamp - item.timestamp;
            double rate = delta * (MS_PER_HOUR / ((double) dt));

            // Pressure Falling Rapidly:
            // A decrease in station pressure at a rate of 0.06 inch of mercury (~2.0 hPa) or more per hour which totals 0.02 inch (~0.6 hPa) or more.
            // Pressure Rising Rapidly:
            // An increase in station pressure at a rate of 0.06 inch of mercury (~2.0 hPa) or more per hour which totals 0.02 inch (~0.6 hPa) or more.
            if (trendAcc >= 0.0 && rate >= 2.0) {
                trendAcc += delta;
                if (trendAcc >= 0.6) {
                    return Rising;
                }
            } else if (trendAcc <= 0.0 && rate <= -2.0) {
                trendAcc += delta;
                if (trendAcc <= -0.6) {
                    return Falling;
                }
            } else {
                trendAcc = NAN;
            }
        }
        next = &item;

        if (count == 0 || item.press < min) {
            min = item.press;
        }

        if (count == 0 || item.press > max) {
            max = item.press;
        }

        sum += item.press;
        count++;
    }

    // calc avg
    double avg = sum / ((double) count);

    // Pressure Unsteady:
    // A pressure that fluctuates by 0.03 inch of mercury (~1.0 hPa) or more from the mean pressure during the period of measurement.
    if (min <= (avg - 1.0) || max >= (avg + 1.0)) {
        return Unsteady;
    }

    return Steady;
}
