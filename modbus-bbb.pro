QT += core network serialport
QT -= gui


CONFIG += c++11 console
CONFIG -= app_bundle

TARGET = modbus-bbb

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0



linux-beagleboard-g++:{
    target.path = /opt/matilda/bin
    INSTALLS += target
}
linux-g++:{
#uncomment for testing sqlite-medium
#DEFINES += ENABLE_DBG_SQLTMEDIUM_TEST_MODE #connect to sqlite-medium over tcp/ip
DEFINES += ENABLE_DBG_LOCALTCP_TEST_MODE #connect to peredavator via not local tcp/ip (from app's argumens
DEFINES += DISABLE_LOCALSOCKETVERBOSE

}


VERSION = 0.0.1

#DEFINES += APPLCTN_NAME=\\\"quick-collect\\\" it is only for GUI
DEFINES += "MYAPPNAME=\"\\\"modbus-bbb\\\"\""
DEFINES += "MYAPPOWNER=\"\\\"KTS-Intek Ltd\\\"\""
DEFINES += "MYAPPOWNERSITE=\"\\\"http://kts-intek.com\\\"\""


#DEFINES += ENABLE_VERBOSE
#DEFINES += ENABLE_EXTSUPPORT_OF_IFACES
#DEFINES += ZBYRATOR_BBB
#DEFINES += DEV_TYPE_UC=1
#DEFINES += ENABLE_TEST_MODE0
DEFINES += DONOTINCLUDEFIREFLY
#DEFINES += DONOTINCLUDEIFACEPROCESSOR

DEFINES += DISABLE_UART_PRIORITY
DEFINES += ENABLE_EXTSUPPORT_OF_IFACES
DEFINES += DISABLE_M2M_MODULE

include(../../../Matilda-units/ipc/localsockets/localsockets.pri)

include(../../../Matilda-units/meter-plugin-shared/modbus-base/modbus-base.pri)

include(../../../Matilda-units/matilda-base/type-converter/type-converter.pri)

include(../../../Matilda-units/ifaces/ifaces/ifaces.pri)

include(../../../Matilda-units/matilda-base/MatildaIO/MatildaIO.pri)

include(../../../Matilda-units/matilda-bbb/matilda-bbb-settings/matilda-bbb-settings.pri)


SOURCES += \
        main.cpp \
        modbus-bbb-src/modbusconfigurationfilewatcher.cpp \
        modbus-bbb-src/modbusdataholderclient.cpp \
        modbus-bbb-src/modbuselectricitymeterhelper.cpp \
        modbus-bbb-src/modbusencoderdecoder.cpp \
        modbus-bbb-src/modbusmatildalsclient.cpp \
        modbus-bbb-src/modbusresourcemanager.cpp \
        modbus-bbb-src/modbusserialportcover.cpp \
        modbus-bbb-src/modbusstreamreader.cpp \
        modbus-bbb-src/modbustcpserver.cpp



HEADERS += \
    modbus-bbb-src/modbusconfigurationfilewatcher.h \
    modbus-bbb-src/modbusdataholderclient.h \
    modbus-bbb-src/modbuselectricitymeterhelper.h \
    modbus-bbb-src/modbusencoderdecoder.h \
    modbus-bbb-src/modbusencoderdecodertypes.h \
    modbus-bbb-src/modbusmatildalsclient.h \
    modbus-bbb-src/modbusresourcemanager.h \
    modbus-bbb-src/modbusserialportcover.h \
    modbus-bbb-src/modbusstreamreader.h \
    modbus-bbb-src/modbustcpserver.h
