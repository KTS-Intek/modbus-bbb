#ifndef MODBUSMETERHELPER_H
#define MODBUSMETERHELPER_H


#include <QVariantHash>
#include <QDateTime>
#include <QDebug>

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


struct MODBUSDIVIDED_INT64
{
    quint16 register1; //highest
    quint16 register2; //hight

    quint16 register3; //low
    quint16 register4; //lowest

    MODBUSDIVIDED_INT64() : register1(0), register2(0), register3(0), register4(0) {}
    MODBUSDIVIDED_INT64(const quint16 &register1, const quint16 &register2, const quint16 &register3, const quint16 &register4)
        : register1(register1), register2(register2), register3(register3), register4(register4) {}
};



class ModbusMeterHelper
{
public:
    static MODBUSDIVIDED_INT32 getDividedInt32(const qint32 &value);
    static MODBUSDIVIDED_UINT32 getDividedUInt32(const quint32 &value);

    static MODBUSDIVIDED_INT64 getDividedInt64(const qint64 &value);


    static void addDividedInt322thelist(ModbusAnswerList &list, const qint32 &value);
    static void addDividedUInt322thelist(ModbusAnswerList &list, const quint32 &value);
    static void addDividedInt642thelist(ModbusAnswerList &list, const qint64 &value);

    static QList<quint32> getMeterValuesUIN32(const QStringList &listKeys, const qreal &precision, const QVariantHash &hdata, QList<qreal> &realv, QStringList &tablekeys);


    static int getAcceptableMeterNis(const QVariantList &l, ModbusVirtualDevices &vdevs);
    static int getAcceptableMeterNisExt(const QVariantList &l, ModbusVirtualDevices &vdevs, QStringList &listMeterNIs);

    static quint8 meterAddressFromHash(const QVariantHash &hash, OneModbusVirtualDevice &onedev);

};

#endif // MODBUSMETERHELPER_H
