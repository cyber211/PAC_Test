#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "serialreadingpollingthread.h"


#include <QTime>
#include <QDebug>
#include <qmath.h>
#include <QFile>
#include <QMessageBox>
#include <QThread>
#include <QTimer>
//#include <QStyle>
#include <QSettings>
#include <QString>
#include <QRegExp>

#include <QComboBox>

#include ".\qextserialport\qextserialport.h"
#include ".\xlsx\xlsxdocument.h"
#include ".\xlsx\xlsxconditionalformatting.h"

#include "typedefs.h"

//*********** 7252 CMD **********************************************************
#define CMD_7252i_MeasureMode "OUTP:PRES:MODE MEASURE\n"
#define CMD_7252i_ReadReadyStatus "stat:oper:cond\n"

#define CMD_7252i_VentModeA "OUTP:PRES:MODE VENT\n"      // Channel A
#define CMD_7252i_VentModeB "OUTP:PRES11:MODE VENT\n"// Channel B

#define CMD_7252i_UnitSetA "UNIT:PRES KPA;*OPC?\r"// Channel A
#define CMD_7252i_UnitSetB "UNIT:PRES11 KPA;*OPC?\r"// Channel B

#define CMD_7252i_CONTROLModeA "OUTP:PRES:MODE CONTROL\n"// Channel A
#define CMD_7252i_CONTROLModeB "OUTP:PRES11:MODE CONTROL\n"// Channel B

#define CMD_7252i_GetValue_ChannelA "MEAS:PRES?\n"// Channel A
#define CMD_7252i_GetValue_ChannelB "MEAS:PRES11?\n"// Channel B

//*********** UUT CMD **********************************************************
#define CMD_UUT_EnterShell "sh -c set -t p\n"
#define CMD_UUT_EnterMeasuereMode "state -c set -n meas\n"

#define CMD_UUT_Read_Pres_Measurement "x -n m.tffdata\n"  // Read Pressure Reading

#define CMD_UUT_Read_VOLT_Measurement "x -n m.vfdata2\n"  // Read voltage
#define CMD_UUT_Read_MA_Measurement "x -n m.vfdata2\n"  // Read voltage

//*********** ISO VALVE CMD **********************************************************
#define CMD_ISO_All_off "iso -c set -v 0x000\r\n"  // Close input valve and all the output valves;


//*********** Timer settings ，THIS has been removed , and set in the portCOM.ini ****************************
//#define WAIT_TIME_SOURCE 2*60*1000    //unit = ms, 2min,  1min for test. Include 7252 outp stable +
//#define WAIT_TIME_READ 900    //unit = ms, 1s,

#define SecondWatch 1000  //1s

//*********** excel record settings **********************************************************
#define StartRow_Pres 5  //
#define StartRow_VDC   69  //
#define StartRow_MA   72  //
#define StartRow_RTD   78  //
#define StartCol 10  //




MainWindow::MainWindow(QWidget *parent):
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //ui->MainWindow()
    //设置下拉框的初始COM口， 暂时如此，优化可以利用串口枚举只列出所有已连接串口的COM号；
    ui->comboBox_UUT1->blockSignals(true);
    ui->comboBox_UUT2->blockSignals(true);
    ui->comboBox_UUT3->blockSignals(true);
    ui->comboBox_UUT4->blockSignals(true);
    ui->comboBox_UUT5->blockSignals(true);
    ui->comboBox_UUT6->blockSignals(true);
    ui->comboBox_UUT7->blockSignals(true);
    ui->comboBox_UUT8->blockSignals(true);
    ui->comboBox_FIX_ISO->blockSignals(true);
    ui->comboBox_7252->blockSignals(true);


    int i = 0;
    for (i = 0;i <254; i++)
    {
       QString strCOM = "COM";
       strCOM = tr("COM%1").arg(i+1);

       ui->comboBox_UUT1->addItem(strCOM);
       ui->comboBox_UUT2->addItem(strCOM);
       ui->comboBox_UUT3->addItem(strCOM);
       ui->comboBox_UUT4->addItem(strCOM);
       ui->comboBox_UUT5->addItem(strCOM);
       ui->comboBox_UUT6->addItem(strCOM);
       ui->comboBox_UUT7->addItem(strCOM);
       ui->comboBox_UUT8->addItem(strCOM);
       ui->comboBox_FIX_ISO->addItem(strCOM);
       ui->comboBox_7252->addItem(strCOM);
    }
    ui->comboBox_UUT1->blockSignals(false);
    ui->comboBox_UUT2->blockSignals(false);
    ui->comboBox_UUT3->blockSignals(false);
    ui->comboBox_UUT4->blockSignals(false);
    ui->comboBox_UUT5->blockSignals(false);
    ui->comboBox_UUT6->blockSignals(false);
    ui->comboBox_UUT7->blockSignals(false);
    ui->comboBox_UUT8->blockSignals(false);
    ui->comboBox_FIX_ISO->blockSignals(false);
    ui->comboBox_7252->blockSignals(false);

    CurrentRow = StartRow_Pres;
    Total_UUT_Number = 0;

    InitialCOM_Time();

     //ui->statusBar->addPermanentWidget(ui->label_4);
     ui->statusBar->addWidget(ui->label_CurrentTime);

    // 新建设备和UUT的串口并初始化串口设置
    PortSettings settings_Eqquipments = {BAUD9600, DATA_8, PAR_NONE, STOP_1, FLOW_OFF, 10};
    //Port_F5520 = new QextSerialPort(settings_Eqquipments, QextSerialPort::EventDriven);
    //Port_F6270 = new QextSerialPort(settings_Eqquipments, QextSerialPort::EventDriven);
    Port_F7252 = new QextSerialPort(settings_Eqquipments, QextSerialPort::EventDriven);
    Port_UUT1 = new QextSerialPort(settings_Eqquipments, QextSerialPort::EventDriven);
    Port_UUT2 = new QextSerialPort(settings_Eqquipments, QextSerialPort::EventDriven);
    Port_UUT3 = new QextSerialPort(settings_Eqquipments, QextSerialPort::EventDriven);
    Port_UUT4 = new QextSerialPort(settings_Eqquipments, QextSerialPort::EventDriven);
    Port_UUT5 = new QextSerialPort(settings_Eqquipments, QextSerialPort::EventDriven);
    Port_UUT6 = new QextSerialPort(settings_Eqquipments, QextSerialPort::EventDriven);
    Port_UUT7 = new QextSerialPort(settings_Eqquipments, QextSerialPort::EventDriven);
    Port_UUT8 = new QextSerialPort(settings_Eqquipments, QextSerialPort::EventDriven);
    Port_ISO = new QextSerialPort(settings_Eqquipments, QextSerialPort::EventDriven);




    timer = new QTimer(this);  //Timer for control of 7252 to output the pressure of test point value.
    timer_GetReading = new QTimer(this);
    timer_SECOND =  new QTimer(this);//FOR TIME TO START TEST TIMER

    connect(timer, SIGNAL(timeout()), this, SLOT(slot_Send_Next_CMD()));// For control of 7252
    connect(timer_GetReading, SIGNAL(timeout()), this, SLOT(slot_Send_PollingReading_CMD()));// for Get readings from UUT
    connect(timer_SECOND, SIGNAL(timeout()), this, SLOT(slot_TIMEtoStart()));// for timer to startthe test

    connect(this, SIGNAL(sig_Get_Reading_CMD(QextSerialPort*)), this, SLOT(slot_Get_Reading_CMD(QextSerialPort*)));// for Get readings from UUT

    connect(ui->actionAbout_This_Program,SIGNAL(triggered()),this,SLOT(AboutThis()));
    connect(ui->actionHelp_File,SIGNAL(triggered()),this,SLOT(HelpFile()));
    connect(this,SIGNAL(sig_Parse_Reading(QString,PortType)),this,SLOT(Slot_Parse_Reading(QString,PortType)));

    connect(Port_F7252,SIGNAL(readyRead()),this,SLOT(readF7252_port()),Qt::QueuedConnection);
    connect(Port_UUT1,SIGNAL(readyRead()),this,SLOT(readUUT1_port()),Qt::QueuedConnection);
    connect(Port_UUT2,SIGNAL(readyRead()),this,SLOT(readUUT2_port()),Qt::QueuedConnection);
    connect(Port_UUT3,SIGNAL(readyRead()),this,SLOT(readUUT3_port()),Qt::QueuedConnection);
    connect(Port_UUT4,SIGNAL(readyRead()),this,SLOT(readUUT4_port()),Qt::QueuedConnection);
    connect(Port_UUT5,SIGNAL(readyRead()),this,SLOT(readUUT5_port()),Qt::QueuedConnection);
    connect(Port_UUT6,SIGNAL(readyRead()),this,SLOT(readUUT6_port()),Qt::QueuedConnection);
    connect(Port_UUT7,SIGNAL(readyRead()),this,SLOT(readUUT7_port()),Qt::QueuedConnection);
    connect(Port_UUT8,SIGNAL(readyRead()),this,SLOT(readUUT8_port()),Qt::QueuedConnection);

    connect(ui->textBrowser, SIGNAL(cursorPositionChanged()), this, SLOT(autoScroll()));

    timer_SECOND->start(SecondWatch);    
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::AboutThis()
{

    QString AboutString;

    AboutString = "This is a AutoTest program for PAC project\r\n\
            Release Version: V2.1\r\n\r\n\
            Designed by Bob Cao       Email:yongbo.cao@fluke.com\r\n\
            Fluke CPD Product Evaluation Team Copyright (c) 2016";
    QMessageBox::about(this,"About",AboutString);

}
void MainWindow::HelpFile()
{
    QProcess* process = new QProcess();
    QString path;
    QDir dir;
    path=dir.currentPath();
    QString notepadPath = "notepad.exe ";
    notepadPath.append(path);
    notepadPath.append("\\Help.txt");
    process->start(notepadPath);
}

void MainWindow::autoScroll()//textbrowser auto scroll
{
    QTextCursor cursor =  ui->textBrowser->textCursor();
    cursor.movePosition(QTextCursor::End);
    ui->textBrowser->setTextCursor(cursor);
}

void MainWindow::InitialCOM_Time()
{
    //读取COM，并装入UI
    QSettings *Test_COMINI = new QSettings("TestCOM.ini", QSettings::IniFormat);
    //将读取到的ini文件保存在QString中，先取值，然后通过toString()函数转换成QString类型
    //int Total_MA_NUM = TestPointINI->value("/mAMeasure/MANUM").toInt();

    Test_COMINI->beginGroup("COM");// [Section]
    QStringList COM_KeyList = Test_COMINI->allKeys();//  Key, keyvalue = TestPointINI->value(mA_PointList[i]).toString()

    //qDebug() <<Test_COMINI->value(COM_KeyList[1]).toString().toInt();
    //qDebug() <<Test_COMINI->value(COM_KeyList[2]).toString().toInt();
    //for(int i = 0; i < COM_KeyList.size(); i++)
    //{
    //    qDebug() <<COM_KeyList[i] << " = " << Test_COMINI->value(COM_KeyList[i]).toString().toInt();
    //}

    ui->comboBox_7252->setCurrentIndex(Test_COMINI->value(COM_KeyList[0]).toString().toInt()-1);//7252 COM
    ui->comboBox_FIX_ISO->setCurrentIndex(Test_COMINI->value(COM_KeyList[2]).toString().toInt()-1);//UUT1
    //UUT COM
    ui->comboBox_UUT1->setCurrentIndex(Test_COMINI->value(COM_KeyList[3]).toString().toInt()-1);//UUT1
    ui->comboBox_UUT2->setCurrentIndex(Test_COMINI->value(COM_KeyList[4]).toString().toInt()-1);//UUT2
    ui->comboBox_UUT3->setCurrentIndex(Test_COMINI->value(COM_KeyList[5]).toString().toInt()-1);//UUT3
    ui->comboBox_UUT4->setCurrentIndex(Test_COMINI->value(COM_KeyList[6]).toString().toInt()-1);//UUT4
    ui->comboBox_UUT5->setCurrentIndex(Test_COMINI->value(COM_KeyList[7]).toString().toInt()-1);//UUT5
    ui->comboBox_UUT6->setCurrentIndex(Test_COMINI->value(COM_KeyList[8]).toString().toInt()-1);//UUT6
    ui->comboBox_UUT7->setCurrentIndex(Test_COMINI->value(COM_KeyList[9]).toString().toInt()-1);//UUT7
    ui->comboBox_UUT8->setCurrentIndex(Test_COMINI->value(COM_KeyList[10]).toString().toInt()-1);//UUT8
    Test_COMINI->endGroup();

    Test_COMINI->beginGroup("TimeSetting_ms");// [Section]
    QStringList Timer_Setting_KeyList = Test_COMINI->allKeys();//  Key, keyvalue = TestPointINI->value(mA_PointList[i]).toString()
    //qDebug()<<Timer_Setting_KeyList<<endl;

    WAIT_Time_Source = Test_COMINI->value(Timer_Setting_KeyList[1]).toString().toInt();
    qDebug()<<"WAIT_Time_Source(ms):"<<WAIT_Time_Source<<endl;
    WAIT_Time_Read = Test_COMINI->value(Timer_Setting_KeyList[0]).toString().toInt();
    qDebug()<<"WAIT_Time_Read(ms):"<<WAIT_Time_Read<<endl;

    Test_COMINI->endGroup();


    //读入入完成后删除指针
    delete Test_COMINI;
}

void MainWindow::Write_COM_INI(QString KeyString, int COM_Index)
{
    //当COM 口改变时，自动写入INI文件中
    QSettings *Test_COMINI = new QSettings("TestCOM.ini", QSettings::IniFormat);
    Test_COMINI->beginGroup("COM");// [Section]
    Test_COMINI->setValue(KeyString,COM_Index + 1);

    Test_COMINI->endGroup();

    //读入入完成后删除指针
    delete Test_COMINI;
}

void MainWindow::on_pushButton_Connect_clicked()
{
    ui->pushButton_DisConnect->setDisabled(false);
    ui->pushButton_Connect->setDisabled(true);

    QString Port_COM;
    Port_COM = ui->comboBox_7252->currentText();
    Port_F7252->setPortName(Port_COM);
    Port_F7252->open(QIODevice::ReadWrite);

    TEMPSTR = Port_COM;
    if(Port_F7252->isOpen())
    {
        TEMPSTR.append("-Port_F7252 Connect successfully.\r\n");
    }
    else
    {
        TEMPSTR.append("-Port_F7252 Connect Failed.\r\n");
    }
    ui->textBrowser->setText(TEMPSTR + "\r\n");
    qDebug() << TEMPSTR;

    Port_COM = ui->comboBox_FIX_ISO->currentText();
    Port_ISO->setPortName(Port_COM);
    Port_ISO->open(QIODevice::ReadWrite);

    TEMPSTR.append(Port_COM);
    if(Port_ISO->isOpen())
    {
        TEMPSTR.append("-Port_FIX ISO Connect successfully.\r\n");
    }
    else
    {
        TEMPSTR.append("-Port_FIX ISO Connect Failed.\r\n");
    }
    ui->textBrowser->setText(TEMPSTR + "\r\n");
    qDebug() << TEMPSTR;
}

void MainWindow::Pressure_Test()//
{
    on_pushButton_Connect_clicked();// Connect to UUT
    on_pushButton_Iso_Setting_clicked();//SET FIx ISO
    on_pushButton_Start_clicked();// Connect to UUT and start to test

}

void MainWindow::slot_TIMEtoStart()// stopwatch and timer to start the test
{
    QDateTime time = QDateTime::currentDateTime();//获取系统现在的时间
    QString str = time.toString("yyyy-MM-dd hh:mm:ss"); //设置显示格式
    ui->label_CurrentTime->setText("Current Time: " + str);//在标签上显示时间


    //......Wait For the 1st TIMER COMING................//
    if(str == Time_Setting_str)
    {
        // Start to Test
        Pressure_Test();
    }

    //......Wait For the 2ND TIMER COMING................//
    if(str == Time_Setting_str2)// Test the second times
    {
        // Start to Test
        Pressure_Test();
    }

    //......Wait For the 3rd TIMER COMING................//
    if(str == Time_Setting_str3)// Test the second times
    {
        // Start to Test
        Pressure_Test();
    }
    //......Wait For the 4th TIMER COMING................//
    if(str == Time_Setting_str4)// Test the second times
    {
        // Start to Test
        Pressure_Test();
    }
    //......Wait For the 5th TIMER COMING................//
    if(str == Time_Setting_str5)// Test the second times
    {
        // Start to Test
        Pressure_Test();
    }
    //......Wait For the 6th TIMER COMING................//
    if(str == Time_Setting_str6)// Test the second times
    {
        // Start to Test
        Pressure_Test();
    }
}



void MainWindow::on_pushButton_DisConnect_clicked()
{
    ui->pushButton_DisConnect->setDisabled(true);
    ui->pushButton_Connect->setDisabled(false);

    if(Port_F7252->isOpen())
    {
        Port_F7252->close();
        TEMPSTR.append("-Port_F7252 DisConnectted.\r\n");
    }

    if(Port_ISO->isOpen())
    {
        Port_ISO->close();
        TEMPSTR.append("-Port_FIX ISO DisConnectted.\r\n");

    }
    ui->textBrowser->setText(TEMPSTR + "\r\n");
    qDebug() << TEMPSTR;
}

void MainWindow::SendCMD(QString CMD, QextSerialPort *Port)
{
    QDateTime currenttime = QDateTime::currentDateTime();//获取系统现在的时间
    QString str = currenttime.toString("yyyy-MM-dd hh:mm:ss"); //设置显示格式

    QString SendCmd = CMD;
    CurrentCMD = CMD;

    str.append(" ");
    str.append(Port->portName());
    str.append(" TX:  ");
    str.append(SendCmd);

    QextSerialPort *Target_Port = Port;

    Target_Port->readAll();
    Target_Port->setRts();
    Target_Port->write(SendCmd.toLatin1());
    Target_Port->flush();

    TEMPSTR.append(str);
    ui->textBrowser->setPlainText(TEMPSTR + "\r\n");
    qDebug() << TEMPSTR ;

}

void MainWindow::sleep(unsigned int msec)
{
    QTime dieTime;
    dieTime = QTime::currentTime();
    dieTime.addMSecs(msec);
    while( QTime::currentTime() < dieTime )
    {
       QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    }
}


QStringList MainWindow::GetReadingsFromUI()  //Get Readings
{
    QStringList ReadingList;
    if(ui->checkBox_Reading1->isChecked())
    {
        ReadingList.append(ui->label_Reading1->text());
    }
    if(ui->checkBox_Reading2->isChecked())
    {
        ReadingList.append(ui->label_Reading2->text());
    }
    if(ui->checkBox_Reading3->isChecked())
    {
        ReadingList.append(ui->label_Reading3->text());
    }
    if(ui->checkBox_Reading4->isChecked())
    {
        ReadingList.append(ui->label_Reading4->text());
    }
    if(ui->checkBox_Reading5->isChecked())
    {
        ReadingList.append(ui->label_Reading5->text());
    }
    if(ui->checkBox_Reading6->isChecked())
    {
        ReadingList.append(ui->label_Reading6->text());
    }
    if(ui->checkBox_Reading7->isChecked())
    {
        ReadingList.append(ui->label_Reading7->text());
    }
    if(ui->checkBox_Reading8->isChecked())
    {
        ReadingList.append(ui->label_Reading8->text());
    }
    ReadingList.append(ui->label_7252Reading->text());

    return ReadingList;
}

void MainWindow::WriteTitle2Excel()
{
    QStringList COM_List;
    COM_List.append(ui->comboBox_UUT1->currentText());
    COM_List.append(ui->comboBox_UUT2->currentText());
    COM_List.append(ui->comboBox_UUT3->currentText());
    COM_List.append(ui->comboBox_UUT4->currentText());
    COM_List.append(ui->comboBox_UUT5->currentText());
    COM_List.append(ui->comboBox_UUT6->currentText());
    COM_List.append(ui->comboBox_UUT7->currentText());
    COM_List.append(ui->comboBox_UUT8->currentText());

    qDebug() << COM_List;
    QXlsx::Document xlsxFile("PAC.xlsx");

    int DataNUM = COM_List.size();

    for(int i = 0;i < DataNUM; i++)
    {
        xlsxFile.write(CurrentRow -1,StartCol + i,COM_List[i]);
    }
    xlsxFile.save();
    COM_List.clear();

}

void MainWindow::WriteReadings2Excel(QStringList Readings)  //Write Readings
{
    QStringList DataList = Readings;
    QXlsx::Document xlsxFile("PAC.xlsx");

    int DataNUM = DataList.size();

    for(int i = 0;i < DataNUM; i++)
    {
        xlsxFile.write(CurrentRow,StartCol + i,DataList[i].toFloat());
    }
    xlsxFile.save();
    CurrentRow++;

    qDebug() << DataList;
    qDebug() <<"File saved!";
}

void MainWindow::slot_Send_Next_CMD()  //timerout：3min
{
    timer->stop();
    timer_GetReading->stop();
    //1. Read the UI readings to a ReadingList, QString

    QStringList ReadingList = GetReadingsFromUI();

    //2. Write the readings to excel
    WriteReadings2Excel(ReadingList);

    // send the next pressure point to 7252
    CurrentTestPoint++;
    //if(CurrentTestPoint < PRES_PointList.size() + 1)
    if(CurrentTestPoint < PRES_PointList.size()) // to resolve the dash when finished
    {
        //组织命令
        QString cmd;
        if(Channel_Range == "B")
        {
            cmd = "PRES11 " + PRES_PointList[CurrentTestPoint] + "\n";
        }
        else
        {
            cmd = "PRES " + PRES_PointList[CurrentTestPoint] + "\n";
        }
        SendCMD(cmd,Port_F7252);

        QString ProcessString;
        ui->label_TestPoint->setText(PRES_PointList[CurrentTestPoint]);
        ProcessString.append(QString::number(CurrentTestPoint + 1));
        ProcessString.append("/");
        ProcessString.append(QString::number(PRES_PointList.size()));

        ui->label_StepofProcess->setText(ProcessString);

        //Start timer for readings and source
        timer->start(WAIT_Time_Source);
        timer_GetReading->start(WAIT_Time_Read);
    }
    else
    {
        // Stop pressure test
        //Venting pressure
        if(Channel_Range == "B")
        {
            SendCMD(CMD_7252i_VentModeB,Port_F7252);
        }
        else
        {
            SendCMD(CMD_7252i_VentModeA,Port_F7252);
        }
        sleep(5000);

        // Close all the iso valve
        SendCMD(CMD_ISO_All_off,Port_ISO);

        // Drop Equipments COM: 7252 and iso
        on_pushButton_DisConnect_clicked();

        // Drop UUTs
        Drop_UUT_COM();

        ui->pushButton_Start->setEnabled(true);
        TEMPSTR.append("Information: Test Finished,测试已完成，7252 VENT\r\n");
        ui->textBrowser->setText(TEMPSTR + "\r\n");
        qDebug() << TEMPSTR;
    }
}

void MainWindow::Drop_UUT_COM()
{
    if(Port_UUT1->isOpen())
    {
        Port_UUT1->close();
        TEMPSTR.append("-Port_UUT1 DisConnectted.\r\n");
        ui->textBrowser->setText(TEMPSTR + "\r\n");
        qDebug() << TEMPSTR;
    }

    if(Port_UUT2->isOpen())
    {
        Port_UUT2->close();
        TEMPSTR.append("-Port_UUT2 DisConnectted.\r\n");
        ui->textBrowser->setText(TEMPSTR + "\r\n");
        qDebug() << TEMPSTR;
    }

    if(Port_UUT3->isOpen())
    {
        Port_UUT3->close();
        TEMPSTR.append("-Port_UUT3 DisConnectted.\r\n");
        ui->textBrowser->setText(TEMPSTR + "\r\n");
        qDebug() << TEMPSTR;
    }

    if(Port_UUT4->isOpen())
    {
        Port_UUT4->close();
        TEMPSTR.append("-Port_UUT4 DisConnectted.\r\n");
        ui->textBrowser->setText(TEMPSTR + "\r\n");
        qDebug() << TEMPSTR;
    }

    if(Port_UUT5->isOpen())
    {
        Port_UUT5->close();
        TEMPSTR.append("-Port_UUT5 DisConnectted.\r\n");
        ui->textBrowser->setText(TEMPSTR + "\r\n");
        qDebug() << TEMPSTR;
    }

    if(Port_UUT6->isOpen())
    {
        Port_UUT6->close();
        TEMPSTR.append("-Port_UUT6 DisConnectted.\r\n");
        ui->textBrowser->setText(TEMPSTR + "\r\n");
        qDebug() << TEMPSTR;
    }

    if(Port_UUT7->isOpen())
    {
        Port_UUT7->close();
        TEMPSTR.append("-Port_UUT7 DisConnectted.\r\n");
        ui->textBrowser->setText(TEMPSTR + "\r\n");
        qDebug() << TEMPSTR;
    }

    if(Port_UUT8->isOpen())
    {
        Port_UUT8->close();
        TEMPSTR.append("-Port_UUT8 DisConnectted.\r\n");
        ui->textBrowser->setText(TEMPSTR + "\r\n");
        qDebug() << TEMPSTR;
    }
}

void MainWindow::slot_Get_Reading_CMD(QextSerialPort *Port_UUT) //
{   
        SendCMD(CMD_UUT_Read_Pres_Measurement,Port_UUT);
}

void MainWindow::slot_Send_PollingReading_CMD() // send cmd to UUT
{
    if(ui->checkBox_Reading1->isChecked())
    {
        SendCMD(CMD_UUT_Read_Pres_Measurement,Port_UUT1);
    }
    if(ui->checkBox_Reading2->isChecked())
    {
        SendCMD(CMD_UUT_Read_Pres_Measurement,Port_UUT2);
    }
    if(ui->checkBox_Reading3->isChecked())
    {
        SendCMD(CMD_UUT_Read_Pres_Measurement,Port_UUT3);
    }
    if(ui->checkBox_Reading4->isChecked())
    {
       SendCMD(CMD_UUT_Read_Pres_Measurement,Port_UUT4);
    }
    if(ui->checkBox_Reading5->isChecked())
    {
        SendCMD(CMD_UUT_Read_Pres_Measurement,Port_UUT5);
    }
    if(ui->checkBox_Reading6->isChecked())
    {
        SendCMD(CMD_UUT_Read_Pres_Measurement,Port_UUT6);
    }
    if(ui->checkBox_Reading7->isChecked())
    {
        SendCMD(CMD_UUT_Read_Pres_Measurement,Port_UUT7);
    }
    if(ui->checkBox_Reading8->isChecked())
    {
        SendCMD(CMD_UUT_Read_Pres_Measurement,Port_UUT8);
    }

    if(ui->comboBox_7252_Channel->currentText() == "B")
    {
       SendCMD(CMD_7252i_GetValue_ChannelB,Port_F7252);
    }
    else if(ui->comboBox_7252_Channel->currentText() == "A")
    {
       SendCMD(CMD_7252i_GetValue_ChannelA,Port_F7252);
    }
    else //default to large range channel
    {
       SendCMD(CMD_7252i_GetValue_ChannelA,Port_F7252);
    }

}

void MainWindow::UpdateUI_Reading(QString Reading,PortType UUT)//
{
    float fReading = Reading.toFloat();
    //float fCurrentTestPoint = PRES_PointList[CurrentTestPoint].toFloat();
    //if(fReading >= fCurrentTestPoint*(1.0 + FullSpecError *0.5))
    QString strReading = QString::number(fReading, 'f', 3);
   // QString strStyleSheet;
   // QString strReading = tr("%1").arg(fReading);

    switch(UUT)
    {
        case Port_UUT_1:
        {
            ui->label_Reading1->clear();
            //ui->label_Reading1->style();
            ui->label_Reading1->setText(strReading);
            break;
        }
        case Port_UUT_2:
        {
            ui->label_Reading2->clear();
            ui->label_Reading2->setText(strReading);
            break;
        }
        case Port_UUT_3:
        {
            ui->label_Reading3->clear();
            ui->label_Reading3->setText(strReading);
            break;
        }
        case Port_UUT_4:
        {
            ui->label_Reading4->clear();
            ui->label_Reading4->setText(strReading);
            break;
        }
        case Port_UUT_5:
        {
            ui->label_Reading5->clear();
            ui->label_Reading5->setText(strReading);
            break;
        }
        case Port_UUT_6:
        {
            ui->label_Reading6->clear();
            ui->label_Reading6->setText(strReading);
            break;
        }
        case Port_UUT_7:
        {
            ui->label_Reading7->clear();
            ui->label_Reading7->setText(strReading);
            break;
        }
        case Port_UUT_8:
        {
            ui->label_Reading8->clear();
            ui->label_Reading8->setText(strReading);
            break;
        }
        case Port_Equipment_7252I:
        {
            ui->label_7252Reading->clear();
            ui->label_7252Reading->setText(strReading);
            qDebug() <<"7252 Reading: " <<strReading;
            break;
        }
        default: break;
    }
}

void MainWindow::Slot_Parse_Reading(QString ReceiveData,PortType UUT)//2,REGEX
{
    QString Reading;
    QRegExp rx(QString("([- +]*\\d+\\.\\d+E[-+]\\d+)"));

    QDateTime currenttime = QDateTime::currentDateTime();//获取系统现在的时间
    QString str = currenttime.toString("yyyy-MM-dd hh:mm:ss"); //设置显示格式
    str.append(" RX:  ");
    str.append(ReceiveData);


    //RX GET READINGS

    int pos = ReceiveData.indexOf(rx);          // 0, position of the first match.
                                                // Returns -1 if str is not found.
                                                // You can also use rx.indexIn(str);
    if ( pos >= 0 )
    {
        QStringList List = rx.capturedTexts();
        Reading = List[0];
        qDebug() << Reading;
        UpdateUI_Reading(Reading,UUT);
    }
    //UpdateUI_Reading(Reading,UUT);

}

void MainWindow::ReadUUT(QextSerialPort *UUTPort,PortType UUT )//1. parse, 2,REGEX
{
    QString ReceiveByte;
    QString ReceiveData;   

    if(UUTPort->bytesAvailable())
    {
        ReceiveByte = UUTPort->readAll();
        ReceiveData += ReceiveByte;
        if(CurrentCMD == CMD_UUT_Read_Pres_Measurement)
        {
            if(ReceiveData.contains("#>"))
            {
                emit sig_Parse_Reading(ReceiveData,UUT);
                ReceiveData.clear();
            }
         }

        if((CurrentCMD == CMD_7252i_GetValue_ChannelA)||(CurrentCMD == CMD_7252i_GetValue_ChannelB))
        {
            //QRegExp rx(QString("([- +]*\\d+\\.\\d+E[-+]\\d+\\r)"));//'^([-+ ]?\d+\.\d+E[-+]?\d+)\r\n'
            //int pos = ReceiveData.indexOf(rx);
            //if ( pos >= 0 )
            if ( ReceiveData.contains("\r\n") )
            {
                emit sig_Parse_Reading(ReceiveData,UUT);
                qDebug() << ReceiveData;
                ReceiveData.clear();
            }

        }
    }
}



void MainWindow::readUUT1_port()// slot for readrRead
{
    //串口响应函数，只接受数据，然后发送信号，去处理，显示数据；修改
    ReadUUT(Port_UUT1,Port_UUT_1);
}

void MainWindow::readUUT2_port()// slot for readrRead
{
    ReadUUT(Port_UUT2,Port_UUT_2);
}

void MainWindow::readUUT3_port()// slot for readrRead
{
    ReadUUT(Port_UUT3,Port_UUT_3);
}

void MainWindow::readUUT4_port()// slot for readrRead
{
    ReadUUT(Port_UUT4,Port_UUT_4);
}

void MainWindow::readUUT5_port()// slot for readrRead
{
    ReadUUT(Port_UUT5,Port_UUT_5);
}

void MainWindow::readUUT6_port()// slot for readrRead
{
    ReadUUT(Port_UUT6,Port_UUT_6);
}

void MainWindow::readUUT7_port()// slot for readrRead
{
    ReadUUT(Port_UUT7,Port_UUT_7);
}

void MainWindow::readUUT8_port()// slot for readrRead
{
    ReadUUT(Port_UUT8,Port_UUT_8);

}

void MainWindow::readF7252_port() //slot for read 7252
{
    ReadUUT(Port_F7252,Port_Equipment_7252I);
}

bool MainWindow::Check_UUTCOM(QComboBox *CurrentComboBox,QextSerialPort *UUT_PORT) //Connect and  Prepare for Pressure Measurement MODE of UUT, get ready for reading
{
    QString Port_COM = CurrentComboBox->currentText();
    UUT_PORT->setPortName(Port_COM);
    UUT_PORT->open(QIODevice::ReadWrite);

    TEMPSTR = Port_COM;
    if(UUT_PORT->isOpen())
    {
        TEMPSTR.append("- Connect successfully.\r\n");
        qDebug() << TEMPSTR;

        return true;
    }
    else
    {
        TEMPSTR.append("- Connect Failed.\r\n");
         qDebug() << TEMPSTR;
         return false;
    }

}

void MainWindow::Prepare_UUT(QextSerialPort *UUT_PORT)//
{
    // Prepare for Pressure Measurement MODE of UUT, get ready for reading
    SendCMD(CMD_UUT_EnterShell,UUT_PORT);
    sleep(2000);

    SendCMD(CMD_UUT_EnterMeasuereMode,UUT_PORT);
    sleep(2000);
    timer_GetReading->start(WAIT_Time_Read);

}


void MainWindow::Prepare_UUT_ForTest()
{
  if(ui->checkBox_Reading1->isChecked())
  {
      Prepare_UUT(Port_UUT1);
  }
  if(ui->checkBox_Reading2->isChecked())
  {
      Prepare_UUT(Port_UUT2);
  }
  if(ui->checkBox_Reading3->isChecked())
  {
      Prepare_UUT(Port_UUT3);
  }
  if(ui->checkBox_Reading4->isChecked())
  {
      Prepare_UUT(Port_UUT4);
  }
  if(ui->checkBox_Reading5->isChecked())
  {
      Prepare_UUT(Port_UUT5);
  }
  if(ui->checkBox_Reading6->isChecked())
  {
      Prepare_UUT(Port_UUT6);
  }
  if(ui->checkBox_Reading7->isChecked())
  {
      Prepare_UUT(Port_UUT7);
  }
  if(ui->checkBox_Reading8->isChecked())
  {
      Prepare_UUT(Port_UUT8);
  }
}



bool MainWindow::ConnectTOUUT()
{
    bool Connect_UUT1_Result= true;
    bool Connect_UUT2_Result= true;
    bool Connect_UUT3_Result= true;
    bool Connect_UUT4_Result= true;
    bool Connect_UUT5_Result= true;
    bool Connect_UUT6_Result= true;
    bool Connect_UUT7_Result= true;
    bool Connect_UUT8_Result= true;

    if(ui->checkBox_Reading1->isChecked())
    {
        Connect_UUT1_Result = Check_UUTCOM(ui->comboBox_UUT1,Port_UUT1);
    }
    if(ui->checkBox_Reading2->isChecked())
    {
       Connect_UUT2_Result = Check_UUTCOM(ui->comboBox_UUT2,Port_UUT2);
    }
    if(ui->checkBox_Reading3->isChecked())
    {
       Connect_UUT3_Result = Check_UUTCOM(ui->comboBox_UUT3,Port_UUT3);
    }
    if(ui->checkBox_Reading4->isChecked())
    {
       Connect_UUT4_Result = Check_UUTCOM(ui->comboBox_UUT4,Port_UUT4);
    }
    if(ui->checkBox_Reading5->isChecked())
    {
       Connect_UUT5_Result = Check_UUTCOM(ui->comboBox_UUT5,Port_UUT5);
    }
    if(ui->checkBox_Reading6->isChecked())
    {
        Connect_UUT6_Result = Check_UUTCOM(ui->comboBox_UUT6,Port_UUT6);
    }
    if(ui->checkBox_Reading7->isChecked())
    {
        Connect_UUT7_Result = Check_UUTCOM(ui->comboBox_UUT7,Port_UUT7);
    }
    if(ui->checkBox_Reading8->isChecked())
    {
        Connect_UUT8_Result = Check_UUTCOM(ui->comboBox_UUT8,Port_UUT8);
    }
    if(Connect_UUT1_Result && Connect_UUT2_Result && Connect_UUT3_Result && Connect_UUT4_Result && Connect_UUT5_Result && Connect_UUT6_Result && Connect_UUT7_Result && Connect_UUT8_Result)
    {
        return true;
    }
    else
    {
        return false;
    }
}

void MainWindow::ReadTestPoint()
{
    //读取测试点，并装入测试点的LIST  public变量里面，供后续调用；
    QString FileName;
    int SensorType;
    int i;
    SensorType = ui->comboBox_SensorType->currentIndex();
    switch(SensorType)
    {
        case 0:FileName = "TestPoint_30PSI.ini";break;
        case 2:FileName = "TestPoint_300PSI.ini";break;
        case 1:FileName = "TestPoint_150PSI.ini";break;
    }
    // FileName = "TestPoint_30PSI.ini"; //Fix test Point
    QSettings *TestPointINI = new QSettings(FileName, QSettings::IniFormat);
    //将读取到的ini文件保存在QString中，先取值，然后通过toString()函数转换成QString类型
    //int Total_MA_NUM = TestPointINI->value("/mAMeasure/MANUM").toInt();

    TestPointINI->beginGroup("mAMeasure");// [Section]
    QStringList mA_PointKeyList = TestPointINI->allKeys();//  Key, keyvalue = TestPointINI->value(mA_PointList[i]).toString()

    for(i=0; i < mA_PointKeyList.size(); i++)
    {
       //qDebug()<<mA_PointKeyList[i]<<endl;
       mA_PointList.append(TestPointINI->value(mA_PointKeyList[i]).toString());
    }
    TestPointINI->endGroup();

    TestPointINI->beginGroup("PressureMeasure");// [Section]
    QStringList PRES_PointKeyList = TestPointINI->allKeys();//  Key, keyvalue = TestPointINI->value(mA_PointList[i]).toString()
    qDebug()<< PRES_PointKeyList;

    for(i=0; i < PRES_PointKeyList.size(); i++)
    {
       qDebug()<<PRES_PointKeyList[i]<<" = "<<TestPointINI->value(PRES_PointKeyList[i]).toString()<<endl;
       PRES_PointList.append(TestPointINI->value(PRES_PointKeyList[i]).toString());
    }
    //qDebug()<< PRES_PointList;
    TestPointINI->endGroup();

    //读入入完成后删除指针
    delete TestPointINI;
}

void MainWindow::Clear_READINGS()
{
    ui->label_Reading1->clear();
    ui->label_Reading2->clear();
    ui->label_Reading3->clear();
    ui->label_Reading4->clear();
    ui->label_Reading5->clear();
    ui->label_Reading6->clear();
    ui->label_Reading7->clear();
    ui->label_Reading8->clear();

}

void MainWindow::on_pushButton_Start_clicked()
{
    // Initial the variables for timer test.
    CurrentTestPoint = 0;
    PRES_PointList.clear();

    //WriteTitle2Excel();//title, not needed , it is deleted when for more timer test.

    // Reading test point from file
    ReadTestPoint();

    Clear_READINGS();// clear UI
    Sensor_Range = ui->comboBox_SensorType->currentText();//Get the selected sensor type
    qDebug() << "Test Sensor Type:" << Sensor_Range;

    Channel_Range = ui->comboBox_7252_Channel->currentText();//Get the selected sensor type
    qDebug() << "7252 Channel :" << Channel_Range;

    // Prepare for UUT serial COM;
    if(!ConnectTOUUT())
    {
       QMessageBox::warning(this,"Warning","请勾选正确的UUT， 选择正确的COM口，连接好USB线！",QMessageBox::Yes);
       return;
    }

    // Prepare for Pressure Measurement MODE of UUT, get ready for reading
    Prepare_UUT_ForTest();

    ui->pushButton_Start->setEnabled(false);

    // Prepare for pressure output MODE of 7252
    QString CMD_7252i_VentStr;
    QString CMD_7252i_CONTROLModeStr;
    QString CMD_7252i_UNIT_Setting_str;
    QString cmd;

    if(Channel_Range == "B")
    {
        CMD_7252i_VentStr = CMD_7252i_VentModeB;
        CMD_7252i_CONTROLModeStr = CMD_7252i_CONTROLModeB;
        CMD_7252i_UNIT_Setting_str = CMD_7252i_UnitSetB;
        cmd = "PRES11 " + PRES_PointList[CurrentTestPoint] + "\n";
    }
    else
    {
        CMD_7252i_VentStr = CMD_7252i_VentModeA;
        CMD_7252i_CONTROLModeStr = CMD_7252i_CONTROLModeA;
        CMD_7252i_UNIT_Setting_str = CMD_7252i_UnitSetA;
        cmd = "PRES " + PRES_PointList[CurrentTestPoint] + "\n";
    }

    QString ProcessString;
    ProcessString.append(PRES_PointList[CurrentTestPoint]);
    ui->label_TestPoint->setText(ProcessString);

    ProcessString.clear();
    ProcessString.append(QString::number(CurrentTestPoint + 1));
    ProcessString.append("/");
    ProcessString.append(QString::number(PRES_PointList.size()));

    ui->label_StepofProcess->setText(ProcessString);

    qDebug()  << CMD_7252i_CONTROLModeStr;


    SendCMD(CMD_7252i_VentStr,Port_F7252);
    sleep(5000);
    //SendCMD(CMD_7252i_UNIT_Setting_str,Port_F7252);
    //sleep(5000);
    SendCMD(CMD_7252i_CONTROLModeStr,Port_F7252);
    sleep(1000);
    //SendCMD(CMD_7252i_OUTPRES_1,Port_F7252);
    //sleep(1000);

    // 发送第一个压力点的命令,放到slot_Send_Next_CMD中去发

    SendCMD(cmd,Port_F7252);

    timer->start(WAIT_Time_Source);// 3MIN
    timer_GetReading->start(WAIT_Time_Read);// START TO POLLING READINGS

}

void MainWindow::on_pushButton_Stop_clicked()
{
    timer->stop();
    timer_GetReading->stop();
    if(QMessageBox::Yes == QMessageBox::warning(this,"Warning","将要停止测试，7252 将置为Vent mode，！",QMessageBox::Yes,QMessageBox::No))
    {
        // Stop pressure test
        // venting
        if(Channel_Range == "B")
        {
            SendCMD(CMD_7252i_VentModeB,Port_F7252);
        }
        else
        {
            SendCMD(CMD_7252i_VentModeA,Port_F7252);
        }
        sleep(5000);
        ui->pushButton_Start->setEnabled(true);
    }
    else
    {
        timer->start(WAIT_Time_Source);// 3MIN
        timer_GetReading->start(WAIT_Time_Read);// START TO POLLING READINGS
    }
}

void MainWindow::on_pushButton_Pause_clicked()
{
    timer->stop();
    //timer_GetReading->stop();


    /* TEST CODE  *******************************************************************
    QString ReceiveData = tr(" -2.8888E-01 \r\n#>");

    Slot_Parse_Reading(ReceiveData,Port_Equipment_7252I);
    return;
    //float fReading = 266.35694159599;
    //qDebug() << "floatReading:" << fReading;
    //QString strReading = QString::number(fReading, 'f', 5);
   // QString strReading = tr("%1").arg(fReading);
    //qDebug() << "strReading:" << strReading;

    //"[+-]?[\\d]+([\\.][\\d]*)?([Ee][+-]?[0-9]{0,2})?";
    QRegExp rx(QString("([- +]*\\d+\\.\\d+E[-+]\\d+) \\r"));
    //QString ReceiveData = tr(" -2.8888E-01 \r\n#>");
    int pos = ReceiveData.indexOf(rx);          // 0, position of the first match.
                                                // Returns -1 if str is not found.
                                                // You can also use rx.indexIn(str);
    qDebug() << pos;
    if ( pos >= 0 )
    {
       qDebug() << rx.matchedLength();     // 5, length of the last matched string
                                           // or -1 if there was no match
       qDebug() << rx.capturedTexts();     // QStringList(“a=100″, ”a”, ”100″),
                                                //   0: text matching pattern
                                                //   1: text captured by the 1st ()
                                                //   2: text captured by the 2nd ()
       QStringList List = rx.capturedTexts();
       for(int i = 0; i< List.size();i++)
       {
           qDebug()<<List[i];
       }
    }
   TEST CODE END *************************************************/
}

void MainWindow::on_pushButton_Voltage_clicked()
{
    //For Test
    // blue: rgb(xxx,xxx,xxx)
    //yellow  setStyleSheet("color: blue; background-color: yellow");
    //red
    qDebug()  << ui->label_Reading1->style();
    qDebug()  << ui->label_Reading2->style();
    qDebug()  << ui->label_Reading3->style();
    ui->label_Reading5->setStyleSheet("color: blue; background-color: gray");
    ui->label_Reading6->setStyleSheet("color: rgb(255,235,17); background-color: gray");
    ui->label_Reading7->setStyleSheet("color: red; background-color: gray");
    return;


    if(!ConnectTOUUT())
    {
       QMessageBox::warning(this,"Warning","请勾选正确的UUT， 选择正确的COM口，连接好USB线！",QMessageBox::Yes);
       return;
    }

    // Prepare for Pressure Measurement MODE of UUT, get ready for reading
    Prepare_UUT_ForTest(); //
    timer_GetReading->start(WAIT_Time_Read);


    // for test end
}

void MainWindow::on_checkBox_Reading1_clicked()
{
    if(ui->checkBox_Reading1->isChecked())
    {
        Total_UUT_Number++;
    }
    else
    {
        Total_UUT_Number--;
    }
}

void MainWindow::on_checkBox_Reading2_clicked()
{
    if(ui->checkBox_Reading2->isChecked())
    {
        Total_UUT_Number++;
    }
    else
    {
        Total_UUT_Number--;
    }
}

void MainWindow::on_checkBox_Reading3_clicked()
{
    if(ui->checkBox_Reading3->isChecked())
    {
        Total_UUT_Number++;
    }
    else
    {
        Total_UUT_Number--;
    }
}

void MainWindow::on_checkBox_Reading4_clicked()
{
    if(ui->checkBox_Reading4->isChecked())
    {
        Total_UUT_Number++;
    }
    else
    {
        Total_UUT_Number--;
    }
}

void MainWindow::on_checkBox_Reading5_clicked()
{
    if(ui->checkBox_Reading5->isChecked())
    {
        Total_UUT_Number++;
    }
    else
    {
        Total_UUT_Number--;
    }
}

void MainWindow::on_checkBox_Reading6_clicked()
{
    if(ui->checkBox_Reading6->isChecked())
    {
        Total_UUT_Number++;
    }
    else
    {
        Total_UUT_Number--;
    }
}

void MainWindow::on_checkBox_Reading7_clicked()
{
    if(ui->checkBox_Reading7->isChecked())
    {
        Total_UUT_Number++;
    }
    else
    {
        Total_UUT_Number--;
    }
}

void MainWindow::on_checkBox_Reading8_clicked()
{
    if(ui->checkBox_Reading8->isChecked())
    {
        Total_UUT_Number++;
    }
    else
    {
        Total_UUT_Number--;
    }
}

void MainWindow::on_pushButton_TimerSet_clicked()
{
    QDateTime Time_Setting =ui->dateTimeEdit->dateTime();
    Time_Setting_str = Time_Setting.toString("yyyy-MM-dd hh:mm:ss"); //设置显示格式
    QMessageBox::information(this,"Time Setting:","定时测试时间1 已设定：" + Time_Setting_str,QMessageBox::Ok);
    qDebug() << Time_Setting_str;
}


void MainWindow::on_pushButton_TimerSet_2_clicked()
{
    QDateTime Time_Setting =ui->dateTimeEdit_2->dateTime();
    Time_Setting_str2 = Time_Setting.toString("yyyy-MM-dd hh:mm:ss"); //设置显示格式
    QMessageBox::information(this,"Time Setting:","定时测试时间2 已设定：" + Time_Setting_str2,QMessageBox::Ok);
    qDebug() << Time_Setting_str;

}

void MainWindow::on_pushButton_TimerSet_3_clicked()
{
    QDateTime Time_Setting =ui->dateTimeEdit_3->dateTime();
    Time_Setting_str3 = Time_Setting.toString("yyyy-MM-dd hh:mm:ss"); //设置显示格式
    QMessageBox::information(this,"Time Setting:","定时测试时间3 已设定：" + Time_Setting_str3,QMessageBox::Ok);
    qDebug() << Time_Setting_str;

}



void MainWindow::on_pushButton_TimerSet_4_clicked()
{
    QDateTime Time_Setting =ui->dateTimeEdit_4->dateTime();
    Time_Setting_str4 = Time_Setting.toString("yyyy-MM-dd hh:mm:ss"); //设置显示格式
    QMessageBox::information(this,"Time Setting:","定时测试时间2 已设定：" + Time_Setting_str4,QMessageBox::Ok);
    qDebug() << Time_Setting_str;

}

void MainWindow::on_pushButton_TimerSet_5_clicked()
{
    QDateTime Time_Setting =ui->dateTimeEdit_5->dateTime();
    Time_Setting_str5 = Time_Setting.toString("yyyy-MM-dd hh:mm:ss"); //设置显示格式
    QMessageBox::information(this,"Time Setting:","定时测试时间2 已设定：" + Time_Setting_str5,QMessageBox::Ok);
    qDebug() << Time_Setting_str;

}

void MainWindow::on_pushButton_TimerSet_6_clicked()
{
    QDateTime Time_Setting =ui->dateTimeEdit_6->dateTime();
    Time_Setting_str6 = Time_Setting.toString("yyyy-MM-dd hh:mm:ss"); //设置显示格式
    QMessageBox::information(this,"Time Setting:","定时测试时间2 已设定：" + Time_Setting_str6,QMessageBox::Ok);
    qDebug() << Time_Setting_str;

}


void MainWindow::on_comboBox_7252_currentIndexChanged(int index)
{
    QString KeyString = "7252i";
    Write_COM_INI(KeyString,index);
}

void MainWindow::on_comboBox_UUT1_currentIndexChanged(int index)
{
    QString KeyString = "UUT1";
    Write_COM_INI(KeyString,index);
}


void MainWindow::on_comboBox_UUT2_currentIndexChanged(int index)
{
    QString KeyString = "UUT2";
    Write_COM_INI(KeyString,index);
}

void MainWindow::on_comboBox_UUT3_currentIndexChanged(int index)
{
    QString KeyString = "UUT3";
    Write_COM_INI(KeyString,index);
}

void MainWindow::on_comboBox_UUT4_currentIndexChanged(int index)
{
    QString KeyString = "UUT4";
    Write_COM_INI(KeyString,index);
}

void MainWindow::on_comboBox_UUT5_currentIndexChanged(int index)
{
    QString KeyString = "UUT5";
    Write_COM_INI(KeyString,index);
}

void MainWindow::on_comboBox_UUT6_currentIndexChanged(int index)
{
    QString KeyString = "UUT6";
    Write_COM_INI(KeyString,index);
}

void MainWindow::on_comboBox_UUT7_currentIndexChanged(int index)
{
    QString KeyString = "UUT7";
    Write_COM_INI(KeyString,index);
}

void MainWindow::on_comboBox_UUT8_currentIndexChanged(int index)
{
    QString KeyString = "UUT8";
    Write_COM_INI(KeyString,index);
}
void MainWindow::on_comboBox_FIX_ISO_currentIndexChanged(int index)
{
    QString KeyString = "Fix_ISO";
    Write_COM_INI(KeyString,index);

}

void MainWindow::on_pushButton_Iso_Setting_clicked()
{
    QString ISOsetting = GetISOSetting();
    QString cmdISO = "iso -c set -v ";
    cmdISO = cmdISO + ISOsetting +"\r\n";
    SendCMD(cmdISO, Port_ISO);
}

QString MainWindow::GetISOSetting()
{
    QString ISOsetting;
    // CMD = "ISO -c set -v 0x100
//   V:| 8    7    6    5    |    4    3    2    1
// bit:| 7    6    5    4    |    3    2    1    0
//0x1xy|         x           |           y
//
    int i = 0x00;
    if(ui->checkBox_MasterSwitch->isChecked())
    {
        i = 0x100;
    }
    if(ui->checkBox_SubSwitch_1->isChecked())
    {
        i = i + 0x001;
    }
    if(ui->checkBox_SubSwitch_2->isChecked())
    {
        i = i + 0x002;
    }
    if(ui->checkBox_SubSwitch_3->isChecked())
    {
        i = i + 0x004;
    }
    if(ui->checkBox_SubSwitch_4->isChecked())
    {
        i = i + 0x008;
    }
    if(ui->checkBox_SubSwitch_5->isChecked())
    {
        i = i + 0x010;
    }
    if(ui->checkBox_SubSwitch_6->isChecked())
    {
        i = i + 0x020;
    }
    if(ui->checkBox_SubSwitch_7->isChecked())
    {
        i = i + 0x040;
    }
    if(ui->checkBox_SubSwitch_8->isChecked())
    {
        i = i + 0x080;
    }
    qDebug() << i;
    ISOsetting = "0x";
    ISOsetting.append(QString::number(i,16));
    qDebug() << ISOsetting;

    return ISOsetting;

}

