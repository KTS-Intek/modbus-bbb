#ifndef MODBUSELECTRICITYMETERHELPER_H
#define MODBUSELECTRICITYMETERHELPER_H


#include "modbusmeterhelper.h"


class ModbusElectricityMeterHelper : public ModbusMeterHelper
{

public:

    static QList<quint8> getAcceptableEMeterNis();


    static QHash<quint8,QString> getMapDevAddr2ni();

    static QHash<quint8,QString> getMapDevAddr2niExt(QString &serilaPortName, bool &isParityNone);

    static QString getSerialPortName(bool &isParityNone);

    static QStringList getDevNIList();//it is a temporary solution, because I don't want to change protcol





    static ModbusAnswerList getVoltageAnswer(const QVariantHash &hdata, const bool &verboseMode);
   static ModbusAnswerList getTotalEnergyAnswer(const QVariantHash &hdata, const bool &verboseMode);



//   static QList<quint32> convertEnergy2answerFormat(QList<qreal>);

   static QList<quint32> getEnergyValues(const QVariantHash &hdata, QList<qreal> &realv, QStringList &tablekeys);


   static OnePhaseRealData getOnePhaseData(const QVariantHash &hdata, const QString &phasename);

   static OnePhaseRealData getSummPhaseData(const OnePhaseRealData &phasea, const OnePhaseRealData &phaseb, const OnePhaseRealData &phasec);


   static OnePhaseModbusAnswerFormat convert2answerFormat(const OnePhaseRealData &indata, const bool &useHighPrec4power);


};

#endif // MODBUSELECTRICITYMETERHELPER_H
