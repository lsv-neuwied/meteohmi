#include "canreceiver.h"

#include <stdio.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/fd.h>
#include <net/if.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <linux/types.h>
#include <sys/eventfd.h>

CanReceiver::CanReceiver(QObject *parent) : QObject(parent)
{
    fd = -1;
    sn = NULL;
}

int CanReceiver::startup(const QString &interface) {
    int err = CANRECEIVER_ERR_OK;

    if (fd >= 0) {
        err = CANRECEIVER_ERR_ALREADY_OPEN;
        goto fail0;
    }

    // open socket
    if ((fd = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0) {
        err = CANRECEIVER_ERR_CREATE_SOCKET;
        goto fail0;
    }

    // get interface index
    struct ifreq ifr;
    strcpy(ifr.ifr_name, interface.toLocal8Bit().constData());
    if (ioctl(fd, SIOCGIFINDEX, &ifr) < 0) {
        err = CANRECEIVER_ERR_SET_IFACE;
        goto fail1;
    }

    // bind to socket
    struct sockaddr_can addr;
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;
    if (bind(fd, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
        err = CANRECEIVER_ERR_BIND;
        goto fail1;
    }

    // create Socket Notitication
    sn = new QSocketNotifier(fd, QSocketNotifier::Read);
    connect(sn, SIGNAL(activated(int)), this, SLOT(readyRead(int)));

    // everything is fine
    return CANRECEIVER_ERR_OK;

    // error handling
fail1:
    close(fd);
fail0:
    return err;
}


void CanReceiver::shutdown() {
    if (fd < 0) {
        return;
    }

    delete(sn);
    close(fd);

    fd = -1;
    sn = NULL;
}

void CanReceiver::readyRead(int socket) {
    struct can_frame rcvd_frame;
    ssize_t n = read(socket, &rcvd_frame, sizeof(rcvd_frame));

    if (n != sizeof(struct can_frame)) {
        return;
    }

    // get flags
    bool isEff = (rcvd_frame.can_id & CAN_EFF_FLAG);
    bool isRtr = (rcvd_frame.can_id & CAN_RTR_FLAG);
    bool isErr = (rcvd_frame.can_id & CAN_ERR_FLAG);

    // get can id
    quint32 canId = rcvd_frame.can_id & (isEff ? CAN_EFF_MASK : CAN_SFF_MASK);

    // get data
    QByteArray data = QByteArray((const char *)rcvd_frame.data, rcvd_frame.can_dlc);

    emit received(isEff, isRtr, isErr, canId, data);
}
