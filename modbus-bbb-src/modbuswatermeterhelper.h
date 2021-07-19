#ifndef MODBUSWATERMETERHELPER_H
#define MODBUSWATERMETERHELPER_H

#include "modbusmeterhelper.h"


class ModbusWaterMeterHelper : public ModbusMeterHelper
{
public:
    static int getAcceptableWMeterNis(ModbusVirtualDevices &vdevs);

    static ModbusAnswerList getTotalWaterAnswer(const QVariantHash &hdata, const bool &verboseMode);

    static QList<quint32> getWaterValues(const QVariantHash &hdata, QList<qreal> &realv, QStringList &tablekeys);



};

#endif // MODBUSWATERMETERHELPER_H
