#include "modbustcpserver.h"

#include "modbusstreamreader.h"

//----------------------------------------------------------------------------

ModbusTCPServer::ModbusTCPServer(const bool &verboseMode, QObject *parent) : QTcpServer(parent)
{
    myserverstate.verboseMode = verboseMode;
}

//----------------------------------------------------------------------------

void ModbusTCPServer::onThrdStarted()
{
    startServer();

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
    if(myserverstate.verboseMode)
        qDebug() << "ModbusTCPServer::startServer() failed " << errorString()  << int(myserverstate.serverStartCounter);

    if(myserverstate.serverStartCounter > 59){

        emit restartApp();

    }else{
        myserverstate.serverStartCounter++;
    }
    QTimer::singleShot(1111, this, SLOT(startServer()));
}

//----------------------------------------------------------------------------

void ModbusTCPServer::incomingConnection(qintptr handle)
{

    emit checkDHClientConnection();

    ModbusStreamReader *streamr = new ModbusStreamReader(
                QString("tcp%1").arg(myserverstate.connectionCounter),
                true,
                myserverstate.verboseMode,
                this);



    if(!streamr->socket->setSocketDescriptor(handle)){
        streamr->deleteLater();
        if(myserverstate.verboseMode)
            qDebug() << "!incomingConnection " << streamr->socket->errorString();

        return;
    }


    if(myserverstate.verboseMode)
        qDebug() << "incomingConnection " << streamr->socket->peerAddress().toString() <<  myserverstate.connectionCounter << myserverstate.activeConnectionCounter;

    myserverstate.connectionCounter++;
    myserverstate.activeConnectionCounter++;

    connect(streamr->socket, &QTcpSocket::disconnected, streamr, &ModbusStreamReader::deleteLater);
    connect(streamr, SIGNAL(destroyed(QObject*)), this, SLOT(onConnectionDown()));

    connect(streamr, &ModbusStreamReader::sendCommand2dataHolder, this, &ModbusTCPServer::sendCommand2dataHolder);
    connect(streamr, &ModbusStreamReader::sendCommand2zbyrator, this, &ModbusTCPServer::sendCommand2zbyrator);

    connect(this, &ModbusTCPServer::onCommandReceived, streamr, &ModbusStreamReader::onCommandReceived);
    connect(this, &ModbusTCPServer::dataFromCache, streamr, &ModbusStreamReader::dataFromCache);
    connect(this, &ModbusTCPServer::onMatildaCommandReceived, streamr, &ModbusStreamReader::onMatildaCommandReceived);


    connect(this, &ModbusTCPServer::stopAllSocket, streamr, &ModbusStreamReader::deleteLater);
    connect(this, &ModbusTCPServer::killAllObjects, streamr, &ModbusStreamReader::deleteLater);

    connect(this, &ModbusTCPServer::onConfigChanged, streamr, &ModbusStreamReader::onConfigChanged);

    connect(streamr, &ModbusStreamReader::dataReadWriteReal, this, &ModbusTCPServer::dataReadWriteReal);

    if(streamr->socket->state() != QTcpSocket::ConnectedState){
        streamr->deleteLater();
    }else{
        streamr->createObjects();
    }


}
//----------------------------------------------------------------------------

