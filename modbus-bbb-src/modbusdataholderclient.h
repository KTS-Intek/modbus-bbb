#ifndef MODBUSDATAHOLDERCLIENT_H
#define MODBUSDATAHOLDERCLIENT_H

///[!] ipc
#include "localsockets/regularlocalsocket.h"


class ModbusDataHolderClient : public RegularLocalSocket
{
    Q_OBJECT
public:
    explicit ModbusDataHolderClient(const QString &namesuffix, const bool &verboseMode, QObject *parent = nullptr);

    bool reconnectWasForced;
    QString mtdExtNameStr;

    QVariant getExtName();

    QString getPath2server();

    void decodeReadData(const QVariant &dataVar, const quint16 &command);


signals:
    void onCommandReceived(quint16 messagetag, quint16 objecttag, bool isok, QString messageerror);

    void dataFromCache(quint16 messagetag, quint16 objecttag, QVariantHash lastHash);

public slots:
    void onThreadStartedL0();

    void checkYourConnection();

    //for client side
    void sendCommand2dataHolder(quint16 pollCode, QString ni, quint16 messagetag, quint16 objecttag);


    void sendCommand2zbyrator(QVariantHash hash , quint16 messagetag);


private:
    void onDATAHOLDER_GET_POLLDATA(const QVariantHash &hash);


};

#endif // MODBUSDATAHOLDERCLIENT_H
