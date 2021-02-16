#include "modbusencoderdecoder.h"

//--------------------------------------------------------------------

ModbusEncoderDecoder::ModbusEncoderDecoder(const quint8 &modbusDecoderMode, const bool &isModbusMasterSide, QObject *parent) :
    ModbusMessanger(parent)
{
    setModbusMode(modbusDecoderMode);
    setModbusSide(isModbusMasterSide);
}

//--------------------------------------------------------------------

const quint8 ModbusEncoderDecoder::getModbusMode()
{
    return myparams.modbusDecoderMode;
}

//--------------------------------------------------------------------

const bool ModbusEncoderDecoder::isModbusMasterSide()
{
    return myparams.isModbusMasterSide;
}

//--------------------------------------------------------------------

bool ModbusEncoderDecoder::isMessageReadingFinished(const QByteArray &readArr)
{
    bool r = false;
    if(readArr.isEmpty() || readArr.length() < 3)
        return false;

    switch (myparams.modbusDecoderMode) {
    case MODBUS_MODE_RTU:
        r = isMessageReadingFinishedRTU(readArr);
        break;

    case MODBUS_MODE_TCP:
        r = isMessageReadingFinishedTCP(readArr);
        break;

    default:
        r = (isMessageReadingFinishedRTU(readArr) || isMessageReadingFinishedTCP(readArr));
        break;
    }
    return r;
}

//--------------------------------------------------------------------

bool ModbusEncoderDecoder::isMessageReadingFinishedRTU(const QByteArray &readArr)
{
//    if(myparams.isModbusMasterSide){
        return isThisMessageYoursLoopRTU(readArr, myparams.lastmessageparams);
//    }


}

//--------------------------------------------------------------------

bool ModbusEncoderDecoder::isMessageReadingFinishedTCP(const QByteArray &readArr)
{
//    if(myparams.isModbusMasterSide){
        return isThisMessageYoursLoopTCP(readArr, myparams.lastmessageparams);
//    }

}

//--------------------------------------------------------------------

void ModbusEncoderDecoder::setModbusMode(const quint8 &modbusmode)
{
    myparams.modbusDecoderMode = modbusmode;
}

//--------------------------------------------------------------------

void ModbusEncoderDecoder::setModbusSide(const bool &isModbusMasterSide)
{
    myparams.isModbusMasterSide = isModbusMasterSide;
}

//--------------------------------------------------------------------
