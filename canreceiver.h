#ifndef CANRECEIVER_H
#define CANRECEIVER_H

#include <QObject>
#include <QSocketNotifier>

#define CANRECEIVER_ERR_OK             0
#define CANRECEIVER_ERR_ALREADY_OPEN  -1
#define CANRECEIVER_ERR_CREATE_SOCKET -2
#define CANRECEIVER_ERR_SET_IFACE     -3
#define CANRECEIVER_ERR_BIND          -4

class CanReceiver : public QObject
{
    Q_OBJECT
public:
    explicit CanReceiver(QObject *parent = 0);

    int startup(const QString &interface);
    void shutdown();

private:
    int fd;
    QSocketNotifier *sn;

signals:
    void received(bool isEff, bool isRtr, bool isErr, quint32 canId, const QByteArray &data);

private slots:
    void readyRead(int socket);

};

#endif // CANRECEIVER_H
