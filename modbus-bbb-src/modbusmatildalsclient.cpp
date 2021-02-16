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
//    case MTD_EXT_CUSTOM_COMMAND_0: {
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

void ModbusMatildaLSClient::sendCommand2zbyrator(QVariantHash hash, quint16 messagetag, quint16 objecttag)
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

    if(state() != QLocalSocket::UnconnectedState){
      emit onCommandReceived(messagetag, objecttag, false, "there is no connection");
      return;
    }

     QVariantHash h;
     h.insert("e", MTD_EXT_NAME_ZBYRATOR);//destination
     h.insert("c", MTD_EXT_CUSTOM_COMMAND_0);//command to process on the destination side
     h.insert("d", hash);//poll arguments

     const qint64 r = mWrite2extensionF(QVariant(h), MTD_EXT_COMMAND_2_OTHER_APP);
     if(r > 0){
         emit onCommandReceived(messagetag, objecttag, true, QString::number(r));
         return;
     }
     emit onCommandReceived(messagetag, objecttag, false, QString("Write error, %1, %2").arg(r).arg(errorString()));

}
