#include "modbusfireflydeviceshelper.h"


#include <QtMath>
#include <QDebug>
#include <QDateTime>



///[!] MatildaIO
#include "matilda-bbb-src/shared/pathsresolver.h"
#include "src/shared/compressfilehelper.h"


#include <QFile>
#include <QDataStream>


#include "matildalimits.h"


//-----------------------------------------------------------------------------

int ModbusFireflyDevicesHelper::getAcceptableLcuNis(ModbusVirtualDevices &vdevs)
{
    //LCUs come with NIs that equals ATSL , so probably the counter will be always 0
    int counter = 0;

    const auto varl = CompressFileHelper::readCompressedVarListFromTheFile(PathsResolver::path2fireflyLampsListV2());
    for(int i = 0, imax = varl.size(); i < imax; i++){
        const auto hash = varl.at(i).toHash();

        OneModbusVirtualDevice onedev;
        const quint8 add = meterAddressFromHash(hash, onedev);
        if(add > 0){
            vdevs.insert(add, onedev);
            counter++;
        }
    }
    return counter;
}

//-----------------------------------------------------------------------------

int ModbusFireflyDevicesHelper::getAcceptableGroupsNis(ModbusVirtualDevices &vdevs)
{
    //this_is_a_unique_ni_for_power2groups
    Q_UNUSED(vdevs);
    return 0;
}

//-----------------------------------------------------------------------------

quint16 ModbusFireflyDevicesHelper::getTypeCode(const QString &typeStr, bool &ok)
{
//    QString BusDevice::getDevVersionStrExt(const quint8 &type, bool &isVersionV3)
//    {
//        isVersionV3 = false;
//        QString typestr = tr("Unknown device");
//        switch(type){
//        case 2  : typestr = tr("BUS-2-1"); break;
//        case 3  : typestr = tr("BUS-2-3"); isVersionV3 = true; break;

//            //        case 5: type = tr("КУЛ-1-2"); break;
//        case 98 : typestr = tr("LCU-1.0"); break;//Current version
//        case 103: typestr = tr("LCU-1.1"); break;//has settings 4 NFC, current sensor //Light sensor, Motion detector
//        default: {
//            if(type > 103)
//                typestr = tr("LCU-1.%1").arg(int(type) - 103 + 1);
//            else
//                typestr = QString("DEV-%1").arg(int(type));
//            break; }
//        }
    ok = false;
    quint16 val = 0xFFFF;
    if(typeStr.startsWith("LCU-1.")){
        val = typeStr.split(".").last().toUInt(&ok);

        if(ok){
            switch(val){
            case 0: val = 98; break;
            case 1: val = 103; break;
            default: val += 102; break; //+103 - 1
            }
        }
    }
    return val;
}

//-----------------------------------------------------------------------------

ModbusAnswerList ModbusFireflyDevicesHelper::getLcuStateAnswer(const QVariantHash &hdata, const bool &verboseMode)
{
  ModbusAnswerList l;
  // 44401 - 44423
//  1687013109336, $who, exchng,
//  $Isens - ?
//  $NI - 7a8a60
//  $Pna - 30
//  $Pstart - 50
//  $SN - 7a8a60
//  $Tna - 1800
//  $Uref - ?
//  $Usens - 0
//  $counter - 0
//  $crdnt - 50.429680,30.660821
//  $evntType - exchng
//  $groupIndx - 2
//  $hmsec - 2023-06-17 17:45:09 EEST
//  $lcu-grpw - 2
//  $lcu-pnaw - 30
//  $lcu-pstrtw - 50
//  $lcu-tnaw - 1800
//  $memo - Віктор
//  $model -
//  $msec - 1687013109336
//  $power - 0
//  $powerWt - 0.00
//  $powerWt-C - 1
//  $prog - 4
//  $refpower - 80
//  $rez - 0
//  $type - LCU-1.1
//  $uptime - 29292
//  $useUref - true
//  $vrsn -

   QStringList valnames = verboseMode ? QString("msec msec msec msec").split(" ") : QStringList();

  addDividedInt642thelist(l, hdata.value("msec", 0).toLongLong());//44401 , 44402, 44403, 44404

  QStringList listKeys = QString("Isens Usens powerWt").split(" ");
  QList<qreal> valFactor = QList<qreal>() << 0.001 << 1.0 << 1.0;
//44405 44406 - 44407 44408 - 44409 44410
  //value and value sensor type, some of them can be calculated
  for(int i = 0, imax = listKeys.size(); i < imax; i++){
      const QString key = listKeys.at(i);
      if(verboseMode){
          valnames.append(key);
          valnames.append(key + "-C");

      }
      bool ok;
      qreal valReal = hdata.value(key).toDouble(&ok);
      if(!ok){
          l.append(0xFFFF);
          l.append(0xFFFF);
          continue;
      }

      l.append(quint16(valReal/valFactor.at(i)));
      l.append( (hdata.value(QString("%1-C")).toInt() == 1) ? 1 : 0);
  }

  listKeys = QString("lcu-Preactive lcu-Pfactor lcu-Frequen refpower type "
                     "prog power groupIndx Pstart Pna "
                     "Tna").split(" ");//3 first are in jsn, so lcu- must be added
// 44411   44412    44413   44414  44415
// 44416   44417    44418   44419   44420
  //44421
  for(int i = 0, imax = listKeys.size(); i < imax; i++){
      const QString key = listKeys.at(i);
      if(verboseMode)
          valnames.append(key);
      bool ok;
      qreal valReal = hdata.value(key).toDouble(&ok);
      if(key == "type"){
          valReal = getTypeCode(hdata.value(key).toString(), ok);
      }
      if(!ok){
          l.append((key == "Tna") ? 0x0 : 0xFFFF);
          continue;
      }
      l.append(quint16(valReal));
  }


    addDividedInt322thelist(l, hdata.value("uptime", 0).toInt());//44422 44423

    if(verboseMode){
        valnames.append("uptime");
        valnames.append("uptime");

        qDebug()  << "LCU state table  " << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
        qDebug()  << " ----------------------------- ";
         auto lk = hdata.keys();
         std::sort(lk.begin(), lk.end());
         for(int i = 0, imax = lk.size(); i < imax; i++){
             qDebug()  << lk.at(i) <<  hdata.value(lk.at(i));
         }
         qDebug()  << " ----------------------------- ";

        for(int i = 0, imax = l.size(); i < imax; i++){
            qDebug()  << valnames.at(i) <<  QString::number(l.at(i), 'f', 3);
        }
        qDebug()  << "----------------------------- ";
        qDebug()  << "LCU state table ----------------------------- ";
    }

  return l;
}

//-----------------------------------------------------------------------------

ModbusAnswerList ModbusFireflyDevicesHelper::getGroupsStateAnswer(const QVariantHash &hdata, const bool &verboseMode)
{
    ModbusAnswerList l;
    // 44201-44332
//    1687013107860, $who, exchng,
//    $NI - this_is_a_unique_ni_for_power2groups
//    $SN - this_is_a_unique_ni_for_power2groups
//    $counter - 0
//    $crdnt -
//    $evntType - exchng
//    $grp-1 - 50
//    $grp-2 - 0
//    $grp-3 - 35
//    $grp-4 - 0
//    $grp-5 - 114
//    $grp-6 - 68
//    $grp-7 - 190
//    $hmsec - 2023-06-17 17:45:07 EEST
//    $memo -
//    $model - lamp group
//    $msec - 1687013107860
//    $rez - 0
//    $vrsn -

    addDividedInt642thelist(l, hdata.value("msec", 0).toLongLong());//44201 , 44202, 44203, 44204

    for(int i = 0; i < MAXFIREFLYGROUPID; i++){
        bool ok;
        const quint32 grpval = hdata.value(QString("grp-%1").arg(i), 0xFFFF).toUInt(&ok);
        l.append( (!ok || grpval > 0xFE) ? 0xFFFF : grpval);
    }


    if(verboseMode){
        qDebug()  << "group state table  " << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz") <<  hdata.value("msec", 0).toLongLong();
        qDebug()  << " ----------------------------- ";

        for(int i = 4, imax = l.size(); i < imax; i++){
            qDebug()  << QString("grp-%1").arg(i - 4) <<  QString::number(l.at(i));
        }
        qDebug()  << "----------------------------- ";
        qDebug()  << "group state table ----------------------------- ";

    }


    return l;
}

//-----------------------------------------------------------------------------
