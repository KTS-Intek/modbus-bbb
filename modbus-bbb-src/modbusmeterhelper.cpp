#include "modbusmeterhelper.h"



MODBUSDIVIDED_INT32 ModbusMeterHelper::getDividedInt32(const qint32 &value)
{
    MODBUSDIVIDED_INT32 r;
    r.lowword = value & 0xFFFF;
    r.highword = (value >> 16);
    return r;
}

//-------------------------------------------------------------------------------------------

MODBUSDIVIDED_UINT32 ModbusMeterHelper::getDividedUInt32(const quint32 &value)
{
    MODBUSDIVIDED_UINT32 r;
    r.lowword = value & 0xFFFF;
    r.highword = (value >> 16);
    return r;
}

//-------------------------------------------------------------------------------------------

void ModbusMeterHelper::addDividedInt322thelist(ModbusAnswerList &list, const qint32 &value)
{//big endian
    const MODBUSDIVIDED_INT32 r = getDividedInt32(value);
    list.append(r.highword);
    list.append(r.lowword);


}

//-------------------------------------------------------------------------------------------

void ModbusMeterHelper::addDividedUInt322thelist(ModbusAnswerList &list, const quint32 &value)
{
    const MODBUSDIVIDED_UINT32 r = getDividedUInt32(value);
    list.append(r.highword);
    list.append(r.lowword);

}

//-------------------------------------------------------------------------------------------

QList<quint32> ModbusMeterHelper::getMeterValuesUIN32(const QStringList &listKeys, const qreal &precision, const QVariantHash &hdata, QList<qreal> &realv, QStringList &tablekeys)
{

    QList<quint32> l;

//    const quint16 baduint16 = 0xFFFF;
    const quint32 baduint32 = 0xFFFFFFFF;

//    const qint16 badint16 = 0x8000;
//    const qint32 badint32 = 0x80000000;

    for(int i = 0, imax = listKeys.size(); i < imax; i++){

            bool okv;
            const QString key = listKeys.at(i);

            const qreal v = hdata.value(key).toDouble(&okv);
            tablekeys.append(key);
            realv.append(v);
            const quint32 value = okv ? quint32(qAbs(v)/precision) : baduint32;
            l.append(value);


    }
    return l;
}

//-------------------------------------------------------------------------------------------

QList<quint8> ModbusMeterHelper::getAcceptableMeterNis(const QVariantList &l)
{
    QList<quint8> devadds;

    for(int i = 0, imax = l.size(); i < imax; i++){
        const QVariantHash hash = l.at(i).toHash();


        const quint8 add = meterAddressFromHash(hash);
        if(add > 0)
            devadds.append(quint8(add));
    }

    return devadds;
}

//-------------------------------------------------------------------------------------------

quint8 ModbusMeterHelper::meterAddressFromHash(const QVariantHash &hash)
{
    if(hash.isEmpty())
        return 0;

    if((hash.value("on",false).toBool() && hash.value("on").toString() != "-")|| hash.value("on").toString() == "+"){
        bool ok;
        const quint64 add = hash.value("NI").toULongLong(&ok);
        if(ok && add > 0 && add <= 248)//I need only acceptable modbus addresses
            return quint8(add); // devadds.append(quint8(add));

    }
    return 0;
}

//-------------------------------------------------------------------------------------------