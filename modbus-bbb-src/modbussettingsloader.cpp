#include "modbussettingsloader.h"


///[!] matilda-bbb-settings
#include "src/matilda/settloader4matilda.h"
#include "src/matilda/ifacesettconverter.h"


#include "ifaceconnectiondefs.h"


//---------------------------------------------------------------------------

ModbusSettingsLoader::ModbusSettingsLoader(QObject *parent) : QObject(parent)
{

}

//---------------------------------------------------------------------------

int ModbusSettingsLoader::insertModbusForwardingTable(ModbusVirtualDevices &vdevs)
{
    int counter = 0;
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
         counter++;
    }

    return counter;
}

//---------------------------------------------------------------------------

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

//---------------------------------------------------------------------------

ModbusGeneralSettings ModbusSettingsLoader::getModbusGeneralSettings()
{
    const QVariantHash h = SettLoader4matilda().loadOneSett(SETT_MODBUS_SETTINGS).toHash();


//    h.insert("dataTmsec", 3000);
//    h.insert("defCacheTsec", 60);//-1 - ignore, seconds, cached data valid timeout, in other case try to update the data or say there is not data
//    h.insert("cacheTsec", QStringList());//<poll code><space><valid timeout>, seconds, -1 - ignore

//    h.insert("devSrc", 0);//bits - bit0 - emeter, bit1 - wmeter, bit2 - gas meters, bit3 - pulse, it always uses own forwarding table


    ModbusGeneralSettings settings;

    settings.dataTmsec = h.value("dataTmsec").toUInt();
    settings.defCacheTsec = h.value("defCacheTsec").toUInt();
    settings.devSrc = h.value("devSrc").toUInt();

    settings.cacheTsec = mapTsecFromList( h.value("cacheTsec").toStringList());

    return settings;

}

//---------------------------------------------------------------------------

QMap<quint8, quint32> ModbusSettingsLoader::mapTsecFromList(const QStringList &l)
{
    QMap<quint8, quint32> map;
    for(int i = 0, imax = l.size(); i < imax; i++){
        const QStringList ll = l.at(i).split(" ");
        if(ll.size() < 2)
            continue;


        bool okcode, oktimeout;
        const quint8 pollCode = ll.at(0).toUInt(&okcode);
        const quint32 timeout = ll.at(1).toUInt(&oktimeout);

        if(!okcode || !oktimeout || pollCode  < 1 )
            continue;

        map.insert(pollCode, timeout);

    }
    return map;
}

//---------------------------------------------------------------------------

ModbusTcpSettings ModbusSettingsLoader::getModbusTcpSettings()
{
    const QVariantHash h = SettLoader4matilda().loadOneSett(SETT_MODBUS_TCP_IFACE).toHash();

    ModbusTcpSettings settings;


    settings.tcpTimeout = h.value("tcpTimeout").toUInt();
    settings.tcpBlcokTimeout = h.value("tcpBlcokTimeout").toUInt();
    settings.mode = h.value("mode").toUInt();
    settings.IPs = h.value("IPs").toStringList();


    return settings;
}

//---------------------------------------------------------------------------

ModbusSerialSettings ModbusSettingsLoader::getModbusSerialSettings()
{
//    QVariantHash h = inData.toHash();
//    h.insert("ifaceMode", IFACECONNTYPE_UART);
//    SettLoader4matilda().saveOneSett(SETT_MODBUS_SERIAL_IFACE, h);

    const QVariantHash h = SettLoader4matilda().loadOneSett(SETT_MODBUS_SERIAL_IFACE).toHash();
    ModbusSerialSettings settings;
    settings.enRTU = h.value("enRTU").toBool();


    settings.serialport = h; // IfaceSett2human::fromVarHash(h).uart;   convert it to RezUpdateConnSettings
    settings.serialport.insert("ifaceMode", IFACECONNTYPE_UART);//force to use UART
    settings.serialport.remove("enRTU");//

    return settings;

}

QVariantMap ModbusSettingsLoader::getInterafaceSettMap(const QVariantHash &inh, QString &errMessage)
{
   return IfaceSettConverter::convertHashSett2varMap(inh, QVariantHash(), errMessage);
}

//---------------------------------------------------------------------------


