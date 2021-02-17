#include "modbusencoderdecoder.h"



///[!] type-converter
#include "src/base/convertatype.h"
#include "src/base/prettyvalues.h"

#include "definedpollcodes.h"

#define MYDECODER_READF_FIRST_REGISTER  40001
#define MYDECODER_READF_LAST_REGISTER   40068

#define MYDECODER_VOLTAGE_REGISTER_FIRST        MYDECODER_READF_FIRST_REGISTER
#define MYDECODER_TOTALENERGY_REGISTER_FIRST    MYDECODER_READF_FIRST_REGISTER + 28 //40029


//--------------------------------------------------------------------

ModbusEncoderDecoder::ModbusEncoderDecoder(const quint8 &modbusDecoderMode, const bool &isModbusMasterSide, QObject *parent) :
    ModbusMessanger(parent)
{
    setModbusMode(modbusDecoderMode);
    setModbusSide(isModbusMasterSide);
#ifdef __x86_64
    //for test only
    myparams.listMeterNIs.append(1);
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
    myparams.isDecoderBusy = true;
    myparams.busyCounter = 0;
    myparams.processintTimeoutCounter = 0;
    myparams.lastmessageparams = lastmessageparams;
    restartTimerProcessing();




    //registers check
    if(lastmessageparams.functionCode == MODBUS_READFUNCTION){//read function has fixed length

        quint16 startRegister = 0;
        quint16 registerCount = 0;

        if(!isReadFunctionOk(lastmessageparams, startRegister, registerCount)){
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
    connect(timer, &QTimer::timeout, this, &ModbusEncoderDecoder::onTmrProcessingTimeOut);

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

void ModbusEncoderDecoder::onDataHolderCommandReceived(quint16 messagetag, bool isok, QString messageerror)
{
    if(isok){
        //good
        return;
    }


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
    for(int i = 0, imax = myparams.lastPollCodes2receive.size(); i < imax; i++){

        const quint8 pollCode = myparams.lastPollCodes2receive.at(i);

        const QString messagetag = QString("%1 %2 %3 %4 %5")
                .arg(myparams.messageCounter).arg(QTime::msec()).arg(startRegister).arg(count).arg(int(pollCode));

        myparams.messageTags.append(messagetag);
        emit sendCommand2dataHolderWOObjectTag(pollCode, ni, messagetag);

        if((i + 1) >= imax)
            break;

        QThread::msleep(111);
    }

    //so wait for the answer from the Data Holder


}

//--------------------------------------------------------------------

void ModbusEncoderDecoder::sendErrorCodeAndResetTheState(const quint8 &errorCode)
{
    emit onData2write(getErrorMessageArray(myparams.lastmessageparams, errorCode));
    myparams.isDecoderBusy = false;
    myparams.lastmessageparams = ModbusDecodedParams();
    emit stopTmrProcessing();

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

    firstRegister = quint16(getNumberFromTheList(messageparams.byteslist, listlencheck - 4)) + quint16(MYDECODER_READF_FIRST_REGISTER);
    registerCount = quint16(getNumberFromTheList(messageparams.byteslist, listlencheck - 2));

    return true;
}

//--------------------------------------------------------------------

QVariantHash ModbusEncoderDecoder::getHash4pollCodeAndDevAddr(const quint8 &pollCode, const quint8 &devaddr)
{
    //steps
    // 1 - try to find fresh data in the Data Holder cache
    // 2 - if data fresh send it to the master, othervise ask matilda-bbb to start poll
    // 3 - wait for poll data or timeout (9 seconds is the maximum)
    // 4 - if timeout happens - send error Slave Device Busy, forget about previous request
    // 5 - if fresh data is received send it

//    const QString params = modbusprocessor->generateQuickPollLine(readArr);
//    if(params.isEmpty()){
//        if(verboseMode)
//            qDebug() << "generateQuickPollLine bad request" << readArr.toHex() ;
//        //QVH
////        h.insert("c", (indx < 0) ? "" : l.at(indx));//name of the command  ui->cbDevCommand->currentData(Qt::UserRole).toString());
////        h.insert("pc", command);//pollCode
////        h.insert("d", args);// ui->leDevCommand->text().simplified().trimmed());


////        QVariantHash h;
////        h.insert("c", command);
////        h.insert("d", args);
////        if(verboseMode)
////            qDebug() << "UcCommandServer::command4dev " << command << args;
////        emit command2extension(MTD_EXT_NAME_ZBYRATOR, MTD_EXT_CUSTOM_COMMAND_0, h);

//        QVariantHash pollargs;
//        pollargs.insert("c", int(pollCode));
//        pollargs.insert("pc", int(pollCode));
//        pollargs.insert("d", QString("-n %1 -i").arg(QString::number(devaddr)));

//        pollargs.insert("d", "");
//        emit sendCommand2zbyrator(pollargs, );


//        return;//write error
//    }

}

//--------------------------------------------------------------------
