#include "modbusmeterhelper.h"



//-------------------------------------------------------------------------------------------

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

MODBUSDIVIDED_INT64 ModbusMeterHelper::getDividedInt64(const qint64 &value)
{
    MODBUSDIVIDED_INT64 r;
    r.register4 = value & 0xFFFF;
    r.register3 = (value >> 16) & 0xFFFF;
    r.register2 = (value >> 32) & 0xFFFF;
    r.register1 = (value >> 48);
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

void ModbusMeterHelper::addDividedInt642thelist(ModbusAnswerList &list, const qint64 &value)
{
    const MODBUSDIVIDED_INT64 v = getDividedInt64(value);
    list.append(v.register1);
    list.append(v.register2);
    list.append(v.register3);
    list.append(v.register4);
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

int ModbusMeterHelper::getAcceptableMeterNis(const QVariantList &l, ModbusVirtualDevices &vdevs)
{
    QStringList listMeterNIs;
    return getAcceptableMeterNisExt(l, vdevs, listMeterNIs);
}
//-------------------------------------------------------------------------------------------
int ModbusMeterHelper::getAcceptableMeterNisExt(const QVariantList &l, ModbusVirtualDevices &vdevs, QStringList &listMeterNIs)
{
    int counter = 0;
    for(int i = 0, imax = l.size(); i < imax; i++){
        const QVariantHash hash = l.at(i).toHash();

        const QString ni = hash.value("NI").toString();
        if(ni.isEmpty())
            continue;
        listMeterNIs.append(ni);

        OneModbusVirtualDevice onedev;
        const quint8 add = meterAddressFromHash(hash, onedev);
        if(add > 0){
            vdevs.insert(add, onedev);
            counter++;
        }

    }

    return counter;
}

//-------------------------------------------------------------------------------------------

quint8 ModbusMeterHelper::meterAddressFromHash(const QVariantHash &hash, OneModbusVirtualDevice &onedev)
{
    if(hash.isEmpty())
        return 0;

    if((hash.value("on",false).toBool() && hash.value("on").toString() != "-")|| hash.value("on").toString() == "+"){
        bool ok;
        const quint64 add = hash.value("NI").toULongLong(&ok);
        if(ok && add > 0 && add <= 248){//I need only acceptable modbus addresses
            onedev.memo = hash.value("memo").toString();
            onedev.ni = QString::number(add);
            return quint8(add); // devadds.append(quint8(add));
        }

    }
    return 0;
}

//-------------------------------------------------------------------------------------------
