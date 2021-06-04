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

    connect(streamr, &ModbusStreamReader::onSerialPortName, this, &ModbusSerialPortCover::onSerialPortName);

    if(mystate.verboseMode)
        connect(streamr, &ModbusStreamReader::currentOperation, this, &ModbusSerialPortCover::currentOperation);

    connect(streamr, &ModbusStreamReader::currentOperation, this, &ModbusSerialPortCover::append2log);

    streamr->createObjects();

    connect(this, &ModbusSerialPortCover::onConnectionUp, this, &ModbusSerialPortCover::checkDHClientConnection);

    connect(this, &ModbusSerialPortCover::onConfigChanged, streamr, &ModbusStreamReader::onConfigChanged);


    connect(streamr, &ModbusStreamReader::dataReadWriteReal, this, &ModbusSerialPortCover::dataReadWriteReal);

    QTimer::singleShot(111, streamr, SLOT(createObjects()));// this, SLOT(reloadSettings()));

}



void ModbusSerialPortCover::reconnect2serialPort()
{
    emit append2log(QString("The port name is '%1'").arg(mystate.portName));

    if(mystate.portName.isEmpty())
        return;
    emit append2log(QString("%1 is going to be opened").arg(mystate.portName));

    //const bool &workWithoutAPI, const QString &portName, const qint32 &baudRate, const QStringList &uarts, const qint8 &databits, const qint8 &stopbits, const qint8 &parity, const qint8 &flowcontrol)

    //it has parity correction
    if(streamr->openSerialPort(true, mystate.portName, mystate.baudRate, mystate.portName.split(" "),
                               8, 1, mystate.isParityNone ? 0 : 1, QSerialPort::NoFlowControl)){
        //const qint8 &databits, const qint8 &stopbits, const qint8 &parity, const qint8 &flowcontrol
        qDebug() << "ModbusSerialPortCover port is opened " << mystate.portName;

        emit append2log(QString("%1 is opened, parity=%2,%3").arg(mystate.portName).arg(int(mystate.isParityNone)).arg(int(streamr->serialPort->parity())));
        return;
    }

    if(mystate.serialOpentCounter < 70)
        mystate.serialOpentCounter++;
     emit restartReConnectTimer();

    if(mystate.serialOpentCounter > 60)
        emit restartApp();



}

void ModbusSerialPortCover::reloadSettings()
{
    emit append2log(QString("%1, ModbusSerialPortCover::reloadSettings").arg(mystate.portName));

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

void ModbusSerialPortCover::onSerialPortName(QString serialportname, bool isParityNone)
{
    qDebug() << "ModbusSerialPortCover serialportname " << serialportname << isParityNone;
    emit append2log(QString("%1, %2, None=%3 ModbusSerialPortCover::onSerialPortName").arg(mystate.portName).arg(serialportname).arg(int(isParityNone)));

    if(serialportname != mystate.portName){
        mystate.serialOpentCounter = 0;
        mystate.portName = serialportname;
        mystate.isParityNone = isParityNone;
        reloadSettings();
    }
}
