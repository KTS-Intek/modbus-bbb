#include "modbusmatildalsclient.h"


#include "moji_defy.h"
#include "dbgaboutsourcetype.h"


ModbusMatildaLSClient::ModbusMatildaLSClient(bool verboseMode, QObject *parent) :
    RegularLocalSocket(verboseMode, parent)
{
    // dont forget to     void initializeSocket(quint16 mtdExtName);

}

void ModbusMatildaLSClient::decodeReadData(const QVariant &dataVar, const quint16 &command)
{
    //only commands for modbus-bbb
    switch(command){
    case MTD_EXT_CUSTOM_COMMAND_0: {
        qDebug() << "default MTD_EXT_CUSTOM_COMMAND_0 " << command << mtdExtName << dataVar; emit onConfigChanged(command,dataVar);

        break;}

    case MTD_EXT_COMMAND_RELOAD_SETT: {
        qDebug() << "default MTD_EXT_COMMAND_RELOAD_SETT " << command << mtdExtName << dataVar; emit onConfigChanged(command,dataVar);

        break;}
//        if(verboseMode) qDebug() << "ext " << mtdExtName << dataVar;
// #ifdef ENABLE_VERBOSE_SERVER
//        if(activeDbgMessages)
//            emit appendDbgExtData(DBGEXT_THELOCALSOCKET, QString("command r: %1, data=%2").arg(command).arg(dataVar.toHash().value("d").toString()));
// #endif
//        emit command4dev(dataVar.toHash().value("c").toUInt(), dataVar.toHash().value("d").toString());
//        break;}



    default: {  if(verboseMode) qDebug() << "default ext " << command << mtdExtName << dataVar; emit onConfigChanged(command,dataVar);  break;}
    }

}

void ModbusMatildaLSClient::sendCommand2zbyrator(quint16 pollCode, QString ni, QString messagetag, QString objecttag)
{
    //ni of the emeter, wmeter, pmeter

    //    const QString params = modbusprocessor->generateQuickPollLine(readArr);
    //    if(params.isEmpty()){
    //        if(verboseMode)
    //            qDebug() << "generateQuickPollLine bad request" << readArr.toHex() ;
    //        //QVH
    ////        h.insert("c", (indx < 0) ? "" : l.at(indx));//name of the command  ui->cbDevCommand->currentData(Qt::UserRole).toString());
    ////        h.insert("pc", command);//pollCode
    ////        h.insert("d", args);// ui->leDevCommand->text().simplified().trimmed());


    ////        QVariantHash h;
    ////        h.insert("c", command);
    ////        h.insert("d", args);
    ////        if(verboseMode)
    ////            qDebug() << "UcCommandServer::command4dev " << command << args;
    ////        emit command2extension(MTD_EXT_NAME_ZBYRATOR, MTD_EXT_CUSTOM_COMMAND_0, h);

    //        QVariantHash pollargs;
    //        pollargs.insert("c", int(pollCode));
    //        pollargs.insert("pc", int(pollCode));
    //        pollargs.insert("d", QString("-n %1 -i").arg(QString::number(devaddr)));

    //        pollargs.insert("d", "");
    //        emit sendCommand2zbyrator(pollargs, );

    QVariantHash pollargs;
    pollargs.insert("c", int(pollCode));
    pollargs.insert("pc", int(pollCode)); //commnad
    pollargs.insert("d", QString("-n %1 -i").arg(ni)); //args


    sendCommand2zbyratorHash(pollargs, messagetag, objecttag);

}


bool ModbusMatildaLSClient::sendCommand2zbyratorHash(QVariantHash hash, QString messagetag, QString objecttag)
{
    //it starts poll, if necessary

    //        case MTD_EXT_COMMAND_2_OTHER_APP: {
    //            QVariantHash h = dataVar.toHash();
    //            quint16 e = h.value("e").toUInt();
    //            quint16 c = h.value("c").toUInt();

    //            if(e > 0 && c > 0 && e != mtdExtName)
    //                emit sendCommand2extension(e, c, h.value("d"));

    //            break;}
            //    void sendCommand2extension(quint16 extName, quint16 extCommand, QVariant data );

    if(state() != QLocalSocket::ConnectedState){
      emit onMatildaCommandReceived(messagetag, objecttag, false, "there is no connection");
      return false;
    }

     QVariantHash h;
     h.insert("e", MTD_EXT_NAME_ZBYRATOR);//destination
     h.insert("c", MTD_EXT_CUSTOM_COMMAND_0);//command to process on the destination side
     h.insert("d", hash);//poll arguments

     const qint64 r = mWrite2extensionF(QVariant(h), MTD_EXT_COMMAND_2_OTHER_APP);



     if(r > 0){
         emit onMatildaCommandReceived(messagetag, objecttag, true, QString::number(r));
         return true;
     }
     emit onMatildaCommandReceived(messagetag, objecttag, false, QString("Write error, %1, %2").arg(r).arg(errorString()));
     return false;

}

