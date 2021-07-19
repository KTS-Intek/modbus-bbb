#include "modbustcpserver.h"

///[!] modbus-bbb
#include "modbustcpsocketcover.h"


///[!] type-converter
#include "src/shared/ifacehelper.h"



#include "moji_defy.h"

//----------------------------------------------------------------------------

ModbusTCPServer::ModbusTCPServer(const bool &verboseMode, QObject *parent) : QTcpServer(parent)
{
    myserverstate.verboseMode = verboseMode;
}

//----------------------------------------------------------------------------

void ModbusTCPServer::onThrdStarted()
{
    QTimer *tmrRestart = new QTimer(this);
    tmrRestart->setSingleShot(true);
    tmrRestart->setInterval(9999);

    connect(this, SIGNAL(restartTmrRestart()), tmrRestart, SLOT(start()));
    connect(tmrRestart, &QTimer::timeout, this, &ModbusTCPServer::startServer);


    reloadSettings();

//    startServer();

}

//----------------------------------------------------------------------------

void ModbusTCPServer::onConnectionDown()
{
    if(myserverstate.activeConnectionCounter > 0)
        myserverstate.activeConnectionCounter--;


    if(myserverstate.verboseMode)
        qDebug() << "onConnectionDown " <<  myserverstate.connectionCounter << myserverstate.activeConnectionCounter;

}

//----------------------------------------------------------------------------

void ModbusTCPServer::kickOffServer()
{
    emit killAllObjects();
    stopServerNow();
//    deleteLater();
    QTimer::singleShot(11, this, SLOT(deleteLater()));
}

//----------------------------------------------------------------------------

void ModbusTCPServer::stopServerNow()
{
    emit stopAllSocket();
    close();
}

//----------------------------------------------------------------------------

void ModbusTCPServer::startServer()
{
    if(!myserverstate.allowTcpServer)
        return;

    if(isListening())
        return;

    const quint16 serverp =
#ifdef __x86_64
    5002
#else
    502
#endif
            ;

    if(listen(QHostAddress::Any, serverp)){
        if(myserverstate.verboseMode)
            qDebug() << "ModbusTCPServer::startServer() ok" ;

        myserverstate.serverStartCounter = 0;
        return;
    }

    emit append2log(QString("Failed to start tcp server, %1 ").arg(errorString()));

    if(myserverstate.verboseMode)
        qDebug() << "ModbusTCPServer::startServer() failed " << errorString()  << int(myserverstate.serverStartCounter);

    if(myserverstate.serverStartCounter > 59){

        emit restartApp();

    }else{
        myserverstate.serverStartCounter++;
    }
//    QTimer::singlShot(1111, this, SLOT(startServer()));
    emit restartTmrRestart();
}

void ModbusTCPServer::onConfigChanged(quint16 command, QVariant datavar)
{
    if(verboseMode)
        qDebug() << "========================== ModbusTCPSocketCover::onConfigChanged " << command << datavar << socket->peerAddress().toString();

    if(command == MTD_EXT_COMMAND_RELOAD_SETT || command == MTD_EXT_CUSTOM_COMMAND_0){

        reloadSettings();
    }
}

void ModbusTCPServer::reloadSettings()
{


    const auto tcpMode = ModbusSettingsLoader::getModbusTcpSettings().mode;

    const bool allowTcpServer = myserverstate.allowTcpServer;
    myserverstate.allowTcpServer = (tcpMode != 0);

    if(myserverstate.allowTcpServer){
        if(!isListening()){
            if(allowTcpServer != myserverstate.allowTcpServer){
                emit append2log(QString("Modbus tcp server is enabled"));
                myserverstate.serverStartCounter = 0;
            }
//            startServer();
            emit restartTmrRestart();
            return;
        }

    }else{

        if(isListening()){
            emit append2log(QString("Modbus tcp server is disabled"));
            stopServerNow();
            return;
        }
    }

    emit reloadTcpSettings();

}



//----------------------------------------------------------------------------

void ModbusTCPServer::incomingConnection(qintptr handle)
{

    emit checkDHClientConnection();

    ModbusTCPSocketCover *streamr = new ModbusTCPSocketCover(
                QString("tcp%1").arg(myserverstate.connectionCounter),
                true,
                myserverstate.verboseMode,
                this);


    QString lastError;
    if(!streamr->isAble2setSocketDescriptor(handle, lastError)){
        if(myserverstate.verboseMode)
            qDebug() << "!incomingConnection " << QString("tcp%1").arg(myserverstate.connectionCounter) << lastError;
        return;
    }


    if(myserverstate.verboseMode)
        qDebug() << "incomingConnection " << streamr->socket->peerAddress().toString() <<  myserverstate.connectionCounter << myserverstate.activeConnectionCounter;

    myserverstate.connectionCounter++;
    myserverstate.activeConnectionCounter++;

    connect(streamr->socket, &QTcpSocket::disconnected, streamr, &ModbusTCPSocketCover::deleteLater);
    connect(streamr, SIGNAL(destroyed(QObject*)), this, SLOT(onConnectionDown()));

    connect(streamr, &ModbusTCPSocketCover::sendCommand2dataHolder, this, &ModbusTCPServer::sendCommand2dataHolder);
    connect(streamr, &ModbusTCPSocketCover::sendCommand2zbyrator, this, &ModbusTCPServer::sendCommand2zbyrator);

    connect(this, &ModbusTCPServer::onCommandReceived, streamr, &ModbusTCPSocketCover::onCommandReceived);
    connect(this, &ModbusTCPServer::dataFromCache, streamr, &ModbusTCPSocketCover::dataFromCache);
    connect(this, &ModbusTCPServer::onMatildaCommandReceived, streamr, &ModbusTCPSocketCover::onMatildaCommandReceived);


    connect(this, &ModbusTCPServer::stopAllSocket, streamr, &ModbusTCPSocketCover::deleteLater);
    connect(this, &ModbusTCPServer::killAllObjects, streamr, &ModbusTCPSocketCover::deleteLater);

    connect(this, &ModbusTCPServer::reloadTcpSettings, streamr, &ModbusTCPSocketCover::reloadSettings);



    IfaceHelper *ifceHlpr = new IfaceHelper(true, this);
    connect(ifceHlpr, &IfaceHelper::ifaceLogStr, this, &ModbusTCPServer::ifaceLogStr);
    connect(streamr, &ModbusTCPSocketCover::dataReadWriteReal, ifceHlpr, &IfaceHelper::showHexDump);

    if(streamr->socket->state() != QTcpSocket::ConnectedState){
        streamr->deleteLater();
    }else{
        streamr->createObjects();
        streamr->reloadSettings();
    }


}
//----------------------------------------------------------------------------

