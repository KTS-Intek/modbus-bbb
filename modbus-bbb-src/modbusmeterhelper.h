#ifndef MODBUSMETERHELPER_H
#define MODBUSMETERHELPER_H


#include <QVariantHash>
#include "modbusencoderdecodertypes.h"
#include "dataholdertypes.h"
#include "modbustypes.h"


struct MODBUSDIVIDED_INT32
{
    quint16 highword;
    quint16 lowword;
    MODBUSDIVIDED_INT32() : highword(0), lowword(0) {}
    MODBUSDIVIDED_INT32(const quint16 &highword, const quint16 &lowword) : highword(highword), lowword(lowword) {}
};



struct MODBUSDIVIDED_UINT32
{
    quint16 highword;
    quint16 lowword;
    MODBUSDIVIDED_UINT32() : highword(0), lowword(0) {}
    MODBUSDIVIDED_UINT32(const quint16 &highword, const quint16 &lowword) : highword(highword), lowword(lowword) {}
};



class ModbusMeterHelper
{
public:
    static MODBUSDIVIDED_INT32 getDividedInt32(const qint32 &value);
    static MODBUSDIVIDED_UINT32 getDividedUInt32(const quint32 &value);

    static void addDividedInt322thelist(ModbusAnswerList &list, const qint32 &value);
    static void addDividedUInt322thelist(ModbusAnswerList &list, const quint32 &value);

    static QList<quint32> getMeterValuesUIN32(const QStringList &listKeys, const qreal &precision, const QVariantHash &hdata, QList<qreal> &realv, QStringList &tablekeys);


    static void getAcceptableMeterNis(const QVariantList &l, ModbusVirtualDevices &vdevs);

    static quint8 meterAddressFromHash(const QVariantHash &hash, OneModbusVirtualDevice &onedev);

};

#endif // MODBUSMETERHELPER_H
