#include "mqttclient.h"

static void connect_callback(struct mosquitto *mosq, void *obj, int rc)
{
    Q_UNUSED(mosq)
    MqttClient *client = (MqttClient *) obj;
    client->connectCallback(rc);
}

static void disconnect_callback(struct mosquitto *mosq, void *obj, int rc)
{
    Q_UNUSED(mosq)
    MqttClient *client = (MqttClient *) obj;
    client->disconnectCallback(rc);
}

static void publish_callback(struct mosquitto *mosq, void *obj, int mid)
{
    Q_UNUSED(mosq)
    MqttClient *client = (MqttClient *) obj;
    client->publishCallback(mid);
}

static void message_callback(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message)
{
    Q_UNUSED(mosq)
    MqttClient *client = (MqttClient *) obj;
    client->messageCallback(message);
}

static void subscribe_callback(struct mosquitto *mosq, void *obj, int mid, int qos_count, const int *granted_qos)
{
    Q_UNUSED(mosq)
    MqttClient *client = (MqttClient *) obj;
    client->subscribeCallback(mid, qos_count, granted_qos);
}

static void unsubscribe_callback(struct mosquitto *mosq, void *obj, int mid)
{
    Q_UNUSED(mosq)
    MqttClient *client = (MqttClient *) obj;
    client->unsubscribeCallback(mid);
}

static void log_callback(struct mosquitto *mosq, void *obj, int level, const char *str)
{
    Q_UNUSED(mosq)
    MqttClient *client = (MqttClient *) obj;
    client->logCallback(level, str);
}

MqttClient::MqttClient(QString clientId, bool cleanSession, QObject *parent) : QObject(parent)
{
    isConnected = false;

    mosq = mosquitto_new(clientId.isNull() ? NULL : clientId.toLocal8Bit().constData(), cleanSession, (void *) this);

    mosquitto_connect_callback_set(mosq, connect_callback);
    mosquitto_disconnect_callback_set(mosq, disconnect_callback);
    mosquitto_publish_callback_set(mosq, publish_callback);
    mosquitto_message_callback_set(mosq, message_callback);
    mosquitto_subscribe_callback_set(mosq, subscribe_callback);
    mosquitto_unsubscribe_callback_set(mosq, unsubscribe_callback);
    mosquitto_log_callback_set(mosq, log_callback);
}

MqttClient::~MqttClient()
{
    disconnectBroker();
    mosquitto_destroy(mosq);
}

int MqttClient::setUsernamePassword(const QString &username, const QString &password)
{
    return mosquitto_username_pw_set(mosq, username.toLocal8Bit().constData(), password.toLocal8Bit().constData());
}

int MqttClient::connectBroker(const QString &host, int port, int keepalive, unsigned int reconnect_delay)
{
    int rc;

    if (isConnected) {
        return MOSQ_ERR_SUCCESS;
    }

    rc =  mosquitto_reconnect_delay_set(mosq, reconnect_delay, reconnect_delay, false);
    if (rc != MOSQ_ERR_SUCCESS) {
        return rc;
    }

    mosquitto_connect_async(mosq, host.toLocal8Bit().constData(), port, keepalive);

    rc = mosquitto_loop_start(mosq);
    if (rc != MOSQ_ERR_SUCCESS) {
        mosquitto_disconnect(mosq);
        return rc;
    }

    isConnected = true;

    return MOSQ_ERR_SUCCESS;
}

void MqttClient::disconnectBroker()
{
    if (!isConnected) {
        return;
    }

    isConnected = false;

    mosquitto_disconnect(mosq);
    mosquitto_loop_stop(mosq, false);
}

int MqttClient::publish(const QString &topic, const QByteArray &payload, int qos, bool retain, int *mid)
{
    return mosquitto_publish(mosq,
                             mid,
                             topic.toLocal8Bit().constData(),
                             payload.length(),
                             payload.constData(),
                             qos,
                             retain);

}

int MqttClient::subscribe(const QString &sub, int qos, int *mid)
{
    return mosquitto_subscribe(mosq, mid, sub.toLocal8Bit().constData(), qos);
}

int MqttClient::unsubscribe(const QString &sub, int *mid)
{
    return mosquitto_unsubscribe(mosq, mid, sub.toLocal8Bit().constData());
}

void MqttClient::connectCallback(int rc)
{
    isOnline = true;
    emit onConnect(rc);
}

void MqttClient::disconnectCallback(int rc)
{
    isOnline = false;
    emit onDisconnect(rc);
}

void MqttClient::publishCallback(int mid)
{
    emit onPublish(mid);
}

void MqttClient::messageCallback(const struct mosquitto_message *message)
{
    emit onMessage(
                message->mid,
                QString::fromLocal8Bit(message->topic),
                QByteArray((const char *) message->payload, message->payloadlen),
                message->qos,
                message->retain);
}

void MqttClient::subscribeCallback(int mid, int qos_count, const int *granted_qos)
{
    QVector<int> qos_vect(qos_count);
    std::copy(granted_qos, granted_qos + qos_count, std::back_inserter(qos_vect));
    emit onSubscribe(mid, qos_vect);
}

void MqttClient::unsubscribeCallback(int mid)
{
    emit onUnsubscribe(mid);
}

void MqttClient::logCallback(int level, const char *str)
{
    emit onLog(level, QString::fromLocal8Bit(str));
}
