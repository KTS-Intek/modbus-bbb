#ifndef MODBUSCONFIGURATIONFILEWATCHER_H
#define MODBUSCONFIGURATIONFILEWATCHER_H

#include <QObject>

class ModbusConfigurationFileWatcher : public QObject
{
    Q_OBJECT
public:
    explicit ModbusConfigurationFileWatcher(QObject *parent = nullptr);

signals:

};

#endif // MODBUSCONFIGURATIONFILEWATCHER_H
