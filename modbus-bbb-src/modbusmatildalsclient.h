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
    void onCommandReceived(quint16 messagetag, quint16 objecttag, bool isok, QString messageerror);


public slots:
    //for client side
    void sendCommand2zbyrator(QVariantHash hash , quint16 messagetag, quint16 objecttag);


};

#endif // MODBUSMATILDALSCLIENT_H
