#ifndef MODBUSELECTRICITYMETERHELPER_H
#define MODBUSELECTRICITYMETERHELPER_H


#include "modbusmeterhelper.h"
#include "modbusencoderdecodertypes.h"


class ModbusElectricityMeterHelper : public ModbusMeterHelper
{

public:

    static ModbusVirtualDevices getAcceptableEMeterNis();





    static ModbusAnswerList getVoltageAnswer(const QVariantHash &hdata, const bool &verboseMode);
   static ModbusAnswerList getTotalEnergyAnswer(const QVariantHash &hdata, const bool &verboseMode);



//   static QList<quint32> convertEnergy2answerFormat(QList<qreal>);

   static QList<quint32> getEnergyValues(const QVariantHash &hdata, QList<qreal> &realv, QStringList &tablekeys);


   static OnePhaseRealData getOnePhaseData(const QVariantHash &hdata, const QString &phasename);

   static OnePhaseRealData getSummPhaseData(const OnePhaseRealData &phasea, const OnePhaseRealData &phaseb, const OnePhaseRealData &phasec);


   static OnePhaseModbusAnswerFormat convert2answerFormat(const OnePhaseRealData &indata, const bool &useHighPrec4power);


};

#endif // MODBUSELECTRICITYMETERHELPER_H
