#ifndef MODBUSENCODERDECODERTYPES_H
#define MODBUSENCODERDECODERTYPES_H

#include <QVariantHash>


struct OnePhaseRealData
{
    bool okVoltage;
    qreal voltage;

    bool okCurrent;
    qreal current;

    bool okActivePower;
    qreal activepower;

    bool okReactivePower;
    qreal reactivepower;

    bool okApparentPower;
    qreal apparentpower;

    bool okPowerFactor;
    qreal powerfactor;

    OnePhaseRealData() {}
};

struct OnePhaseModbusAnswerFormat
{
    quint16 voltage;//0.01 V
    qint32 current;//0.001 A
    qint32 activepower;//0.1 W
    qint32 reactivepower;//0.1 var
    qint32 apparentpower;// 0.1 VA
    qint16 powerfactor;//0.001

    OnePhaseModbusAnswerFormat() {}
};



struct OneModbusVirtualDevice
{
    QString ni;//network id
    QString sn;//additional id, if empty ignore,
    QString memo;//keep it here also
    OneModbusVirtualDevice() {}
};


typedef QMap<quint8, OneModbusVirtualDevice> ModbusVirtualDevices;


struct ModbusGeneralSettings
{
    quint32 dataTmsec;
    quint32 defCacheTsec;
    QMap<quint8, quint32> cacheTsec;

    quint8 devSrc;//bits
    ModbusGeneralSettings() : dataTmsec(3000), defCacheTsec(100), devSrc(0) {}
};



struct ModbusTcpSettings
{
    quint8 mode;
    quint32 tcpTimeout;
    quint32 tcpBlcokTimeout;
    QStringList IPs;

    ModbusTcpSettings() : mode(0), tcpTimeout(1000), tcpBlcokTimeout(100) {}
};

struct ModbusSerialSettings
{
    QVariantHash serialport;

    bool enRTU;
    ModbusSerialSettings() : enRTU(false) {}
};



#endif // MODBUSENCODERDECODERTYPES_H
