#ifndef MODBUSENCODERDECODER_H
#define MODBUSENCODERDECODER_H


///[!] modbus-base
#include "modbus-shared/modbusmessanger.h"




class ModbusEncoderDecoder : public ModbusMessanger
{
    Q_OBJECT
public:
    explicit ModbusEncoderDecoder(const quint8 &modbusDecoderMode, const bool &isModbusMasterSide, QObject *parent = nullptr);

    const quint8 getModbusMode();

    const bool isModbusMasterSide();

    bool isMessageReadingFinished(const QByteArray &readArr);


    bool isMessageReadingFinishedRTU(const QByteArray &readArr);

    bool isMessageReadingFinishedTCP(const QByteArray &readArr);

    ModbusRequestParams decodeRequestParamsSmart(const QByteArray &readArr);

    QString generateQuickPollLine(const QByteArray &readArr);

signals:
    void sendCommand2zbyrator(QVariantHash hash, quint16 messagetag) ;// don't forget to add a tag, quint16 messagetag);



public slots:
    void setModbusMode(const quint8 &modbusmode);

    void setModbusSide(const bool &isModbusMasterSide);

private:

    struct MyDecoderParams
    {

        quint8 modbusDecoderMode;
        bool isModbusMasterSide;

        ModbusDecodedParams lastmessageparams;

        MyDecoderParams() : modbusDecoderMode(MODBUS_MODE_RTU), isModbusMasterSide(false) {}
    } myparams;



};

#endif // MODBUSENCODERDECODER_H
