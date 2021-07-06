#include "modbussettingsloader.h"


///[!] matilda-bbb-settings
#include "src/matilda/settloader4matilda.h"



ModbusSettingsLoader::ModbusSettingsLoader(QObject *parent) : QObject(parent)
{

}

void ModbusSettingsLoader::insertModbusForwardingTable(ModbusVirtualDevices &vdevs)
{
    const QVariantHash h =  SettLoader4matilda().loadOneSett(SETT_MODBUS_DEVICES_SETTINGS).toHash();//modbus address to other device type and address


    const auto lk = h.keys();

    for(int i = 0, imax = lk.size(); i < imax; i++){
        bool ok;
        const QString key = lk.at(i);
         const quint32 modbusAddress = key.toUInt(&ok);


         const auto onedevice = oneModbusDeviceFromLine(h.value(key).toString());

         if(onedevice.ni.isEmpty() || modbusAddress < 1 || modbusAddress > 250 || !ok)
             continue;

         vdevs.insert(modbusAddress, onedevice);
    }
}

OneModbusVirtualDevice ModbusSettingsLoader::oneModbusDeviceFromLine(const QString &line)
{
    OneModbusVirtualDevice onedevice;

    const QStringList l = line.split(" ");

    const int len = l.size();

    if(len > 0)
        onedevice.ni = l.at(0);
    if(len > 1)
        onedevice.sn = l.at(1);
    if(len > 2)
        onedevice.memo = l.mid(2).join(" ");

    return onedevice;
}


