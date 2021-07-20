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
      case DATAHOLDER_GET_POLLDATA_EXT: {
          onDATAHOLDER_GET_POLLDATA_EXT(dataVar.toHash());
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

void ModbusDataHolderClient::sendCommand2dataHolder(quint16 pollCode, QString devID, bool useSn4devID, QString messagetag, QString objecttag)
{
    if(state() != QLocalSocket::ConnectedState){
        emit onCommandReceived(messagetag, objecttag, false, "there is no connection");
        return;
    }

//    const QVariant messagetag = hash.value("messagetag");
//    const QVariant objecttag = hash.value("objecttag");


//    bool hasPollCode;
//    const quint16 pollCode = hash.value("pollCode").toUInt(&hasPollCode);


//    const QStringList devIDs = hash.value("devIDs").toStringList();
//    const bool useSn4devID = hash.value("useSn4devID", false).toBool();


    QVariantHash h;

    h.insert("messagetag", messagetag);
    h.insert("objecttag", objecttag);
    //main
    h.insert("pollCode", pollCode);
    h.insert("devIDs", devID.split("\r\n\r"));// devIDs);
    h.insert("useSn4devID", useSn4devID);

    const qint64 r = mWrite2extensionF(h, DATAHOLDER_GET_POLLDATA_EXT);
    if(r > 0){
        emit onCommandReceived(messagetag, objecttag, true, QString::number(r));
        return;
    }
    emit onCommandReceived(messagetag, objecttag, false, QString("Write error, %1, %2").arg(r).arg(errorString()));

}

//----------------------------------------------------------------------------------------------

void ModbusDataHolderClient::onDATAHOLDER_GET_POLLDATA_EXT(const QVariantHash &hash)
{
    const QString messagetag = hash.value("messagetag").toString();
    const QString objecttag = hash.value("objecttag").toString();

    if(hash.contains("varlist")){
        const QVariantList l = hash.value("varlist").toList();
        //varlist pulse meters have different values in few rows, so send varlist instead of the last varlist value
        //varlist can be empty if there is no data or request was wrong, so send
        if(!l.isEmpty()){

            if(verboseMode){
                for(int i = 0, imax = l.size(); i < imax; i++)
                    qDebug() << "onDATAHOLDER_GET_POLLDATA_EXT " << i << l.at(i).toHash();
            }

            emit dataFromCache(messagetag, objecttag, l);//It contains NI and SN (devID or additionalID)
            return;
        }
//        QVariantHash h;
//        QVariantList ll;
//        ll.append(h);
        emit dataFromCache(messagetag, objecttag, l);
//        emit onCommandReceived(messagetag, objecttag, false, QString("Empty object is received, 'varlist'"));
        return;

    }
    emit onCommandReceived(messagetag, objecttag, false, QString("Error, %1, %2")
                           .arg(hash.value("result").toInt())
                           .arg(hash.value("message").toString()));


}

//----------------------------------------------------------------------------------------------
