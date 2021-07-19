#include "modbusencoderdecoder.h"



///[!] type-converter
#include "src/base/convertatype.h"
#include "src/base/prettyvalues.h"

#include "myucdevicetypes.h"
#include "definedpollcodes.h"

#define MYDECODER_CACHE_MINIMUM_MSEC    10000

#define MYDECODER_READF_FIRST_REGISTER          40001

#define MYDECODER_VOLTAGE_REGISTER_FIRST        MYDECODER_READF_FIRST_REGISTER
#define MYDECODER_TOTALENERGY_REGISTER_FIRST    40201



#define MYDECODER_TOTALWATER_REGISTER_FIRST     41201

#define MYDECODER_TOTALGAS_REGISTER_FIRST       42201

#define MYDECODER_TOTALPULSE_REGISTER_FIRST     43201



#define MYDECODER_READF_FIRST_VREGISTER MYDECODER_VOLTAGE_REGISTER_FIRST
#define MYDECODER_READF_LAST_VREGISTER  40039

#define MYDECODER_READF_FIRST_EREGISTER MYDECODER_TOTALENERGY_REGISTER_FIRST
#define MYDECODER_READF_LAST_EREGISTER  40240



#define MYDECODER_READF_FIRST_WREGISTER MYDECODER_TOTALWATER_REGISTER_FIRST
#define MYDECODER_READF_LAST_WREGISTER  41212

#define MYDECODER_READF_FIRST_GREGISTER MYDECODER_TOTALGAS_REGISTER_FIRST
#define MYDECODER_READF_LAST_GREGISTER  42202

#define MYDECODER_READF_FIRST_PREGISTER MYDECODER_TOTALPULSE_REGISTER_FIRST
#define MYDECODER_READF_LAST_PREGISTER  43208

//--------------------------------------------------------------------

ModbusEncoderDecoder::ModbusEncoderDecoder(const bool &verboseMode, const quint8 &modbusDecoderMode, const bool &isModbusMasterSide, QObject *parent) :
    ModbusMessanger(parent)
{
    this->verboseMode = verboseMode;
    setModbusMode(modbusDecoderMode);
    setModbusSide(isModbusMasterSide);


    createObjects();
}

//--------------------------------------------------------------------

quint8 ModbusEncoderDecoder::getModbusMode()
{
    return myparams.modbusDecoderMode;
}

//--------------------------------------------------------------------

bool ModbusEncoderDecoder::isModbusMasterSide()
{
    return myparams.isModbusMasterSide;
}

//--------------------------------------------------------------------

ModbusList ModbusEncoderDecoder::getErrorMessage(const ModbusDecodedParams &messageparams, const quint8 &errorcode)
{

    ModbusList list;
    if(messageparams.devaddress == 0xFF)
        return list;//there is nothing to send

    list.append(messageparams.devaddress);
    quint8 functionCode = messageparams.functionCode;
    if(functionCode < 0x80)
        functionCode += quint8(0x80);

    //Should a slave want to report an error, it will reply with the requested function code plus 128 (hex 0x80) (3 becomes 131 = hex 0x83),
    list.append( functionCode );
    list.append(errorcode);


    const MODBUSDIVIDED_UINT16 crc = getCrc16(list);
    list.append(crc.hightbyte);
    list.append(crc.lowbyte);
    return list;
}

//--------------------------------------------------------------------

QByteArray ModbusEncoderDecoder::getErrorMessageArray(const ModbusDecodedParams &messageparams, const quint8 &errorcode)
{
    Q_UNUSED(messageparams);
    Q_UNUSED(errorcode);

    if(verboseMode)
        qDebug() << "ModbusEncoderDecoder::getErrorMessageArray " << errorcode << int(messageparams.devaddress );
    return QByteArray();
    //    const ModbusList list = getErrorMessage(messageparams, errorcode);
    //    return  ConvertAtype::uint8list2array(list, 0, list.length());//is ready to send message

}

//--------------------------------------------------------------------

bool ModbusEncoderDecoder::isMessageReadingFinished(const QByteArray &readArr, ModbusDecodedParams &messageparams)
{
    bool r = false;
    messageparams = ModbusDecodedParams();
    if(readArr.isEmpty() || readArr.length() < 6)
        return false;

    switch (myparams.modbusDecoderMode) {
    case MODBUS_MODE_RTU:
        r = isMessageReadingFinishedRTU(readArr, messageparams);
        break;

    case MODBUS_MODE_TCP:
        r = isMessageReadingFinishedTCP(readArr, messageparams);
        break;

    default:
        r = (isMessageReadingFinishedRTU(readArr, messageparams) || isMessageReadingFinishedTCP(readArr, messageparams));
        break;
    }
    return r;
}

//--------------------------------------------------------------------

bool ModbusEncoderDecoder::isMessageReadingFinishedRTU(const QByteArray &readArr, ModbusDecodedParams &messageparams)
{
    //    if(myparams.isModbusMasterSide){
    return isThisMessageYoursLoopRTU(readArr, messageparams);
    //    }


}

//--------------------------------------------------------------------

bool ModbusEncoderDecoder::isMessageReadingFinishedTCP(const QByteArray &readArr, ModbusDecodedParams &messageparams)
{
    //    if(myparams.isModbusMasterSide){
    return isThisMessageYoursLoopTCP(readArr, messageparams);
    //    }

}

//--------------------------------------------------------------------

void ModbusEncoderDecoder::canProcessTheLine(const QByteArray &readArr)
{
    //check that message is valid
    //it returns a message to the master
    //1. Check the message is valid
    //2. If the message is invalid - send error code, if possible
    //3. If the message is valid -
    //3.1 Another message is processed - send error code I am busy
    //3.2 There is no any task - try to process the request, start Timeout timer
    //3.3 If Timeout timer occur - send error code I am Acknowledge
    //3.4 Data from the request is found - send answer to the master

    if(readArr.isEmpty())
        return;//nothing has happened

    ModbusDecodedParams lastmessageparams;
    if(!isMessageReadingFinished(readArr, lastmessageparams)){
        sendErrorCode4thisDecoderError(lastmessageparams);
        return;
    }


    if(!myparams.myDevices.contains(lastmessageparams.devaddress)){
        if(verboseMode)
            qDebug() << "canProcessTheLine is not my addr " << int(lastmessageparams.devaddress);
        return;//it is not my meter, this request is not to me

    }
    if(myparams.isDecoderBusy){
        emit onData2write(getErrorMessageArray(lastmessageparams, MODBUS_ERROR_SLAVE_DEVICE_BUSY));
        if(myparams.busyCounter > 0xFFF0){
            if(verboseMode)
                qDebug()  << "ModbusEncoderDecoder::canProcessTheLine WTF? " ;
            emit startTmrProcessing(1);//try to restart the timer
            myparams.busyCounter = 0;
        }else{
            myparams.busyCounter++;
        }
        emit append2textLog(QString("myparams.isDecoderBusy, address '%1'").arg(int(lastmessageparams.devaddress)));

        return;
        //
    }

    startProcessing(lastmessageparams);

    //registers check
    if(lastmessageparams.functionCode == MODBUS_READFUNCTION){//read function has fixed length

        quint16 startRegister = 0;
        quint16 registerCount = 0;

        if(!isReadFunctionOk(lastmessageparams, startRegister, registerCount)){//data length check
            sendErrorCodeAndResetTheState(MODBUS_ERROR_ILLEGAL_DATA_VALUE);
            return;
        }

        if(!isStartRegisterGood(startRegister)){
            sendErrorCodeAndResetTheState(MODBUS_ERROR_ILLEGAL_DATA_VALUE);
            return;
        }
        emit append2textLog(QString("findData4theseRegister, address '%1', startRegister '%2', registerCount '%3'")
                            .arg(int(lastmessageparams.devaddress)).arg(int(startRegister)).arg(int(registerCount)));

        findData4theseRegister(startRegister, registerCount);
        //check all registers
        return;

    }

    //    if(lastmessageparams.functionCode != MODBUS_READFUNCTION){
    sendErrorCodeAndResetTheState(MODBUS_ERROR_ILLEGAL_FUNCT_CODE);
    //        return;
    //    }

}

//--------------------------------------------------------------------

void ModbusEncoderDecoder::createObjects()
{
    QTimer *timer = new QTimer(this);
    timer->setInterval(9000);
    timer->setSingleShot(true);
    connect(this, SIGNAL(startTmrProcessing(int)), timer, SLOT(start(int)));
    connect(this, &ModbusEncoderDecoder::stopTmrProcessing, timer, &QTimer::stop);
    connect(timer, &QTimer::timeout, this, &ModbusEncoderDecoder::onTmrProcessingTimeOut);//MODBUS_ERROR_ACKNOWLEDGE

    QTimer *timerKickOffCurrentState = new QTimer(this);
    timerKickOffCurrentState->setInterval(55555);
    timerKickOffCurrentState->setSingleShot(true);
    connect(this, SIGNAL(startTmrFinitaLaComedia(int)), timerKickOffCurrentState, SLOT(start(int)));
    connect(this, &ModbusEncoderDecoder::stopTmrProcessing, timerKickOffCurrentState, &QTimer::stop);
    connect(timerKickOffCurrentState, &QTimer::timeout, this, &ModbusEncoderDecoder::onTmrFinitaLaComedia);//MODBUS_ERROR_ILLEGAL_SLVEDEVFLR

    QTimer *timerDataHolder = new QTimer(this);
    timerDataHolder->setInterval(555);
    timerDataHolder->setSingleShot(true);
    connect(this, SIGNAL(startTmrDataHolderProcessing(int)), timerDataHolder, SLOT(start(int)));
    connect(this, &ModbusEncoderDecoder::stopTmrProcessing, timerDataHolder, &QTimer::stop);
    connect(timerDataHolder, &QTimer::timeout, this, &ModbusEncoderDecoder::onTmrDataHolderProcessingTimeOut);//create a new requests to Data holder, to get the data
}

//--------------------------------------------------------------------

void ModbusEncoderDecoder::setModbusMode(const quint8 &modbusmode)
{
    myparams.modbusDecoderMode = modbusmode;
}

//--------------------------------------------------------------------

void ModbusEncoderDecoder::setModbusSide(const bool &isModbusMasterSide)
{
    myparams.isModbusMasterSide = isModbusMasterSide;
}

//--------------------------------------------------------------------

void ModbusEncoderDecoder::sendErrorCode4thisDecoderError(const ModbusDecodedParams &messageparams)
{
    quint8 errorcode = MODBUS_ERROR_HAS_NO_ERRORS;

    switch(messageparams.decodeErr){
    case MODBUS_DECODER_ERROR_ILLEGAL_FUNCT_CODE    : errorcode = MODBUS_ERROR_ILLEGAL_FUNCT_CODE   ; break;
    case MODBUS_DECODER_ERROR_ILLEGAL_DATA_ADDR     : errorcode = MODBUS_ERROR_ILLEGAL_DATA_ADDR    ; break;
    case MODBUS_DECODER_ERROR_ILLEGAL_LEN           : errorcode = MODBUS_ERROR_ILLEGAL_DATA_VALUE   ; break;
    case MODBUS_DECODER_ERROR_ILLEGAL_CRC           : errorcode = MODBUS_ERROR_ILLEGAL_DATA_VALUE   ; break;
    case MODBUS_DECODER_ERROR_ILLEGAL_DEVICEADDR    : break;//do not do anything

    }

    if(errorcode > MODBUS_ERROR_HAS_NO_ERRORS){
        emit onData2write(getErrorMessageArray(messageparams, errorcode));

    }
}

//--------------------------------------------------------------------

void ModbusEncoderDecoder::startProcessing(const ModbusDecodedParams &lastmessageparams)
{
    myparams.isDecoderBusy = true;
    myparams.busyCounter = 0;
    myparams.processintTimeoutCounter = 0;
    myparams.lastmessageparams = lastmessageparams;
    myparams.msecOfTheRequest = QDateTime::currentMSecsSinceEpoch();

    restartTimerProcessing();
    emit startTmrFinitaLaComedia(55555);
    emit startTmrDataHolderProcessing(2222);

    if(verboseMode)
        qDebug()  << "ModbusEncoderDecoder::startProcessing startTmrDataHolderProcessing ";
}

//--------------------------------------------------------------------

void ModbusEncoderDecoder::restartTimerProcessing()
{
    qint32 msec = myparams.generalSettings.dataTmsec;// processingmsec;
    if(msec < 1000)
        msec = 1000;
    else if(msec > 30000)
        msec = 30000;

    emit startTmrProcessing(msec);
}

//--------------------------------------------------------------------

void ModbusEncoderDecoder::onTmrProcessingTimeOut()
{
    if(!myparams.isDecoderBusy)
        return;


    if(myparams.lastmessageparams.devaddress > 0 && myparams.lastmessageparams.devaddress < 248){


        const QList<quint8> lk = cachedDataHolderAnswers.keys();

        if(verboseMode)
            qDebug()  << "ModbusEncoderDecoder::onTmrProcessingTimeOut() " << lk.isEmpty() ;

        if(!lk.isEmpty()){

            bool add2dataHolder;
            for(int i = 0, imax = lk.size(); i < imax; i++){

                isCachedDataAcceptable(cachedDataHolderAnswers.value(lk.at(i)), true, add2dataHolder);



            }

            if(verboseMode)
                qDebug()  << "ModbusEncoderDecoder::onTmrProcessingTimeOut() " << cachedDataHolderAnswers.isEmpty() ;


            if(cachedDataHolderAnswers.isEmpty() && checkSendDataToTheMaster()){
                emit append2textLog("onTmrProcessingTimeOut checkSendDataToTheMaster");

                if(verboseMode)
                    qDebug()  << "ModbusEncoderDecoder::onTmrProcessingTimeOut() lastSecond";

                return;//last second send
            }

        }



        restartTimerProcessing();
        myparams.processintTimeoutCounter++;
        if(myparams.processintTimeoutCounter > 100){

            emit append2textLog("sendErrorCodeAndResetTheState(MODBUS_ERROR_ILLEGAL_SLVEDEVFLR)");
            //            sendErrorCodeAndResetTheState(MODBUS_ERROR_ILLEGAL_SLVEDEVFLR);
            return;
        }

        //if it has any cached acceptable data send it


        //        emit onData2write(getErrorMessageArray(myparams.lastmessageparams, MODBUS_ERROR_ACKNOWLEDGE));


    }

}

//--------------------------------------------------------------------

void ModbusEncoderDecoder::onTmrFinitaLaComedia()
{
    if(!myparams.isDecoderBusy)
        return;

    if(myparams.lastmessageparams.devaddress > 0 && myparams.lastmessageparams.devaddress < 248){
        if(verboseMode)
            qDebug()  << "ModbusEncoderDecoder::onTmrFinitaLaComedia ";//it is processing to long

        sendDataToTheMaster();

        sendErrorCodeAndResetTheState(MODBUS_ERROR_ILLEGAL_SLVEDEVFLR);
    }
}

//--------------------------------------------------------------------

void ModbusEncoderDecoder::onTmrDataHolderProcessingTimeOut()
{

    if(verboseMode)
        qDebug()  << "ModbusEncoderDecoder::onTmrDataHolderProcessingTimeOut() myparams.isDecoderBusy=" << myparams.isDecoderBusy;

    if(!myparams.isDecoderBusy)
        return;

    //it tries to receive data from the Data Holder
    //it cant be here if dataFromDataHolder is full

    const int zbyrlen = myparams.zbyratoRmessageTags.size();
    const int dhlen = myparams.messageTags.size();

    const int buflen = myparams.dataFromDataHolder.size();

    if(verboseMode)
        qDebug()  << "ModbusEncoderDecoder::onTmrDataHolderProcessingTimeOut() " << zbyrlen << dhlen << buflen;


    const QList<QString> lk = myparams.messageTags.keys();


    if( (zbyrlen + buflen) >= dhlen){
        //all commands to zbyrator were sent, so Data holder client must be checked
        //or some commands were sent
        //        const QString ni = QString::number(myparams.lastmessageparams.devaddress);
        QString ni = myparams.myDevices.value(myparams.lastmessageparams.devaddress).ni;//, QString::number(myparams.lastmessageparams.devaddress));
        QString sn = myparams.myDevices.value(myparams.lastmessageparams.devaddress).sn; //if empty ignore, in other case use sn to find data


        for(int i = 0, imax = lk.size(); i < imax; i++){
            const QString messagetag = lk.at(i);
            const quint8 pollCode = myparams.messageTags.value(messagetag);

            if(myparams.dataFromDataHolder.contains(pollCode))
                continue;//I have data

            emit sendCommand2dataHolderWOObjectTag(pollCode, sn.isEmpty() ? ni : sn, !sn.isEmpty(), messagetag);


        }


    }else{
        //ask zbyrator for more data
        for(int i = 0, imax = lk.size(); i < imax; i++){
            const QString messagetag = lk.at(i);
            const quint8 pollCode = myparams.messageTags.value(messagetag);

            if(myparams.dataFromDataHolder.contains(pollCode))
                continue;//I have data
            startZbyratorPoll(messagetag);//It starts poll, if it didn't do it
        }


    }
    if(verboseMode)
        qDebug()  << "ModbusEncoderDecoder::onTmrDataHolderProcessingTimeOut startTmrDataHolderProcessing 1111";

    emit startTmrDataHolderProcessing(555);

}

//--------------------------------------------------------------------

void ModbusEncoderDecoder::onDataHolderCommandReceived(QString messagetag, bool isok, QString messageerror)
{
    //answer sendCommand2dataHolderWOObjectTag, accepted
    //it is acknowledgement
    if(isok){
        //good
        if(verboseMode)
            qDebug()  << "ModbusEncoderDecoder::onDataHolderCommandReceived ok " << isok << messagetag << messageerror;

        return;
    }

    //    if(!myparams.messageTags.contains(messagetag))
    //        return;//that is not mine

    if(verboseMode)
        qDebug()  << "ModbusEncoderDecoder::onDataHolderCommandReceived failed " << isok << messagetag << messageerror;
    //force zbyrator to start poll, if It stil didn't do

    startZbyratorPoll(messagetag);

}

//--------------------------------------------------------------------

void ModbusEncoderDecoder::dataFromCache(QString messagetag, QVariantHash lastHash)
{
    //data from DataHolder
    if(myparams.messageTags.contains(messagetag)){
        const qint64 msec = lastHash.value("msec").toLongLong();
        const QString ni = lastHash.value("NI").toString();
        const QString sn = lastHash.value("SN").toString();

        const qint64 currmsec = QDateTime::currentMSecsSinceEpoch();
        //force zbyrator to poll
        if(verboseMode)
            qDebug()  << "ModbusEncoderDecoder::dataFromCache msec " << ni << sn << messagetag
                      << QDateTime::fromMSecsSinceEpoch(currmsec).toString("yyyy-MM-dd hh:mm:ss.zzz")
                      << QDateTime::fromMSecsSinceEpoch(msec).toString("yyyy-MM-dd hh:mm:ss.zzz");

        //the average delay was 8 seconds
        bool add2dataHolder;
        if(!isCachedDataAcceptable(lastHash, false, add2dataHolder)){

            if(add2dataHolder){
                const quint8 pollCode = lastHash.value("pollCode").toUInt();
                cachedDataHolderAnswers.insert(pollCode, lastHash);
            }


            //force zbyrator to poll
            startZbyratorPoll(messagetag);

            return;
        }

        if(checkSendDataToTheMaster()){
            emit append2textLog("dataFromCache checkSendDataToTheMaster");
            return;//everything is fine, it is ready to process the next request
        }
    }
}

//--------------------------------------------------------------------

void ModbusEncoderDecoder::onMatildaCommandReceived(QString messagetag, bool isok, QString messageerror)
{
    //answer sendCommand2dataHolderWOObjectTag, accepted
    //it is acknowledgement
    if(isok){
        //good
        if(verboseMode)
            qDebug()  << "ModbusEncoderDecoder::onMatildaCommandReceived ok " << isok << messagetag << messageerror;

        return;
    }

    if(!myparams.zbyratoRmessageTags.contains(messagetag))
        return;//that is not mine

    myparams.zbyratoRmessageTags.remove(messagetag);//try again later

    if(verboseMode)
        qDebug()  << "ModbusEncoderDecoder::onMatildaCommandReceived failed " << isok << messagetag << messageerror;
    //force zbyrator to start poll, if It stil didn't do
    // use  onTmrDataHolderProcessingTimeOut to do it


}

//--------------------------------------------------------------------

void ModbusEncoderDecoder::reloadAllSettings()
{
    onModbusSettingsChanged();
    onMeterListChanged();//it uses general settings

}

//--------------------------------------------------------------------

void ModbusEncoderDecoder::onMeterListChanged()
{
    //it calls when any meter list changed
    //you need to add a fileWatcher for all the meters settings files
    myparams.myDevices.clear();

    const QBitArray srcDevices = ConvertAtype::uint8ToBitArray(myparams.generalSettings.devSrc);

    QStringList listDevicesCount;


    if(srcDevices.at(UC_METER_ELECTRICITY)){
        const int counter = ModbusElectricityMeterHelper::getAcceptableEMeterNis(myparams.myDevices);
        listDevicesCount.append(QString("%1 meters - '%2'").arg(QString("electricity")).arg(counter));
    }


    if(srcDevices.at(UC_METER_WATER)){
        const int counter = ModbusWaterMeterHelper::getAcceptableWMeterNis(myparams.myDevices);
        listDevicesCount.append(QString("%1 meters - '%2'").arg(QString("water")).arg(counter));

    }

//    ModbusGasMeterHelper::getAcceptableGMeterNis(myparams.myDevices);

    //ewgMeterCounter
    if(srcDevices.at(UC_METER_PULSE)){
        const int counter = ModbusPulseMeterHelper::getAcceptablePMeterNis(myparams.myDevices);
        listDevicesCount.append(QString("%1 meters - '%2'").arg(QString("pulses")).arg(counter));

    }


    const int ftablecounter = ModbusSettingsLoader::insertModbusForwardingTable(myparams.myDevices);//it must be the last to overwrite any existing address

    if(ftablecounter > 0){
        listDevicesCount.append(QString("forwarding table devices - '%1'").arg(ftablecounter));
    }

    if(myparams.myDevices.isEmpty())
        listDevicesCount.append(QString("There is no any acceptable modbus address"));

    emit append2textLog(QString("Total device count - '%1', %2")
                        .arg(myparams.myDevices.size())
                        .arg(listDevicesCount.join(", ")));


    //NI to addres





}

//--------------------------------------------------------------------

void ModbusEncoderDecoder::onModbusSettingsChanged()
{
    myparams.generalSettings = ModbusSettingsLoader::getModbusGeneralSettings();

    if(myparams.generalSettings.defCacheTsec < MYDECODER_CACHE_MINIMUM_MSEC)
        myparams.generalSettings.defCacheTsec = MYDECODER_CACHE_MINIMUM_MSEC;


}

//--------------------------------------------------------------------

bool ModbusEncoderDecoder::isStartRegisterGood(const quint16 &startRegister)
{
    return (isVoltageRegister(startRegister) ||
            isEnergyRegister(startRegister) ||
            isWaterTotalRegister(startRegister) ||
            isGasTotalRegister(startRegister) ||
            isPulseTotalRegister(startRegister));
}

//--------------------------------------------------------------------

bool ModbusEncoderDecoder::isVoltageRegister(const quint16 &startRegister)
{
    return (startRegister >= MYDECODER_VOLTAGE_REGISTER_FIRST && startRegister <= MYDECODER_READF_LAST_VREGISTER);
}

//--------------------------------------------------------------------

bool ModbusEncoderDecoder::isEnergyRegister(const quint16 &startRegister)
{
    return (startRegister >= MYDECODER_TOTALENERGY_REGISTER_FIRST && startRegister <=  MYDECODER_READF_LAST_EREGISTER);
}

//--------------------------------------------------------------------

bool ModbusEncoderDecoder::isWaterTotalRegister(const quint16 &startRegister)
{
    return (startRegister >= MYDECODER_TOTALWATER_REGISTER_FIRST && startRegister <=  MYDECODER_READF_LAST_WREGISTER);

}

//--------------------------------------------------------------------

bool ModbusEncoderDecoder::isGasTotalRegister(const quint16 &startRegister)
{
    return (startRegister >= MYDECODER_TOTALGAS_REGISTER_FIRST && startRegister <=  MYDECODER_READF_LAST_GREGISTER);

}

//--------------------------------------------------------------------

bool ModbusEncoderDecoder::isPulseTotalRegister(const quint16 &startRegister)
{
    return (startRegister >= MYDECODER_TOTALPULSE_REGISTER_FIRST && startRegister <=  MYDECODER_READF_LAST_PREGISTER);

}

//--------------------------------------------------------------------

void ModbusEncoderDecoder::findData4theseRegister(const quint16 &startRegister, const quint16 &count)
{
    //registers range must be checked before

    myparams.lastPollCodes2send.clear();
    myparams.lastPollCodes2receive.clear();
    myparams.lastStartRegister = startRegister;
    myparams.lastRegisterCount = count;

    myparams.messageTags.clear();

    //#define MYDECODER_VOLTAGE_REGISTER_FIRST        MYDECODER_READF_FIRST_REGISTER
    //#define MYDECODER_TOTALENERGY_REGISTER_FIRST    MYDECODER_READF_FIRST_REGISTER + 28 //40029


    //#define MYDECODER_READF_FIRST_VREGISTER MYDECODER_VOLTAGE_REGISTER_FIRST
    //#define MYDECODER_READF_LAST_VREGISTER  40039

    //#define MYDECODER_READF_FIRST_EREGISTER MYDECODER_TOTALENERGY_REGISTER_FIRST
    //#define MYDECODER_READF_LAST_EREGISTER  40240


    if(isVoltageRegister(startRegister)){
        myparams.lastPollCodes2receive.append(POLL_CODE_READ_VOLTAGE);
        //get voltage
    }

    if(isEnergyRegister(startRegister)){
        myparams.lastPollCodes2receive.append(POLL_CODE_READ_TOTAL);
    }

    if(isWaterTotalRegister(startRegister)){
        myparams.lastPollCodes2receive.append(POLL_CODE_WTR_TOTAL);
    }

    if(isGasTotalRegister(startRegister)){
        myparams.lastPollCodes2receive.append(POLL_CODE_GAS_TOTAL);
    }
    if(isPulseTotalRegister(startRegister)){
        myparams.lastPollCodes2receive.append(POLL_CODE_PLSS_TOTAL);
    }

    if(myparams.lastPollCodes2receive.isEmpty()){
        sendErrorCodeAndResetTheState(MODBUS_ERROR_ILLEGAL_DATA_ADDR);
        return;
    }

    myparams.isWaitingDataHolder = true;
    //    QString ni = QString::number(myparams.lastmessageparams.devaddress);
    QString ni = myparams.myDevices.value(myparams.lastmessageparams.devaddress).ni;//, QString::number(myparams.lastmessageparams.devaddress));
    QString sn = myparams.myDevices.value(myparams.lastmessageparams.devaddress).sn; //if empty ignore, in other case use sn to find data

    //    bool hasCachedDataAcceptable = false;

    for(int i = 0, imax = myparams.lastPollCodes2receive.size(); i < imax; i++){

        const quint8 pollCode = myparams.lastPollCodes2receive.at(i);

        const QString messagetag = QString("%1 %2 %3 %4 %5")
                .arg(myparams.messageCounter).arg(QTime::currentTime().msec()).arg(startRegister).arg(count).arg(int(pollCode));

        myparams.messageTags.insert(messagetag, pollCode);

        if(verboseMode)
            qDebug() << "sendCommand2dataHolderWOObjectTag " << pollCode << ni << sn << int(myparams.lastmessageparams.devaddress) << messagetag;



        emit sendCommand2dataHolderWOObjectTag(pollCode, sn.isEmpty() ? ni : sn, !sn.isEmpty(), messagetag);

        //add some time btwn requests
        //        QThread::msleep(111);
    }

    //so wait for the answer from the Data Holder

    //    if(hasCachedDataAcceptable && checkSendDataToTheMaster()){

    //        return;
    //    }
    //in other case never mind, it will receive data from the data holder or it will send message SLave internal failure

}

//--------------------------------------------------------------------

void ModbusEncoderDecoder::sendErrorCodeAndResetTheState(const quint8 &errorCode)
{
    Q_UNUSED(errorCode);//do not send errors, it stops the exchange
    //    emit onData2write(getErrorMessageArray(myparams.lastmessageparams, errorCode));
    resetLastMessageVariables();


}

//--------------------------------------------------------------------

bool ModbusEncoderDecoder::isReadFunctionOk(const ModbusDecodedParams &messageparams, quint16 &firstRegister, quint16 &registerCount)
{

    const int listlen = messageparams.byteslist.size();

    //RTU read 01 03 00 00 00 62 C4 23
    //TCP read 0001 0000 0006 01  03  00 00  00 62
    const int listlencheck = (messageparams.modbusmode == MODBUS_MODE_RTU) ? 8 : 12;
    if(listlen != listlencheck)
        return false;

    const int minusindx = (messageparams.modbusmode == MODBUS_MODE_RTU) ? 4 : 2;

    firstRegister = quint16(getNumberFromTheList(messageparams.byteslist, listlencheck - 2 - minusindx)) + quint16(MYDECODER_READF_FIRST_REGISTER);
    registerCount = quint16(getNumberFromTheList(messageparams.byteslist, listlencheck - minusindx));

    return true;
}


//--------------------------------------------------------------------

bool ModbusEncoderDecoder::isCachedDataAcceptable(const QVariantHash &lastHash, const bool &ignoreMsec, bool &add2dataHolder)
{

    add2dataHolder = false;
    //I'm sure that it is my tag
    //    const QVariantList varlist = lastHash.value("data").toHash().value("varlist").toList();

    //    if(varlist.isEmpty()){
    //        //force zbyrator to poll

    //        return false;
    //    }

    //it must contain one record
    const QVariantHash h = lastHash;
    const quint8 pollCode = h.value("pollCode").toUInt();
    const QString ni = h.value("NI").toString();
    const QString sn = h.value("SN").toString();

    const QString nimapped = myparams.listDevAddr2meterNI.value(myparams.lastmessageparams.devaddress, QString::number(myparams.lastmessageparams.devaddress));

    if(!myparams.lastPollCodes2receive.contains(pollCode) || ni.isEmpty() || myparams.myDevices.contains(ni)){ //  nimapped != ni){
        //force zbyrator to poll

        return false;
    }

    add NI and SN checker

    const qint64 msec = h.value("msec").toLongLong();
    const qint64 currmsec = QDateTime::currentMSecsSinceEpoch();
    const qint64 msecdiff = currmsec - msec;

    const quint32 msecdiffallowed = myparams.generalSettings.cacheTsec.value(pollCode, myparams.generalSettings.defCacheTsec);// 300000; //5 minutes
//    IF(msecdiffallowed) less than MYDECODER_CACHE_MINIMUM_MSEC, it is unlimited

    if(!ignoreMsec && msecdiffallowed >= MYDECODER_CACHE_MINIMUM_MSEC && quint32(qAbs(msecdiff)) > msecdiffallowed){
        add2dataHolder = (!myparams.lastPollCodes2send.contains(pollCode));//only if msec is bad


        //add2dataHolder - true - received from DataHolderClient, false - internal cache

        //force zbyrator to poll
        if(verboseMode)
            qDebug()  << "ModbusEncoderDecoder::isCachedDataAcceptable msec "
                      << QDateTime::fromMSecsSinceEpoch(currmsec).toString("yyyy-MM-dd hh:mm:ss.zzz")
                      << QDateTime::fromMSecsSinceEpoch(msec).toString("yyyy-MM-dd hh:mm:ss.zzz");

        if(verboseMode)
            qDebug()  << "ModbusEncoderDecoder::isCachedDataAcceptable msec diff " << msecdiff << msecdiffallowed << add2dataHolder << lastHash;


        return false;
    }

    cachedDataHolderAnswers.remove(pollCode);//it must remove acceptable values

    if(myparams.lastPollCodes2send.contains(pollCode)){
        //wtf??? it can't be, but check to be sure
        if(verboseMode)
            qDebug()  << "ModbusEncoderDecoder::isCachedDataAcceptable " << lastHash << myparams.lastPollCodes2send;
        return true;//say that everything is fine
    }



    myparams.lastPollCodes2send.append(pollCode);

    myparams.dataFromDataHolder.insert(pollCode, h);


    return true;
}

//--------------------------------------------------------------------

bool ModbusEncoderDecoder::checkSendDataToTheMaster()
{
    if(!myparams.lastPollCodes2receive.isEmpty() && myparams.lastPollCodes2receive.size() == myparams.lastPollCodes2send.size() ){
        //it is time to send data to the master

        sendDataToTheMaster();
        resetLastMessageVariables();
        return true;

    }
    return false;
}

//--------------------------------------------------------------------

void ModbusEncoderDecoder::sendDataToTheMaster()
{

    if(verboseMode)
        qDebug()  << "sendDataToTheMaster stopTmrProcessing ";
    emit stopTmrProcessing();//to prevent bad situations

    QMap<quint16, quint16> mapRegisters;


    for(int i = 0, imax = myparams.lastPollCodes2send.size(); i < imax; i++){
        fillTheAnswerHash(myparams.lastPollCodes2send.at(i), mapRegisters);
    }

    ModbusList answer2master;
    quint16 regs = myparams.lastStartRegister;

    for(int i = 0, imax = myparams.lastRegisterCount; i < imax; i++, regs++){
        const quint16 value = mapRegisters.value(regs, 0);
        addDivided2thelist(answer2master, value);
    }



    answer2master.prepend(quint8(answer2master.size()));//len
    answer2master.prepend(myparams.lastmessageparams.functionCode);
    answer2master.prepend(myparams.lastmessageparams.devaddress);

    if(myparams.lastmessageparams.modbusmode == MODBUS_MODE_TCP){
        const MODBUSDIVIDED_UINT16 len = getDivided(answer2master.size());
        answer2master.prepend(len.lowbyte);
        answer2master.prepend(len.hightbyte);

        const MODBUSDIVIDED_UINT16 protocolid = getDivided(myparams.lastmessageparams.protocolid);
        answer2master.prepend(protocolid.lowbyte);
        answer2master.prepend(protocolid.hightbyte);

        const MODBUSDIVIDED_UINT16 transactionid = getDivided(myparams.lastmessageparams.transactionid);
        answer2master.prepend(transactionid.lowbyte);
        answer2master.prepend(transactionid.hightbyte);

    }else{
        const MODBUSDIVIDED_UINT16 crc = getCrc16(answer2master);
        answer2master.append(crc.hightbyte);
        answer2master.append(crc.lowbyte);
    }

    const QByteArray writeArr = ConvertAtype::uint8list2array(answer2master, 0, answer2master.length());//is ready to send message

    emit onData2write(writeArr);
}

//--------------------------------------------------------------------

void ModbusEncoderDecoder::resetLastMessageVariables()
{
    if(verboseMode)
        qDebug()  << "resetLastMessageVariables stopTmrProcessing ";

    emit stopTmrProcessing();

    myparams.isDecoderBusy = false;
    myparams.lastmessageparams = ModbusDecodedParams();
    myparams.lastPollCodes2receive.clear();
    myparams.lastPollCodes2send.clear();
    myparams.messageTags.clear();
    myparams.isWaitingDataHolder = false;
    myparams.dataFromDataHolder.clear();
    myparams.zbyratoRmessageTags.clear();
    cachedDataHolderAnswers.clear();

}


//--------------------------------------------------------------------
void ModbusEncoderDecoder::fillTheAnswerHash(const quint8 &pollCode, QMap<quint16, quint16> &mapRegisters)
{

    ModbusAnswerList l;
    const QVariantHash h = myparams.dataFromDataHolder.value(pollCode);

    quint16 startRegister = 0;

    switch(pollCode){
    case POLL_CODE_READ_VOLTAGE : l = getVoltageAnswer(h)       ; startRegister = MYDECODER_READF_FIRST_VREGISTER; break;
    case POLL_CODE_READ_TOTAL   : l = getTotalEnergyAnswer(h)   ; startRegister = MYDECODER_READF_FIRST_EREGISTER; break;
    case POLL_CODE_WTR_TOTAL    : l = getTotalWaterAnswer(h)    ; startRegister = MYDECODER_READF_FIRST_WREGISTER; break;
    case POLL_CODE_GAS_TOTAL    : l = getTotalGasAnswer(h)      ; startRegister = MYDECODER_READF_FIRST_GREGISTER; break;
    case POLL_CODE_PLSS_TOTAL   : l = getTotalPulsesAnswer(h)   ; startRegister = MYDECODER_READF_FIRST_PREGISTER; break;


    }


    for(int i = 0, imax = l.size(); i < imax; i++, startRegister++){
        mapRegisters.insert(startRegister, l.at(i));
    }


}

//--------------------------------------------------------------------

void ModbusEncoderDecoder::startZbyratorPoll(const QString &messagetag)
{
    if(verboseMode)
        qDebug()  << "ModbusEncoderDecoder::startZbyratorPoll " << messagetag << myparams.zbyratoRmessageTags.contains(messagetag) << myparams.messageTags.contains(messagetag);
    if(myparams.zbyratoRmessageTags.contains(messagetag) || !myparams.messageTags.contains(messagetag))
        return;//it was done before

    const quint8 pollCode = myparams.messageTags.value(messagetag);

    myparams.zbyratoRmessageTags.insert(messagetag, pollCode);

    QString ni = myparams.myDevices.value(myparams.lastmessageparams.devaddress).ni;//, QString::number(myparams.lastmessageparams.devaddress));
    QString sn = myparams.myDevices.value(myparams.lastmessageparams.devaddress).sn; //if empty ignore, in other case use sn to find data

    if(verboseMode)
        qDebug()  << "ModbusEncoderDecoder::startZbyratorPoll " << ni << sn << int(pollCode) << QString::number(myparams.lastmessageparams.devaddress) << messagetag;

    emit sendCommand2zbyrator(pollCode, ni, sn, messagetag);
}

//--------------------------------------------------------------------

ModbusAnswerList ModbusEncoderDecoder::getVoltageAnswer(const QVariantHash &h)
{
    return ModbusElectricityMeterHelper::getVoltageAnswer(h.value("data").toHash(), verboseMode);
}

//--------------------------------------------------------------------

ModbusAnswerList ModbusEncoderDecoder::getTotalEnergyAnswer(const QVariantHash &h)
{
    return ModbusElectricityMeterHelper::getTotalEnergyAnswer(h.value("data").toHash(), verboseMode);
}

//--------------------------------------------------------------------

ModbusAnswerList ModbusEncoderDecoder::getTotalWaterAnswer(const QVariantHash &h)
{
    return ModbusWaterMeterHelper::getTotalWaterAnswer(h.value("data").toHash(), verboseMode);

}

//--------------------------------------------------------------------

ModbusAnswerList ModbusEncoderDecoder::getTotalGasAnswer(const QVariantHash &h)
{
    return ModbusGasMeterHelper::getTotalGasAnswer(h.value("data").toHash(), verboseMode);

}

//--------------------------------------------------------------------

ModbusAnswerList ModbusEncoderDecoder::getTotalPulsesAnswer(const QVariantHash &h)
{
    return ModbusPulseMeterHelper::getTotalPulseAnswer(h.value("data").toHash(), verboseMode);

}

//--------------------------------------------------------------------
