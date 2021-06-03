#ifndef MODBUSTCPOUTSOCKET_H
#define MODBUSTCPOUTSOCKET_H

#include <QTcpSocket>

class ModbusTcpOutSocket : public QTcpSocket
{
    Q_OBJECT
public:
    explicit ModbusTcpOutSocket(QObject *parent = nullptr);

signals:

public slots:
    void write2socket(QByteArray lines);


};

#endif // MODBUSTCPOUTSOCKET_H
