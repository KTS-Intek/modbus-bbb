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
    createDevices();

//    activateAsyncModeExt( myparams.isTcpMode ? IFACECONNTYPE_TCPCLNT : IFACECONNTYPE_UART );
    if(myparams.isTcpMode){
        modbusprocessor = new ModbusEncoderDecoder(MODBUS_MODE_RTU, false, this);
        connect(socket, SIGNAL(readyRead()), this, SLOT(mReadyRead()) );
    }else{
        modbusprocessor = new ModbusEncoderDecoder(MODBUS_MODE_TCP, false, this);
        connect(serialPort, SIGNAL(readyRead()), this, SLOT(mReadyRead()) );
    }

    connect(modbusprocessor, &ModbusEncoderDecoder::sendCommand2zbyrator, this, &ModbusStreamReader::onSendCommand2zbyrator);

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
        if(isok){
            //wait for a new data
            //5 Acknowledge, master should wait
        }else{
            //write error
        }
    }
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
    //steps
    // 1 - try to find fresh data in the Data Holder cache
    // 2 - if data fresh send it to the master, othervise ask matilda-bbb to start poll
    // 3 - wait for poll data or timeout (9 seconds is the maximum)
    // 4 - if timeout happens - send error Slave Device Busy, forget about previous request
    // 5 - if fresh data is received send it

    const QString params = modbusprocessor->generateQuickPollLine(readArr);
    if(params.isEmpty()){
        if(verboseMode)
            qDebug() << "generateQuickPollLine bad request" << readArr.toHex() ;
        //QVH
//        h.insert("c", (indx < 0) ? "" : l.at(indx));//name of the command  ui->cbDevCommand->currentData(Qt::UserRole).toString());
//        h.insert("pc", command);//pollCode
//        h.insert("d", args);// ui->leDevCommand->text().simplified().trimmed());


//        QVariantHash h;
//        h.insert("c", command);
//        h.insert("d", args);
//        if(verboseMode)
//            qDebug() << "UcCommandServer::command4dev " << command << args;
//        emit command2extension(MTD_EXT_NAME_ZBYRATOR, MTD_EXT_CUSTOM_COMMAND_0, h);

        QVariantHash pollargs;
        pollargs.insert("c", 140);
        pollargs.insert("d", "");
        emit sendCommand2zbyrator(pollargs, );


        return;//write error
    }


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
                    isReadingFinished = true;
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
    return modbusprocessor->isMessageReadingFinished(readArr);
}


//----------------------------------------------------------------------

