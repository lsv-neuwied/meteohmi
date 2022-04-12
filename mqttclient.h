#ifndef MQTTCLIENT_H
#define MQTTCLIENT_H

#include <cstdlib>
#include <mosquitto.h>

#include <QObject>
#include <QVariant>
#include <QVector>
#include <QSocketNotifier>

class MqttClient : public QObject
{
    Q_OBJECT
public:
    explicit MqttClient(QString clientId = QString(), bool cleanSession = true, QObject *parent = 0);
    virtual ~MqttClient();

    int setUsernamePassword(const QString &username, const QString &password);

    int connectBroker(const QString &host, int port, int keepalive, unsigned int reconnect_delay);
    void disconnectBroker();

    bool getIsConnected() { return isConnected; }
    bool getIsOnline() { return isOnline; }

    int publish(const QString &topic, const QByteArray &payload, int qos = 0, bool retain = false, int *mid = NULL);
    int subscribe(const QString &sub, int qos, int *mid = NULL);
    int unsubscribe(const QString &sub, int *mid = NULL);

    void connectCallback(int rc);
    void disconnectCallback(int rc);
    void publishCallback(int mid);
    void messageCallback(const struct mosquitto_message *message);
    void subscribeCallback(int mid, int qos_count, const int *granted_qos);
    void unsubscribeCallback(int mid);
    void logCallback(int level, const char *str);

private:
    void createSocketNotifier();
    void deleteSocketNotifier();
    void enableWriteSocketNotifier();

    struct mosquitto *mosq;

    bool isConnected;
    bool isOnline;

signals:
    // please use queued connections for this!!!
    void onConnect(int rc);
    void onDisconnect(int rc);
    void onPublish(int mid);
    void onMessage(int mid, const QString &topic, const QByteArray &payload, int qos, bool retain);
    void onSubscribe(int mid, const QVector<int> &granted_qos);
    void onUnsubscribe(int mid);
    void onLog(int level, const QString &str);

};

#endif // MQTTCLIENT_H
