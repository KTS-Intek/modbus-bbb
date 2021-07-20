#include "modbusgasmeterhelper.h"




//-------------------------------------------------------------------------------------------

QList<quint8> ModbusGasMeterHelper::getAcceptableGMeterNis()
{
    return QList<quint8>();//there is no water
}

//-------------------------------------------------------------------------------------------

ModbusAnswerList ModbusGasMeterHelper::getTotalGasAnswer(const QVariantHash &hdata, const bool &verboseMode)
{
    //tvlu - 42201-202
    QList<qreal> energytable;
    QStringList tablenames;

    const QList<quint32> enrgs = getGasValues(hdata, energytable, tablenames);


    ModbusAnswerList l;
    for(int i = 0, imax = enrgs.size(); i < imax; i++){
        addDividedUInt322thelist(l, enrgs.at(i));
    }

    if(verboseMode){
        qDebug()  << "Gas table  " << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
        qDebug()  << " ----------------------------- ";
        for(int i = 0, imax = energytable.size(); i < imax; i++){
            qDebug()  << tablenames.at(i) <<  QString::number(energytable.at(i), 'f', 3);
        }
        qDebug()  << "----------------------------- ";
        qDebug()  << "Gas table ----------------------------- ";

    }

    return l;
}

//-------------------------------------------------------------------------------------------

QList<quint32> ModbusGasMeterHelper::getGasValues(const QVariantHash &hdata, QList<qreal> &realv, QStringList &tablekeys)
{
    return getMeterValuesUIN32(QString("tvlu").split(" ", QString::SkipEmptyParts), 0.001, hdata, realv, tablekeys);


}

//-------------------------------------------------------------------------------------------
