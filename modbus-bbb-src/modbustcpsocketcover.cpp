#include "modbustcpsocketcover.h"

///[!] type-converter
#include "src/shared/networkconverthelper.h"





ModbusTCPSocketCover::ModbusTCPSocketCover(const QString &objecttag, const bool &isTcpMode, const bool &verboseMode, QObject *parent) :
    ModbusStreamReader(objecttag, isTcpMode, verboseMode, parent)
{
    setTheIsServerSide();
    connect(this, &ModbusTCPSocketCover::dataReadWriteReal, this, &ModbusTCPSocketCover::onDataReadWriteReal);
//    connect(socket, &QTcpSocket::disconnected, this, onConnectionDown();)
    connect(this, &ModbusTCPSocketCover::onConnectionDown, this, &ModbusTCPSocketCover::onTheConnectionDown);

    connect(this, &ModbusTCPSocketCover::onConnectionClosed, this, &ModbusTCPSocketCover::onTheConnectionDown);
}

bool ModbusTCPSocketCover::isAble2setSocketDescriptor(qintptr handle, QString &lastError)
{

    if(!socket->setSocketDescriptor(handle)){
        lastError = socket->errorString();
        QTimer::singleShot(111, this, SLOT(deleteLater()));
      return false;
    }
    return true;
}

void ModbusTCPSocketCover::onDataReadWriteReal(QByteArray arr, QString ifaceName, bool isRead)
{
    Q_UNUSED(ifaceName);
    if(arr.isEmpty())
        return;

    const auto len = arr.size();
    if(isRead){
        theconnection.rb += len;
    }else{
        theconnection.wb += len;
    }

    if(len > 23){
        theconnection.lastmessage = arr.left(23).toHex() ;
        theconnection.lastmessage.append("...");
    }else
        theconnection.lastmessage = arr.toHex();

    sendTheLastConnectionState();
}

void ModbusTCPSocketCover::setupTheConnectionStruct(const QString &conntype, const QString &connid)
{
    if(!theconnection.connid.isEmpty() && !theconnection.conntype.isEmpty()){
        onTheConnectionDown(); // in case of respawn
    }

    theconnection.conntype = conntype;
    theconnection.connid = connid;// "tcps";
    theconnection.msecstart = QDateTime::currentMSecsSinceEpoch();
}

void ModbusTCPSocketCover::closeTheConnectionAndSelfDestr()
{
    closeDevice();
    onTheConnectionDown();
    deleteLater();
}

void ModbusTCPSocketCover::onTheConnectionUp()
{
    theconnection.msecend = 0;
    theconnection.msecstart = QDateTime::currentMSecsSinceEpoch();
    sendTheLastConnectionState();
}

void ModbusTCPSocketCover::setTheIsServerSide()
{
    theconnection.isServerSide = true;//false by default

}

void ModbusTCPSocketCover::onTheConnectionDown()
{

    if(theconnection.msecend != 0)
        return;
    theconnection.msecend = QDateTime::currentMSecsSinceEpoch();
    sendTheLastConnectionState();
}

void ModbusTCPSocketCover::sendTheLastConnectionState()
{
    if(theconnection.conntype.isEmpty())
        return;

    emit setServerInConnIdExtData(theconnection.conntype, theconnection.connid, theconnection.msecstart, theconnection.msecend, theconnection.rb, theconnection.wb, theconnection.lastmessage);

}


void ModbusTCPSocketCover::reloadSettings()
{

    const QString remaddr = NetworkConvertHelper::showNormalIP(socket->peerAddress());

    const ModbusTcpSettings settings = ModbusSettingsLoader::getModbusTcpSettings();

    if(!settings.IPs.isEmpty()){

        if(theconnection.conntype.isEmpty()){
            //it must enter here only once
            setupTheConnectionStruct("tcps", ConnectionTableInSharedMemoryTypes::getConnid4tcpServerSocket(remaddr, socket->localPort(), socket->socketDescriptor()));
            onTheConnectionUp();
            setTheIsServerSide();
            ifaceName = remaddr;
        }

        if(!NetworkConvertHelper::isIpGood(remaddr, settings.IPs)){

            theconnection.lastmessage = QString("This IP is not in the allowed list");

            emit currentOperation(tr("IP '%1' is not in the allowed list").arg(remaddr));
            QTimer::singleShot(11, this, SLOT(closeTheConnectionAndSelfDestr()));
            return ;
        }
    }
    modbusprocessor->reloadAllSettings();
    modbusprocessor->setModbusMode(settings.mode);//0 - disabled, 1 - rtu, 2 - tcp, other - both

    const QString rmDescr = QString::number(socket->socketDescriptor());

    emit currentOperation(QString("Updating %1:%2 settings").arg(remaddr).arg(rmDescr));

    setTimeouts(settings.tcpTimeout, settings.tcpBlcokTimeout);
    setIgnoreUartChecks(true);



}
