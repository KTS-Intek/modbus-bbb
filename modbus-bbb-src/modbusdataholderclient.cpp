#include "modbusdataholderclient.h"
#include <QtCore>

///[!] MatildaIO
#include "matilda-bbb-src/shared/pathsresolver.h"


#include "dataholderlocalservercommands.h"



//----------------------------------------------------------------------------------------------

ModbusDataHolderClient::ModbusDataHolderClient(const QString &namesuffix, const bool &verboseMode, QObject *parent) :
   RegularLocalSocket(verboseMode, parent)
{
    mtdExtNameStr = qAppName() + namesuffix;
    reconnectWasForced = false;
}

//----------------------------------------------------------------------------------------------

QVariant ModbusDataHolderClient::getExtName()
{
    return QVariant(mtdExtNameStr);

}

//----------------------------------------------------------------------------------------------

QString ModbusDataHolderClient::getPath2server()
{
    return PathsResolver::localServerNameModbusBBB();

}

//----------------------------------------------------------------------------------------------

void ModbusDataHolderClient::decodeReadData(const QVariant &dataVar, const quint16 &command)
{
    switch(command){

//    case DATAHOLDER_ADD_POLLDATA: {
//        onDATAHOLDER_ADD_POLLDATA(dataVar.toHash());
//        break;}
      case DATAHOLDER_GET_POLLDATA: {
          onDATAHOLDER_GET_POLLDATA(dataVar.toHash());
          break;}
    }
}

//----------------------------------------------------------------------------------------------

void ModbusDataHolderClient::onThreadStartedL0()
{
    connect(this, &ModbusDataHolderClient::connected, [=]{
        reconnectWasForced = false;
    });
    onThreadStarted();
}

//----------------------------------------------------------------------------------------------

void ModbusDataHolderClient::checkYourConnection()
{
    if(state() != QLocalSocket::ConnectedState){
        if(reconnectWasForced)
            return;
        reconnectWasForced = true;
        emit startReconnTmr();
    }
}

//----------------------------------------------------------------------------------------------

void ModbusDataHolderClient::sendCommand2dataHolder(quint16 pollCode, QString ni, QString messagetag, QString objecttag)
{
    if(state() != QLocalSocket::ConnectedState){
        emit onCommandReceived(messagetag, objecttag, false, "there is no connection");
        return;
    }

    QVariantHash h;

    h.insert("messagetag", messagetag);
    h.insert("objecttag", objecttag);
    //main
    h.insert("pollCode", pollCode);
    h.insert("NI", ni);

    const qint64 r = mWrite2extensionF(h, DATAHOLDER_GET_POLLDATA);
    if(r > 0){
        emit onCommandReceived(messagetag, objecttag, true, QString::number(r));
        return;
    }
    emit onCommandReceived(messagetag, objecttag, false, QString("Write error, %1, %2").arg(r).arg(errorString()));

}

//----------------------------------------------------------------------------------------------

void ModbusDataHolderClient::onDATAHOLDER_GET_POLLDATA(const QVariantHash &hash)
{
    const QString messagetag = hash.value("messagetag").toString();
    const QString objecttag = hash.value("objecttag").toString();

    if(hash.contains("varlist")){
        const QVariantList l = hash.value("varlist").toList();
        if(!l.isEmpty()){
            emit dataFromCache(messagetag, objecttag, l.last().toHash());
            return;
        }
        emit onCommandReceived(messagetag, objecttag, false, QString("Empty object is received, 'varlist'"));
        return;

    }
    emit onCommandReceived(messagetag, objecttag, false, QString("Error, %1, %2")
                           .arg(hash.value("result").toInt())
                           .arg(hash.value("message").toString()));


}

//----------------------------------------------------------------------------------------------
