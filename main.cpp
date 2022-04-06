#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "canreceiver.h"
#include "n2kparser.h"
#include "meteocollector.h"
#include "meteobinding.h"
#include "mqttclient.h"
#include "mqttsender.h"

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication app(argc, argv);

    if (argc < 7) {
        printf("usage: MeteoHMI <mqttClientId> <mqttHost> <mqttPort> <runwayAngle> <windDirOffset> <airPressOffset> [<mqttUser> <mqttPasswd>]\n");
        return 1;
    }

    QString mqttClientId(argv[1]);
    QString mqttHost(argv[2]);
    int mqttPort = atoi(argv[3]);
    double runwayAngle = atof(argv[4]);
    double windDirOffset = atof(argv[5]);
    double airPressOffset = atof(argv[6]);

    QString mqttUser, mqttPasswd;
    if (argc >= 9) {
        mqttUser = QString(argv[7]);
        mqttPasswd = QString(argv[8]);
    }

    CanReceiver receiver;
    N2kParser parser(&receiver);
    MeteoCollector collector(&parser, windDirOffset, airPressOffset);
    MeteoBinding meteo(&collector, runwayAngle);

    MqttClient mqtt(mqttClientId);
    if (!mqttUser.isNull()) {
        mqtt.setUsernamePassword(mqttUser, mqttPasswd);
    }
    mqtt.connectBroker(mqttHost, mqttPort, 0, 5);

    MqttSender sender(&mqtt, &collector);

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("meteo", &meteo);
    engine.load(QUrl(QLatin1String("qrc:/main.qml")));

    receiver.startup("can0");

    return app.exec();
}
