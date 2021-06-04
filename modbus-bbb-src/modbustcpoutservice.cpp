#include "modbustcpoutservice.h"

#include <QTcpSocket>
#include <QDateTime>
#include <QTimer>

#include "modbustcpoutsocket.h"

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

void ModbusTcpOutService::dataReadWriteReal(QByteArray arr, QString ifaceName, bool isRead)
{
    appendTextLog(QString("%1 %2 %3").arg(ifaceName).arg( isRead ? " > " : " < " ).arg(QString(arr.toHex())));
}



void ModbusTcpOutService::appendTextLog(QString lines)
{
    lastLines.append(lines);
    if(lastLines.size() > 250)
        lastLines = lastLines.mid(50);
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

    ModbusTcpOutSocket *socket = new ModbusTcpOutSocket(this);

    if(!socket->setSocketDescriptor(handle)){
        socket->deleteLater();
        return;
    }
    socket->lastLines = lastLines;

    connect(this, &ModbusTcpOutService::write2socket, socket, &ModbusTcpOutSocket::write2socket);/* [=](QByteArray lines){
        socket->write(lines); it makes big problems, after sockets connect and disconnect
    });*/

    connect(this, &ModbusTcpOutService::killAll, socket, &ModbusTcpOutSocket::close);
    connect(socket, &QTcpSocket::disconnected, socket, &ModbusTcpOutSocket::deleteLater);



}
