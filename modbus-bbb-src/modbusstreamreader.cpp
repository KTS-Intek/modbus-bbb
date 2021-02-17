#include "modbusstreamreader.h"

#include "embeelimits.h"

//----------------------------------------------------------------------

ModbusStreamReader::ModbusStreamReader(const quint16 &objecttag, const bool &isTcpMode, const bool &verboseMode, QObject *parent) :
    Conn2modem(0, verboseMode, parent)
{
    myparams.isTcpMode = isTcpMode;
    myparams.objecttag = objecttag;
}

//----------------------------------------------------------------------

void ModbusStreamReader::createObjects()
{
//    createDevices(); it calls in the constructor

//    activateAsyncModeExt( myparams.isTcpMode ? IFACECONNTYPE_TCPCLNT : IFACECONNTYPE_UART );
    if(myparams.isTcpMode){
        modbusprocessor = new ModbusEncoderDecoder(MODBUS_MODE_RTU, false, this);
        connect(socket, SIGNAL(readyRead()), this, SLOT(mReadyRead()) );
    }else{
        modbusprocessor = new ModbusEncoderDecoder(MODBUS_MODE_TCP, false, this);
        connect(serialPort, SIGNAL(readyRead()), this, SLOT(mReadyRead()) );
    }

    connect(modbusprocessor, &ModbusEncoderDecoder::sendCommand2zbyrator, this, &ModbusStreamReader::onSendCommand2zbyrator);
    connect(modbusprocessor, &ModbusEncoderDecoder::sendCommand2dataHolderWOObjectTag, this, &ModbusStreamReader::sendCommand2dataHolderWOObjectTag);

}

//----------------------------------------------------------------------

void ModbusStreamReader::sendCommand2dataHolderWOObjectTag(quint16 pollCode, QString ni, quint16 messagetag)
{
    emit sendCommand2dataHolder(pollCode, ni, messagetag, myparams.objecttag);
}

//----------------------------------------------------------------------

void ModbusStreamReader::onSendCommand2zbyrator(QVariantHash hash, quint16 messagetag)
{
    emit sendCommand2zbyrator(hash, messagetag, myparams.objecttag);
}

//----------------------------------------------------------------------

void ModbusStreamReader::onCommandReceived(quint16 messagetag, quint16 objecttag, bool isok, QString messageerror)
{

    if(objecttag == myparams.objecttag){
        modbusprocessor->onDataHolderCommandReceived(messagetag, isok, messageerror);

    }
}

void ModbusStreamReader::onData2write(QByteArray writearr)
{
    write2dev(writearr);
}

//----------------------------------------------------------------------

void ModbusStreamReader::mReadyReadUART()
{
    disconnect(serialPort, SIGNAL(readyRead()), this, SLOT(mReadyRead()) );
    const QByteArray readArr = readFromTheDevice();
    connect(serialPort, SIGNAL(readyRead()), this, SLOT(mReadyRead()) );

    decodeArray(readArr);

}

//----------------------------------------------------------------------

void ModbusStreamReader::mReadyReadTCP()
{
    disconnect(socket, SIGNAL(readyRead()), this, SLOT(mReadyRead()) );
    const QByteArray readArr = readFromTheDevice();
    connect(socket, SIGNAL(readyRead()), this, SLOT(mReadyRead()) );
    decodeArray(readArr);


}

//----------------------------------------------------------------------

void ModbusStreamReader::decodeArray(const QByteArray &readArr)
{
    modbusprocessor->canProcessTheLine(readArr);




}

//----------------------------------------------------------------------

QByteArray ModbusStreamReader::readFromTheDevice()
{
    QByteArray readArr;

    if(isConnectionWorking()){
        QTime globalTime;
        globalTime.start();

        const int globalTimeMax = 1000;
        const qint32 timeOut_c = qMax(20, 100);
        readArr = readAll();// ->readAll();

        QTime time;
        time.start();

        //MeterPlugin *plg = (!model2checkCrc.isEmpty() && model2plugin.contains(model2checkCrc)) ? model2plugin.value(model2checkCrc, 0) : 0;

        const int minReadLen = 5;
        bool isReady2readPlg = false;//checkCrcAllow


        bool readArrHasNewData = !readArr.isEmpty();//it means that readArr has new data (any data = !isEmpty)



        for(int counter = 0; counter < MAX_READ_TRYES_FROM_UART && (readArr.isEmpty() || time.elapsed() < timeOut_c) && globalTime.elapsed() < globalTimeMax; counter++){

            if(readArrHasNewData){
                readArrHasNewData = false;


                const int readArrLen = readArr.length();

                if(readArrLen > MAX_READ_FROM_UART){
                    incrementApiErrCounter();
                    break;
                }

                if(!isReady2readPlg && minReadLen > 0 && readArrLen >= minReadLen){
                    isReady2readPlg = true;
                }

                if(isReady2readPlg && isThisYourRead(readArr)){// plg->isItYourRead(readArr).value("Tak", false).toBool())
                    break;
                }
            }

            if(!isConnectionWorking())
                break;

            if(waitForReadyRead(timeOut_c)){
                int loops = 0;
                for(int i = 0; i < 35; i++, loops++){
                    readArr.append(readAll());
                    QThread::msleep(10);
                    if(!waitForReadyRead(20))
                        break;
                }
                //                readArr.append(lastIface.confModemHelper->readAll());
                readArrHasNewData = true;

                time.restart();
            }
        }

//        if(!readArr.isEmpty())
//            onDataFromIface(readArr, uartIsBusy);

    }

    return readArr;

}

//----------------------------------------------------------------------

bool ModbusStreamReader::isThisYourRead(const QByteArray &readArr)
{
    ModbusDecodedParams lastmessageparams;
    return modbusprocessor->isMessageReadingFinished(readArr, lastmessageparams);
}


//----------------------------------------------------------------------

