#ifndef N2KPARSER_H
#define N2KPARSER_H

#include <QObject>

typedef enum {
    N2K_WIND_REF_GEO_NORTH = 0,
    N2K_WIND_REF_MAG_NORTH,
    N2K_WIND_REF_APPARENT,
    N2K_WIND_REF_BOAT,
    N2K_WIND_REF_WATER,
    _N2K_WIND_REF_EOL
} N2K_WIND_REF_T;

typedef enum {
    N2K_TEMP_SRC_INVAL = -1,
    N2K_TEMP_SRC_SEA = 0,
    N2K_TEMP_SRC_OUTSIDE,
    N2K_TEMP_SRC_INSIDE,
    N2K_TEMP_SRC_ENGINE_ROOM,
    N2K_TEMP_SRC_MAIN_CABIN,
    N2K_TEMP_SRC_LIVE_WELL,
    N2K_TEMP_SRC_BAIT_WELL,
    N2K_TEMP_SRC_REFRIDGE,
    N2K_TEMP_SRC_HEATING_SYSTEM,
    N2K_TEMP_SRC_DEW_POING,
    N2K_TEMP_SRC_APP_WIND_CHILL,
    N2K_TEMP_SRC_THEO_WIND_CHILL,
    N2K_TEMP_SRC_HEAT_INDEX,
    N2K_TEMP_SRC_FREEZER,
    N2K_TEMP_SRC_EXHAUST_GAS,
    _N2K_TEMP_SRC_EOL
} N2K_TEMP_SRC_T;

typedef enum {
    N2K_HUMI_SRC_INVAL = -1,
    N2K_HUMI_SRC_INSIDE = 0,
    N2K_HUMI_SRC_OUTSIDE,
    _N2K_HUMI_SRC_EOL
} N2K_HUMI_SRC_T;

typedef enum {
    N2K_PRESS_SRC_INVAL = -1,
    N2K_PRESS_SRC_ATMOSPHERIC = 0,
    N2K_PRESS_SRC_WATER,
    N2K_PRESS_SRC_STEAM,
    N2K_PRESS_SRC_COMP_AIR,
    N2K_PRESS_SRC_HYDRAULIC,
    _N2K_PRESS_SRC_EOL
} N2K_PRESS_SRC_T;

class N2kParser : public QObject
{
    Q_OBJECT
public:
    explicit N2kParser(const QObject *receiver, QObject *parent = 0);

signals:
    void receivedWindData(int sid, N2K_WIND_REF_T ref, double velo, double dir);
    void receivedEnvParams(int sid, N2K_TEMP_SRC_T tempSrc, double temp, N2K_HUMI_SRC_T humiSrc, double humi, double press);
    void receivedTemperature(int sid, int inst, N2K_TEMP_SRC_T source, double temp, double setp);
    void receivedActualPressure(int sid, int inst, N2K_PRESS_SRC_T source, double press);

private slots:
    void canReceived(bool isEff, bool isRtr, bool isErr, quint32 canId, const QByteArray &data);
};

#endif // N2KPARSER_H
