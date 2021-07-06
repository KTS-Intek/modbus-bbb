#ifndef MODBUSSETTINGSLOADER_H
#define MODBUSSETTINGSLOADER_H

#include <QObject>
#include "modbusencoderdecodertypes.h"

class ModbusSettingsLoader : public QObject
{
    Q_OBJECT
public:
    explicit ModbusSettingsLoader(QObject *parent = nullptr);

    static void insertModbusForwardingTable(ModbusVirtualDevices &vdevs);

    static OneModbusVirtualDevice oneModbusDeviceFromLine(const QString &line);

signals:

};

#endif // MODBUSSETTINGSLOADER_H
