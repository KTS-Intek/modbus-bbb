#include "modbusserialportcover.h"


///[!] type-converter
#include "src/shared/ifacehelper.h"

///[!] ifaces
#include "src/emb/conf2modem.h"


#include "moji_defy.h"



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

    connect(this, SIGNAL(restartReConnectTimerMsec(int)), tmrReconnect, SLOT(start(int)));
    connect(tmrReconnect, &QTimer::timeout, this, &ModbusSerialPortCover::reconnect2serialPort);
    connect(this, &ModbusSerialPortCover::onConnectionUp, tmrReconnect, &QTimer::stop);


//    connect(streamr, &ModbusStreamReader::need2closeSerialPort, [=]{
//        streamr->closeDevice();
//    });

    connect(streamr, &ModbusStreamReader::onConnectionClosed, this, &ModbusSerialPortCover::restartReConnectTimer);
    connect(streamr, &ModbusStreamReader::onConnectionDown, this, &ModbusSerialPortCover::restartReConnectTimer);
    connect(streamr, SIGNAL(onSerialPortOpened(QString)), this, SIGNAL(onConnectionUp()));

    connect(streamr, &ModbusStreamReader::sendCommand2dataHolder, this, &ModbusSerialPortCover::sendCommand2dataHolder);
    connect(streamr, &ModbusStreamReader::sendCommand2zbyrator, this, &ModbusSerialPortCover::sendCommand2zbyrator);
    connect(streamr, &ModbusStreamReader::sendCommand2firefly, this, &ModbusSerialPortCover::sendCommand2firefly);

    connect(this, &ModbusSerialPortCover::onCommandReceived, streamr, &ModbusStreamReader::onCommandReceived);
    connect(this, &ModbusSerialPortCover::dataFromCache, streamr, &ModbusStreamReader::dataFromCache);
    connect(this, &ModbusSerialPortCover::onMatildaCommandReceived, streamr, &ModbusStreamReader::onMatildaCommandReceived);

    connect(this, &ModbusSerialPortCover::killAllObjects, streamr, &ModbusStreamReader::closeSerialPortDirect);

    connect(this, &ModbusSerialPortCover::killAllObjects, streamr, &ModbusStreamReader::deleteLater);


    if(mystate.verboseMode)
        connect(streamr, &ModbusStreamReader::currentOperation, this, &ModbusSerialPortCover::currentOperation);

    connect(streamr, &ModbusStreamReader::currentOperation, this, &ModbusSerialPortCover::append2logSmpl);

    streamr->createObjects();

    connect(this, &ModbusSerialPortCover::onConnectionUp, this, &ModbusSerialPortCover::checkDHClientConnection);

    //    connect(this, &ModbusSerialPortCover::onConfigChanged, streamr, &ModbusStreamReader::onConfigChanged);



    IfaceHelper *ifceHlpr = new IfaceHelper(true, this);
    connect(ifceHlpr, &IfaceHelper::ifaceLogStr, this, &ModbusSerialPortCover::ifaceLogStr);
    connect(streamr, &ModbusStreamReader::dataReadWriteReal, ifceHlpr, &IfaceHelper::showHexDump);

    reloadSettings();


    //    QTimer::singleShot(111, streamr, SLOT(createObjects()));// this, SLOT(reloadSettings()));

}



void ModbusSerialPortCover::reconnect2serialPort()
{
    if(!mystate.serialSettings.enRTU){
        append2logSmpl(QString("Modbus RTU is disabled"));
        return;
    }

    QString errstr;

    const auto interfaceSettings = ModbusSettingsLoader::getInterafaceSettMap(mystate.serialSettings.serialport, errstr);

    if(!errstr.isEmpty()){
        append2logSmpl(QString("Modbus RTU is disabled, %1").arg(errstr));
        onSerialPortErrorHappened();
        return;
    }

    const auto serialp = Conf2modem::convertFromVarMap(interfaceSettings);
    const auto connSett = serialp.connSett;

    streamr->setTimeouts(connSett.timeOutG, connSett.timeOutB);
    streamr->setIgnoreUartChecks(true);


    append2logSmpl(QString("%1 is going to be opened").arg(serialp.ifaceParams.simplified()));

    //const bool &workWithoutAPI, const QString &portName, const qint32 &baudRate, const QStringList &uarts, const qint8 &databits, const qint8 &stopbits, const qint8 &parity, const qint8 &flowcontrol)

    //it has parity correction
    if(streamr->openSerialPort(true, connSett.prdvtrAddr, connSett.prdvtrPort, connSett.uarts, connSett.databits, connSett.stopbits, connSett.parity, connSett.flowcontrol)){
        //const qint8 &databits, const qint8 &stopbits, const qint8 &parity, const qint8 &flowcontrol
        if(streamr->verboseMode)
            qDebug() << "ModbusSerialPortCover port is opened " << connSett.prdvtrAddr;
        //if everything is fine, streamr tells it
        //        append2logSmpl(QString("%1 is opened, parity=%3").arg(mystate.prdvtrAddr).arg(int(streamr->serialPort->parity())));
        onSerialPortEverythingIsFine();
        return;
    }

    onSerialPortErrorHappened();


}

void ModbusSerialPortCover::reloadSettings()
{
    //    append2logSmpl(QString("%1, ModbusSerialPortCover::reloadSettings").arg(mystate.portName));


    mystate.serialOpentCounter = 0;//reset the counter

    streamr->modbusprocessor->reloadAllSettings();
    append2logSmpl(QString("Updating serial port settings"));

    if(!checkReloadSerialPortSettings()){
        append2logSmpl(QString("serial port settings: nothing has changed"));
        return; //port settings are the same
    }
//    append2logSmpl(QString("serial port settings: nothing has changed"));

    streamr->closeDevice();

    restartReConnectTimer();
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

void ModbusSerialPortCover::append2logSmpl(QString message)
{
    emit append2log(QDateTime::currentMSecsSinceEpoch(), message);
}


void ModbusSerialPortCover::restartReConnectTimer()
{
    emit restartReConnectTimerMsec(1111);
}

void ModbusSerialPortCover::onConfigChanged(quint16 command, QVariant datavar)
{



    if(streamr->verboseMode)
        qDebug() << "========================== ModbusSerialPortCover::onConfigChanged " << command << datavar;

    if(command == MTD_EXT_COMMAND_RELOAD_SETT || command == MTD_EXT_CUSTOM_COMMAND_0){

        reloadSettings();
    }

    //     switch(command){
    //     case MTD_EXT_COMMAND_RELOAD_SETT: {//reaload all settings
    //         modbusprocessor->onMeterListChanged();
    ////         stopWait4conf(); //it sends reloadSettings();
    // //        reloadMeters(UC_METER_ELECTRICITY);

    //         break;}

    //     case MTD_EXT_CUSTOM_COMMAND_0:{
    //         modbusprocessor->onMeterListExtChanged();

    //         break;}
    //     }


}

void ModbusSerialPortCover::onSerialPortErrorHappened()
{

    if(mystate.serialOpentCounter < 70)
        mystate.serialOpentCounter++;
    emit restartReConnectTimerMsec(9999); //approximatelly 10 restart cycle

    if(mystate.serialOpentCounter > 60)
        emit restartApp();

}

void ModbusSerialPortCover::onSerialPortEverythingIsFine()
{
    if(mystate.serialOpentCounter > 0)
        append2logSmpl(QString("The self-destruct counter was %1").arg(int(mystate.serialOpentCounter)));
    mystate.serialOpentCounter = 0;
}

bool ModbusSerialPortCover::checkReloadSerialPortSettings()
{
    //return true if something is changed

    const ModbusSerialSettings serialSettingsOld = mystate.serialSettings;



    mystate.serialSettings = ModbusSettingsLoader::getModbusSerialSettings();

    if(mystate.serialSettings.enRTU != serialSettingsOld.enRTU)
        return true;


    QString errstr;
    const auto serialpOld = Conf2modem::convertFromVarMap(ModbusSettingsLoader::getInterafaceSettMap(serialSettingsOld.serialport, errstr));
    const auto serialpNew = Conf2modem::convertFromVarMap(ModbusSettingsLoader::getInterafaceSettMap(mystate.serialSettings.serialport, errstr));


    return (serialpOld.ifaceParams != serialpNew.ifaceParams); //it checks all uart settings

}
