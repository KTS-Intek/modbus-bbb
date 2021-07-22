#ifndef MODBUSTCPSOCKETCOVER_H
#define MODBUSTCPSOCKETCOVER_H

#include "modbusstreamreader.h"


#include "connectiontableinsharedmemorytypes.h"


class ModbusTCPSocketCover : public ModbusStreamReader
{
    Q_OBJECT
public:
    explicit ModbusTCPSocketCover(const QString &objecttag, const bool &isTcpMode, const bool &verboseMode, QObject *parent = nullptr);

    bool isAble2setSocketDescriptor(qintptr handle, QString &lastError);

    ConnectionTableInSharedMemoryConnection theconnection;

signals:

    void setServerInConnIdExtData(QString conntype, QString connid, qint64 msecstart, qint64 msecend, qint64 rb, qint64 wb, QString lastmessage); //список вхідних IP адрес (АйПі збиратора ігнорується)

public slots:
    void onDataReadWriteReal(QByteArray arr, QString ifaceName, bool isRead);

    void setupTheConnectionStruct(const QString &conntype, const QString &connid);

    //from Ucon socket
        void onTheConnectionUp();

        void setTheIsServerSide();

        void onTheConnectionDown();

        void sendTheLastConnectionState();



    void reloadSettings();


};

#endif // MODBUSTCPSOCKETCOVER_H
