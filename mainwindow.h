#ifndef MAINWINDOW_H
#define MAINWINDOW_H


#include <QMainWindow>
#include <QComboBox>
#include <QLabel>
#include "typedefs.h"
#include ".\qextserialport\qextserialport.h"


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public:
    QextSerialPort *Port_F5520;
    QextSerialPort *Port_F6270;
    QextSerialPort *Port_F7252;
    QextSerialPort *Port_UUT1;
    QextSerialPort *Port_UUT2;
    QextSerialPort *Port_UUT3;
    QextSerialPort *Port_UUT4;
    QextSerialPort *Port_UUT5;
    QextSerialPort *Port_UUT6;
    QextSerialPort *Port_UUT7;
    QextSerialPort *Port_UUT8;
    QextSerialPort *Port_ISO;

    QStringList mA_PointList;
    QStringList PRES_PointList;
    QString TEMPSTR;
    int Total_UUT_Number;

    QString Sensor_Range;
    QString Channel_Range;
    QString Time_Setting_str;
    QString Time_Setting_str2;
    QString Time_Setting_str3;
    QString Time_Setting_str4;
    QString Time_Setting_str5;
    QString Time_Setting_str6;

    QString xlsxFileName;
    QString ReceiveData_UUT;
    QString ReceiveData_7252;


    QTimer *timer;  // For controlling the 7252 output the source pressure at a regular time through cmd.
    QTimer *timer_GetReading; // For get reading qua-realtime(through cmd) form each UUT, 300ms
    QTimer *timer_SECOND;

    int CurrentTestPoint;
    QString CurrentCMD;

    int CurrentRow;//
    int WAIT_Time_Source;
    int WAIT_Time_Read;



signals:
    void sig_Send_Next_CMD();
    void sig_Get_Reading_CMD(QextSerialPort *UUTPort);
    void sig_Parse_Reading(QString ReceiveData,PortType UUT);



private slots:
    void on_pushButton_Connect_clicked();
    void on_pushButton_DisConnect_clicked();
    void sleep(unsigned int msec);
    void SendCMD(QString CMD, QextSerialPort *Port);
    void slot_Send_Next_CMD();//7252
    void slot_Send_PollingReading_CMD();  //UUT
    void slot_Get_Reading_CMD(QextSerialPort *UUTPort);
    void Prepare_UUT(QextSerialPort *UUT_PORT);
    void Prepare_UUT_ForTest();
    void Slot_Parse_Reading(QString ReceiveData,PortType UUT);
    void slot_TIMEtoStart();

    void autoScroll();

    void AboutThis();
    void HelpFile();

    void ReadTestPoint();
    void Clear_READINGS();
    void InitialCOM_Time();

    void CreateFileName();

    void readUUT1_port();
    void readUUT2_port();
    void readUUT3_port();
    void readUUT4_port();
    void readUUT5_port();
    void readUUT6_port();
    void readUUT7_port();
    void readUUT8_port();
    void readF7252_port();

    bool ConnectTOUUT();
    bool Check_UUTCOM(QComboBox *CurrentComboBox,QextSerialPort *UUT_PORT);
    void UpdateUI_Reading(QString Reading, PortType UUT);
    void ReadUUT(QextSerialPort *UUTPort, PortType UUT);//1. parse, 2,REGEX

    void on_pushButton_Start_clicked();
    void on_pushButton_Stop_clicked();
    void on_pushButton_Pause_clicked();
    void on_pushButton_Voltage_clicked();

    QStringList GetReadingsFromUI();  //Get Readings
    QString GetISOSetting();
    void Drop_UUT_COM();
    void Pressure_Test();


    void WriteReadings2Excel(QStringList Readings);  //Write Readings
    void Write_COM_INI(QString KeyString, int COM_Index);
    void WriteTitle2Excel();

    void on_checkBox_Reading1_clicked();
    void on_checkBox_Reading2_clicked();
    void on_checkBox_Reading3_clicked();
    void on_checkBox_Reading4_clicked();
    void on_checkBox_Reading5_clicked();
    void on_checkBox_Reading6_clicked();
    void on_checkBox_Reading7_clicked();
    void on_checkBox_Reading8_clicked();

    void on_pushButton_TimerSet_clicked();

    void on_pushButton_TimerSet_2_clicked();

    void on_comboBox_UUT1_currentIndexChanged(int index);

    void on_comboBox_7252_currentIndexChanged(int index);

    void on_comboBox_UUT2_currentIndexChanged(int index);

    void on_comboBox_UUT3_currentIndexChanged(int index);

    void on_comboBox_UUT4_currentIndexChanged(int index);

    void on_comboBox_UUT5_currentIndexChanged(int index);

    void on_comboBox_UUT6_currentIndexChanged(int index);

    void on_comboBox_UUT7_currentIndexChanged(int index);

    void on_comboBox_UUT8_currentIndexChanged(int index);

    void on_pushButton_TimerSet_3_clicked();

    void on_pushButton_TimerSet_4_clicked();

    void on_pushButton_TimerSet_5_clicked();

    void on_pushButton_TimerSet_6_clicked();

    void on_pushButton_Iso_Setting_clicked();

    void on_comboBox_FIX_ISO_currentIndexChanged(int index);

    void on_checkBox_EnableDisableISO_clicked();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
