#include <QCoreApplication>

#include "modbus-bbb-src/modbusresourcemanager.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    ModbusResourceManager manager;
    manager.createObjectsLater();

    return a.exec();
}
