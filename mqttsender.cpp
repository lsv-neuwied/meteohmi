#include "mqttsender.h"

const QString WIND_TOPIC("meteo/wind");
const QString AIR_PRESS_TOPIC("meteo/air/press");
const QString AIR_TEMP_TOPIC("meteo/air/temp");

MqttSender::MqttSender(MqttClient *mqtt, MeteoCollector *collector, QObject *parent) : QObject(parent), mqtt(mqtt), collector(collector)
{
    connect(collector, SIGNAL(windUpdate()), this, SLOT(windUpdate()));
    connect(collector, SIGNAL(airTempUpdate()), this, SLOT(airTempUpdate()));
    connect(collector, SIGNAL(airPressUpdate()), this, SLOT(airPressUpdate()));
}

void MqttSender::windUpdate()
{
    QString data;
    data.sprintf("{\"d\":%.1f,\"da\":%.1f,\"s\":%.1f,\"sp\":%.1f}",
                 collector->getWindDir(),
                 collector->getWindDirAvg(),
                 collector->getWindVelo(),
                 collector->getWindVeloPeak());

    mqtt->publish(WIND_TOPIC, data.toUtf8());
}

void MqttSender::airPressUpdate()
{
    char trend;
    switch (collector->getAirPressTrend()) {
    case MeteoCollector::Rising:
        trend = 'r';
        break;
    case MeteoCollector::Falling:
        trend = 'f';
        break;
    case MeteoCollector::Unsteady:
        trend = 'u';
        break;
    default:
        trend = 's';
    }

    QString data;
    data.sprintf("{\"p\":%.2f,\"t\":\"%c\"}", collector->getAirPress(), trend);

    mqtt->publish(AIR_PRESS_TOPIC, data.toUtf8());
}

void MqttSender::airTempUpdate()
{
    mqtt->publish(AIR_TEMP_TOPIC, QString::number(collector->getAirTemp(), 'f', 2).toUtf8());
}
