#ifndef MODBUSTCPSERVER_H
#define MODBUSTCPSERVER_H

#include <QTcpServer>

class ModbusTCPServer : public QTcpServer
{
    Q_OBJECT
public:
    explicit ModbusTCPServer(QObject *parent = nullptr);

signals:


public slots:
    void onThrdStarted();

protected:
    void incomingConnection(qintptr handle);

};

#endif // MODBUSTCPSERVER_H
