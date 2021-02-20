#ifndef MODBUSTCPSERVER_H
#define MODBUSTCPSERVER_H

#include <QTcpServer>

class ModbusTCPServer : public QTcpServer
{
    Q_OBJECT
public:
    explicit ModbusTCPServer(const bool &verboseMode, QObject *parent = nullptr);

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
    void stopAllSocket();//kill all objectss


    void checkDHClientConnection();//to data holder client

    //on settings changed
    void onConfigChanged(quint16 command, QVariant datavar);

public slots:
    void onThrdStarted();

    void onConnectionDown();

    void kickOffServer();

    void stopServerNow();

    void startServer();

protected:
    void incomingConnection(qintptr handle);

private:


    struct MyTcpServerState
    {

        int activeConnectionCounter;
        quint64 connectionCounter;
        quint8 serverStartCounter;//if more than 60 kill the app

        bool verboseMode;
        MyTcpServerState() : activeConnectionCounter(0), connectionCounter(0), verboseMode(false) {}
    } myserverstate;


};

#endif // MODBUSTCPSERVER_H
