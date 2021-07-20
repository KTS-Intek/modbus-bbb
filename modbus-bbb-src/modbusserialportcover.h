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



    void ifaceLogStr(QString line); //<iface name> <time> <hex><data>  line 16 bytes




    //from streamreader
    void sendCommand2zbyrator(quint16 pollCode, QString ni, QString messagetag, QString objecttag);

    void sendCommand2dataHolder(quint16 pollCode, QString devID, bool useSn4devID, QString messagetag, QString objecttag);
    //to streamreader

    //matilda local socket answer
    void onCommandReceived(QString messagetag, QString  objecttag, bool isok, QString messageerror);

    void dataFromCache(QString messagetag, QString objecttag, QVariantList lastData);


    void onMatildaCommandReceived(QString messagetag, QString objecttag, bool isok, QString messageerror);


    void killAllObjects();//stop threads


    void onConnectionUp();
    void restartReConnectTimerMsec(int msec);


    void checkDHClientConnection();//to data holder client


public slots:
    void onThreadStarted();


    void reconnect2serialPort();

    void reloadSettings();



    void kickOffAll();


    void currentOperation(QString messageStrr);



    void restartReConnectTimer();

    //on settings changed
        void onConfigChanged(quint16 command, QVariant datavar);

private slots:
    void onSerialPortErrorHappened();

    void onSerialPortEverythingIsFine();

private:

     bool checkReloadSerialPortSettings();


    struct MySerialCoverState
    {


        quint8 serialOpentCounter;//if more than 60 kill the app

        ModbusSerialSettings serialSettings;

//        QString portName;
//        qint32 baudRate;
//        bool isParityNone;


        bool verboseMode;
        MySerialCoverState() : serialOpentCounter(0),
            verboseMode(false) {}
    } mystate;

};

#endif // MODBUSSERIALPORTCOVER_H
