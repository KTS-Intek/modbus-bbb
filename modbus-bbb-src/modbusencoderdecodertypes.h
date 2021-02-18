#ifndef MODBUSENCODERDECODERTYPES_H
#define MODBUSENCODERDECODERTYPES_H

#include <QString>


struct OnePhaseRealData
{
    qreal voltage;
    qreal current;
    qreal activepower;
    qreal reactivepower;
    qreal apparentpower;
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




#endif // MODBUSENCODERDECODERTYPES_H
