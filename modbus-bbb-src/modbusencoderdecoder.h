#ifndef MODBUSENCODERDECODER_H
#define MODBUSENCODERDECODER_H


///[!] modbus-base
#include "modbus-shared/modbusmessanger.h"


#include "modbuselectricitymeterhelper.h"
#include "modbuswatermeterhelper.h"
#include "modbusgasmeterhelper.h"
#include "modbuspulsemeterhelper.h"
#include "modbussettingsloader.h"
#include "modbusfireflydeviceshelper.h"

#include <QtCore>


//a slave mode is used

class ModbusEncoderDecoder : public ModbusMessanger
{
    Q_OBJECT
public:
    explicit ModbusEncoderDecoder(const bool &verboseMode, const quint8 &modbusDecoderMode, const bool &isModbusMasterSide, QObject *parent = nullptr);

    bool verboseMode;

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


    bool isItAPulseMeterPollCode(const quint8 &pollCode);

signals:
    void sendCommand2zbyratorWOObjectTag(quint16 pollCode, QString ni, QString messagetag);

    void sendCommand2fireflyWOObjectTag(quint16 pollCode, QString ni, QString messagetag);


    void sendCommand2dataHolderWOObjectTag(quint16 pollCode, QString devID, bool useSn4devID, QString messagetag);


    void onData2write(QByteArray writearr);



    void startTmrProcessing(int msec);

    void stopTmrProcessing();

    void startTmrFinitaLaComedia(int msec);

    void startTmrDataHolderProcessing(int msec);


    void append2textLog(QString message);

public slots:
    void createObjects();

    void setModbusMode(const quint8 &modbusmode);

    void setModbusSide(const bool &isModbusMasterSide);

    void sendErrorCode4thisDecoderError(const ModbusDecodedParams &messageparams);

    void startProcessing(const ModbusDecodedParams &lastmessageparams);


    void restartTimerProcessing();

    void onTmrProcessingTimeOut();

    void onTmrFinitaLaComedia();

    void onTmrDataHolderProcessingTimeOut();


    void onDataHolderCommandReceived(QString messagetag, bool isok, QString messageerror);

    void dataFromCache(QString messagetag, QVariantList lastData);

    void onMatildaCommandReceived(QString messagetag, bool isok, QString messageerror);


    void reloadAllSettings();

    void onMeterListChanged();

    void onModbusSettingsChanged();

    void updateDataFromDataHolder(const QVariantHash &lastHash, const quint8 &pollCode);

    void updateCachedDataHolderAnswers(const QVariantHash &lastHash, const quint8 &pollCode);

    void updateHash(QHash<quint8,  QVariantHash > &h, const QVariantHash &lastHash, const quint8 &pollCode);

private:


    bool isStartRegisterGood(const quint16 &startRegister);

    bool isVoltageRegister(const quint16 &startRegister);

    bool isEnergyRegister(const quint16 &startRegister);


    bool isWaterTotalRegister(const quint16 &startRegister);

    bool isGasTotalRegister(const quint16 &startRegister);

    bool isPulseTotalRegister(const quint16 &startRegister);

    bool isLcuDevStateRegister(const quint16 &startRegister);

    bool isLcuGroupStateRegister(const quint16 &startRegister);


    void findData4theseRegister(const quint16 &startRegister, const quint16 &count);

    void sendErrorCodeAndResetTheState(const quint8 &errorCode);

    bool isReadFunctionOk(const ModbusDecodedParams &messageparams, quint16 &firstRegister, quint16 &registerCount);

    QVariantHash getHashRequest4dataHolder(const quint8 &pollCode, const quint8 &devaddr);


    bool isCachedDataAcceptable(const QVariantHash &lastHash, const bool &ignoreMsec, bool &add2dataHolder);

    bool isCachedDataAddressAcceptable(const QVariantHash &h);

    bool checkSendDataToTheMaster();

    void sendDataToTheMaster();

    void resetLastMessageVariables();


    void fillTheAnswerHash(const quint8 &pollCode, QMap<quint16, quint16> &mapRegisters);

    void startPollSmart(const QString &messagetag);

    void startZbyratorPoll(const QString &messagetag);
    void startFireflyPoll(const QString &messagetag);


    ModbusAnswerList getVoltageAnswer(const QVariantHash &h);
    ModbusAnswerList getTotalEnergyAnswer(const QVariantHash &h);

    ModbusAnswerList getTotalWaterAnswer(const QVariantHash &h);
    ModbusAnswerList getTotalGasAnswer(const QVariantHash &h);
    ModbusAnswerList getTotalPulsesAnswer(const QList<QVariantHash> &listHash);

    ModbusAnswerList getLcuDevAnswer(const QVariantHash &h);
    ModbusAnswerList getLcuGroupAnswer(const QVariantHash &h);

    struct MyDecoderParams
    {

        bool isDecoderBusy;//only one command can be processed

        quint8 modbusDecoderMode;//RTU or TCP
        bool isModbusMasterSide;//it must false here

        ModbusVirtualDevices myDevices;
        QStringList myPulseMeterNIs;

        ModbusGeneralSettings generalSettings;
//        QList<quint8> listMeterNIs;//only acceptable values
//        QHash<quint8,QString> listDevAddr2meterNI;


        ModbusDecodedParams lastmessageparams; //last message that is processed

        quint16 busyCounter; //it counts requsts when the task is processed

//        qint32 processingmsec; //msec of the timeout

        quint8 processintTimeoutCounter; //it limits count of timeouts

        quint32 messageCounter; //it counts processed messages, do not reset it , it is used in message tag


        QList<quint8> lastPollCodes2receive;//codes that must be with data
        QList<quint8> lastPollCodes2send;//if it is equal to lastPollCodes2receive, the answer to master is ready to be sent

        quint16 lastStartRegister;
        quint16 lastRegisterCount;

        QHash<QString,quint8> messageTags;//I need it to verify the incomming messages

        bool isWaitingDataHolder;// it is going to sent a request to Data Holder, it timeout occurs start poll


        QHash<quint8, QVariantHash> dataFromDataHolder;//data buffer, it is used in fillTheAnswerHash, it has only acceptable data

        qint64 msecOfTheRequest;


        QHash<QString,quint8> zbyratoRmessageTags;//I need it to verify the incomming messages

        bool isFireflyMode;

        MyDecoderParams() :
            isDecoderBusy(false),
            modbusDecoderMode(MODBUS_MODE_RTU),
            isModbusMasterSide(false),
            busyCounter(0),
//            processingmsec(5555),//fucking com'x doesn't accept big timeout
            processintTimeoutCounter(0),
            messageCounter(0),
            lastStartRegister(0), lastRegisterCount(0),
            isFireflyMode(false)
        {}
    } myparams;

    QHash<quint8,  QVariantHash > cachedDataHolderAnswers;//only for one device, all data

};

#endif // MODBUSENCODERDECODER_H
