#ifndef MODBUSMATILDALSCLIENT_H
#define MODBUSMATILDALSCLIENT_H

///[!] ipc
#include "localsockets/regularlocalsocket.h"

#include <QObject>
#include <QtCore>



class ModbusMatildaLSClient : public RegularLocalSocket
{
    Q_OBJECT
public:
    explicit ModbusMatildaLSClient(bool verboseMode, QObject *parent = nullptr);


    void decodeReadData(const QVariant &dataVar, const quint16 &command);

signals:
    void onMatildaCommandReceived(QString messagetag, QString objecttag, bool isok, QString messageerror);


public slots:
    //for client side

    void sendCommand2zbyrator(quint16 pollCode, QString ni, QString messagetag, QString objecttag);

    void sendCommand2firefly(quint16 pollCode, QString ni, QString messagetag, QString objecttag);

private:
    bool sendCommand2zbyratorHash(QVariantHash hash , QString messagetag, QString objecttag);

    bool sendCommand2fireflyHash(QVariantHash hash , QString messagetag, QString objecttag);

    bool sendCommand2fireflyReloadSettHash(QString messagetag, QString objecttag);


};

#endif // MODBUSMATILDALSCLIENT_H
