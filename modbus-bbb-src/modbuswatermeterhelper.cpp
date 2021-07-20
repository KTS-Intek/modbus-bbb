#include "modbuswatermeterhelper.h"


///[!] MatildaIO
#include "matilda-bbb-src/shared/pathsresolver.h"
#include "src/shared/compressfilehelper.h"


//-------------------------------------------------------------------------------------------

int ModbusWaterMeterHelper::getAcceptableWMeterNis(ModbusVirtualDevices &vdevs)
{

    QString errstr;
    const QVariantList l = CompressFileHelper::readCompressedVarListFromTheFile(PathsResolver::path2waterMetersList(), errstr);

    return getAcceptableMeterNis(l, vdevs);


}

//-------------------------------------------------------------------------------------------

ModbusAnswerList ModbusWaterMeterHelper::getTotalWaterAnswer(const QVariantHash &hdata, const bool &verboseMode)
{
    //T0W - 41201-202
    //T1W - 41203-204
    //T2W - 41205-206
    //T3W - 41207-208
    //T4W - 41209-210
    //CW  - 41211-212


    QList<qreal> energytable;
    QStringList tablenames;

    const QList<quint32> enrgs = getWaterValues(hdata, energytable, tablenames);


    ModbusAnswerList l;
    for(int i = 0, imax = enrgs.size(); i < imax; i++){
        addDividedUInt322thelist(l, enrgs.at(i));
    }

    if(verboseMode){
        qDebug()  << "Water table  " << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz") ;
        qDebug()  << " ----------------------------- ";
        for(int i = 0, imax = energytable.size(); i < imax; i++){
            qDebug()  << tablenames.at(i) <<  QString::number(energytable.at(i), 'f', 3);
        }
        qDebug()  << "----------------------------- ";
        qDebug()  << "Water table ----------------------------- ";

    }

    return l;

}

//-------------------------------------------------------------------------------------------

QList<quint32> ModbusWaterMeterHelper::getWaterValues(const QVariantHash &hdata, QList<qreal> &realv, QStringList &tablekeys)
{
    return getMeterValuesUIN32(QString("T0W T1W T2W T3W T4W CW").split(" ", QString::SkipEmptyParts), 0.1, hdata, realv, tablekeys);

}

//-------------------------------------------------------------------------------------------
