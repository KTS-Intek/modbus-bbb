#ifndef MODBUSTCPOUTSOCKET_H
#define MODBUSTCPOUTSOCKET_H

#include <QTcpSocket>
#include <QStringList>
#include <QTimer>

class ModbusTcpOutSocket : public QTcpSocket
{
    Q_OBJECT
public:
    explicit ModbusTcpOutSocket(QObject *parent = nullptr);

    QStringList lastLines;
    bool lockWriting;
signals:

public slots:
    void write2socket(QByteArray lines);

    void unlockWriting();

};

#endif // MODBUSTCPOUTSOCKET_H
