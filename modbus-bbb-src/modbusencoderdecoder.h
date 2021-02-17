#ifndef MODBUSENCODERDECODER_H
#define MODBUSENCODERDECODER_H


///[!] modbus-base
#include "modbus-shared/modbusmessanger.h"

#include <QtCore>


//a slave mode is used

class ModbusEncoderDecoder : public ModbusMessanger
{
    Q_OBJECT
public:
    explicit ModbusEncoderDecoder(const quint8 &modbusDecoderMode, const bool &isModbusMasterSide, QObject *parent = nullptr);

    quint8 getModbusMode();

    bool isModbusMasterSide();

    ModbusList getErrorMessage(const ModbusDecodedParams &messageparams, const quint8 &errorcode);

    QByteArray getErrorMessageArray(const ModbusDecodedParams &messageparams, const quint8 &errorcode);


    bool isMessageReadingFinished(const QByteArray &readArr, ModbusDecodedParams &messageparams);


    bool isMessageReadingFinishedRTU(const QByteArray &readArr, ModbusDecodedParams &messageparams);

    bool isMessageReadingFinishedTCP(const QByteArray &readArr, ModbusDecodedParams &messageparams);



    void canProcessTheLine(const QByteArray &readArr);

    ModbusRequestParams decodeRequestParamsSmart(const QByteArray &readArr);

    QString generateQuickPollLine(const QByteArray &readArr);

signals:
    void sendCommand2zbyrator(QVariantHash hash, quint16 messagetag) ;// don't forget to add a tag, quint16 messagetag);

    void sendCommand2dataHolderWOObjectTag(quint16 pollCode, QString ni, quint16 messagetag);


    void onData2write(QByteArray writearr);



    void startTmrProcessing(int msec);

    void stopTmrProcessing();


public slots:
    void createObjects();

    void setModbusMode(const quint8 &modbusmode);

    void setModbusSide(const bool &isModbusMasterSide);

    void sendErrorCode4thisDecoderError(const ModbusDecodedParams &messageparams);


    void restartTimerProcessing();

    void onTmrProcessingTimeOut();


    void onDataHolderCommandReceived(quint16 messagetag, bool isok, QString messageerror);

private:

    void findData4theseRegister(const quint16 &startRegister, const quint16 &count);

    void sendErrorCodeAndResetTheState(const quint8 &errorCode);

    bool isReadFunctionOk(const ModbusDecodedParams &messageparams, quint16 &firstRegister, quint16 &registerCount);

    QVariantHash getHashRequest4dataHolder(const quint8 &pollCode, const quint8 &devaddr);

    QVariantHash getHash4pollCodeAndDevAddr(const quint8 &pollCode, const quint8 &devaddr);

    struct MyDecoderParams
    {

        bool isDecoderBusy;//only one command can be processed

        quint8 modbusDecoderMode;//RTU or TCP
        bool isModbusMasterSide;//it must false here

        QList<quint8> listMeterNIs;//only acceptable values

        ModbusDecodedParams lastmessageparams; //last message that is processed

        quint16 busyCounter; //it counts requsts when the task is processed

        qint32 processingmsec; //msec of the timeout

        quint8 processintTimeoutCounter; //it limits count of timeouts

        quint32 messageCounter; //it counts processed messages, do not reset it , it is used in message tag


        QList<quint8> lastPollCodes2receive;//codes that must be with data
        QList<quint8> lastPollCodes2send;//if it is equal to lastPollCodes2receive, the answer to master is ready to be sent

        quint16 lastStartRegister;
        quint16 lastRegisterCount;

        QStringList messageTags;//I need it to verify the incomming messages

        bool isWaitingDataHolder;// it is going to sent a request to Data Holder, it timeout occurs start poll
        bool isWaiting4zbyrator;//the data must be received directly from the meter


        MyDecoderParams() :
            isDecoderBusy(false),
            modbusDecoderMode(MODBUS_MODE_RTU),
            isModbusMasterSide(false),
            busyCounter(0),
            processingmsec(7777),
            processintTimeoutCounter(0),
            messageCounter(0),
            lastStartRegister(0), lastRegisterCount(0)
        {}
    } myparams;



};

#endif // MODBUSENCODERDECODER_H
