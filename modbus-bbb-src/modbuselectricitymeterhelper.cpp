#include "modbuselectricitymeterhelper.h"

#include <QtMath>


//-------------------------------------------------------------------------------------------

MODBUSDIVIDED_INT32 ModbusElectricityMeterHelper::getDividedInt32(const qint32 &value)
{
    MODBUSDIVIDED_INT32 r;
    r.lowword = value & 0xFFFF;
    r.highword = (value >> 16);
    return r;
}

//-------------------------------------------------------------------------------------------

MODBUSDIVIDED_UINT32 ModbusElectricityMeterHelper::getDividedUInt32(const quint32 &value)
{
    MODBUSDIVIDED_UINT32 r;
    r.lowword = value & 0xFFFF;
    r.highword = (value >> 16);
    return r;
}

//-------------------------------------------------------------------------------------------

void ModbusElectricityMeterHelper::addDividedInt322thelist(ModbusAnswerList &list, const qint32 &value)
{
    const MODBUSDIVIDED_INT32 r = getDividedInt32(value);
    list.append(r.lowword);
    list.append(r.highword);

}

//-------------------------------------------------------------------------------------------

void ModbusElectricityMeterHelper::addDividedUInt322thelist(ModbusAnswerList &list, const quint32 &value)
{
    const MODBUSDIVIDED_UINT32 r = getDividedUInt32(value);
    list.append(r.lowword);
    list.append(r.highword);
}

//-------------------------------------------------------------------------------------------

ModbusAnswerList ModbusElectricityMeterHelper::getVoltageAnswer(const QVariantHash &hdata)
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


    const OnePhaseModbusAnswerFormat phaseA = convert2answerFormat(phasea);
    const OnePhaseModbusAnswerFormat phaseB = convert2answerFormat(phaseb);
    const OnePhaseModbusAnswerFormat phaseC = convert2answerFormat(phasec);

    const OnePhaseModbusAnswerFormat phaseSumm = convert2answerFormat(phasesumm);


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

    addDividedInt322thelist(l, phaseSumm.activepower);//40011-12
    addDividedInt322thelist(l, phaseSumm.reactivepower);//40013-14
    addDividedInt322thelist(l, phaseSumm.apparentpower);//40015-16

    //phases pf
    l.append(phaseA.powerfactor);//40017
    l.append(phaseB.powerfactor);//40018
    l.append(phaseC.powerfactor);//40019


    //int32 phases Apparent
    addDividedInt322thelist(l, phaseA.apparentpower);//40020-21
    addDividedInt322thelist(l, phaseB.apparentpower);//40022-23
    addDividedInt322thelist(l, phaseC.apparentpower);//40024-25

    //int32 phases Active
    addDividedInt322thelist(l, phaseA.activepower);//40026-27
    addDividedInt322thelist(l, phaseB.activepower);//40028-29
    addDividedInt322thelist(l, phaseC.activepower);//40030-31

    //int32 phases Reactive
    addDividedInt322thelist(l, phaseA.reactivepower);//40032-33
    addDividedInt322thelist(l, phaseB.reactivepower);//40034-35
    addDividedInt322thelist(l, phaseC.reactivepower);//40036-37


    return l;

}

//-------------------------------------------------------------------------------------------

ModbusAnswerList ModbusElectricityMeterHelper::getTotalEnergyAnswer(const QVariantHash &hdata)
{
//    "T0_A+": "854.136",   40041-42
//    "T0_A-": "!",         40043-44
//    "T0_R+": "11.22",     40045-46
//    "T0_R-": "22.33",     40047-48
//    "T1_A+": "835.618",   40049-50
//    "T1_A-": "!",         40051-52
//    "T1_R+": "33.44",     40053-54
//    "T1_R-": "44.55",     40055-56
//    "T2_A+": "18.518",    40057-58
//    "T2_A-": "!",         40059-60
//    "T2_R+": "55.66",     40061-62
//    "T2_R-": "66.77",     40063-64
//    "T3_A+": "?",         40065-66
//    "T3_A-": "77.88",     40067-68
//    "T3_R+": "88.99",     40069-70
//    "T3_R-": "?",         40071-72
//    "T4_A+": "?",         40073-74
//    "T4_A-": "!",         40075-76
//    "T4_R+": "?",         40077-78
//    "T4_R-": "?"          40079-80

    const QList<quint32> enrgs = getEnergyValues(hdata);

    ModbusAnswerList l;
    for(int i = 0, imax = enrgs.size(); i < imax; i++){
        addDividedUInt322thelist(l, enrgs.at(i));
    }
    return l;
}

//-------------------------------------------------------------------------------------------

QList<quint32> ModbusElectricityMeterHelper::getEnergyValues(const QVariantHash &hdata)
{
    QList<quint32> l;

    const QStringList listenrgs = QString("A+ A- R+ R-").split(" ", QString::SkipEmptyParts);
    for(int i = 0, jmax = listenrgs.size(); i < 5; i++){
        for(int j = 0; j < jmax; j++){
            const qreal v = hdata.value(QString("T%1_%2").arg(i).arg(listenrgs.at(j))).toDouble();

            const quint32 value = quint32(qAbs(v)/0.1);
            l.append(value);
        }

    }
    return l;
}

//-------------------------------------------------------------------------------------------

OnePhaseRealData ModbusElectricityMeterHelper::getOnePhaseData(const QVariantHash &hdata, const QString &phasename)
{
    OnePhaseRealData phased;

    bool okActivePower, okPowerFactor, okReactivePower;

    phased.voltage      = hdata.value(QString("U%1").arg(phasename)).toDouble();//V
    phased.current      = hdata.value(QString("I%1").arg(phasename)).toDouble();//A
    phased.activepower  = hdata.value(QString("P%1").arg(phasename)).toDouble(&okActivePower) * 1000; //kW to W
    phased.reactivepower = hdata.value(QString("Q%1").arg(phasename)).toDouble(&okReactivePower) * 1000;//kvar to var
    phased.powerfactor  = hdata.value(QString("cos_f%1").arg(phasename)).toDouble(&okPowerFactor); //cos f


    //  a calculated value (Apparent Power A = |(Active Power A / cos φ A)|), where Active Power A and cos φ A are values from the meter;
    //calculated value
    phased.apparentpower = 0;
    if(okActivePower){
        if(okReactivePower){
            //S = sqrt(P^2 + Q^2)
            const qreal pqpow = qPow(phased.activepower,2.0) + qPow(phased.reactivepower, 2.0);
            phased.apparentpower = qSqrt(pqpow);
        }else{
            if(okPowerFactor && phased.powerfactor != 0.0){//it has some load
                //S = |(P/pf)|
                //Q = |S| * sin(f)
                phased.apparentpower = qAbs(phased.activepower/phased.powerfactor);
                phased.reactivepower = phased.activepower * qSqrt((1 - qPow(phased.powerfactor, 2.0)));

            }
        }
    }

    return phased;
}

//-------------------------------------------------------------------------------------------

OnePhaseRealData ModbusElectricityMeterHelper::getSummPhaseData(const OnePhaseRealData &phasea, const OnePhaseRealData &phaseb, const OnePhaseRealData &phasec)
{
    OnePhaseRealData phasesumm;
    phasesumm.activepower = phasea.activepower + phaseb.activepower + phasec.activepower;
    phasesumm.reactivepower = phasea.reactivepower + phaseb.reactivepower + phasec.reactivepower;
    phasesumm.apparentpower = phasea.apparentpower + phaseb.apparentpower + phasec.apparentpower;

    phasesumm.voltage = 0.0;
    phasesumm.current = 0.0;
    phasesumm.powerfactor = 0.0;
    return phasesumm;

}

//-------------------------------------------------------------------------------------------

OnePhaseModbusAnswerFormat ModbusElectricityMeterHelper::convert2answerFormat(const OnePhaseRealData &indata)
{
//    quint16 voltage;//0.01 V
//    quint32 current;//0.001 A
//    qint32 activepower;//0.1 W
//    qint32 reactivepower;//0.1 var
//    qint32 apparentpower;// 0.1 VA
//    qint16 powerfactor;//0.001

    OnePhaseModbusAnswerFormat phased;
    phased.voltage      = quint16(indata.voltage/0.01);
    phased.current      = qint32(indata.current/0.001);
    phased.activepower  = qint32(indata.activepower/0.1);
    phased.reactivepower = qint32(indata.reactivepower/0.1);
    phased.apparentpower = qint32(indata.apparentpower/0.1);
    phased.powerfactor  = qint16(indata.powerfactor/0.001);

    return phased;
}

//-------------------------------------------------------------------------------------------
