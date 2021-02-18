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

    void append2log(QString message);


    //from streamreader
    void sendCommand2zbyrator(quint16 pollCode, QString ni, QString messagetag, QString objecttag);

    void sendCommand2dataHolder(quint16 pollCode, QString ni, QString messagetag, QString objecttag);
    //to streamreader

    //matilda local socket answer
    void onCommandReceived(QString messagetag, QString  objecttag, bool isok, QString messageerror);

    void dataFromCache(QString messagetag, QString objecttag, QVariantHash lastHash);


    void onMatildaCommandReceived(QString messagetag, QString objecttag, bool isok, QString messageerror);


    void checkDHClientConnection();//to data holder client

public slots:
    void createObjects();


    void restartApp();



private:
    void createTcpServer();

    void createSerialPortReader();

    void createMatildaLSClient();

    void createDataHolderClient();




};

#endif // MODBUSRESOURCEMANAGER_H
