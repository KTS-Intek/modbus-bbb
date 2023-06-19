#ifndef MODBUSTCPSERVER_H
#define MODBUSTCPSERVER_H

#include <QTcpServer>

class ModbusTCPServer : public QTcpServer
{
    Q_OBJECT
public:
    explicit ModbusTCPServer(const bool &verboseMode, QObject *parent = nullptr);

signals:


    void ifaceLogStr(QString line); //<iface name> <time> <hex><data>  line 16 bytes


    //to the resource manager
    void append2log(qint64 msec, QString message);
    void restartApp();



    //from streamreader
    void sendCommand2zbyrator(quint16 pollCode, QString ni, QString messagetag, QString objecttag);

    void sendCommand2firefly(quint16 pollCode, QString ni, QString messagetag, QString objecttag);

    void sendCommand2dataHolder(quint16 pollCode, QString devID, bool useSn4devID, QString messagetag, QString objecttag);
    //to streamreader

    //matilda local socket answer
    void onCommandReceived(QString messagetag, QString  objecttag, bool isok, QString messageerror);

    void dataFromCache(QString messagetag, QString objecttag, QVariantList lastData);


    void onMatildaCommandReceived(QString messagetag, QString objecttag, bool isok, QString messageerror);


    void killAllObjects();//stop threads
    void stopAllSocket();//kill all objectss

    void reloadTcpSettings();

    void checkDHClientConnection();//to data holder client


    void restartTmrRestart();


    void setServerInConnIdExtData(QString conntype, QString connid, qint64 msecstart, qint64 msecend, qint64 rb, qint64 wb, QString lastmessage); //список вхідних IP адрес (АйПі збиратора ігнорується)


public slots:
    void onThrdStarted();

    void onConnectionDown();

    void kickOffServer();

    void stopServerNow();

    void startServer();


    void append2logSmpl(QString message);


    //on settings changed
    void onConfigChanged(quint16 command, QVariant datavar);

    void reloadSettings();




protected:
    void incomingConnection(qintptr handle);

private:

    void createNetHistoryTable();

    struct MyTcpServerState
    {

        int activeConnectionCounter;
        quint64 connectionCounter;
        quint8 serverStartCounter;//if more than 60 kill the app

        bool allowTcpServer;

        bool verboseMode;
        MyTcpServerState() : activeConnectionCounter(0), connectionCounter(0), allowTcpServer(false), verboseMode(false) {}
    } myserverstate;


};

#endif // MODBUSTCPSERVER_H
