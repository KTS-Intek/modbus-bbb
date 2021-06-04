#ifndef MODBUSSTREAMREADER_H
#define MODBUSSTREAMREADER_H

///[!] ifaces
#include "src/emb/conn2modem.h"


#include "modbusencoderdecoder.h"



//this class only receives requests from any modbus master and answers
//

class ModbusStreamReader : public Conn2modem
{
    Q_OBJECT
public:
    explicit ModbusStreamReader(const QString &objecttag, const bool &isTcpMode, const bool &verboseMode, QObject *parent = nullptr);

    ModbusEncoderDecoder *modbusprocessor;//it decodes and creates messages to mosbus masters

    struct MyReaderParams
    {
        bool isTcpMode;
        QString objecttag;//any valid number

        MyReaderParams() : isTcpMode(false) {}
    } myparams;


signals:

    void sendCommand2zbyrator(quint16 pollCode, QString ni, QString messagetag, QString objecttag);

    void sendCommand2dataHolder(quint16 pollCode, QString ni, QString messagetag, QString objecttag);

    void onSerialPortName(QString serialportname, bool isParityNone);



public slots:
    void createObjects();


    //from decoder
    void sendCommand2dataHolderWOObjectTag(quint16 pollCode, QString ni, QString messagetag);



    //send to matidla local socket
    void onSendCommand2zbyrator(quint16 pollCode, QString ni, QString messagetag);



//matilda local socket answer
    void onCommandReceived(QString messagetag, QString objecttag, bool isok, QString messageerror);

    void dataFromCache(QString messagetag, QString objecttag, QVariantHash lastHash);


    void onMatildaCommandReceived(QString messagetag, QString objecttag, bool isok, QString messageerror);


    void onData2write(QByteArray writearr);

    //on settings changed
    void onConfigChanged(quint16 command, QVariant datavar);




private slots:
    void mReadyReadUART();

    void mReadyReadTCP();

    void decodeArray(const QByteArray &readArr);

private:




    void processReadArray(const QByteArray &readArr);

    QByteArray readFromTheDevice();

    bool isThisYourRead(const QByteArray &readArr);


};

#endif // MODBUSSTREAMREADER_H
