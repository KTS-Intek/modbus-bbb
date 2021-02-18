#ifndef MODBUSSERIALPORTCOVER_H
#define MODBUSSERIALPORTCOVER_H

#include <QObject>

#include "modbusstreamreader.h"


class ModbusSerialPortCover : public QObject
{
    Q_OBJECT
public:
    explicit ModbusSerialPortCover(const bool &verboseMode, QObject *parent = nullptr);

    ModbusStreamReader *streamr;

signals:

    //to the resource manager
    void append2log(QString message);
    void restartApp();


    //from streamreader
    void sendCommand2zbyrator(quint16 pollCode, QString ni, QString messagetag, QString objecttag);

    void sendCommand2dataHolder(quint16 pollCode, QString ni, QString messagetag, QString objecttag);
    //to streamreader

    //matilda local socket answer
    void onCommandReceived(QString messagetag, QString  objecttag, bool isok, QString messageerror);

    void dataFromCache(QString messagetag, QString objecttag, QVariantHash lastHash);


    void onMatildaCommandReceived(QString messagetag, QString objecttag, bool isok, QString messageerror);


    void killAllObjects();//stop threads


    void onConnectionUp();
    void restartReConnectTimer();

    void checkDHClientConnection();//to data holder client

public slots:
    void onThreadStarted();


    void reconnect2serialPort();

    void reloadSettings();

    void kickOffAll();


    void currentOperation(QString messageStrr);

private:
    struct MySerialCoverState
    {

        quint8 serialOpentCounter;//if more than 60 kill the app

        QString portName;
        qint32 baudRate;


        bool verboseMode;
        MySerialCoverState() : serialOpentCounter(0),
            baudRate(19200),
            verboseMode(false) {}
    } mystate;

};

#endif // MODBUSSERIALPORTCOVER_H
