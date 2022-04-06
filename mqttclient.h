#ifndef MQTTCLIENT_H
#define MQTTCLIENT_H

#include <QObject>
#include <QVariant>
#include <QVector>
#include <QSocketNotifier>

#include <mosquitto.h>

class MqttClient : public QObject
{
    Q_OBJECT
public:
    explicit MqttClient(QString clientId = QString(), bool cleanSession = true, QObject *parent = 0);
    virtual ~MqttClient();

    int setUsernamePassword(const QString &username, const QString &password);

    int connectBroker(const QString &host, int port, int keepalive, int reconnect_intervall);
    int disconnectBroker();

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

    static int libInitCount;

    struct mosquitto *mosq;
    long long reconnect_intervall_ms;
    long long last_connected_ms;

    bool isConnected;
    QSocketNotifier *snRead;
    QSocketNotifier *snWrite;

signals:
    void onConnect(int rc);
    void onDisconnect(int rc);
    void onPublish(int mid);
    void onMessage(int mid, const QString &topic, const QByteArray &payload, int qos, bool retain);
    void onSubscribe(int mid, const QVector<int> &granted_qos);
    void onUnsubscribe(int mid);
    void onLog(int level, const QString &str);

private slots:
    void readyRead(int socket);
    void readyWrite(int socket);

protected:
    void timerEvent(QTimerEvent *event);

};

#endif // MQTTCLIENT_H
