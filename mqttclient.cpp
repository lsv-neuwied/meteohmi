#include "mqttclient.h"

#include <time.h>

int MqttClient::libInitCount = 0;

#define LOOP_MISC_PERIOD_MS 100

static long long currentTimestamp() {
    struct timespec tp;
    clock_gettime(CLOCK_MONOTONIC, &tp);
    return (long long) tp.tv_sec * 1000LL + ((long long) tp.tv_nsec / 1000000LL);
}

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
    snRead = NULL;
    snWrite = NULL;

    if (libInitCount == 0) {
        mosquitto_lib_init();
    }
    libInitCount++;

    mosq = mosquitto_new(clientId.isNull() ? NULL : clientId.toLocal8Bit().constData(), cleanSession, (void *) this);

    mosquitto_connect_callback_set(mosq, connect_callback);
    mosquitto_disconnect_callback_set(mosq, disconnect_callback);
    mosquitto_publish_callback_set(mosq, publish_callback);
    mosquitto_message_callback_set(mosq, message_callback);
    mosquitto_subscribe_callback_set(mosq, subscribe_callback);
    mosquitto_unsubscribe_callback_set(mosq, unsubscribe_callback);
    mosquitto_log_callback_set(mosq, log_callback);

    startTimer(LOOP_MISC_PERIOD_MS);
}

MqttClient::~MqttClient()
{
    disconnectBroker();
    mosquitto_destroy(mosq);

    libInitCount--;
    if (libInitCount == 0) {
        mosquitto_lib_cleanup();
    }
}

int MqttClient::setUsernamePassword(const QString &username, const QString &password)
{
    return mosquitto_username_pw_set(mosq, username.toLocal8Bit().constData(), password.toLocal8Bit().constData());
}

int MqttClient::connectBroker(const QString &host, int port, int keepalive, int reconnect_intervall)
{
    if (isConnected) {
        return MOSQ_ERR_SUCCESS;
    }

    int rc = mosquitto_connect(mosq, host.toLocal8Bit().constData(), port, keepalive);
    if (rc != MOSQ_ERR_SUCCESS) {
        return rc;
    }

    reconnect_intervall_ms = reconnect_intervall * 1000L;
    last_connected_ms = currentTimestamp();
    isConnected = true;

    createSocketNotifier();

    return MOSQ_ERR_SUCCESS;
}

int MqttClient::disconnectBroker()
{
    if (!isConnected) {
        return MOSQ_ERR_SUCCESS;
    }

    isConnected = false;
    reconnect_intervall_ms = 0;
    last_connected_ms = 0;

    return mosquitto_disconnect(mosq);
}

void MqttClient::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event)
    if (isConnected) {
        createSocketNotifier();
        mosquitto_loop_misc(mosq);

        // check for reconnect
        if (mosquitto_socket(mosq) >= 0) {
            last_connected_ms = currentTimestamp();
        } else if (last_connected_ms < (currentTimestamp() - reconnect_intervall_ms)) {
            mosquitto_reconnect(mosq);
        }
    }
}

void MqttClient::createSocketNotifier() {
    int fd = mosquitto_socket(mosq);
    if (fd < 0) {
        return;
    }

    if (snRead == NULL) {
        snRead = new QSocketNotifier(fd, QSocketNotifier::Read);
        connect(snRead, SIGNAL(activated(int)), this, SLOT(readyRead(int)));
    }
    if (snWrite == NULL) {
        snWrite = new QSocketNotifier(fd, QSocketNotifier::Write);
        snWrite->setEnabled(false);
        connect(snWrite, SIGNAL(activated(int)), this, SLOT(readyWrite(int)));
    }
    enableWriteSocketNotifier();
}

void MqttClient::deleteSocketNotifier() {
    if (snRead != NULL) {
        delete snRead;
        snRead = NULL;
    }
    if (snWrite != NULL) {
        delete snWrite;
        snWrite = NULL;
    }
}

void MqttClient::enableWriteSocketNotifier() {
    if (snWrite != NULL) {
        snWrite->setEnabled(mosquitto_want_write(mosq));
    }
}

int MqttClient::publish(const QString &topic, const QByteArray &payload, int qos, bool retain, int *mid)
{
    int ret = mosquitto_publish(mosq,
                                mid,
                                topic.toLocal8Bit().constData(),
                                payload.length(),
                                payload.constData(),
                                qos,
                                retain);
    enableWriteSocketNotifier();
    return ret;
}

int MqttClient::subscribe(const QString &sub, int qos, int *mid)
{
    return mosquitto_subscribe(mosq, mid, sub.toLocal8Bit().constData(), qos);
}

int MqttClient::unsubscribe(const QString &sub, int *mid)
{
    return mosquitto_unsubscribe(mosq, mid, sub.toLocal8Bit().constData());
}

void MqttClient::readyRead(int socket) {
    Q_UNUSED(socket)
    mosquitto_loop_read(mosq, 1);
    enableWriteSocketNotifier();
}

void MqttClient::readyWrite(int socket) {
    Q_UNUSED(socket)
    mosquitto_loop_write(mosq, 1);
    enableWriteSocketNotifier();
}

void MqttClient::connectCallback(int rc)
{
    emit onConnect(rc);
}

void MqttClient::disconnectCallback(int rc)
{
    deleteSocketNotifier();
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
