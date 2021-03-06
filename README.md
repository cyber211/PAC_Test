# PAC_Test
For Fluke  Product Test

This program is 

# Revision history：

## V2.6 --未升版本号
1. 优化了串口读数的响应时间，修掉了一个串口接收的处理方式影响读数刷新快慢的bug。

## V2.6:
1. 修改了测试记录的方式，将不同的测试分别存储为单独的文件，文件名是测试开始的时间；
    有几个定时测试就存储为几个文件，文件的名称：“PAC_30PSI_2017_03_13_XXXXXX.xlsx”
2. 放开了ISO隔离阀的设定，disable的话，则测试过程不会控制隔离阀的动作，需要提前手动设置好，可供调试用
     enable的话，则测试时会根据设置状态去控制隔离阀治具；

## V2.5: 按照测试需求修改如下：
1. 取消了根据所测试的sensortype自动选取7252通道的功能，Sensor Type的选择只是跟脚本的选择有关联，脚本只设置测试点
2. 增加了设置7252的压力源通道的下拉框选项，测试前需要根据所测量的sensor的range***手动*** 选择合适的7252源通道；
    -- 这会导致定时测试不同量程的meter时需要手动设置7252的通道，不能根据model型号自动选择所需通道。



## V2.4:
1. 去掉了设置7252单位的命令，因为会引起命令错误，导致7252无法控制；
2. 单窗口测试：
     - 手动开始测试： Connect -> set ISO - > pressure test;
     - 定时自动开始测试：open app - > ONLY select the settings( equipment com ,UUT coM , iso setting), DO NOT press any buttons; --> 设置好定时时间，ok；

3. 多窗口测试：
     定时自动开始测试：
           - 各个窗口只需设置好各自的设置，不用点击按钮；
           - 设置好定时器；

## V2.3:
1. 增加Fixture的隔离阀的控制；
2. 增加设置7252单位的命令；防止断电后恢复为psi；去掉了因为会引起命令错误，导致7252无法控制；
3. 试图读取7252的数据， ok；

## Ver 2.2：
1. 记录excel中数据title增加COM号与测试数据一一对应--- 改功能去去掉，会覆盖掉上一组数据的最后一个值；
2. 增加定时测试到6个；
3. 增加记录温度（表温）的数据（待完成）

## Ver 2.1：
1. 增加了支持同时设置两个定时测试功能，晚上可以测试两个温度点；理论上可以增加很多定时器，只要保证温箱能够跟测试时间同步；
2. 增加了自动保存UUT的测试COM口号；
3. 记录excel中数据title增加COM号与测试数据一一对应;
4. 增加记录温度（表温）的数据（待完成）

## Ver 2.0 ：
1. 增加了sensor type的下拉框选项，程序会根据所选的sensortype，自动Load测试点（测试点文件需要事先编辑好，TestPoint_30PSI.ini，TestPoint_150PSI.ini，TestPoint_300PSI.ini），根据seneor Type自动选择7252的气路；
2. 增加了定时自动测试功能，可以实现晚上定时无人值守测试；

## Ver 1.0 ：
1. 基本自动测量功能，有些设置需要手动设置

# User Guide
目前只做了压力测量的功能，最多能同时测试8台表。

## 准备工作：
1. 根据所测量的sensor，接好正确的气路；
2. UUT的压力清零；
3. 编辑好对应的sensor的测试点文件：TestPoint_30PSI.ini，TestPoint_150PSI.ini，TestPoint_300PSI.ini
    Note：编辑测试点时，测试点POINT001等的数字部分需要用0 补齐，否则测试的顺序会不是按照测试点的顺序进行的。


## 测试步骤：
1. 选择好7252的COM口；

2. 点击“CONNECT EQUIPMENTS”按钮，连接设备，如果提示串口连接失败，请选择正确的正确的COM口再试，直到7252连接成功；其他设备的连接不用去管；

3. 选择要测试的UUT ， 在前面CheckBox打勾，必须顺序打勾，如测试6台，则请顺序勾选UUT1,UUT2,UUT3,UUT4....UUT6;

4. 自动测试：
      4.1 手动完成一轮测试： 点击“PRES Test"按钮，立即开始自动测试，自动记录数据，测试完成后，数据记录在程序同一文件夹下PAC.XLSX文件中，请注意，测试时，请不要打开此文件，否则又可能导致某些点的数据丢失（程序无法写入数据）；
      4.2 定时完成一轮测试： 不要点击“PRES Test"按钮，右下角设置好开始自动测试，自动记录数据，测试完成后，数据记录在程序同一文件夹下PAC.XLSX文件中，请注意，测试时，请不要打开此文件，否则又可能导致某些点的数据丢失（程序无法写入数据）；

 

## 自动测试过程：
1. 程序会依次自动设置并输出控制7252输出测试点的压力；
2. 程序会自动读取并记录数据到excel中；
