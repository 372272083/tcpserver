#include "analysethread.h"

#include <QException>
#include <QList>
#include <QException>
#include <QQueue>
#include <QDebug>
#include <QDateTime>

#include <QList>
#include <QMap>

#include "sqlserverdb.h"
#include "tcpserver.h"

#include "FaultFeatureLib/IO_Param.h"
#include "FaultFeatureLib/ElcWaveAnsys.h"
#include "FaultFeatureLib/VibWaveAnsys.h"

AnalyseThread::AnalyseThread(SqlserverDB *db):db(db)
{

}

void AnalyseThread::run()
{
    SVibAnsyDataLib *pVibAnsyDataLib=NULL;
    SElcAnsyDataLib *pElcAnsyDataLib=NULL;

    //SDiagnosisInPutData *pInputDatas = nullptr;
    //SDiagnosisOutPutData *pOutPutDatas = nullptr;

    DOUBLE_VCT vAccWave[6],vVolWave[3],vCurWave[3];

    CVibWaveAnsys *pVibWaveAnsys=NULL;
    CElcWaveAnsys *pElcWaveAnsys=NULL;

    int count = 0;
    int stype = 0;
    int waveIndex = 0;

    while(true)
    {
        if(!TcpServer::enable_analyse)
        {
            continue;
        }

        if (pVibAnsyDataLib != NULL)
        {
            delete pVibAnsyDataLib;
            pVibAnsyDataLib = NULL;
        }
        if (pElcAnsyDataLib != NULL)
        {
            delete pElcAnsyDataLib;
            pElcAnsyDataLib = NULL;
        }

        if (pVibAnsyDataLib == NULL)
        {
            pVibAnsyDataLib=new SVibAnsyDataLib;
        }
        if (pElcAnsyDataLib == NULL)
        {
            pElcAnsyDataLib=new SElcAnsyDataLib;
        }

        if (pVibWaveAnsys != NULL)
        {
            delete pVibWaveAnsys;
            pVibWaveAnsys = NULL;
        }
        if (pElcWaveAnsys != NULL)
        {
            delete pElcWaveAnsys;
            pElcWaveAnsys = NULL;
        }

        if (pVibWaveAnsys == NULL)
        {
            pVibWaveAnsys=new CVibWaveAnsys(pVibAnsyDataLib);
        }
        if (pElcWaveAnsys == NULL)
        {
            pElcWaveAnsys=new CElcWaveAnsys(pElcAnsyDataLib);
        }

        /*
        for(int i=0;i<12;i++)
        {
            pInputDatas->inputWaveDatas.waveData[i].clear();
        }
        */
        if((count % 2) == 0)
        {
            stype = 0;
            waveIndex = 0;
        }
        else
        {
            stype = 1;
            waveIndex = 6;
        }

        for(int i=0;i<3;i++)
        {
            vVolWave[i].clear();
            vCurWave[i].clear();
        }
        for(int i=0;i<6;i++)
        {
            vAccWave[i].clear();
        }

        QMap<int,QString> waves = db->queryWaveSql(stype);

        for(int i=1;i<7;i++)
        {
            if(waves.contains(i))
            {
                QString value = waves[i];
                QStringList items = value.split(',');
                QStringList::ConstIterator it=items.constBegin(),endit=items.constEnd();

                bool tok;
                for(;it != endit;it++)
                {
                    QString v = *it;
                    float v_f = v.toFloat(&tok);
                    if(!tok)
                    {
                        v_f = 0;
                    }
                    if(0 == stype)
                    {
                        if(i<4)
                        {
                            vVolWave[i-1].push_back(v_f);
                        }
                        else
                        {
                            vCurWave[i-1-3].push_back(v_f);
                        }
                    }
                    else if(1 == stype)
                    {
                        vAccWave[i-1].push_back(v_f);
                    }

                    //pInputDatas->inputWaveDatas.waveData[waveIndex + i-1].push_back(v_f);
                }
            }
        }
        try
        {
            int i;

            if (0 == stype)
            {
                bool init = false;
                for(i=0;i<3;i++)
                {
                    if(vVolWave[i].size() <= 0)
                    {
                        init=true;
                    }
                    if(vCurWave[i].size() <= 0)
                    {
                        init=true;
                    }
                }
                for(i=0;i<6;i++)
                {
                    if(vAccWave[i].size() <= 0)
                    {
                        init=true;
                    }
                }
                if(init)
                {
                    continue;
                }
                for (int i = 0; i != 3; i++)
                {
                    if(pElcAnsyDataLib->pVolWave[i] != NULL) continue;
                    pElcWaveAnsys->getAnsyVolWaveByInitVolWave(vVolWave[i],5,-1,FilterTypeNone,i);
                }

                for (int i = 0; i != 3; i++)
                {
                    if(pElcAnsyDataLib->pCurWave[i] != NULL) continue;
                    pElcWaveAnsys->getAnsyCurWaveByInitCurWave(vCurWave[i],5,-1,FilterTypeNone,i);
                }

                //频谱
                for (i = 0; i != 3; i++)
                {
                    if(pElcAnsyDataLib->pVolSpectrum[i] != NULL) continue;
                    pElcWaveAnsys->getVolSpectrum(*(pElcAnsyDataLib->pVolWave[i]),RectangleWindow,i);
                }
                for (i = 0; i != 3; i++)
                {
                    if(pElcAnsyDataLib->pCurSpectrum[i] != NULL) continue;
                    pElcWaveAnsys->getCurSpectrum(*(pElcAnsyDataLib->pCurWave[i]),RectangleWindow,i);
                }

                for (i = 0; i != 3; i++)
                {
                    if(pElcAnsyDataLib->pVolFreqDmnFeat[i] != NULL) continue;
                    pElcWaveAnsys->getVolFreqDmnFeat(*(pElcAnsyDataLib->pVolWave[i]),50,i);

                    if(pElcAnsyDataLib->pCurFreqDmnFeat[i] != NULL) continue;
                    pElcWaveAnsys->getCurFreqDmnFeat(*(pElcAnsyDataLib->pCurWave[i]),50,i);
                }

                //时阈特征
                for (i = 0; i != 3; i++)
                {
                    if(pElcAnsyDataLib->pVolTimeDmnFeat[i] != NULL) continue;
                    pElcWaveAnsys->getVolTimeDmnFeat(*(pElcAnsyDataLib->pVolWave[i]),i);

                    if(pElcAnsyDataLib->pCurTimeDmnFeat[i] != NULL) continue;
                    pElcWaveAnsys->getCurTimeDmnFeat(*(pElcAnsyDataLib->pCurWave[i]),i);

                    if(pElcAnsyDataLib->pPowerAnsysFeat[i] != NULL) continue;
                    pElcWaveAnsys->getPowerAnsyFeat(*(pElcAnsyDataLib->pVolWave[i]),*(pElcAnsyDataLib->pCurWave[i]),i);
                }

            }
            else if(1 == stype)
            {
                //波形
                for (i = 0; i != 6; i++)
                {
                    if(pVibAnsyDataLib->pAccWave[i] != NULL) continue;
                    pVibWaveAnsys->getAnsyAccWaveByInitAccWave(vAccWave[i],5,-1,HghPassFilterType,i);
                }
                for (i = 0; i != 6; i++)
                {
                    if(pVibAnsyDataLib->pSpdWave[i] != NULL) continue;
                    pVibWaveAnsys->getAnsySpdWaveByAnsyAccWave(*(pVibAnsyDataLib->pAccWave[i]),5,-1,HghPassFilterType,i);
                }
                for (i = 0; i != 6; i++)
                {
                    if(pVibAnsyDataLib->pDisWave[i] != NULL) continue;
                    pVibWaveAnsys->getAnsyDisWaveByAnsySpdWave(*(pVibAnsyDataLib->pSpdWave[i]),5,-1,HghPassFilterType,i);
                }

                for (i = 0; i != 6; i++)
                {
                    if(pVibAnsyDataLib->pAccTimeDmnFeat[i] != NULL) continue;
                    pVibWaveAnsys->getAccTimeDmnFeat(*(pVibAnsyDataLib->pAccWave[i]),i);

                    if(pVibAnsyDataLib->pSpdTimeDmnFeat[i] != NULL) continue;
                    pVibWaveAnsys->getSpdTimeDmnFeat(*(pVibAnsyDataLib->pSpdWave[i]),i);
                }

                //频谱
                for (i = 0; i != 6; i++)
                {
                    if(pVibAnsyDataLib->pAccSpectrum[i] != NULL) continue;
                    pVibWaveAnsys->getAccSpectrum(*(pVibAnsyDataLib->pAccWave[i]),RectangleWindow,i);
                }
                for (i = 0; i != 6; i++)
                {
                    if(pVibAnsyDataLib->pSpdSpectrum[i] != NULL) continue;
                    pVibWaveAnsys->getSpdSpectrum(*(pVibAnsyDataLib->pSpdWave[i]),RectangleWindow,i);
                }

                for (i = 0; i != 6; i++)
                {
                    if(pVibAnsyDataLib->pAccFreqDmnFeat[i] != NULL) continue;
                    pVibWaveAnsys->getAccFreqDmnFeat(*(pVibAnsyDataLib->pAccWave[i]),50,i);

                    if(pVibAnsyDataLib->pSpdFreqDmnFeat[i] != NULL) continue;
                    pVibWaveAnsys->getSpdFreqDmnFeat(*(pVibAnsyDataLib->pSpdWave[i]),50,i);
                }

                //包络
                for (i = 0; i != 6; i++)
                {
                    if(pVibAnsyDataLib->pEnvSpectrum[i] != NULL) continue;
                    pVibWaveAnsys->getAccEnvelope(*(pVibAnsyDataLib->pAccWave[i]),5,4000,RectangleWindow,i);
                }
            }
        }
        catch(QException e)
        {
            qDebug() << e.what();
            continue;
        }


        QMap<int,int> waveData;
        if(waves.contains(0))
        {
            QString config = waves[0];

            QStringList items = config.split(',');
            QStringList::ConstIterator it=items.constBegin(),endit=items.constEnd();

            bool tok;
            for(;it != endit;it++)
            {
                QString v = *it;

                QStringList channels = v.split(':');
                if (channels.size() == 2)
                {
                    QString channel_index = channels.at(0);
                    QString channel_id = channels.at(1);
                    int i_index = channel_index.toInt(&tok);
                    int i_id = channel_id.toInt(&tok);

                    waveData[i_index] = i_id;
                }
            }

            QMap<QString,QVariantList> binds;

            //频谱数据
            for (int i = 0; i < 6; ++i)
            {
                QString ampt_str = "";

                int wave_id = 0;
                if(waveData.contains(i+1))
                {
                    wave_id = waveData[i+1];
                }
                if(wave_id < 1)
                {
                    continue;
                }

                DOUBLE_VCT *ampt = nullptr;
                if(0 == stype)
                {
                    if(i<3)
                    {
                        ampt = pElcAnsyDataLib->pVolSpectrum[i];
                    }
                    else
                    {
                        ampt = pElcAnsyDataLib->pCurSpectrum[i-3];
                    }
                }
                else if(1 == stype)
                {
                    ampt = pVibAnsyDataLib->pAccSpectrum[i];
                }
                if (ampt == nullptr)
                {
                    continue;
                }

                DOUBLE_VCT::iterator it;
                for (it = ampt->begin(); it != ampt->end(); it++)
                {
                    float tmp = *it;
                    ampt_str += QString::number(tmp,10,4) + ",";
                }
                int blen = ampt_str.length();
                if (blen > 0)
                {
                    ampt_str = ampt_str.left(blen-1);
                }

                if(binds.contains("ampt"))
                {
                    binds["ampt"].append(ampt_str);
                }
                else
                {
                    QVariantList ampts;
                    ampts.append(ampt_str);
                    binds.insert("ampt",ampts);
                }

                if(binds.contains("wave_type"))
                {
                    binds["wave_type"].append(QString::number(stype));
                }
                else
                {
                    QVariantList stypes;
                    stypes.append(QString::number(stype));
                    binds.insert("wave_type",stypes);
                }

                if(binds.contains("wave_id"))
                {
                    binds["wave_id"].append(QString::number(wave_id));
                }
                else
                {
                    QVariantList ids;
                    ids.append(QString::number(wave_id));
                    binds.insert("wave_id",ids);
                }
                if(binds.contains("wid"))
                {
                    binds["wid"].append(0);
                }
                else
                {
                    QVariantList wids;
                    wids.append(0);
                    binds.insert("wid",wids);
                }
            }
            if(1==stype)
            {
                for (int i = 0; i < 6; ++i)
                {
                    QString ampt_str = "";

                    int wave_id = 0;
                    if(waveData.contains(i+1))
                    {
                        wave_id = waveData[i+1];
                    }
                    if(wave_id < 1)
                    {
                        continue;
                    }

                    DOUBLE_VCT *ampt = nullptr;

                    ampt = pVibAnsyDataLib->pSpdSpectrum[i];
                    if (ampt == nullptr)
                    {
                        continue;
                    }

                    DOUBLE_VCT::iterator it;
                    for (it = ampt->begin(); it != ampt->end(); it++)
                    {
                        float tmp = *it;
                        ampt_str += QString::number(tmp,10,4) + ",";
                    }
                    int blen = ampt_str.length();
                    if (blen > 0)
                    {
                        ampt_str = ampt_str.left(blen-1);
                    }

                    if(binds.contains("ampt"))
                    {
                        binds["ampt"].append(ampt_str);
                    }
                    else
                    {
                        QVariantList ampts;
                        ampts.append(ampt_str);
                        binds.insert("ampt",ampts);
                    }

                    if(binds.contains("wave_type"))
                    {
                        binds["wave_type"].append(QString::number(2));
                    }
                    else
                    {
                        QVariantList stypes;
                        stypes.append(QString::number(2));
                        binds.insert("wave_type",stypes);
                    }

                    if(binds.contains("wave_id"))
                    {
                        binds["wave_id"].append(QString::number(wave_id));
                    }
                    else
                    {
                        QVariantList ids;
                        ids.append(QString::number(wave_id));
                        binds.insert("wave_id",ids);
                    }
                    if(binds.contains("wid"))
                    {
                        binds["wid"].append(0);
                    }
                    else
                    {
                        QVariantList wids;
                        wids.append(0);
                        binds.insert("wid",wids);
                    }
                }

                for (int i = 0; i < 6; ++i)
                {
                    QString ampt_str = "";

                    int wave_id = 0;
                    if(waveData.contains(i+1))
                    {
                        wave_id = waveData[i+1];
                    }
                    if(wave_id < 1)
                    {
                        continue;
                    }

                    DOUBLE_VCT *ampt = nullptr;

                    ampt = pVibAnsyDataLib->pEnvSpectrum[i];
                    if (ampt == nullptr)
                    {
                        continue;
                    }

                    DOUBLE_VCT::iterator it;
                    for (it = ampt->begin(); it != ampt->end(); it++)
                    {
                        float tmp = *it;
                        ampt_str += QString::number(tmp,10,4) + ",";
                    }
                    int blen = ampt_str.length();
                    if (blen > 0)
                    {
                        ampt_str = ampt_str.left(blen-1);
                    }

                    if(binds.contains("ampt"))
                    {
                        binds["ampt"].append(ampt_str);
                    }
                    else
                    {
                        QVariantList ampts;
                        ampts.append(ampt_str);
                        binds.insert("ampt",ampts);
                    }

                    if(binds.contains("wave_type"))
                    {
                        binds["wave_type"].append(QString::number(3));
                    }
                    else
                    {
                        QVariantList stypes;
                        stypes.append(QString::number(3));
                        binds.insert("wave_type",stypes);
                    }

                    if(binds.contains("wave_id"))
                    {
                        binds["wave_id"].append(QString::number(wave_id));
                    }
                    else
                    {
                        QVariantList ids;
                        ids.append(QString::number(wave_id));
                        binds.insert("wave_id",ids);
                    }
                    if(binds.contains("wid"))
                    {
                        binds["wid"].append(0);
                    }
                    else
                    {
                        QVariantList wids;
                        wids.append(0);
                        binds.insert("wid",wids);
                    }
                }
            }
            if(binds.size()>0)
            {
                QString sql = "insert into index_wavefrequencyanalys (ampt,wave_type,wave_id,wid) values (:ampt,:wave_type,:wave_id,:wid)";
                db->execSql(sql,binds);
            }

            //20x频谱数据
            binds.clear();
            if(0 == stype)
            {
                for (int i = 0; i < 3; ++i)
                {
                    QString ampt_str = "";

                    int wave_id = 0;
                    if(waveData.contains(i+1))
                    {
                        wave_id = waveData[i+1];
                    }
                    if(wave_id < 1)
                    {
                        continue;
                    }

                    SVolFreqDmnFeat *ampt = nullptr;

                    ampt = pElcAnsyDataLib->pVolFreqDmnFeat[i];

                    if (ampt == nullptr)
                    {
                        continue;
                    }

                    for(int k=0;k<Vol_Freq_Order;k++)
                    {
                        ampt_str += QString::number(ampt->volMultFrq[k],10,4) + ",";
                    }

                    int blen = ampt_str.length();
                    if (blen > 0)
                    {
                        ampt_str = ampt_str.left(blen-1);
                    }

                    if(binds.contains("ampt"))
                    {
                        binds["ampt"].append(ampt_str);
                    }
                    else
                    {
                        QVariantList ampts;
                        ampts.append(ampt_str);
                        binds.insert("ampt",ampts);
                    }
                    if(binds.contains("wave_type"))
                    {
                        binds["wave_type"].append(QString::number(0));
                    }
                    else
                    {
                        QVariantList stypes;
                        stypes.append(QString::number(0));
                        binds.insert("wave_type",stypes);
                    }

                    if(binds.contains("wave_id"))
                    {
                        binds["wave_id"].append(QString::number(wave_id));
                    }
                    else
                    {
                        QVariantList ids;
                        ids.append(QString::number(wave_id));
                        binds.insert("wave_id",ids);
                    }
                    if(binds.contains("wid"))
                    {
                        binds["wid"].append(0);
                    }
                    else
                    {
                        QVariantList wids;
                        wids.append(0);
                        binds.insert("wid",wids);
                    }
                }

                for (int i = 0; i < 3; ++i)
                {
                    QString ampt_str = "";

                    int wave_id = 0;
                    if(waveData.contains(i+1))
                    {
                        wave_id = waveData[i+1];
                    }
                    if(wave_id < 1)
                    {
                        continue;
                    }

                    SCurFreqDmnFeat *ampt = nullptr;

                    ampt = pElcAnsyDataLib->pCurFreqDmnFeat[i];

                    if (ampt == nullptr)
                    {
                        continue;
                    }

                    for(int k=0;k<Cur_Freq_Order;k++)
                    {
                        ampt_str += QString::number(ampt->curMultFrq[k],10,4) + ",";
                    }

                    int blen = ampt_str.length();
                    if (blen > 0)
                    {
                        ampt_str = ampt_str.left(blen-1);
                    }

                    if(binds.contains("ampt"))
                    {
                        binds["ampt"].append(ampt_str);
                    }
                    else
                    {
                        QVariantList ampts;
                        ampts.append(ampt_str);
                        binds.insert("ampt",ampts);
                    }
                    if(binds.contains("wave_type"))
                    {
                        binds["wave_type"].append(QString::number(1));
                    }
                    else
                    {
                        QVariantList stypes;
                        stypes.append(QString::number(1));
                        binds.insert("wave_type",stypes);
                    }

                    if(binds.contains("wave_id"))
                    {
                        binds["wave_id"].append(QString::number(wave_id));
                    }
                    else
                    {
                        QVariantList ids;
                        ids.append(QString::number(wave_id));
                        binds.insert("wave_id",ids);
                    }
                    if(binds.contains("wid"))
                    {
                        binds["wid"].append(0);
                    }
                    else
                    {
                        QVariantList wids;
                        wids.append(0);
                        binds.insert("wid",wids);
                    }
                }
            }
            else if(1 == stype)
            {
                for (int i = 0; i < 6; ++i)
                {
                    QString ampt_str = "";

                    int wave_id = 0;
                    if(waveData.contains(i+1))
                    {
                        wave_id = waveData[i+1];
                    }
                    if(wave_id < 1)
                    {
                        continue;
                    }

                    SAccFreqDmnFeat *ampt = nullptr;

                    ampt = pVibAnsyDataLib->pAccFreqDmnFeat[i];

                    if (ampt == nullptr)
                    {
                        continue;
                    }

                    for(int k=0;k<Acc_Freq_Order;k++)
                    {
                        ampt_str += QString::number(ampt->accMultFrq[k],10,4) + ",";
                    }

                    int blen = ampt_str.length();
                    if (blen > 0)
                    {
                        ampt_str = ampt_str.left(blen-1);
                    }

                    if(binds.contains("ampt"))
                    {
                        binds["ampt"].append(ampt_str);
                    }
                    else
                    {
                        QVariantList ampts;
                        ampts.append(ampt_str);
                        binds.insert("ampt",ampts);
                    }
                    if(binds.contains("wave_type"))
                    {
                        binds["wave_type"].append(QString::number(2));
                    }
                    else
                    {
                        QVariantList stypes;
                        stypes.append(QString::number(2));
                        binds.insert("wave_type",stypes);
                    }

                    if(binds.contains("wave_id"))
                    {
                        binds["wave_id"].append(QString::number(wave_id));
                    }
                    else
                    {
                        QVariantList ids;
                        ids.append(QString::number(wave_id));
                        binds.insert("wave_id",ids);
                    }
                    if(binds.contains("wid"))
                    {
                        binds["wid"].append(0);
                    }
                    else
                    {
                        QVariantList wids;
                        wids.append(0);
                        binds.insert("wid",wids);
                    }
                }

                for (int i = 0; i < 6; ++i)
                {
                    QString ampt_str = "";

                    int wave_id = 0;
                    if(waveData.contains(i+1))
                    {
                        wave_id = waveData[i+1];
                    }
                    if(wave_id < 1)
                    {
                        continue;
                    }

                    SSpdFreqDmnFeat *ampt = nullptr;

                    ampt = pVibAnsyDataLib->pSpdFreqDmnFeat[i];

                    if (ampt == nullptr)
                    {
                        continue;
                    }

                    for(int k=0;k<Spd_Freq_Order;k++)
                    {
                        ampt_str += QString::number(ampt->spdMultFrq[k],10,4) + ",";
                    }

                    int blen = ampt_str.length();
                    if (blen > 0)
                    {
                        ampt_str = ampt_str.left(blen-1);
                    }

                    if(binds.contains("ampt"))
                    {
                        binds["ampt"].append(ampt_str);
                    }
                    else
                    {
                        QVariantList ampts;
                        ampts.append(ampt_str);
                        binds.insert("ampt",ampts);
                    }
                    if(binds.contains("wave_type"))
                    {
                        binds["wave_type"].append(QString::number(3));
                    }
                    else
                    {
                        QVariantList stypes;
                        stypes.append(QString::number(3));
                        binds.insert("wave_type",stypes);
                    }

                    if(binds.contains("wave_id"))
                    {
                        binds["wave_id"].append(QString::number(wave_id));
                    }
                    else
                    {
                        QVariantList ids;
                        ids.append(QString::number(wave_id));
                        binds.insert("wave_id",ids);
                    }
                    if(binds.contains("wid"))
                    {
                        binds["wid"].append(0);
                    }
                    else
                    {
                        QVariantList wids;
                        wids.append(0);
                        binds.insert("wid",wids);
                    }
                }

                for (int i = 0; i < 6; ++i)
                {
                    QString ampt_str = "";

                    int wave_id = 0;
                    if(waveData.contains(i+1))
                    {
                        wave_id = waveData[i+1];
                    }
                    if(wave_id < 1)
                    {
                        continue;
                    }

                    SEnvFreqDmnFeat *ampt = nullptr;

                    ampt = pVibAnsyDataLib->pEnvFreqDmnFeat[i];

                    if (ampt == nullptr)
                    {
                        continue;
                    }

                    for(int k=0;k<Env_Freq_Order;k++)
                    {
                        ampt_str += QString::number(ampt->enveMultFrq[k],10,4) + ",";
                    }

                    int blen = ampt_str.length();
                    if (blen > 0)
                    {
                        ampt_str = ampt_str.left(blen-1);
                    }

                    if(binds.contains("ampt"))
                    {
                        binds["ampt"].append(ampt_str);
                    }
                    else
                    {
                        QVariantList ampts;
                        ampts.append(ampt_str);
                        binds.insert("ampt",ampts);
                    }
                    if(binds.contains("wave_type"))
                    {
                        binds["wave_type"].append(QString::number(4));
                    }
                    else
                    {
                        QVariantList stypes;
                        stypes.append(QString::number(4));
                        binds.insert("wave_type",stypes);
                    }

                    if(binds.contains("wave_id"))
                    {
                        binds["wave_id"].append(QString::number(wave_id));
                    }
                    else
                    {
                        QVariantList ids;
                        ids.append(QString::number(wave_id));
                        binds.insert("wave_id",ids);
                    }
                    if(binds.contains("wid"))
                    {
                        binds["wid"].append(0);
                    }
                    else
                    {
                        QVariantList wids;
                        wids.append(0);
                        binds.insert("wid",wids);
                    }
                }
            }

            if(binds.size()>0)
            {
                QString sql = "insert into index_wavefrequencyxanalys (ampt,wave_type,wave_id,wid) values (:ampt,:wave_type,:wave_id,:wid)";
                db->execSql(sql,binds);
            }

            //时域信息
            binds.clear();

            for (int i = 0; i < 6; ++i)
            {
                QString wave_str = "";

                int wave_id = 0;
                if(waveData.contains(i+1))
                {
                    wave_id = waveData[i+1];
                }
                if(wave_id < 1)
                {
                    continue;
                }

                DOUBLE_VCT *wave = nullptr;
                float ampValue,rmsValue,pkpkValue;
                ampValue = rmsValue = pkpkValue = 0.0;
                float waveIndex,peakIndex,pulsIndex,kurtIndex;
                waveIndex = peakIndex = pulsIndex = kurtIndex = 0;
                if(0 == stype)
                {
                    if(i<3)
                    {
                        wave = pElcAnsyDataLib->pVolWave[i];
                        ampValue = pElcAnsyDataLib->pVolTimeDmnFeat[i]->AmpValue;
                        rmsValue = pElcAnsyDataLib->pVolTimeDmnFeat[i]->RMSValue;
                        pkpkValue = pElcAnsyDataLib->pVolTimeDmnFeat[i]->PkPkValue;

                        waveIndex = pElcAnsyDataLib->pPowerAnsysFeat[i]->actPower;
                        peakIndex = pElcAnsyDataLib->pPowerAnsysFeat[i]->reactPower;
                        pulsIndex = pElcAnsyDataLib->pPowerAnsysFeat[i]->appPower;
                        kurtIndex = pElcAnsyDataLib->pPowerAnsysFeat[i]->cosf;
                    }
                    else
                    {
                        wave = pElcAnsyDataLib->pCurWave[i-3];
                        ampValue = pElcAnsyDataLib->pCurTimeDmnFeat[i-3]->AmpValue;
                        rmsValue = pElcAnsyDataLib->pCurTimeDmnFeat[i-3]->RMSValue;
                        pkpkValue = pElcAnsyDataLib->pCurTimeDmnFeat[i-3]->PkPkValue;

                        waveIndex = pElcAnsyDataLib->pPowerAnsysFeat[i-3]->actPower;
                        peakIndex = pElcAnsyDataLib->pPowerAnsysFeat[i-3]->reactPower;
                        pulsIndex = pElcAnsyDataLib->pPowerAnsysFeat[i-3]->appPower;
                        kurtIndex = pElcAnsyDataLib->pPowerAnsysFeat[i-3]->cosf;
                    }
                }
                else if(1 == stype)
                {
                    wave = pVibAnsyDataLib->pAccWave[i];

                    ampValue = pVibAnsyDataLib->pAccTimeDmnFeat[i]->AmpValue;
                    rmsValue = pVibAnsyDataLib->pAccTimeDmnFeat[i]->RMSValue;
                    pkpkValue = pVibAnsyDataLib->pAccTimeDmnFeat[i]->PkPkValue;

                    waveIndex = pVibAnsyDataLib->pAccTimeDmnFeat[i]->waveIndex;
                    peakIndex = pVibAnsyDataLib->pAccTimeDmnFeat[i]->peakIndex;
                    pulsIndex = pVibAnsyDataLib->pAccTimeDmnFeat[i]->pulsIndex;
                    kurtIndex = pVibAnsyDataLib->pAccTimeDmnFeat[i]->kurtIndex;
                }
                if (wave == nullptr)
                {
                    continue;
                }

                DOUBLE_VCT::iterator it;
                for (it = wave->begin(); it != wave->end(); it++)
                {
                    float tmp = *it;
                    wave_str += QString::number(tmp,10,4) + ",";
                }
                int blen = wave_str.length();
                if (blen > 0)
                {
                    wave_str = wave_str.left(blen-1);
                }

                if(binds.contains("wave_data"))
                {
                    binds["wave_data"].append(wave_str);
                }
                else
                {
                    QVariantList waveds;
                    waveds.append(wave_str);
                    binds.insert("wave_data",waveds);
                }
                if(binds.contains("wave_type"))
                {
                    binds["wave_type"].append(QString::number(stype));
                }
                else
                {
                    QVariantList stypes;
                    stypes.append(QString::number(stype));
                    binds.insert("wave_type",stypes);
                }
                if(binds.contains("wave_id"))
                {
                    binds["wave_id"].append(QString::number(wave_id));
                }
                else
                {
                    QVariantList ids;
                    ids.append(QString::number(wave_id));
                    binds.insert("wave_id",ids);
                }
                if(binds.contains("wid"))
                {
                    binds["wid"].append(0);
                }
                else
                {
                    QVariantList wids;
                    wids.append(0);
                    binds.insert("wid",wids);
                }

                if(!qIsFinite(ampValue) || qIsNaN(ampValue))
                {
                    ampValue = 0.0;
                }
                QString amp_str = QString::number(ampValue,10,4);
                if(binds.contains("amp"))
                {
                    binds["amp"].append(amp_str);
                }
                else
                {
                    QVariantList amps;
                    amps.append(amp_str);
                    binds.insert("amp",amps);
                }

                if(!qIsFinite(rmsValue) || qIsNaN(rmsValue))
                {
                    rmsValue = 0.0;
                }
                QString rms_str = QString::number(rmsValue,10,4);
                if(binds.contains("rms"))
                {
                    binds["rms"].append(rms_str);
                }
                else
                {
                    QVariantList rmss;
                    rmss.append(rms_str);
                    binds.insert("rms",rmss);
                }

                if(!qIsFinite(pkpkValue) || qIsNaN(pkpkValue))
                {
                    pkpkValue = 0.0;
                }
                QString pkpk_str = QString::number(pkpkValue,10,4);
                if(binds.contains("pkpk"))
                {
                    binds["pkpk"].append(pkpk_str);
                }
                else
                {
                    QVariantList pkpks;
                    pkpks.append(pkpk_str);
                    binds.insert("pkpk",pkpks);
                }

                if(!qIsFinite(waveIndex) || qIsNaN(waveIndex))
                {
                    waveIndex = 0.0;
                }
                QString waveindex_str = QString::number(waveIndex,10,4);
                if(binds.contains("waveindex"))
                {
                    binds["waveindex"].append(waveindex_str);
                }
                else
                {
                    QVariantList waveindexs;
                    waveindexs.append(waveindex_str);
                    binds.insert("waveindex",waveindexs);
                }

                if(!qIsFinite(peakIndex) || qIsNaN(peakIndex))
                {
                    peakIndex = 0.0;
                }
                QString peakVueIndex_str = QString::number(peakIndex,10,4);
                if(binds.contains("peakVueIndex"))
                {
                    binds["peakVueIndex"].append(peakVueIndex_str);
                }
                else
                {
                    QVariantList peakVueIndexs;
                    peakVueIndexs.append(peakVueIndex_str);
                    binds.insert("peakVueIndex",peakVueIndexs);
                }

                if(!qIsFinite(pulsIndex) || qIsNaN(pulsIndex))
                {
                    pulsIndex = 0.0;
                }
                QString pulseIndex_str = QString::number(pulsIndex,10,4);
                if(binds.contains("pulseIndex"))
                {
                    binds["pulseIndex"].append(pulseIndex_str);
                }
                else
                {
                    QVariantList pulseIndexs;
                    pulseIndexs.append(pulseIndex_str);
                    binds.insert("pulseIndex",pulseIndexs);
                }

                if(!qIsFinite(kurtIndex) || qIsNaN(kurtIndex))
                {
                    kurtIndex = 0.0;
                }
                QString kurtosisIndex_str = QString::number(kurtIndex,10,4);
                if(binds.contains("kurtosisIndex"))
                {
                    binds["kurtosisIndex"].append(kurtosisIndex_str);
                }
                else
                {
                    QVariantList kurtosisIndexs;
                    kurtosisIndexs.append(kurtosisIndex_str);
                    binds.insert("kurtosisIndex",kurtosisIndexs);
                }

                /*
                if(binds.contains("marginIndex"))
                {
                    binds["marginIndex"].append("0");
                }
                else
                {
                    QVariantList marginIndexs;
                    marginIndexs.append("0");
                    binds.insert("marginIndex",marginIndexs);
                }

                if(binds.contains("skewnessIndex"))
                {
                    binds["skewnessIndex"].append("0");
                }
                else
                {
                    QVariantList skewnessIndexs;
                    skewnessIndexs.append("0");
                    binds.insert("skewnessIndex",skewnessIndexs);
                }*/
            }

            if(1 == stype)
            {
                for (int i = 0; i < 6; ++i)
                {
                    QString wave_str = "";

                    int wave_id = 0;
                    if(waveData.contains(i+1))
                    {
                        wave_id = waveData[i+1];
                    }
                    if(wave_id < 1)
                    {
                        continue;
                    }

                    DOUBLE_VCT *wave = nullptr;
                    float ampValue,rmsValue,pkpkValue;
                    ampValue = rmsValue = pkpkValue = 0.0;
                    float waveIndex,peakIndex,pulsIndex,kurtIndex;
                    waveIndex = peakIndex = pulsIndex = kurtIndex = 0;

                    wave = pVibAnsyDataLib->pSpdWave[i];

                    ampValue = pVibAnsyDataLib->pSpdTimeDmnFeat[i]->AmpValue;
                    rmsValue = pVibAnsyDataLib->pSpdTimeDmnFeat[i]->RMSValue;
                    pkpkValue = pVibAnsyDataLib->pSpdTimeDmnFeat[i]->PkPkValue;

                    waveIndex = pVibAnsyDataLib->pSpdTimeDmnFeat[i]->waveIndex;
                    peakIndex = pVibAnsyDataLib->pSpdTimeDmnFeat[i]->peakIndex;
                    pulsIndex = pVibAnsyDataLib->pSpdTimeDmnFeat[i]->pulsIndex;
                    kurtIndex = pVibAnsyDataLib->pSpdTimeDmnFeat[i]->kurtIndex;

                    if (wave == nullptr)
                    {
                        continue;
                    }

                    DOUBLE_VCT::iterator it;
                    for (it = wave->begin(); it != wave->end(); it++)
                    {
                        float tmp = *it;
                        wave_str += QString::number(tmp,10,4) + ",";
                    }
                    int blen = wave_str.length();
                    if (blen > 0)
                    {
                        wave_str = wave_str.left(blen-1);
                    }

                    if(binds.contains("wave_data"))
                    {
                        binds["wave_data"].append(wave_str);
                    }
                    else
                    {
                        QVariantList waveds;
                        waveds.append(wave_str);
                        binds.insert("wave_data",waveds);
                    }
                    if(binds.contains("wave_type"))
                    {
                        binds["wave_type"].append(QString::number(2));
                    }
                    else
                    {
                        QVariantList stypes;
                        stypes.append(QString::number(2));
                        binds.insert("wave_type",stypes);
                    }
                    if(binds.contains("wave_id"))
                    {
                        binds["wave_id"].append(QString::number(wave_id));
                    }
                    else
                    {
                        QVariantList ids;
                        ids.append(QString::number(wave_id));
                        binds.insert("wave_id",ids);
                    }
                    if(binds.contains("wid"))
                    {
                        binds["wid"].append(0);
                    }
                    else
                    {
                        QVariantList wids;
                        wids.append(0);
                        binds.insert("wid",wids);
                    }

                    if(!qIsFinite(ampValue) || qIsNaN(ampValue))
                    {
                        ampValue = 0.0;
                    }
                    QString amp_str = QString::number(ampValue,10,4);
                    if(binds.contains("amp"))
                    {
                        binds["amp"].append(amp_str);
                    }
                    else
                    {
                        QVariantList amps;
                        amps.append(amp_str);
                        binds.insert("amp",amps);
                    }

                    if(!qIsFinite(rmsValue) || qIsNaN(rmsValue))
                    {
                        rmsValue = 0.0;
                    }
                    QString rms_str = QString::number(rmsValue,10,4);
                    if(binds.contains("rms"))
                    {
                        binds["rms"].append(rms_str);
                    }
                    else
                    {
                        QVariantList rmss;
                        rmss.append(rms_str);
                        binds.insert("rms",rmss);
                    }

                    if(!qIsFinite(pkpkValue) || qIsNaN(pkpkValue))
                    {
                        pkpkValue = 0.0;
                    }
                    QString pkpk_str = QString::number(pkpkValue,10,4);
                    if(binds.contains("pkpk"))
                    {
                        binds["pkpk"].append(pkpk_str);
                    }
                    else
                    {
                        QVariantList pkpks;
                        pkpks.append(pkpk_str);
                        binds.insert("pkpk",pkpks);
                    }

                    if(!qIsFinite(waveIndex) || qIsNaN(waveIndex))
                    {
                        waveIndex = 0.0;
                    }
                    QString waveindex_str = QString::number(waveIndex,10,4);
                    if(binds.contains("waveindex"))
                    {
                        binds["waveindex"].append(waveindex_str);
                    }
                    else
                    {
                        QVariantList waveindexs;
                        waveindexs.append(waveindex_str);
                        binds.insert("waveindex",waveindexs);
                    }

                    if(!qIsFinite(peakIndex) || qIsNaN(peakIndex))
                    {
                        peakIndex = 0.0;
                    }
                    QString peakVueIndex_str = QString::number(peakIndex,10,4);
                    if(binds.contains("peakVueIndex"))
                    {
                        binds["peakVueIndex"].append(peakVueIndex_str);
                    }
                    else
                    {
                        QVariantList peakVueIndexs;
                        peakVueIndexs.append(peakVueIndex_str);
                        binds.insert("peakVueIndex",peakVueIndexs);
                    }

                    if(!qIsFinite(pulsIndex) || qIsNaN(pulsIndex))
                    {
                        pulsIndex = 0.0;
                    }
                    QString pulseIndex_str = QString::number(pulsIndex,10,4);
                    if(binds.contains("pulseIndex"))
                    {
                        binds["pulseIndex"].append(pulseIndex_str);
                    }
                    else
                    {
                        QVariantList pulseIndexs;
                        pulseIndexs.append(pulseIndex_str);
                        binds.insert("pulseIndex",pulseIndexs);
                    }

                    if(!qIsFinite(kurtIndex) || qIsNaN(kurtIndex))
                    {
                        kurtIndex = 0.0;
                    }
                    QString kurtosisIndex_str = QString::number(kurtIndex,10,4);
                    if(binds.contains("kurtosisIndex"))
                    {
                        binds["kurtosisIndex"].append(kurtosisIndex_str);
                    }
                    else
                    {
                        QVariantList kurtosisIndexs;
                        kurtosisIndexs.append(kurtosisIndex_str);
                        binds.insert("kurtosisIndex",kurtosisIndexs);
                    }
                }

                for (int i = 0; i < 6; ++i)
                {
                    QString wave_str = "";

                    int wave_id = 0;
                    if(waveData.contains(i+1))
                    {
                        wave_id = waveData[i+1];
                    }
                    if(wave_id < 1)
                    {
                        continue;
                    }

                    DOUBLE_VCT *wave = nullptr;
                    float ampValue,rmsValue,pkpkValue;
                    ampValue = rmsValue = pkpkValue = 0.0;
                    float waveIndex,peakIndex,pulsIndex,kurtIndex;
                    waveIndex = peakIndex = pulsIndex = kurtIndex = 0;

                    wave = pVibAnsyDataLib->pDisWave[i];

                    if (wave == nullptr)
                    {
                        continue;
                    }

                    DOUBLE_VCT::iterator it;
                    for (it = wave->begin(); it != wave->end(); it++)
                    {
                        float tmp = *it;
                        wave_str += QString::number(tmp,10,4) + ",";
                    }
                    int blen = wave_str.length();
                    if (blen > 0)
                    {
                        wave_str = wave_str.left(blen-1);
                    }

                    if(binds.contains("wave_data"))
                    {
                        binds["wave_data"].append(wave_str);
                    }
                    else
                    {
                        QVariantList waveds;
                        waveds.append(wave_str);
                        binds.insert("wave_data",waveds);
                    }
                    if(binds.contains("wave_type"))
                    {
                        binds["wave_type"].append(QString::number(3));
                    }
                    else
                    {
                        QVariantList stypes;
                        stypes.append(QString::number(2));
                        binds.insert("wave_type",stypes);
                    }
                    if(binds.contains("wave_id"))
                    {
                        binds["wave_id"].append(QString::number(wave_id));
                    }
                    else
                    {
                        QVariantList ids;
                        ids.append(QString::number(wave_id));
                        binds.insert("wave_id",ids);
                    }
                    if(binds.contains("wid"))
                    {
                        binds["wid"].append(0);
                    }
                    else
                    {
                        QVariantList wids;
                        wids.append(0);
                        binds.insert("wid",wids);
                    }

                    if(!qIsFinite(ampValue) || qIsNaN(ampValue))
                    {
                        ampValue = 0.0;
                    }
                    QString amp_str = QString::number(ampValue,10,4);
                    if(binds.contains("amp"))
                    {
                        binds["amp"].append(amp_str);
                    }
                    else
                    {
                        QVariantList amps;
                        amps.append(amp_str);
                        binds.insert("amp",amps);
                    }

                    if(!qIsFinite(rmsValue) || qIsNaN(rmsValue))
                    {
                        rmsValue = 0.0;
                    }
                    QString rms_str = QString::number(rmsValue,10,4);
                    if(binds.contains("rms"))
                    {
                        binds["rms"].append(rms_str);
                    }
                    else
                    {
                        QVariantList rmss;
                        rmss.append(rms_str);
                        binds.insert("rms",rmss);
                    }

                    if(!qIsFinite(pkpkValue) || qIsNaN(pkpkValue))
                    {
                        pkpkValue = 0.0;
                    }
                    QString pkpk_str = QString::number(pkpkValue,10,4);
                    if(binds.contains("pkpk"))
                    {
                        binds["pkpk"].append(pkpk_str);
                    }
                    else
                    {
                        QVariantList pkpks;
                        pkpks.append(pkpk_str);
                        binds.insert("pkpk",pkpks);
                    }

                    if(!qIsFinite(waveIndex) || qIsNaN(waveIndex))
                    {
                        waveIndex = 0.0;
                    }
                    QString waveindex_str = QString::number(waveIndex,10,4);
                    if(binds.contains("waveindex"))
                    {
                        binds["waveindex"].append(waveindex_str);
                    }
                    else
                    {
                        QVariantList waveindexs;
                        waveindexs.append(waveindex_str);
                        binds.insert("waveindex",waveindexs);
                    }

                    if(!qIsFinite(peakIndex) || qIsNaN(peakIndex))
                    {
                        peakIndex = 0.0;
                    }
                    QString peakVueIndex_str = QString::number(peakIndex,10,4);
                    if(binds.contains("peakVueIndex"))
                    {
                        binds["peakVueIndex"].append(peakVueIndex_str);
                    }
                    else
                    {
                        QVariantList peakVueIndexs;
                        peakVueIndexs.append(peakVueIndex_str);
                        binds.insert("peakVueIndex",peakVueIndexs);
                    }

                    if(!qIsFinite(pulsIndex) || qIsNaN(pulsIndex))
                    {
                        pulsIndex = 0.0;
                    }
                    QString pulseIndex_str = QString::number(pulsIndex,10,4);
                    if(binds.contains("pulseIndex"))
                    {
                        binds["pulseIndex"].append(pulseIndex_str);
                    }
                    else
                    {
                        QVariantList pulseIndexs;
                        pulseIndexs.append(pulseIndex_str);
                        binds.insert("pulseIndex",pulseIndexs);
                    }

                    if(!qIsFinite(kurtIndex) || qIsNaN(kurtIndex))
                    {
                        kurtIndex = 0.0;
                    }
                    QString kurtosisIndex_str = QString::number(kurtIndex,10,4);
                    if(binds.contains("kurtosisIndex"))
                    {
                        binds["kurtosisIndex"].append(kurtosisIndex_str);
                    }
                    else
                    {
                        QVariantList kurtosisIndexs;
                        kurtosisIndexs.append(kurtosisIndex_str);
                        binds.insert("kurtosisIndex",kurtosisIndexs);
                    }
                }
            }
            if(binds.size()>0)
            {
                if(0 == stype)
                {
                    QString sql = "insert into index_wavetimeanalysdata (wave_data,wave_type,amp,rms,pkpk,wave_id,wid) values (:wave_data,:wave_type,:amp,:rms,:pkpk,:wave_id,:wid)";
                    db->execSql(sql,binds);
                }
                else if (1 == stype)
                {
                    QString sql = "insert into index_wavetimeanalysdata (wave_data,wave_type,amp,rms,pkpk,waveindex,peakVueIndex,pulseIndex,kurtosisIndex,wave_id,wid) values (:wave_data,:wave_type,:amp,:rms,:pkpk,:waveindex,:peakVueIndex,:pulseIndex,:kurtosisIndex,:wave_id,:wid)";
                    db->execSql(sql,binds);
                }
            }
        }
        /*
        //调用波形分析算法
        try
        {
            mfds(pInputDatas,pOutPutDatas);

            /*
            int nSize;
            FILE *stream;

            if(0 == stype)
            {
                for (int n=0;n<6;n++)
                {
                    QString filename = "e:\\Wave" + QString::number(n) + ".txt";
                    stream = fopen(filename.toStdString().c_str(), "w");
                    nSize = pInputDatas->inputWaveDatas.waveData[n].size();
                    for (int i = 0; i != nSize; i++)
                    {
                        double tmp = pInputDatas->inputWaveDatas.waveData[n][i];
                        fprintf(stream, "%f\n", tmp);
                    }
                    fclose(stream);
                }
            }
            else
            {
                for (int n=6;n<12;n++)
                {
                    QString filename = "e:\\Wave" + QString::number(n) + ".txt";
                    stream = fopen(filename.toStdString().c_str(), "w");
                    nSize = pInputDatas->inputWaveDatas.waveData[n].size();
                    for (int i = 0; i != nSize; i++)
                    {
                        double tmp = pInputDatas->inputWaveDatas.waveData[n][i];
                        fprintf(stream, "%f\n", tmp);
                    }
                    fclose(stream);
                }
            }
            */
    /*
        }
        catch(QException e)
        {
            qDebug() << e.what();
        }

        if(waves.contains(0))
        {
            QString config = waves[0];
            QString sql;
            if(analyse_id <= 0)
            {
                analyse_id = db->getMaxId("index_analysrecord");
            }
            else
            {
                analyse_id++;
            }
            QDateTime adt = QDateTime::currentDateTime();
            QString adt_str = adt.toString("yyyy-MM-dd hh:mm:ss");
            QString dcode = "";
            if(waves.contains(50))
            {
                dcode = waves[50];
            }
            sql = "insert into index_analysRecord (uid,datatype,waves,crksj,dcode,wid,rksj) values("+QString::number(analyse_id) + "," + QString::number(stype) + ",'" + config + "','" + str_time + "','" + dcode +"',0,'" + adt_str + "')";
            //qDebug() << sql;
            if(db->updatasql(sql))
            {
                //频谱数据
                QMap<QString,QVariantList> binds;
                int min,max;

                if(0 == stype)
                {
                    min = 0;
                    max = 6;
                }
                else
                {
                    min = 6;
                    max = 18;
                }

                double delta = pOutPutDatas->outputFreqDmnDatas.deltaF;
                double rotaFreq = pOutPutDatas->outputFreqDmnDatas.rotaFreq;

                if(!qIsFinite(delta) || qIsNaN(delta))
                {
                    delta = 0.0;
                }
                if(!qIsFinite(rotaFreq) || qIsNaN(rotaFreq))
                {
                    rotaFreq = 0.0;
                }
                QString delta_str = QString::number(delta,10,4);
                QString rotaFreq_str = QString::number(rotaFreq,10,4);

                for (int i = min; i < max; ++i)
                {
                    QString ampt_str = "";
                    QString phase_str = "";

                    DOUBLE_VCT ampt = pOutPutDatas->outputFreqDmnDatas.amptFreq[i];
                    DOUBLE_VCT phase = pOutPutDatas->outputFreqDmnDatas.phasFreq[i];

                    DOUBLE_VCT::iterator it;
                    for (it = ampt.begin(); it != ampt.end(); it++)
                    {
                        float tmp = *it;
                        ampt_str += QString::number(tmp,10,4) + ",";
                    }
                    int blen = ampt_str.length();
                    if (blen > 0)
                    {
                        ampt_str = ampt_str.left(blen-1);
                    }
                    for (it = phase.begin(); it != phase.end(); it++)
                    {
                        float tmp = *it;
                        phase_str += QString::number(tmp,10,4) + ",";
                    }
                    blen = phase_str.length();
                    if (blen > 0)
                    {
                        phase_str = phase_str.left(blen-1);
                    }

                    if(binds.contains("deltaF"))
                    {
                        binds["deltaF"].append(delta_str);
                    }
                    else
                    {
                        QVariantList deltas;
                        deltas.append(delta_str);
                        binds.insert("deltaF",deltas);
                    }
                    if(binds.contains("rotaFreq"))
                    {
                        binds["rotaFreq"].append(rotaFreq_str);
                    }
                    else
                    {
                        QVariantList rotaFreqs;
                        rotaFreqs.append(rotaFreq_str);
                        binds.insert("rotaFreq",rotaFreqs);
                    }
                    if(binds.contains("ampt"))
                    {
                        binds["ampt"].append(ampt_str);
                    }
                    else
                    {
                        QVariantList ampts;
                        ampts.append(ampt_str);
                        binds.insert("ampt",ampts);
                    }
                    if(binds.contains("phase"))
                    {
                        binds["phase"].append(phase_str);
                    }
                    else
                    {
                        QVariantList phases;
                        phases.append(phase_str);
                        binds.insert("phase",phases);
                    }
                    if(binds.contains("wave_type"))
                    {
                        binds["wave_type"].append(QString::number(stype));
                    }
                    else
                    {
                        QVariantList stypes;
                        stypes.append(QString::number(stype));
                        binds.insert("wave_type",stypes);
                    }
                    int pipe_num = i+1;
                    if(1==stype)
                    {
                        pipe_num = i-min + 1;
                    }
                    if(binds.contains("pipe_type"))
                    {
                        binds["pipe_type"].append(QString::number(pipe_num));
                    }
                    else
                    {
                        QVariantList pipes;
                        pipes.append(QString::number(pipe_num));
                        binds.insert("pipe_type",pipes);
                    }
                    if(binds.contains("wave_analys_id"))
                    {
                        binds["wave_analys_id"].append(QString::number(analyse_id));
                    }
                    else
                    {
                        QVariantList analyseids;
                        analyseids.append(QString::number(analyse_id));
                        binds.insert("wave_analys_id",analyseids);
                    }
                    if(binds.contains("wid"))
                    {
                        binds["wid"].append(0);
                    }
                    else
                    {
                        QVariantList wids;
                        wids.append(0);
                        binds.insert("wid",wids);
                    }
                }
                if(binds.size()>0)
                {
                    QString sql = "insert into index_wavefrequencyanalys (deltaF,rotaFreq,ampt,phase,wave_type,pipe_type,wave_analys_id,wid) values (:deltaF,:rotaFreq,:ampt,:phase,:wave_type,:pipe_type,:wave_analys_id,:wid)";
                    db->execSql(sql,binds);
                }

                if(1==stype) //保存频域数据 包络谱
                {
                    sleep(2);
                    binds.clear();
                    for (int i = 0; i < 6; ++i)
                    {
                        QString ampt_str = "";
                        QString phase_str = "";

                        DOUBLE_VCT ampt = pOutPutDatas->outputFreqDmnDatas.amptEnvelopeFreq[i];
                        DOUBLE_VCT phase = pOutPutDatas->outputFreqDmnDatas.phasEnvelopeFreq[i];

                        DOUBLE_VCT::iterator it;
                        for (it = ampt.begin(); it != ampt.end(); it++)
                        {
                            float tmp = *it;
                            ampt_str += QString::number(tmp,10,4) + ",";
                        }
                        int blen = ampt_str.length();
                        if (blen > 0)
                        {
                            ampt_str = ampt_str.left(blen-1);
                        }
                        for (it = phase.begin(); it != phase.end(); it++)
                        {
                            float tmp = *it;
                            phase_str += QString::number(tmp,10,4) + ",";
                        }
                        blen = phase_str.length();
                        if (blen > 0)
                        {
                            phase_str = phase_str.left(blen-1);
                        }

                        if(binds.contains("ampt"))
                        {
                            binds["ampt"].append(ampt_str);
                        }
                        else
                        {
                            QVariantList ampts;
                            ampts.append(ampt_str);
                            binds.insert("ampt",ampts);
                        }
                        if(binds.contains("phase"))
                        {
                            binds["phase"].append(phase_str);
                        }
                        else
                        {
                            QVariantList phases;
                            phases.append(phase_str);
                            binds.insert("phase",phases);
                        }
                        if(binds.contains("wave_type"))
                        {
                            binds["wave_type"].append(QString::number(stype));
                        }
                        else
                        {
                            QVariantList stypes;
                            stypes.append(QString::number(stype));
                            binds.insert("wave_type",stypes);
                        }

                        if(binds.contains("pipe_type"))
                        {
                            binds["pipe_type"].append(QString::number(i+1));
                        }
                        else
                        {
                            QVariantList pipes;
                            pipes.append(QString::number(i+1));
                            binds.insert("pipe_type",pipes);
                        }
                        if(binds.contains("wave_analys_id"))
                        {
                            binds["wave_analys_id"].append(QString::number(analyse_id));
                        }
                        else
                        {
                            QVariantList analyseids;
                            analyseids.append(QString::number(analyse_id));
                            binds.insert("wave_analys_id",analyseids);
                        }
                        if(binds.contains("wid"))
                        {
                            binds["wid"].append(0);
                        }
                        else
                        {
                            QVariantList wids;
                            wids.append(0);
                            binds.insert("wid",wids);
                        }
                    }
                    if(binds.size()>0)
                    {
                        QString sql = "insert into index_waveenvelopeanalys (ampt,phase,wave_type,pipe_type,wave_analys_id,wid) values (:ampt,:phase,:wave_type,:pipe_type,:wave_analys_id,:wid)";
                        db->execSql(sql,binds);
                    }
                }

                //20x频谱数据
                sleep(2);
                binds.clear();
                for (int i = 0; i < 6; ++i)
                {
                    QString ampt_str = "";

                    DOUBLE_VCT ampt;
                    if(0 == stype)
                    {
                        ampt = pOutPutDatas->outputFreqDmnDatas.elcSpectrumTrend[i];
                    }
                    else
                    {
                        ampt = pOutPutDatas->outputFreqDmnDatas.vibSpectrumTrend[i];
                    }

                    DOUBLE_VCT::iterator it;
                    for (it = ampt.begin(); it != ampt.end(); it++)
                    {
                        float tmp = *it;
                        ampt_str += QString::number(tmp,10,4) + ",";
                    }
                    int blen = ampt_str.length();
                    if (blen > 0)
                    {
                        ampt_str = ampt_str.left(blen-1);
                    }

                    if(binds.contains("ampt"))
                    {
                        binds["ampt"].append(ampt_str);
                    }
                    else
                    {
                        QVariantList ampts;
                        ampts.append(ampt_str);
                        binds.insert("ampt",ampts);
                    }
                    if(binds.contains("wave_type"))
                    {
                        binds["wave_type"].append(QString::number(stype));
                    }
                    else
                    {
                        QVariantList stypes;
                        stypes.append(QString::number(stype));
                        binds.insert("wave_type",stypes);
                    }

                    if(binds.contains("pipe_type"))
                    {
                        binds["pipe_type"].append(QString::number(i+1));
                    }
                    else
                    {
                        QVariantList pipes;
                        pipes.append(QString::number(i+1));
                        binds.insert("pipe_type",pipes);
                    }
                    if(binds.contains("wave_analys_id"))
                    {
                        binds["wave_analys_id"].append(QString::number(analyse_id));
                    }
                    else
                    {
                        QVariantList analyseids;
                        analyseids.append(QString::number(analyse_id));
                        binds.insert("wave_analys_id",analyseids);
                    }
                    if(binds.contains("wid"))
                    {
                        binds["wid"].append(0);
                    }
                    else
                    {
                        QVariantList wids;
                        wids.append(0);
                        binds.insert("wid",wids);
                    }
                }
                if(binds.size()>0)
                {
                    QString sql = "insert into index_wavefrequencyxanalys (ampt,wave_type,pipe_type,wave_analys_id,wid) values (:ampt,:wave_type,:pipe_type,:wave_analys_id,:wid)";
                    db->execSql(sql,binds);
                }


                //保存时域数据
                sleep(2);
                binds.clear();
                min = 0;
                max = 24;
                if (stype == 0)
                {
                    max = 6;
                }
                else if (stype == 1)
                {
                    min = 6;
                }
                for (int i = min; i < max; ++i)
                {
                    QString wave_str = "";
                    DOUBLE_VCT wave = pOutPutDatas->outputTimeDmnDatas.outputWaveDatas.waveData[i];

                    DOUBLE_VCT::iterator it;
                    for (it = wave.begin(); it != wave.end(); it++)
                    {
                        float tmp = *it;
                        wave_str += QString::number(tmp,10,4) + ",";
                    }
                    int blen = wave_str.length();
                    if (blen > 0)
                    {
                        wave_str = wave_str.left(blen-1);
                    }

                    if(binds.contains("wave_data"))
                    {
                        binds["wave_data"].append(wave_str);
                    }
                    else
                    {
                        QVariantList waveds;
                        waveds.append(wave_str);
                        binds.insert("wave_data",waveds);
                    }
                    if(binds.contains("wave_type"))
                    {
                        binds["wave_type"].append(QString::number(stype));
                    }
                    else
                    {
                        QVariantList stypes;
                        stypes.append(QString::number(stype));
                        binds.insert("wave_type",stypes);
                    }

                    int pipe_num = i+1;
                    if(1==stype)
                    {
                        pipe_num = i-min + 1;
                    }
                    if(binds.contains("pipe_type"))
                    {
                        binds["pipe_type"].append(QString::number(pipe_num));
                    }
                    else
                    {
                        QVariantList pipes;
                        pipes.append(QString::number(pipe_num));
                        binds.insert("pipe_type",pipes);
                    }
                    if(binds.contains("wave_analys_id"))
                    {
                        binds["wave_analys_id"].append(QString::number(analyse_id));
                    }
                    else
                    {
                        QVariantList analyseids;
                        analyseids.append(QString::number(analyse_id));
                        binds.insert("wave_analys_id",analyseids);
                    }
                    if(binds.contains("wid"))
                    {
                        binds["wid"].append(0);
                    }
                    else
                    {
                        QVariantList wids;
                        wids.append(0);
                        binds.insert("wid",wids);
                    }

                    double tmp = pOutPutDatas->outputTimeDmnDatas.vWaveAmp[i];
                    if(!qIsFinite(tmp) || qIsNaN(tmp))
                    {
                        tmp = 0.0;
                    }
                    QString amp_str = QString::number(tmp,10,4);
                    if(binds.contains("amp"))
                    {
                        binds["amp"].append(amp_str);
                    }
                    else
                    {
                        QVariantList amps;
                        amps.append(amp_str);
                        binds.insert("amp",amps);
                    }

                    tmp = pOutPutDatas->outputTimeDmnDatas.vWaveRMS[i];
                    if(!qIsFinite(tmp) || qIsNaN(tmp))
                    {
                        tmp = 0.0;
                    }
                    QString rms_str = QString::number(tmp,10,4);
                    if(binds.contains("rms"))
                    {
                        binds["rms"].append(rms_str);
                    }
                    else
                    {
                        QVariantList rmss;
                        rmss.append(rms_str);
                        binds.insert("rms",rmss);
                    }

                    tmp = pOutPutDatas->outputTimeDmnDatas.vWavePkPkValue[i];
                    if(!qIsFinite(tmp) || qIsNaN(tmp))
                    {
                        tmp = 0.0;
                    }
                    QString pkpk_str = QString::number(tmp,10,4);
                    if(binds.contains("pkpk"))
                    {
                        binds["pkpk"].append(pkpk_str);
                    }
                    else
                    {
                        QVariantList pkpks;
                        pkpks.append(pkpk_str);
                        binds.insert("pkpk",pkpks);
                    }

                    tmp = pOutPutDatas->outputTimeDmnDatas.waveIndex[i];
                    if(!qIsFinite(tmp) || qIsNaN(tmp))
                    {
                        tmp = 0.0;
                    }
                    QString waveindex_str = QString::number(tmp,10,4);
                    if(binds.contains("waveindex"))
                    {
                        binds["waveindex"].append(waveindex_str);
                    }
                    else
                    {
                        QVariantList waveindexs;
                        waveindexs.append(waveindex_str);
                        binds.insert("waveindex",waveindexs);
                    }

                    tmp = pOutPutDatas->outputTimeDmnDatas.peakVueIndex[i];
                    if(!qIsFinite(tmp) || qIsNaN(tmp))
                    {
                        tmp = 0.0;
                    }
                    QString peakVueIndex_str = QString::number(tmp,10,4);
                    if(binds.contains("peakVueIndex"))
                    {
                        binds["peakVueIndex"].append(peakVueIndex_str);
                    }
                    else
                    {
                        QVariantList peakVueIndexs;
                        peakVueIndexs.append(peakVueIndex_str);
                        binds.insert("peakVueIndex",peakVueIndexs);
                    }

                    tmp = pOutPutDatas->outputTimeDmnDatas.pulseIndex[i];
                    if(!qIsFinite(tmp) || qIsNaN(tmp))
                    {
                        tmp = 0.0;
                    }
                    QString pulseIndex_str = QString::number(tmp,10,4);
                    if(binds.contains("pulseIndex"))
                    {
                        binds["pulseIndex"].append(pulseIndex_str);
                    }
                    else
                    {
                        QVariantList pulseIndexs;
                        pulseIndexs.append(pulseIndex_str);
                        binds.insert("pulseIndex",pulseIndexs);
                    }

                    tmp = pOutPutDatas->outputTimeDmnDatas.kurtosisIndex[i];
                    if(!qIsFinite(tmp) || qIsNaN(tmp))
                    {
                        tmp = 0.0;
                    }
                    QString kurtosisIndex_str = QString::number(tmp,10,4);
                    if(binds.contains("kurtosisIndex"))
                    {
                        binds["kurtosisIndex"].append(kurtosisIndex_str);
                    }
                    else
                    {
                        QVariantList kurtosisIndexs;
                        kurtosisIndexs.append(kurtosisIndex_str);
                        binds.insert("kurtosisIndex",kurtosisIndexs);
                    }

                    if(binds.contains("marginIndex"))
                    {
                        binds["marginIndex"].append("0");
                    }
                    else
                    {
                        QVariantList marginIndexs;
                        marginIndexs.append("0");
                        binds.insert("marginIndex",marginIndexs);
                    }

                    if(binds.contains("skewnessIndex"))
                    {
                        binds["skewnessIndex"].append("0");
                    }
                    else
                    {
                        QVariantList skewnessIndexs;
                        skewnessIndexs.append("0");
                        binds.insert("skewnessIndex",skewnessIndexs);
                    }
                }
                if(binds.size()>0)
                {
                    QString sql = "insert into index_wavetimeanalysdata (wave_data,wave_type,pipe_type,amp,rms,pkpk,waveindex,peakVueIndex,pulseIndex,kurtosisIndex,marginIndex,skewnessIndex,wave_analys_id,wid) values (:wave_data,:wave_type,:pipe_type,:amp,:rms,:pkpk,:waveindex,:peakVueIndex,:pulseIndex,:kurtosisIndex,:marginIndex,:skewnessIndex,:wave_analys_id,:wid)";
                    db->execSql(sql,binds);
                }
            }
        }

        if(nullptr != pInputDatas)
        {
            delete pInputDatas;
        }
        if(nullptr != pOutPutDatas)
        {
            delete pOutPutDatas;
        }

        pInputDatas = nullptr;
        pOutPutDatas = nullptr;

        //save data
        */
        count++;

        sleep(10);
    }
}
