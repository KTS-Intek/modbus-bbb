#ifndef MODBUSTCPSOCKETCOVER_H
#define MODBUSTCPSOCKETCOVER_H

#include "modbusstreamreader.h"

class ModbusTCPSocketCover : public ModbusStreamReader
{
    Q_OBJECT
public:
    explicit ModbusTCPSocketCover(const QString &objecttag, const bool &isTcpMode, const bool &verboseMode, QObject *parent = nullptr);

    bool isAble2setSocketDescriptor(qintptr handle, QString &lastError);
signals:

public slots:

    void reloadSettings();


};

#endif // MODBUSTCPSOCKETCOVER_H
