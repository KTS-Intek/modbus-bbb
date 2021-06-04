#ifndef MODBUSTCPOUTSERVICE_H
#define MODBUSTCPOUTSERVICE_H

#include <QTcpServer>

class ModbusTcpOutService : public QTcpServer
{
    Q_OBJECT
public:
    explicit ModbusTcpOutService(QObject *parent = nullptr);


signals:
    void write2socket(QByteArray lines);

    void killAll();



public slots:
    void initServer();

    void startServer();

    void dataReadWriteReal(QByteArray arr, QString ifaceName, bool isRead);





    void appendTextLog(QString lines);

    void killAllAndStop();

protected:
    void incomingConnection(qintptr handle);

private:
    QStringList lastLines;



};

#endif // MODBUSTCPOUTSERVICE_H
