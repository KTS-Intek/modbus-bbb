#ifndef MODBUSGASMETERHELPER_H
#define MODBUSGASMETERHELPER_H



#include "modbusmeterhelper.h"



class ModbusGasMeterHelper : public ModbusMeterHelper
{
public:
    static QList<quint8> getAcceptableGMeterNis();

    static ModbusAnswerList getTotalGasAnswer(const QVariantHash &hdata, const bool &verboseMode);

    static QList<quint32> getGasValues(const QVariantHash &hdata, QList<qreal> &realv, QStringList &tablekeys);

};

#endif // MODBUSGASMETERHELPER_H
