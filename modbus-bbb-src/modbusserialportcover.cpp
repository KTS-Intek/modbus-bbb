#include "modbusserialportcover.h"



ModbusSerialPortCover::ModbusSerialPortCover(const bool &verboseMode, QObject *parent) : QObject(parent)
{
    mystate.verboseMode = verboseMode;
}

void ModbusSerialPortCover::onThreadStarted()
{
    streamr = new ModbusStreamReader(
                QString("sp0"),
                false,
                mystate.verboseMode,
                this);


    QTimer *tmrReconnect = new QTimer(this);
    tmrReconnect->setInterval(1111);
    tmrReconnect->setSingleShot(true);

    connect(this, SIGNAL(restartReConnectTimer()), tmrReconnect, SLOT(start()));
    connect(tmrReconnect, &QTimer::timeout, this, &ModbusSerialPortCover::reconnect2serialPort);
    connect(this, &ModbusSerialPortCover::onConnectionUp, tmrReconnect, &QTimer::stop);



    connect(streamr, &ModbusStreamReader::onConnectionClosed, this, &ModbusSerialPortCover::restartReConnectTimer);
    connect(streamr, &ModbusStreamReader::onConnectionDown, this, &ModbusSerialPortCover::restartReConnectTimer);
    connect(streamr, SIGNAL(onSerialPortOpened(QString)), this, SIGNAL(onConnectionUp()));

    connect(streamr, &ModbusStreamReader::sendCommand2dataHolder, this, &ModbusSerialPortCover::sendCommand2dataHolder);
    connect(streamr, &ModbusStreamReader::sendCommand2zbyrator, this, &ModbusSerialPortCover::sendCommand2zbyrator);

    connect(this, &ModbusSerialPortCover::onCommandReceived, streamr, &ModbusStreamReader::onCommandReceived);
    connect(this, &ModbusSerialPortCover::dataFromCache, streamr, &ModbusStreamReader::dataFromCache);
    connect(this, &ModbusSerialPortCover::onMatildaCommandReceived, streamr, &ModbusStreamReader::onMatildaCommandReceived);

    connect(this, &ModbusSerialPortCover::killAllObjects, streamr, &ModbusStreamReader::closeSerialPortDirect);

    connect(this, &ModbusSerialPortCover::killAllObjects, streamr, &ModbusStreamReader::deleteLater);

    if(mystate.verboseMode)
        connect(streamr, &ModbusStreamReader::currentOperation, this, &ModbusSerialPortCover::currentOperation);

    streamr->createObjects();

    connect(this, &ModbusSerialPortCover::onConnectionUp, this, &ModbusSerialPortCover::checkDHClientConnection);


    QTimer::singleShot(111, this, SLOT(reloadSettings()));

}



void ModbusSerialPortCover::reconnect2serialPort()
{
    if(mystate.portName.isEmpty())
        return;

    //const bool &workWithoutAPI, const QString &portName, const qint32 &baudRate, const QStringList &uarts, const qint8 &databits, const qint8 &stopbits, const qint8 &parity, const qint8 &flowcontrol)

    if(streamr->openSerialPort(true, mystate.portName, mystate.baudRate, mystate.portName.split(" "),
                               8, 1, 1, 0)){
        return;
    }

     emit restartReConnectTimer();

}

void ModbusSerialPortCover::reloadSettings()
{
    //for test only
    mystate.portName = "ttyUSB0";
    streamr->closeDevice();
    emit restartReConnectTimer();
}

void ModbusSerialPortCover::kickOffAll()
{
    emit killAllObjects();
    QThread::msleep(11);
}

void ModbusSerialPortCover::currentOperation(QString messageStrr)
{
    qDebug() << "ModbusSerialPortCover " << messageStrr;

}
