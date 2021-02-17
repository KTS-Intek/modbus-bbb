#include "modbusresourcemanager.h"

#include <QTimer>

#include "modbusmatildalsclient.h"



#include "moji_defy.h"

//--------------------------------------------------------------------------------

ModbusResourceManager::ModbusResourceManager(QObject *parent) : QObject(parent)
{

}

//--------------------------------------------------------------------------------

void ModbusResourceManager::createObjectsLater()
{
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

void ModbusResourceManager::createTcpServer()
{

}

//--------------------------------------------------------------------------------

void ModbusResourceManager::createSerialPortReader()
{

}

//--------------------------------------------------------------------------------

void ModbusResourceManager::createMatildaLSClient()
{
    ModbusMatildaLSClient *extSocket = new ModbusMatildaLSClient(verboseMode);
    extSocket->activeDbgMessages = false;// zbyrator->activeDbgMessages;

    extSocket->initializeSocket(MTD_EXT_NAME_MODBUS);
    QThread *extSocketThrd = new QThread(this); //QT2
    extSocketThrd->setObjectName("ModbusMatildaLSClient");
    extSocket->moveToThread(extSocketThrd);
#ifdef ENABLE_VERBOSE_SERVER
    connect(extSocket, &ModbusMatildaLSClient::appendDbgExtData, this, &ZbyratorManager::appendDbgExtData );
#endif
    connect(extSocketThrd, &QThread::started, extSocket, &ModbusMatildaLSClient::onThreadStarted);

//    connect(extSocket, &ModbusMatildaLSClient::onConfigChanged , zbyrator, &MeterManager::onConfigChanged  );
//    connect(extSocket, &ModbusMatildaLSClient::command4dev     , zbyrator, &MeterManager::command4devStr      );

//    connect(this, &ModbusResourceManager::command2extensionClient, extSocket, &ModbusMatildaLSClient::command2extensionClient   ); do not use it here

    connect(this, &ModbusResourceManager::sendCommand2zbyrator, extSocket, &ModbusMatildaLSClient::sendCommand2zbyrator);
    connect(extSocket, &ModbusMatildaLSClient::onCommandReceived, this, &ModbusResourceManager::onCommandReceived);

    extSocketThrd->start();
}

//--------------------------------------------------------------------------------

void ModbusResourceManager::createDataHolderClient()
{

}

//--------------------------------------------------------------------------------
