#ifndef MODBUSFIREFLYDEVICESHELPER_H
#define MODBUSFIREFLYDEVICESHELPER_H

#include "modbusmeterhelper.h"
#include "modbusencoderdecodertypes.h"

class ModbusFireflyDevicesHelper : public ModbusMeterHelper
{
public:
    static int getAcceptableLcuNis(ModbusVirtualDevices &vdevs);

    static int getAcceptableGroupsNis(ModbusVirtualDevices &vdevs);

    static quint16 getTypeCode(const QString &typeStr, bool &ok);

    static ModbusAnswerList getLcuStateAnswer(const QVariantHash &hdata, const bool &verboseMode);

    static ModbusAnswerList getGroupsStateAnswer(const QVariantHash &hdata, const bool &verboseMode);




};

#endif // MODBUSFIREFLYDEVICESHELPER_H
