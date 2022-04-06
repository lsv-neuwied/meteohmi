#ifndef MQTTSENDER_H
#define MQTTSENDER_H

#include <QObject>

#include "mqttclient.h"
#include "meteocollector.h"

class MqttSender : public QObject
{
    Q_OBJECT
public:
    explicit MqttSender(MqttClient *mqtt, MeteoCollector *collector, QObject *parent = 0);

private:
    MqttClient *mqtt;
    MeteoCollector *collector;

private slots:
    void windUpdate();
    void airTempUpdate();
    void airPressUpdate();

};

#endif // MQTTSENDER_H
