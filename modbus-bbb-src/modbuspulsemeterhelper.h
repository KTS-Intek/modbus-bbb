#ifndef MODBUSPULSEMETERHELPER_H
#define MODBUSPULSEMETERHELPER_H

#include "modbusmeterhelper.h"


class ModbusPulseMeterHelper : public ModbusMeterHelper
{
public:
    static int getAcceptablePMeterNis(ModbusVirtualDevices &vdevs, QStringList &listMetersNIs);

    static ModbusAnswerList getTotalPulseAnswer(const QList<QVariantHash> &listHash, const bool &verboseMode);

    static QList<quint32> getPulseValues(const QVariantHash &hdata, QList<qreal> &realv, QStringList &tablekeys);

};

#endif // MODBUSPULSEMETERHELPER_H
