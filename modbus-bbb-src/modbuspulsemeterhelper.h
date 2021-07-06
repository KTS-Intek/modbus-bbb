#ifndef MODBUSPULSEMETERHELPER_H
#define MODBUSPULSEMETERHELPER_H

#include "modbusmeterhelper.h"


class ModbusPulseMeterHelper : public ModbusMeterHelper
{
public:
    static void getAcceptablePMeterNis(ModbusVirtualDevices &vdevs);

    static ModbusAnswerList getTotalPulseAnswer(const QVariantHash &hdata, const bool &verboseMode);

    static QList<quint32> getPulseValues(const QVariantHash &hdata, QList<qreal> &realv, QStringList &tablekeys);

};

#endif // MODBUSPULSEMETERHELPER_H
