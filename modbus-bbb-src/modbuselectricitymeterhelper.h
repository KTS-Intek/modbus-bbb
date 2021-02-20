#ifndef MODBUSELECTRICITYMETERHELPER_H
#define MODBUSELECTRICITYMETERHELPER_H


#include <QVariantHash>
#include "modbusencoderdecodertypes.h"
#include "dataholdertypes.h"
#include "modbustypes.h"


struct MODBUSDIVIDED_INT32
{
    quint16 highword;
    quint16 lowword;
    MODBUSDIVIDED_INT32() : highword(0), lowword(0) {}
    MODBUSDIVIDED_INT32(const quint16 &highword, const quint16 &lowword) : highword(highword), lowword(lowword) {}
};



struct MODBUSDIVIDED_UINT32
{
    quint16 highword;
    quint16 lowword;
    MODBUSDIVIDED_UINT32() : highword(0), lowword(0) {}
    MODBUSDIVIDED_UINT32(const quint16 &highword, const quint16 &lowword) : highword(highword), lowword(lowword) {}
};

class ModbusElectricityMeterHelper
{

public:

    static QList<quint8> getAcceptableEMeterNis();


    static QHash<quint8,QString> getMapDevAddr2ni();

    static QHash<quint8,QString> getMapDevAddr2niExt(QString &serilaPortName);

    static QString getSerialPortName();

    static QStringList getDevNIList();//it is a temporary solution, because I don't want to change protcol


    static MODBUSDIVIDED_INT32 getDividedInt32(const qint32 &value);
    static MODBUSDIVIDED_UINT32 getDividedUInt32(const quint32 &value);

    static void addDividedInt322thelist(ModbusAnswerList &list, const qint32 &value);
    static void addDividedUInt322thelist(ModbusAnswerList &list, const quint32 &value);


    static ModbusAnswerList getVoltageAnswer(const QVariantHash &hdata, const bool &verboseMode);
   static ModbusAnswerList getTotalEnergyAnswer(const QVariantHash &hdata, const bool &verboseMode);



//   static QList<quint32> convertEnergy2answerFormat(QList<qreal>);

   static QList<quint32> getEnergyValues(const QVariantHash &hdata, QList<qreal> &realv, QStringList &tablekeys);


   static OnePhaseRealData getOnePhaseData(const QVariantHash &hdata, const QString &phasename);

   static OnePhaseRealData getSummPhaseData(const OnePhaseRealData &phasea, const OnePhaseRealData &phaseb, const OnePhaseRealData &phasec);


   static OnePhaseModbusAnswerFormat convert2answerFormat(const OnePhaseRealData &indata, const bool &useHighPrec4power);


};

#endif // MODBUSELECTRICITYMETERHELPER_H
