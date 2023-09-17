#### 20230526

ECG（KS1081 / KS1082）

PPG（MAX30102）

IMU（MPU6050）

**现存问题**

VO1（IO6）与 SD_MISO 冲突。

使用 ADC 采集 VO1 电压时，需去掉 SD_MISO 的上拉电阻。

