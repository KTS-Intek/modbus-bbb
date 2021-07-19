#ifndef MODBUSSETTINGSLOADER_H
#define MODBUSSETTINGSLOADER_H

#include <QObject>



#include "modbusencoderdecodertypes.h"

class ModbusSettingsLoader : public QObject
{
    Q_OBJECT
public:
    explicit ModbusSettingsLoader(QObject *parent = nullptr);

    static int insertModbusForwardingTable(ModbusVirtualDevices &vdevs);

    static OneModbusVirtualDevice oneModbusDeviceFromLine(const QString &line);

    static ModbusGeneralSettings getModbusGeneralSettings();

    static QMap<quint8, quint32> mapTsecFromList(const QStringList &l);

    static ModbusTcpSettings getModbusTcpSettings();

    static ModbusSerialSettings getModbusSerialSettings();

    static QVariantMap getInterafaceSettMap(const QVariantHash &inh, QString &errMessage);

signals:

};

#endif // MODBUSSETTINGSLOADER_H
