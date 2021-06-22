#ifndef MODBUSWATERMETERHELPER_H
#define MODBUSWATERMETERHELPER_H

#include "modbusmeterhelper.h"


class ModbusWaterMeterHelper : public ModbusMeterHelper
{
public:
    static QList<quint8> getAcceptableWMeterNis();

    static ModbusAnswerList getTotalWaterAnswer(const QVariantHash &hdata, const bool &verboseMode);

    static QList<quint32> getWaterValues(const QVariantHash &hdata, QList<qreal> &realv, QStringList &tablekeys);



};

#endif // MODBUSWATERMETERHELPER_H
