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
    void onCommandReceived(QString messagetag, QString objecttag, bool isok, QString messageerror);

    void dataFromCache(QString messagetag, QString objecttag, QVariantHash lastHash);

public slots:
    void onThreadStartedL0();

    void checkYourConnection();

    //for client side
    void sendCommand2dataHolder(quint16 pollCode, QString devID, bool useSn4devID, QString messagetag, QString objecttag);




private:
    void onDATAHOLDER_GET_POLLDATA_EXT(const QVariantHash &hash);


};

#endif // MODBUSDATAHOLDERCLIENT_H
