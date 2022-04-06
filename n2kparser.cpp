#include "n2kparser.h"

#define KELVIN_OFFSET (-273.15)

class NmeaBuffer
{
public:
    NmeaBuffer(const QByteArray &data) : data(data), pos(0) {}

    int length() {
        return data.length();
    }

    quint8 getByte() {
        return (quint8) data[pos++];
    }

    qint8 getSignedByte() {
        return getByte();
    }

    quint16 getShort() {
        return ((quint16) getByte()) | (((quint16) getByte()) << 8);
    }

    qint16 getSignedShort() {
        return getShort();
    }

    quint32 getLong() {
        return ((quint32) getByte()) | (((quint32) getByte()) << 8) | (((quint32) getByte()) << 16) | (((quint32) getByte()) << 24);
    }

    qint32 getSignedLong() {
        return getLong();
    }

private:
    const QByteArray data;
    int pos;

};

N2kParser::N2kParser(const QObject *receiver, QObject *parent) : QObject(parent)
{
    connect(receiver, SIGNAL(received(bool, bool, bool, quint32, const QByteArray &)), this, SLOT(canReceived(bool, bool, bool, quint32, const QByteArray &)));
}

void N2kParser::canReceived(bool isEff, bool isRtr, bool isErr, quint32 canId, const QByteArray &data) {
    Q_UNUSED(isRtr);

    // ignore error frames
    if (isErr) {
        return;
    }

    // need extended frame
    if (!isEff) {
        return;
    }

    NmeaBuffer buf(data);
    qint32 pgn = (canId >> 8) & 0x3ffff;

    // Wind Data
    if (pgn == 130306 && buf.length() >= 6) {
        int sid = buf.getByte();
        int veloRaw = buf.getShort();
        int dirRaw = buf.getSignedShort();
        int ref = buf.getByte() & 0x07;

        if (ref < _N2K_WIND_REF_EOL) {
            double velo = (double) veloRaw * 0.01;
            double dir = (double) dirRaw * 0.0001;
            emit receivedWindData(sid, (N2K_WIND_REF_T) ref, velo, dir);
        }
        return;
    }

    // Environmental Parameters
    if (pgn == 130311 && buf.length() >= 8) {
        int sid = buf.getByte();
        int source = buf.getByte();
        int tempRaw = buf.getShort();
        int humiRaw = buf.getShort();
        int pressRaw = buf.getShort();

        double temp = 0.0;
        int tempSrc = source & 0x3f;
        if (tempSrc >= _N2K_TEMP_SRC_EOL) {
            tempSrc = N2K_TEMP_SRC_INVAL;
        } else {
            temp = (double) tempRaw * 0.01 + KELVIN_OFFSET;
        }

        double humi = 0.0;
        int humiSrc = (source >> 6) & 0x03;
        if (humiSrc >= _N2K_HUMI_SRC_EOL) {
            humiSrc = N2K_HUMI_SRC_INVAL;
        } else {
            humi = (double) humiRaw * 0.004;
        }

        double press = (double) pressRaw * 1.0;

        emit receivedEnvParams(sid, (N2K_TEMP_SRC_T) tempSrc, temp, (N2K_HUMI_SRC_T) humiSrc, humi, press);
        return;
    }

    // Temperature
    if (pgn == 130312 && buf.length() >= 7) {
        int sid = buf.getByte();
        int inst = buf.getByte();
        int source = buf.getByte();
        int tempRaw = buf.getShort();
        int setpRaw = buf.getShort();

        if (source < _N2K_TEMP_SRC_EOL) {
            double temp = (double) tempRaw * 0.01 + KELVIN_OFFSET;
            double setp = (double) setpRaw * 0.01 + KELVIN_OFFSET;
            emit receivedTemperature(sid, inst, (N2K_TEMP_SRC_T) source, temp, setp);
        }
        return;
    }

    // Actual Pressure
    if (pgn == 130314 && buf.length() >= 7) {
        int sid = buf.getByte();
        int inst = buf.getByte();
        int source = buf.getByte();
        unsigned int pressRaw = buf.getLong();

        if (source < _N2K_PRESS_SRC_EOL) {
            double press = (double) pressRaw * 0.001;
            emit receivedActualPressure(sid, inst, (N2K_PRESS_SRC_T) source, press);
        }
        return;
    }
}
