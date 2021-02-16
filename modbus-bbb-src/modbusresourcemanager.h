#ifndef MODBUSRESOURCEMANAGER_H
#define MODBUSRESOURCEMANAGER_H

#include <QObject>

class ModbusResourceManager : public QObject
{
    Q_OBJECT
public:
    explicit ModbusResourceManager(QObject *parent = nullptr);

    bool verboseMode;

    void createObjectsLater();


signals:
    void killAllAndDie();



//to matilda local socket
    void sendCommand2zbyrator(QVariantHash hash, quint16 messagetag, quint16 objecttag);
    //from matilda local socket
    void onCommandReceived(quint16 messagetag, quint16  objecttag, bool isok, QString messageerror);


public slots:
    void createObjects();



private:
    void createTcpServer();

    void createSerialPortReader();

    void createMatildaLSClient();

    void createDataHolderClient();




};

#endif // MODBUSRESOURCEMANAGER_H
