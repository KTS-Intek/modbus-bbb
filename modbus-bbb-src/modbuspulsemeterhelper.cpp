#include "modbuspulsemeterhelper.h"



///[!] MatildaIO
#include "matilda-bbb-src/shared/pathsresolver.h"
#include "src/shared/compressfilehelper.h"


//-------------------------------------------------------------------------------------------

int ModbusPulseMeterHelper::getAcceptablePMeterNis(ModbusVirtualDevices &vdevs)
{

    QString errstr;
    const QVariantList l = CompressFileHelper::readCompressedVarListFromTheFile(PathsResolver::path2pulseMetersList(), errstr);

    return getAcceptableMeterNis(l, vdevs);


}

//-------------------------------------------------------------------------------------------

ModbusAnswerList ModbusPulseMeterHelper::getTotalPulseAnswer(const QVariantHash &hdata, const bool &verboseMode)
{
    //tvlu_0 - 42201-202
    //tvlu_1 - 42203-204
    //tvlu_2 - 42205-206
    //tvlu_3 - 42207-208

    QList<qreal> energytable;
    QStringList tablenames;

    const QList<quint32> enrgs = getPulseValues(hdata, energytable, tablenames);


    ModbusAnswerList l;
    for(int i = 0, imax = enrgs.size(); i < imax; i++){
        addDividedUInt322thelist(l, enrgs.at(i));
    }

    if(verboseMode){
        qDebug()  << "Pulse table  " << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
        qDebug()  << " ----------------------------- ";
        for(int i = 0, imax = energytable.size(); i < imax; i++){
            qDebug()  << tablenames.at(i) <<  QString::number(energytable.at(i), 'f', 3);
        }
        qDebug()  << "----------------------------- ";
        qDebug()  << "Pulse table ----------------------------- ";

    }

    return l;
}

//-------------------------------------------------------------------------------------------

QList<quint32> ModbusPulseMeterHelper::getPulseValues(const QVariantHash &hdata, QList<qreal> &realv, QStringList &tablekeys)
{
    QStringList lkeys;
    for(int i = 0; i < 4; i++)
        lkeys.append(QString("tvlu_%1").arg(i));

    return getMeterValuesUIN32(lkeys, 1.0, hdata, realv, tablekeys);

}

//-------------------------------------------------------------------------------------------
