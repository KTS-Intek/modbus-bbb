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
    explicit ModbusStreamReader(const quint16 &objecttag, const bool &isTcpMode, const bool &verboseMode, QObject *parent = nullptr);

    ModbusEncoderDecoder *modbusprocessor;//it decodes and creates messages to mosbus masters

    struct MyReaderParams
    {
        bool isTcpMode;
        quint16 objecttag;//any valid number

        MyReaderParams() : isTcpMode(false) {}
    } myparams;


signals:
    void sendCommand2zbyrator(QVariantHash hash, quint16 messagetag, quint16 objecttag);
    void sendCommand2dataHolder(quint16 pollCode, QString ni, quint16 messagetag, quint16 objecttag);


public slots:
    void createObjects();


    void sendCommand2dataHolderWOObjectTag(quint16 pollCode, QString ni, quint16 messagetag);



    //send to matidla local socket
    void onSendCommand2zbyrator(QVariantHash hash, quint16 messagetag);
//matilda local socket answer
    void onCommandReceived(quint16 messagetag, quint16  objecttag, bool isok, QString messageerror);


    void onData2write(QByteArray writearr);

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
