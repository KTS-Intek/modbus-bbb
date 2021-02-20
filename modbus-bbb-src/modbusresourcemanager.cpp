#include "modbusresourcemanager.h"

#include <QTimer>

#include "modbusmatildalsclient.h"
#include "modbustcpserver.h"
#include "modbusserialportcover.h"
#include "modbusdataholderclient.h"


#include "moji_defy.h"

//--------------------------------------------------------------------------------

ModbusResourceManager::ModbusResourceManager(QObject *parent) : QObject(parent)
{

}

//--------------------------------------------------------------------------------

void ModbusResourceManager::createObjectsLater()
{
    verboseMode = qApp->arguments().contains("-vv");
    QTimer::singleShot(111, this, SLOT(createObjects()));
}

//--------------------------------------------------------------------------------

void ModbusResourceManager::createObjects()
{
    createMatildaLSClient();
    createDataHolderClient();
    createTcpServer();
    createSerialPortReader();
}

//--------------------------------------------------------------------------------

void ModbusResourceManager::restartApp()
{
    if(verboseMode)
        qDebug() << "ModbusResourceManager::restartApp() " << QObject::sender()->objectName();
    qApp->exit(0);
}

//--------------------------------------------------------------------------------

void ModbusResourceManager::createTcpServer()
{
    QThread *thread = new QThread;
    ModbusTCPServer *server = new ModbusTCPServer(verboseMode);
    thread->setObjectName("ModbusTCPServer");
    //    server->setObjectName("ModbusTCPServer");

    server->moveToThread(thread);

    connect(thread, &QThread::started, server, &ModbusTCPServer::onThrdStarted);
    connect(server, SIGNAL(destroyed(QObject*)), thread, SLOT(quit()));
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);

    connect(this, &ModbusResourceManager::killAllAndDie, server, &ModbusTCPServer::kickOffServer);

    connect(server, &ModbusTCPServer::restartApp            , this, &ModbusResourceManager::restartApp);
    connect(server, &ModbusTCPServer::sendCommand2dataHolder, this, &ModbusResourceManager::sendCommand2dataHolder);
    connect(server, &ModbusTCPServer::sendCommand2zbyrator  , this, &ModbusResourceManager::sendCommand2zbyrator);

    connect(this, &ModbusResourceManager::onCommandReceived         , server, &ModbusTCPServer::onCommandReceived);
    connect(this, &ModbusResourceManager::onMatildaCommandReceived  , server, &ModbusTCPServer::onMatildaCommandReceived);
    connect(this, &ModbusResourceManager::dataFromCache             , server, &ModbusTCPServer::dataFromCache);


    connect(server, &ModbusTCPServer::checkDHClientConnection, this, &ModbusResourceManager::checkDHClientConnection);


    connect(this, &ModbusResourceManager::onConfigChanged, server, &ModbusTCPServer::onConfigChanged);

    thread->start();

}

//--------------------------------------------------------------------------------

void ModbusResourceManager::createSerialPortReader()
{


    ModbusSerialPortCover *serialp = new ModbusSerialPortCover(verboseMode);


    QThread *thread = new QThread;
    thread->setObjectName("ModbusSerialPortCover");

    serialp->moveToThread(thread);

    connect(thread, &QThread::started, serialp, &ModbusSerialPortCover::onThreadStarted);
    connect(serialp, SIGNAL(destroyed(QObject*)), thread, SLOT(quit()));
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);

    connect(this, &ModbusResourceManager::killAllAndDie, serialp, &ModbusSerialPortCover::kickOffAll);

    connect(serialp, &ModbusSerialPortCover::sendCommand2dataHolder, this, &ModbusResourceManager::sendCommand2dataHolder);
    connect(serialp, &ModbusSerialPortCover::sendCommand2zbyrator  , this, &ModbusResourceManager::sendCommand2zbyrator);

    connect(this, &ModbusResourceManager::onCommandReceived         , serialp, &ModbusSerialPortCover::onCommandReceived);
    connect(this, &ModbusResourceManager::onMatildaCommandReceived  , serialp, &ModbusSerialPortCover::onMatildaCommandReceived);
    connect(this, &ModbusResourceManager::dataFromCache             , serialp, &ModbusSerialPortCover::dataFromCache);

    connect(this, &ModbusResourceManager::onConfigChanged, serialp, &ModbusSerialPortCover::onConfigChanged);

    thread->start();


}

//--------------------------------------------------------------------------------

void ModbusResourceManager::createMatildaLSClient()
{
#ifdef __x86_64
    if(true)
        return;//for test only

#endif
    ModbusMatildaLSClient *extSocket = new ModbusMatildaLSClient(verboseMode);
    extSocket->activeDbgMessages = false;// zbyrator->activeDbgMessages;
    extSocket->verboseMode = verboseMode;//force to enable
    extSocket->initializeSocket(MTD_EXT_NAME_MODBUS);
    QThread *extSocketThrd = new QThread(this); //QT2
    extSocketThrd->setObjectName("ModbusMatildaLSClient");

    extSocket->moveToThread(extSocketThrd);
#ifdef ENABLE_VERBOSE_SERVER
    connect(extSocket, &ModbusMatildaLSClient::appendDbgExtData, this, &ZbyratorManager::appendDbgExtData );
#endif
    connect(extSocketThrd, &QThread::started, extSocket, &ModbusMatildaLSClient::onThreadStarted);

    connect(extSocket, &ModbusMatildaLSClient::onConfigChanged , this, &ModbusResourceManager::onConfigChanged  );
//    connect(extSocket, &ModbusMatildaLSClient::command4dev     , zbyrator, &MeterManager::command4devStr      );

//    connect(this, &ModbusResourceManager::command2extensionClient, extSocket, &ModbusMatildaLSClient::command2extensionClient   ); do not use it here

    connect(this, &ModbusResourceManager::sendCommand2zbyrator, extSocket, &ModbusMatildaLSClient::sendCommand2zbyrator);
    connect(extSocket, &ModbusMatildaLSClient::onMatildaCommandReceived, this, &ModbusResourceManager::onMatildaCommandReceived);

    extSocketThrd->start();
}

//--------------------------------------------------------------------------------

void ModbusResourceManager::createDataHolderClient()
{
    ModbusDataHolderClient *dhclient = new ModbusDataHolderClient("all", verboseMode);
    QThread *thread = new QThread;
    thread->setObjectName("ModbusDataHolderClient");

    connect(thread, &QThread::started, dhclient, &ModbusDataHolderClient::onThreadStartedL0);
    connect(dhclient, SIGNAL(destroyed(QObject*)), thread, SLOT(quit()));
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);


    connect(this, &ModbusResourceManager::checkDHClientConnection, dhclient, &ModbusDataHolderClient::checkYourConnection);
    connect(this, &ModbusResourceManager::killAllAndDie, dhclient, &ModbusDataHolderClient::killAllObjects);

    connect(this, &ModbusResourceManager::sendCommand2dataHolder, dhclient, &ModbusDataHolderClient::sendCommand2dataHolder);

    connect(dhclient, &ModbusDataHolderClient::onCommandReceived, this, &ModbusResourceManager::onCommandReceived);
    connect(dhclient, &ModbusDataHolderClient::dataFromCache, this, &ModbusResourceManager::dataFromCache);

    thread->start();

}

//--------------------------------------------------------------------------------
