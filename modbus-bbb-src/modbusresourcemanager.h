#ifndef MODBUSRESOURCEMANAGER_H
#define MODBUSRESOURCEMANAGER_H

#include <QObject>
#include <QVariantHash>

class ModbusResourceManager : public QObject
{
    Q_OBJECT
public:
    explicit ModbusResourceManager(QObject *parent = nullptr);

    bool verboseMode;

    void createObjectsLater();


signals:
    void killAllAndDie();

    void append2log(qint64 msec, QString message);




    void appendLogDataLine(QString key, QString line, QString splitter, int maxLogSize); //to shared memory

    void appendLogDataList(QString key, QStringList log, QString splitter, int maxLogSize); //I need it bcs I want to use new format of connecting signals/slots


    //from streamreader
    void sendCommand2zbyrator(quint16 pollCode, QString ni, QString messagetag, QString objecttag);

    void sendCommand2firefly(quint16 pollCode, QString ni, QString messagetag, QString objecttag);

    void sendCommand2dataHolder(quint16 pollCode, QString devID, bool useSn4devID, QString messagetag, QString objecttag);
    //to streamreader

    //matilda local socket answer
    void onCommandReceived(QString messagetag, QString  objecttag, bool isok, QString messageerror);

    void dataFromCache(QString messagetag, QString objecttag, QVariantList lastData);


    void onMatildaCommandReceived(QString messagetag, QString objecttag, bool isok, QString messageerror);


    void checkDHClientConnection();//to data holder client


//on settings chagned
    void onConfigChanged(quint16 command, QVariant datavar);


public slots:
    void createObjects();


    void restartApp();


    void onLogingServiceIsReady();



    void append2logSlot(qint64 msec, QString message);

    void appendUart2logSlot(QString lines);

    void appendTcp2logSlot(QString lines);

private:

    void createSharedMemoryObjects();




    void createTcpServer();

    void createSerialPortReader();

    void createMatildaLSClient();

    void createDataHolderClient();




};

#endif // MODBUSRESOURCEMANAGER_H
