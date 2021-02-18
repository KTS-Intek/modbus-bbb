#include "modbusencoderdecoder.h"



///[!] type-converter
#include "src/base/convertatype.h"
#include "src/base/prettyvalues.h"

#include "definedpollcodes.h"

#define MYDECODER_READF_FIRST_REGISTER  40001
#define MYDECODER_READF_LAST_REGISTER   40068

#define MYDECODER_VOLTAGE_REGISTER_FIRST        MYDECODER_READF_FIRST_REGISTER
#define MYDECODER_TOTALENERGY_REGISTER_FIRST    MYDECODER_READF_FIRST_REGISTER + 40 //40041


//--------------------------------------------------------------------

ModbusEncoderDecoder::ModbusEncoderDecoder(const quint8 &modbusDecoderMode, const bool &isModbusMasterSide, QObject *parent) :
    ModbusMessanger(parent)
{
    setModbusMode(modbusDecoderMode);
    setModbusSide(isModbusMasterSide);
#ifdef __x86_64
    //for test only
    onMeterListChanged();
#endif

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
    const ModbusList list = getErrorMessage(messageparams, errorcode);
    return  ConvertAtype::uint8list2array(list, 0, list.length());//is ready to send message

}

//--------------------------------------------------------------------

bool ModbusEncoderDecoder::isMessageReadingFinished(const QByteArray &readArr, ModbusDecodedParams &messageparams)
{
    bool r = false;
    messageparams = ModbusDecodedParams();
    if(readArr.isEmpty() || readArr.length() < 3)
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


    if(!myparams.listMeterNIs.contains(lastmessageparams.devaddress))
        return;//it is not my meter, this request is not to me

    if(myparams.isDecoderBusy){
        emit onData2write(getErrorMessageArray(lastmessageparams, MODBUS_ERROR_SLAVE_DEVICE_BUSY));
        if(myparams.busyCounter > 0xFFF0){
            qDebug() << "ModbusEncoderDecoder::canProcessTheLine WTF? " ;
            emit startTmrProcessing(1);//try to restart the timer
            myparams.busyCounter = 0;
        }else{
            myparams.busyCounter++;
        }
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



        if( (startRegister < MYDECODER_READF_FIRST_REGISTER) ||
                ((startRegister + registerCount) > MYDECODER_READF_LAST_REGISTER) ||
                (startRegister > MYDECODER_READF_LAST_REGISTER) ||
                ((startRegister + registerCount) < MYDECODER_READF_FIRST_REGISTER)){
            sendErrorCodeAndResetTheState(MODBUS_ERROR_ILLEGAL_DATA_VALUE);
            return;
        }

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
    timer->setInterval(7777);
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
    timerDataHolder->setInterval(1111);
    timerDataHolder->setSingleShot(true);
    connect(this, SIGNAL(startTmrDataHolderProcessing(int)), timer, SLOT(start(int)));
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
}

//--------------------------------------------------------------------

void ModbusEncoderDecoder::restartTimerProcessing()
{
    qint32 msec = myparams.processingmsec;
    if(msec < 1111)
        msec = 1111;
    else if(msec > 22222)
        msec = 22222;

    emit startTmrProcessing(msec);
}

//--------------------------------------------------------------------

void ModbusEncoderDecoder::onTmrProcessingTimeOut()
{
    if(!myparams.isDecoderBusy)
        return;


    if(myparams.lastmessageparams.devaddress > 0 && myparams.lastmessageparams.devaddress < 248){
        restartTimerProcessing();
        myparams.processintTimeoutCounter++;
        if(myparams.processintTimeoutCounter > 100){
            sendErrorCodeAndResetTheState(MODBUS_ERROR_ILLEGAL_SLVEDEVFLR);
            return;
        }
        emit onData2write(getErrorMessageArray(myparams.lastmessageparams, MODBUS_ERROR_ACKNOWLEDGE));


    }

}

//--------------------------------------------------------------------

void ModbusEncoderDecoder::onTmrFinitaLaComedia()
{
    if(!myparams.isDecoderBusy)
        return;

    if(myparams.lastmessageparams.devaddress > 0 && myparams.lastmessageparams.devaddress < 248){
        qDebug() << "ModbusEncoderDecoder::onTmrFinitaLaComedia ";//it is processing to long
        sendErrorCodeAndResetTheState(MODBUS_ERROR_ILLEGAL_SLVEDEVFLR);
    }
}

//--------------------------------------------------------------------

void ModbusEncoderDecoder::onTmrDataHolderProcessingTimeOut()
{
    if(!myparams.isDecoderBusy)
        return;

//it tries to receive data from the Data Holder
    //it cant be here if dataFromDataHolder is full

    const int zbyrlen = myparams.zbyratoRmessageTags.size();
    const int dhlen = myparams.messageTags.size();

    const int buflen = myparams.dataFromDataHolder.size();

    qDebug() << "ModbusEncoderDecoder::onTmrDataHolderProcessingTimeOut() " << zbyrlen << dhlen << buflen;


    const QList<QString> lk = myparams.messageTags.keys();


    if( (zbyrlen + buflen) >= dhlen){
        //all commands to zbyrator were sent, so Data holder client must be checked
//or some commands were sent
        const QString ni = QString::number(myparams.lastmessageparams.devaddress);

        for(int i = 0, imax = lk.size(); i < imax; i++){
            const QString messagetag = lk.at(i);
            const quint8 pollCode = myparams.messageTags.value(messagetag);

            if(myparams.dataFromDataHolder.contains(pollCode))
                continue;//I have data

            emit sendCommand2dataHolderWOObjectTag(pollCode, ni, messagetag);


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

    emit startTmrDataHolderProcessing(1111);

}

//--------------------------------------------------------------------

void ModbusEncoderDecoder::onDataHolderCommandReceived(QString messagetag, bool isok, QString messageerror)
{
    //answer sendCommand2dataHolderWOObjectTag, accepted
    //it is acknowledgement
    if(isok){
        //good
        qDebug() << "ModbusEncoderDecoder::onDataHolderCommandReceived ok " << isok << messagetag << messageerror;

        return;
    }

//    if(!myparams.messageTags.contains(messagetag))
//        return;//that is not mine

    qDebug() << "ModbusEncoderDecoder::onDataHolderCommandReceived failed " << isok << messagetag << messageerror;
    //force zbyrator to start poll, if It stil didn't do

    startZbyratorPoll(messagetag);

}

//--------------------------------------------------------------------

void ModbusEncoderDecoder::dataFromCache(QString messagetag, QVariantHash lastHash)
{
    //data from DataHolder
    if(myparams.messageTags.contains(messagetag)){

        if(!isCachedDataAcceptable(lastHash, true)){

            //force zbyrator to poll
            startZbyratorPoll(messagetag);

            return;
        }

        if(checkSendDataToTheMaster()){
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
        qDebug() << "ModbusEncoderDecoder::onMatildaCommandReceived ok " << isok << messagetag << messageerror;

        return;
    }

    if(!myparams.zbyratoRmessageTags.contains(messagetag))
        return;//that is not mine

    myparams.zbyratoRmessageTags.remove(messagetag);//try again later

    qDebug() << "ModbusEncoderDecoder::onMatildaCommandReceived failed " << isok << messagetag << messageerror;
    //force zbyrator to start poll, if It stil didn't do
// use  onTmrDataHolderProcessingTimeOut to do it


}

//--------------------------------------------------------------------

void ModbusEncoderDecoder::onMeterListChanged()
{
    //it calls when the emeter list changed
    myparams.listMeterNIs.clear();
    myparams.listMeterNIs.append(1);

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

    if(startRegister < MYDECODER_TOTALENERGY_REGISTER_FIRST){

        myparams.lastPollCodes2receive.append(POLL_CODE_READ_VOLTAGE);
        //get voltage
    }

    if((startRegister >= MYDECODER_TOTALENERGY_REGISTER_FIRST) || ((startRegister + count) >  MYDECODER_TOTALENERGY_REGISTER_FIRST) ){
        myparams.lastPollCodes2receive.append(POLL_CODE_READ_TOTAL);

    }

    if(myparams.lastPollCodes2receive.isEmpty()){
        sendErrorCodeAndResetTheState(MODBUS_ERROR_ILLEGAL_DATA_ADDR);
        return;
    }

    myparams.isWaitingDataHolder = true;
    QString ni = QString::number(myparams.lastmessageparams.devaddress);
    bool hasCachedDataAcceptable = false;

    for(int i = 0, imax = myparams.lastPollCodes2receive.size(); i < imax; i++){

        const quint8 pollCode = myparams.lastPollCodes2receive.at(i);

        const QString messagetag = QString("%1 %2 %3 %4 %5")
                .arg(myparams.messageCounter).arg(QTime::currentTime().msec()).arg(startRegister).arg(count).arg(int(pollCode));

        myparams.messageTags.insert(messagetag, pollCode);



         const QVariantHash lastHash = cachedDataHolderAnswers.value(pollCode).value(ni);

        if(isCachedDataAcceptable(lastHash, false)){
            hasCachedDataAcceptable = true;
            continue;
        }

        emit sendCommand2dataHolderWOObjectTag(pollCode, ni, messagetag);

      //add some time btwn requests
        QThread::msleep(111);
    }

    //so wait for the answer from the Data Holder

    if(hasCachedDataAcceptable && checkSendDataToTheMaster()){

        return;
    }
    //in other case never mind, it will receive data from the data holder or it will send message SLave internal failure

}

//--------------------------------------------------------------------

void ModbusEncoderDecoder::sendErrorCodeAndResetTheState(const quint8 &errorCode)
{
    emit onData2write(getErrorMessageArray(myparams.lastmessageparams, errorCode));
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

bool ModbusEncoderDecoder::isCachedDataAcceptable(const QVariantHash &lastHash, const bool &add2dataHolder)
{


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

    if(!myparams.lastPollCodes2receive.contains(pollCode) || ni.isEmpty() || QString::number(myparams.lastmessageparams.devaddress) != ni){
        //force zbyrator to poll

        return false;
    }

    const qint64 msec = h.value("msec").toLongLong();
    const qint64 msecdiff = myparams.msecOfTheRequest - msec;
    if(qAbs(msecdiff) > 20000){
        //force zbyrator to poll
        qDebug() << "ModbusEncoderDecoder::isCachedDataAcceptable msec diff " << msecdiff << lastHash;

//        return false;
    }

    if(myparams.lastPollCodes2send.contains(pollCode)){
        //wtf??? it can't be, but check to be sure
        qDebug() << "ModbusEncoderDecoder::isCachedDataAcceptable " << lastHash << myparams.lastPollCodes2send;
        return true;//say that everything is fine
    }



    myparams.lastPollCodes2send.append(pollCode);

    if(add2dataHolder){
        QHash<QString, QVariantHash> ni2data = cachedDataHolderAnswers.value(pollCode);
        ni2data.insert(ni, h);
        cachedDataHolderAnswers.insert(pollCode, ni2data);
    }

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
    emit stopTmrProcessing();

    myparams.isDecoderBusy = false;
    myparams.lastmessageparams = ModbusDecodedParams();
    myparams.lastPollCodes2receive.clear();
    myparams.lastPollCodes2send.clear();
    myparams.messageTags.clear();
    myparams.isWaitingDataHolder = false;
    myparams.dataFromDataHolder.clear();
    myparams.zbyratoRmessageTags.clear();

}


//--------------------------------------------------------------------
void ModbusEncoderDecoder::fillTheAnswerHash(const quint8 &pollCode, QMap<quint16, quint16> &mapRegisters)
{

    ModbusAnswerList l;
    const QVariantHash h = myparams.dataFromDataHolder.value(pollCode);

    quint16 startRegister = 0;

    switch(pollCode){
    case POLL_CODE_READ_VOLTAGE : l = getVoltageAnswer(h)    ; startRegister = MYDECODER_READF_FIRST_REGISTER; break;
    case POLL_CODE_READ_TOTAL   : l = getTotalEnergyAnswer(h); startRegister = MYDECODER_TOTALENERGY_REGISTER_FIRST; break;
    }


    for(int i = 0, imax = l.size(); i < imax; i++, startRegister++){
        mapRegisters.insert(startRegister, l.at(i));
    }


}

//--------------------------------------------------------------------

void ModbusEncoderDecoder::startZbyratorPoll(const QString &messagetag)
{
    if(myparams.zbyratoRmessageTags.contains(messagetag) || !myparams.messageTags.contains(messagetag))
        return;//it was done before

    const quint8 pollCode = myparams.messageTags.value(messagetag);

    myparams.zbyratoRmessageTags.insert(messagetag, pollCode);

    QString ni = QString::number(myparams.lastmessageparams.devaddress);

    emit sendCommand2zbyrator(pollCode, ni, messagetag);
}

//--------------------------------------------------------------------

ModbusAnswerList ModbusEncoderDecoder::getVoltageAnswer(const QVariantHash &h)
{
    return ModbusElectricityMeterHelper::getVoltageAnswer(h.value("data").toHash());
}

//--------------------------------------------------------------------

ModbusAnswerList ModbusEncoderDecoder::getTotalEnergyAnswer(const QVariantHash &h)
{
    return ModbusElectricityMeterHelper::getTotalEnergyAnswer(h.value("data").toHash());
}

//--------------------------------------------------------------------
