#include "modbustcpoutsocket.h"

ModbusTcpOutSocket::ModbusTcpOutSocket(QObject *parent) : QTcpSocket(parent)
{
    lockWriting = true;
    QTimer::singleShot(3333, this, SLOT(unlockWriting()));
}

void ModbusTcpOutSocket::write2socket(QByteArray lines)
{
   if(lockWriting){
       lastLines.append(QString(lines));
       return;
   }


   write(lines);
}

void ModbusTcpOutSocket::unlockWriting()
{
    lockWriting = false;
    write2socket(lastLines.join("\n").toUtf8());
    lastLines.clear();
}
