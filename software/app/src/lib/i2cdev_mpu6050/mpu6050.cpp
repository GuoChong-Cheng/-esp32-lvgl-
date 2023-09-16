
#include "mpu6050.hpp"

#define MPU6050_ADDRESS_AD0_LOW  0x68
#define MPU6050_ADDRESS_AD0_HIGH 0x69
#define MPU6050_WHO_AM_I         0x75

#define MPU6050_PWR_MGMT_1       0x6B
#define MPU6050_PWR_MGMT_2       0x6C
#define MPU6050_SMPLRT_DIV       0x19
#define MPU6050_CONFIG           0x1A
#define MPU6050_GYRO_CONFIG      0x1B
#define MPU6050_ACCEL_CONFIG     0x1C

#define MPU6050_ACCEL_FULLSCALE  2
#define MPU6050_GYRO_FULLSCALE   250

#define MPU6050_ACCEL_XOUT       0x3B
#define MPU6050_ACCEL_YOUT       0x3D
#define MPU6050_ACCEL_ZOUT       0x3F
#define MPU6050_TEMP_OUT         0x41
#define MPU6050_GYRO_XOUT        0x43
#define MPU6050_GYRO_YOUT        0x45
#define MPU6050_GYRO_ZOUT        0x47

#define MPU6050_FIFO_EN          0x23
#define MPU6050_INT_ENABLE       0x38
#define MPU6050_USER_CTRL        0x6A

#define MPU6050_ACCEL_SENSITIVE  (float)(16384)
#define MPU6050_GYRO_SENSITIVE   (float)(131.0)

#define MPU6050_TEMP_SENSITIVE   (float)340
#define MPU6050_TEMP_OFFSET      (float)36.5

void mpu6050::init(void)
{
RESET:
    write_byte(MPU6050_PWR_MGMT_1, 0x80);  // 复位
    delay_ms(100);
    write_byte(MPU6050_PWR_MGMT_1, 0x00);  // 唤醒
    if (read_byte(MPU6050_WHO_AM_I) != 0x68) {
        printf("[IMU] fail to init mpu6050\r\n");
        delay_ms(1000);
        goto RESET;
    }

    write_byte(MPU6050_SMPLRT_DIV, 0x00);         // 采样率分频:不分频
    write_byte(MPU6050_CONFIG, 0x00);             // 低通滤波器:256Hz~260Hz
    write_byte(MPU6050_GYRO_CONFIG, 0x00 << 3);   // 陀螺仪量程:±250°
    write_byte(MPU6050_ACCEL_CONFIG, 0x00 << 3);  // 加速度计量程:±2g
    // set other params
    write_byte(MPU6050_FIFO_EN, 0x00);     // disable fifo
    write_byte(MPU6050_INT_ENABLE, 0x00);  // disable interrupt
    write_byte(MPU6050_USER_CTRL, 0x00);   // disable i2c master
    write_byte(MPU6050_PWR_MGMT_2, 0x00);  // enable accel & gyro
}

void mpu6050::update(void)
{
    uint8_t data[14] = {0};

    read_block(MPU6050_ACCEL_XOUT, data, 14);

    ax = (int16_t)(((data[0] << 8) | data[1])) / MPU6050_ACCEL_SENSITIVE;
    ay = (int16_t)(((data[2] << 8) | data[3])) / MPU6050_ACCEL_SENSITIVE;
    az = (int16_t)(((data[4] << 8) | data[5])) / MPU6050_ACCEL_SENSITIVE;
    t  = (int16_t)(((data[6] << 8) | data[7])) / MPU6050_TEMP_SENSITIVE + MPU6050_TEMP_OFFSET;
    gx = (int16_t)(((data[8] << 8) | data[9])) / MPU6050_GYRO_SENSITIVE;
    gy = (int16_t)(((data[10] << 8) | data[11])) / MPU6050_GYRO_SENSITIVE;
    gz = (int16_t)(((data[12] << 8) | data[13])) / MPU6050_GYRO_SENSITIVE;

#if 1
    az = -az;
#endif
    // roll: (tilt left/right, about X axis)
    roll  = atan2(ay, az) * RAD_TO_DEG;
    // pitch: (nose up/down, about Y axis)
    pitch = atan2(ax, az) * RAD_TO_DEG;
    // yaw: (about Z axis)
    yaw   = atan2(ay, ax) * RAD_TO_DEG;
}