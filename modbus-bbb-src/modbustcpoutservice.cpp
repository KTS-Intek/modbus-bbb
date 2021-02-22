#include "modbustcpoutservice.h"

#include <QTcpSocket>
#include <QDateTime>
#include <QTimer>

ModbusTcpOutService::ModbusTcpOutService(QObject *parent) : QTcpServer(parent)
{

}

void ModbusTcpOutService::initServer()
{
    startServer();
}

void ModbusTcpOutService::startServer()
{
    if(isListening()){
        close();

    }

    if(listen(QHostAddress::Any, 9091)){
        return;
    }
    QTimer::singleShot(1111, this, SLOT(startServer()));
}

void ModbusTcpOutService::appendTextLog(QString lines)
{
    emit write2socket(QString("%1 %2\n").arg(QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss.zzz t")).arg(lines).toUtf8());
}

void ModbusTcpOutService::killAllAndStop()
{
    close();
    emit killAll();
    QTimer::singleShot(11, this, SLOT(deleteLater()));
}

void ModbusTcpOutService::incomingConnection(qintptr handle)
{
    QTcpSocket *socket = new QTcpSocket(this);

    if(!socket->setSocketDescriptor(handle)){
        socket->deleteLater();
        return;
    }

    connect(this, &ModbusTcpOutService::write2socket, [=](QByteArray lines){
        socket->write(lines);
    });

    connect(this, &ModbusTcpOutService::killAll, socket, &QTcpSocket::close);
    connect(socket, &QTcpSocket::disconnected, socket, &QTcpSocket::deleteLater);

}
