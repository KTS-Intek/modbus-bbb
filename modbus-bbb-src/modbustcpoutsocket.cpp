#include "modbustcpoutsocket.h"

ModbusTcpOutSocket::ModbusTcpOutSocket(QObject *parent) : QTcpSocket(parent)
{

}

void ModbusTcpOutSocket::write2socket(QByteArray lines)
{
    write(lines);
}
