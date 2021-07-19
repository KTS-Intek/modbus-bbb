#include "modbustcpsocketcover.h"

///[!] type-converter
#include "src/shared/networkconverthelper.h"





ModbusTCPSocketCover::ModbusTCPSocketCover(const QString &objecttag, const bool &isTcpMode, const bool &verboseMode, QObject *parent) :
    ModbusStreamReader(objecttag, isTcpMode, verboseMode, parent)
{

}

bool ModbusTCPSocketCover::isAble2setSocketDescriptor(qintptr handle, QString &lastError)
{

    if(!socket->setSocketDescriptor(handle)){
        lastError = socket->errorString();
        QTimer::singleShot(111, this, SLOT(deleteLater()));
      return false;
    }
    return true;
}


void ModbusTCPSocketCover::reloadSettings()
{

    const QString remaddr = NetworkConvertHelper::showNormalIP(socket->peerAddress());

    const ModbusTcpSettings settings = ModbusSettingsLoader::getModbusTcpSettings();

    if(!settings.IPs.isEmpty()){

        if(!NetworkConvertHelper::isIpGood(remaddr, settings.IPs)){

            emit currentOperation(tr("IP '%1' is not in the allowed list").arg(mysett.remip));
            QTimer::singleShot(11, this, SLOT(deleteLater()));
            return ;
        }
    }
    modbusprocessor->reloadAllSettings();
    modbusprocessor->setModbusMode(settings.mode);//0 - disabled, 1 - rtu, 2 - tcp, other - both

    const QString rmDescr = QString::number(socket->socketDescriptor());

    emit currentOperation(QString("Updating %1:%2 settings").arg(remaddr).arg(rmDescr));

    setTimeouts(settings.tcpTimeout, settings.tcpBlcokTimeout);
    setIgnoreUartChecks(true);


}
