# BMI160 / BMX160

## 简介

本软件包是为 BOSCH BMI160 6轴传感器以及 BMX160 9轴传感器提供的通用传感器驱动包。

本驱动软件包会检测所连接传感器的ID从而自动配置为BMI160或者BMX160的驱动，无须用户手动选择。

本文档介绍该软件包的基本功能和使用方法，本文主要内容如下：

- 传感器介绍
- 支持情况
- 使用说明

## 传感器介绍

BMI160 是 BOSCH（博世）公司专为可穿戴设备和 IOT 市场开发的一款超低功耗6轴传感器，尺寸小巧，内置加速计和陀螺仪。

BMX160 是 BOSCH（博世）公司专为可穿戴设备和 IOT 市场开发的一款超低功耗9轴传感器，内置加速计、陀螺仪和磁力计。

## 支持情况

| 包含设备         | 加速计 | 陀螺仪 | 磁力计 |
| ---------------- | -------- | ------ | ------ |
| **通讯接口**     |          |        |        |
| IIC              | √        | √      | √      |
| SPI              |          |        |        |
| **工作模式**     |          |        |        |
| 轮询             | √        | √      | √      |
| 中断             | √        | √      | √      |
| FIFO             | √        | √      | √      |
| **电源模式**     |          |        |        |
| 掉电             | √        | √      | √      |
| 低功耗           | √        |        | √      |
| 普通             | √        | √      | √      |
| 高功耗           |          |        |        |
| **数据输出速率** | √        | √      | √      |
| **测量范围**     | √        | √      |        |
| **自检**         |          |        |        |
| **多实例**       | √        | √      | √      |

## 使用说明

### 依赖

- RT-Thread 4.0.0+
- Sensor 组件
- IIC 驱动：BMI160 / BMX160 设备使用 IIC 进行数据通讯，需要系统 IIC 驱动框架支持；

### 获取软件包

使用 BMI160/BMX160 软件包需要在 RT-Thread 的包管理中选中它，具体路径如下：

```
BMI160: BMI160 Digital 6-axis / BMX160 Digital 9-axis sensor
    [*]   Enable BMI160 / BMX160 accelerometer
    [*]   Enable BMI160 / BMX160 gyroscope
    [*]   Enable BMX160 magnetometer
        Version (latest)  --->
```

**Enable BMI160 accelerometer**： 配置开启加速度测量功能

**Enable BMI160 gyroscope**：配置开启陀螺仪功能

**Enable BMI160 magnetometer**：配置开启磁力计功能（只有BMX160支持磁力计）

**Version**：软件包版本选择

### 使用软件包

BMA400 软件包初始化函数如下所示：

```
int rt_hw_bmx160_init(const char *name, struct rt_sensor_config *cfg);
```

`rt_hw_bmx160_init`函数需要由用户调用，函数主要完成的功能有，

- 设备配置和初始化（根据传入的配置信息，配置接口设备和中断引脚）；
- 注册相应的传感器设备，完成 BMI160 / BMX160 设备的注册；

BMI160和BMX160共用一个驱动，当程序检测到传感器芯片ID为BMI160时会注册加速计和陀螺仪的驱动程序，如果ID为BMX160还会加载磁力计的驱动程序。如果你只需要某些特定的传感器，请到`menuconfig`菜单中进行配置。

#### 初始化示例

```
#include "sensor_bosch_bmx160.h"

int bmx160_port(void)
{
    struct rt_sensor_config cfg;
    cfg.intf.dev_name = "i2c1";
    cfg.intf.user_data = (void *)0x69;
    cfg.irq_pin.pin = GET_PIN(B, 0);
    cfg.irq_pin.mode = PIN_MODE_INPUT_PULLDOWN;
    rt_hw_bmx160_init("bmx160", &cfg);
    return 0;
}
INIT_APP_EXPORT(bmx160_port);
```

## 注意事项

暂无

## 联系人信息

维护人:

- [gztss](https://github.com/gztss)
