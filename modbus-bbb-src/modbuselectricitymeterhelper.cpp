#include "modbuselectricitymeterhelper.h"

#include <QtMath>
#include <QDebug>
#include <QDateTime>




///[!] MatildaIO
#include "matilda-bbb-src/shared/pathsresolver.h"

#include <QFile>
#include <QDataStream>


#include "matildalimits.h"

//-------------------------------------------------------------------------------------------

ModbusVirtualDevices ModbusElectricityMeterHelper::getAcceptableEMeterNis()
{
    ModbusVirtualDevices devadds;


    QFile file(PathsResolver::path2electricityMetersList());
    if(file.open(QFile::ReadOnly)){

        QDataStream stream(&file);
        stream.setVersion(QDataStream::Qt_5_6);

        for(quint32 i = 0; !stream.atEnd() && i < MAX_METER_COUNT; i++){
            QVariantHash hash;
            stream >> hash;

            OneModbusVirtualDevice onedev;
            const quint8 add = meterAddressFromHash(hash, onedev);
            if(add > 0)
                devadds.insert(add, onedev);

        }
        file.close();
    }
    return devadds;
}



//-------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------

ModbusAnswerList ModbusElectricityMeterHelper::getVoltageAnswer(const QVariantHash &hdata, const bool &verboseMode)
{

    //40001-40037

//    const QVariantHash hdata = h.value("data").toHash();
//#define MYDECODER_READF_FIRST_REGISTER  40001
//#define MYDECODER_READF_LAST_REGISTER   40068

//#define MYDECODER_VOLTAGE_REGISTER_FIRST        MYDECODER_READF_FIRST_REGISTER
//#define MYDECODER_TOTALENERGY_REGISTER_FIRST    MYDECODER_READF_FIRST_REGISTER + 28 //40029
//    "data": {
//               "F": "50",
//               "IA": "4.94",
//               "IB": "5.94",
//               "IC": "6.94",
//               "PA": "1.0597",
//               "PB": "2.0597",
//               "PC": "3.0597",
//               "QA": "!",
//               "QB": "!",
//               "QC": "!",
//               "UA": "214.52",
//               "UB": "215.52",
//               "UC": "216.52",
//               "cos_fA": "1",
//               "cos_fB": "0.9",
//               "cos_fC": "0.8"
//           },

    const qreal freq = hdata.value("F").toDouble();
    const quint16 freqHz =  quint16(freq/0.01);
//            phased.voltage      = quint16(indata.voltage/0.01);

    const OnePhaseRealData phasea = getOnePhaseData(hdata, "A");
    const OnePhaseRealData phaseb = getOnePhaseData(hdata, "B");
    const OnePhaseRealData phasec = getOnePhaseData(hdata, "C");

    const OnePhaseRealData phasesumm = getSummPhaseData(phasea, phaseb, phasec);


    const OnePhaseModbusAnswerFormat phaseA = convert2answerFormat(phasea, true);
    const OnePhaseModbusAnswerFormat phaseB = convert2answerFormat(phaseb, true);
    const OnePhaseModbusAnswerFormat phaseC = convert2answerFormat(phasec, true);

    const OnePhaseModbusAnswerFormat phaseSumm = convert2answerFormat(phasesumm, true);


    ModbusAnswerList l;
    //uint16
    l.append(phaseA.voltage);//40001
    l.append(phaseB.voltage);//40002
    l.append(phaseC.voltage);//40003

    //int32
    addDividedInt322thelist(l, phaseA.current);//40004-5
    addDividedInt322thelist(l, phaseB.current);//40006-7
    addDividedInt322thelist(l, phaseC.current);//40008-9

    //frequency uint16
    l.append(freqHz);//40010

    l.append(0xFFFF);//40011 nothing

    //phases pf
    l.append(phaseSumm.powerfactor);//40012

    l.append(phaseA.powerfactor);//40013
    l.append(phaseB.powerfactor);//40014
    l.append(phaseC.powerfactor);//40015


    addDividedInt322thelist(l, phaseSumm.activepower);//40016-17
    addDividedInt322thelist(l, phaseSumm.reactivepower);//40018-19
    addDividedInt322thelist(l, phaseSumm.apparentpower);//40020-21



    //int32 phases Apparent
    addDividedInt322thelist(l, phaseA.apparentpower);//40022-23
    addDividedInt322thelist(l, phaseB.apparentpower);//40024-25
    addDividedInt322thelist(l, phaseC.apparentpower);//40026-27

    //int32 phases Active
    addDividedInt322thelist(l, phaseA.activepower);//40028-29
    addDividedInt322thelist(l, phaseB.activepower);//40030-31
    addDividedInt322thelist(l, phaseC.activepower);//40032-33

    //int32 phases Reactive
    addDividedInt322thelist(l, phaseA.reactivepower);//40034-35
    addDividedInt322thelist(l, phaseB.reactivepower);//40036-37
    addDividedInt322thelist(l, phaseC.reactivepower);//40038-39


    QList<qreal> voltagetable;
    QStringList tablenames;

   voltagetable.append(phasesumm.activepower);
   tablenames.append("Active");

   voltagetable.append(phasesumm.reactivepower);
   tablenames.append("Reactive");

   voltagetable.append(phasesumm.apparentpower);
   tablenames.append("Apparent");


   voltagetable.append(phasea.apparentpower);
   tablenames.append("Apparent A");

   voltagetable.append(phaseb.apparentpower);
   tablenames.append("Apparent B");

   voltagetable.append(phasec.apparentpower);
   tablenames.append("Apparent C");


   voltagetable.append(phasesumm.powerfactor);
   tablenames.append("Power factor");


   voltagetable.append(phasea.current);
   tablenames.append("Current A");

   voltagetable.append(phaseb.current);
   tablenames.append("Current B");

   voltagetable.append(phasec.current);
   tablenames.append("Current C");

   if(verboseMode){
       qDebug()  << "Voltage table  " << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
       qDebug()  << " ----------------------------- ";
       for(int i = 0, imax = voltagetable.size(); i < imax; i++){
           qDebug()  << tablenames.at(i) <<  QString::number(voltagetable.at(i), 'f', 3);
       }
       qDebug()  << "----------------------------- ";
       qDebug()  << "Voltage table ----------------------------- ";

   }
   return l;

}

//-------------------------------------------------------------------------------------------

ModbusAnswerList ModbusElectricityMeterHelper::getTotalEnergyAnswer(const QVariantHash &hdata, const bool &verboseMode)
{
//    "T0_A+": "854.136",   40201-202
//    "T0_A-": "!",         40203-204
//    "T0_R+": "11.22",     40205-206
//    "T0_R-": "22.33",     40207-208
//    "T1_A+": "835.618",   40209-210
//    "T1_A-": "!",         40211-212
//    "T1_R+": "33.44",     40213-214
//    "T1_R-": "44.55",     40215-216
//    "T2_A+": "18.518",    40217-218
//    "T2_A-": "!",         40219-220
//    "T2_R+": "55.66",     40221-222
//    "T2_R-": "66.77",     40223-224
//    "T3_A+": "?",         40225-226
//    "T3_A-": "77.88",     40227-228
//    "T3_R+": "88.99",     40229-230
//    "T3_R-": "?",         40231-232
//    "T4_A+": "?",         40233-234
//    "T4_A-": "!",         40235-236
//    "T4_R+": "?",         40237-238
//    "T4_R-": "?"          40239-240

    QList<qreal> energytable;
    QStringList tablenames;

    const QList<quint32> enrgs = getEnergyValues(hdata, energytable, tablenames);

    ModbusAnswerList l;
    for(int i = 0, imax = enrgs.size(); i < imax; i++){
        addDividedUInt322thelist(l, enrgs.at(i));
    }

    if(verboseMode){
        qDebug()  << "Energy table  " << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
        qDebug()  << " ----------------------------- ";
        for(int i = 0, imax = energytable.size(); i < imax; i++){
            qDebug()  << tablenames.at(i) <<  QString::number(energytable.at(i), 'f', 3);
        }
        qDebug()  << "----------------------------- ";
        qDebug()  << "Energy table ----------------------------- ";

    }

    return l;
}

//-------------------------------------------------------------------------------------------

QList<quint32> ModbusElectricityMeterHelper::getEnergyValues(const QVariantHash &hdata, QList<qreal> &realv, QStringList &tablekeys)
{
    QList<quint32> l;

//    const quint16 baduint16 = 0xFFFF;
    const quint32 baduint32 = 0xFFFFFFFF;

//    const qint16 badint16 = 0x8000;
//    const qint32 badint32 = 0x80000000;

    const QStringList listenrgs = QString("A+ A- R+ R-").split(" ", QString::SkipEmptyParts);
    for(int i = 0, jmax = listenrgs.size(); i < 5; i++){
        for(int j = 0; j < jmax; j++){
            bool okv;
            const QString key = QString("T%1_%2").arg(i).arg(listenrgs.at(j));

            const qreal v = hdata.value(key).toDouble(&okv);

            tablekeys.append(key);
            realv.append(v);
            const quint32 value = okv ? quint32(qAbs(v)/0.1) : baduint32;
            l.append(value);
        }

    }
    return l;
}

//-------------------------------------------------------------------------------------------

OnePhaseRealData ModbusElectricityMeterHelper::getOnePhaseData(const QVariantHash &hdata, const QString &phasename)
{
    OnePhaseRealData phased;

    phased.voltage      = hdata.value(QString("U%1").arg(phasename)).toDouble(&phased.okVoltage);//V

    phased.current      = hdata.value(QString("I%1").arg(phasename)).toDouble(&phased.okCurrent);//A

    phased.activepower  = hdata.value(QString("P%1").arg(phasename)).toDouble(&phased.okActivePower) ; //kW

    phased.reactivepower = hdata.value(QString("Q%1").arg(phasename)).toDouble(&phased.okReactivePower) ;//kvar
    phased.powerfactor  = hdata.value(QString("cos_f%1").arg(phasename)).toDouble(&phased.okPowerFactor); //cos f


    //  a calculated value (Apparent Power A = |(Active Power A / cos φ A)|), where Active Power A and cos φ A are values from the meter;
    //calculated value
    phased.apparentpower = 0; //kVA;
    phased.okApparentPower = false;

    if(phased.okActivePower){
        if(phased.okReactivePower){
            //S = sqrt(P^2 + Q^2)
            const qreal pqpow = qPow(phased.activepower,2.0) + qPow(phased.reactivepower, 2.0);
            phased.apparentpower = qSqrt(pqpow);
            phased.okApparentPower = true;
        }else{
            if(phased.okPowerFactor && phased.powerfactor != 0.0){//it has some load
                //S = |(P/pf)|
                //Q = |S| * sin(f)
                phased.apparentpower = qAbs(phased.activepower/phased.powerfactor);
                phased.reactivepower = phased.activepower * qSqrt((1 - qPow(phased.powerfactor, 2.0)));
                phased.okReactivePower = phased.okApparentPower = true;
            }
        }
    }
    return phased;
}

//-------------------------------------------------------------------------------------------

OnePhaseRealData ModbusElectricityMeterHelper::getSummPhaseData(const OnePhaseRealData &phasea, const OnePhaseRealData &phaseb, const OnePhaseRealData &phasec)
{
    OnePhaseRealData phasesumm;

    phasesumm.okActivePower = (phasea.okActivePower || phaseb.okActivePower || phasec.okActivePower);
    phasesumm.activepower = phasea.activepower + phaseb.activepower + phasec.activepower;



    phasesumm.okReactivePower = (phasea.okReactivePower || phaseb.okReactivePower || phasec.okReactivePower);
    phasesumm.reactivepower = phasea.reactivepower + phaseb.reactivepower + phasec.reactivepower;

    phasesumm.okApparentPower = (phasea.okApparentPower || phaseb.okApparentPower || phasec.okApparentPower);
    phasesumm.apparentpower = phasea.apparentpower + phaseb.apparentpower + phasec.apparentpower;

    qreal minv = 0.0;
    phasesumm.powerfactor = 0.0;
    if(phasea.okPowerFactor && phasea.powerfactor != 0.0){
        minv++;
        phasesumm.powerfactor += phasea.powerfactor;
    }
    if(phaseb.okPowerFactor && phaseb.powerfactor != 0.0){
        minv++;
        phasesumm.powerfactor += phaseb.powerfactor;
    }
    if(phasec.okPowerFactor && phasec.powerfactor != 0.0){
        minv++;
        phasesumm.powerfactor += phasec.powerfactor;
    }

    phasesumm.okPowerFactor = (minv > 0);

    if(phasesumm.okPowerFactor)
        phasesumm.powerfactor = phasesumm.powerfactor / minv;

    phasesumm.voltage = 0.0;    
    phasesumm.current = 0.0;

    phasesumm.okVoltage = phasesumm.okCurrent = false;




       return phasesumm;

}

//-------------------------------------------------------------------------------------------

OnePhaseModbusAnswerFormat ModbusElectricityMeterHelper::convert2answerFormat(const OnePhaseRealData &indata, const bool &useHighPrec4power)
{
//    quint16 voltage;//0.01 V
//    quint32 current;//0.001 A
//    qint32 activepower;//0.01 kW
//    qint32 reactivepower;//0.01 kvar
//    qint32 apparentpower;// 0.01 kVA
    //    qint16 powerfactor;//0.001

    const quint16 baduint16 = 0xFFFF;
//    const quint32 baduint32 = 0xFFFFFFFF;

    const qint16 badint16 = 0x8000;
    const qint32 badint32 = 0x80000000;

    const  qreal divisor = useHighPrec4power ? 0.01 : 0.1;

    OnePhaseModbusAnswerFormat phased;

    phased.voltage      = indata.okVoltage ? quint16(indata.voltage/0.01) : baduint16;

    phased.current      = indata.okCurrent ? qint32(indata.current/0.001) : badint32;

    phased.activepower  = indata.okActivePower ? qint32(indata.activepower/divisor) : badint32;

    phased.reactivepower = indata.okReactivePower ? qint32(indata.reactivepower/divisor) : badint32;

    phased.apparentpower = indata.okApparentPower ? qint32(indata.apparentpower/divisor) : badint32;

    phased.powerfactor  = indata.okPowerFactor ? qint16(indata.powerfactor/0.001) : badint16;

    return phased;
}

//-------------------------------------------------------------------------------------------
