# BMI160传感器API

## 介绍

该软件包包含Bosch Sensortec的BMI160传感器驱动程序（传感器API）

传感器驱动程序包包括bmi160.h，bmi160.c和bmi160_defs.h文件

## 版

| 文件          | 版    | 日期          |
| ------------- | ----- | ------------- |
| bmi160.c      | 3.7.5 | 2018年1月11日 |
| bmi160.h      | 3.7.5 | 2018年1月11日 |
| bmi160_defs.h | 3.7.5 | 2018年1月11日 |

## 整合细节

- 将bmi160.h，bmi160_defs.h和bmi160.c文件集成到您的项目中。
- 如下所示，将bmi160.h文件包含在您的代码中。

```
＃包括 “ bmi160.h ”
```

## 档案资讯

- bmi160_defs.h：此头文件包含常量，宏和数据类型声明。
- bmi160.h：此头文件包含传感器驱动程序API的声明。
- bmi160.c：此源文件包含传感器驱动程序API的定义。

## 支持的传感器接口

- SPI 4线
- I2C

## 使用指南

### 初始化传感器

要初始化传感器，您首先需要创建一个设备结构。您可以通过创建结构bmi160_dev的实例来做到这一点。然后继续填写各种参数，如下所示。

#### SPI 4线示例

```c
struct bmi160_dev传感器;

/ *您可以分配一个片选标识符，以供以后处理* / 
sensor.id = 0 ;
sensor.interface = BMI160_SPI_INTF;
sensor.read = user_spi_read;
sensor.write = user_spi_write;
sensor.delay_ms = user_delay_ms;


int8_t rslt = BMI160_OK;
rslt = bmi160_init（＆sensor）;
/ *在上述函数调用之后，设备
结构中的accel_cfg和gyro_cfg参数设置为默认值，该默认值位于传感器的数据表中* /
```

#### I2C示例

```c
struct bmi160_dev传感器;

sensor.id = BMI160_I2C_ADDR;
sensor.interface = BMI160_I2C_INTF;
sensor.read = user_i2c_read;
sensor.write = user_i2c_write;
sensor.delay_ms = user_delay_ms;

int8_t rslt = BMI160_OK;
rslt = bmi160_init（＆sensor）;
/ *在上述函数调用之后，设备结构
中的加速度和陀螺仪参数设置为默认值，可在传感器的数据表中找到* /
```

### 配置加速度传感器和陀螺仪传感器

#### 在普通模式下配置加速度传感器和陀螺仪传感器的示例

```c
int8_t rslt = BMI160_OK;

/ *选择输出数据速率，加速度传感器的范围* /
sensor.accel_cfg.odr = BMI160_ACCEL_ODR_1600HZ;
sensor.accel_cfg.range = BMI160_ACCEL_RANGE_2G;
sensor.accel_cfg.bw = BMI160_ACCEL_BW_NORMAL_AVG4;

/ *选择加速度传感器的电源模式* /
sensor.accel_cfg.power = BMI160_ACCEL_NORMAL_MODE;

/ *选择输出数据速率，陀螺仪传感器的范围* /
sensor.gyro_cfg.odr = BMI160_GYRO_ODR_3200HZ;
sensor.gyro_cfg.range = BMI160_GYRO_RANGE_2000_DPS;
sensor.gyro_cfg.bw = BMI160_GYRO_BW_NORMAL_MODE;

/ *选择陀螺仪传感器的电源模式* /
sensor.gyro_cfg.power = BMI160_GYRO_NORMAL_MODE; 

/ *设置传感器配置* / 
rslt = bmi160_set_sens_conf（＆sensor）;
```

### 读取传感器数据

#### 读取传感器数据的示例

```c
int8_t rslt = BMI160_OK;
struct bmi160_sensor_data accel;
struct bmi160_sensor_data陀螺仪;

/ *仅读取Accel数据* / 
rslt = bmi160_get_sensor_data（BMI160_ACCEL_SEL，＆accel， NULL，＆sensor）;

/ *仅读取陀螺仪数据* / 
rslt = bmi160_get_sensor_data（BMI160_GYRO_SEL， NULL，＆gyro，＆sensor）;

/ *同时读取Accel和Gyro数据* /
 bmi160_get_sensor_data（（BMI160_ACCEL_SEL | BMI160_GYRO_SEL），＆accel，＆gyro和＆sensor）;

/ *要与时间一起读取Accel数据* / 
rslt = bmi160_get_sensor_data（（BMI160_ACCEL_SEL | BMI160_TIME_SEL），＆accel， NULL，＆sensor）;

/ *与时间一起读取陀螺仪数据* / 
rslt = bmi160_get_sensor_data（（BMI160_GYRO_SEL | BMI160_TIME_SEL）， NULL，＆gyro，＆sensor）;

/ *要同时读取Accel和Gyro数据以及时间* /
 bmi160_get_sensor_data（（BMI160_ACCEL_SEL | BMI160_GYRO_SEL | BMI160_TIME_SEL），＆accel，＆gyro和＆sensor）;
```

### 设置传感器的电源模式

#### 设置加速度和陀螺仪功率模式的示例

```c
int8_t rslt = BMI160_OK;

/ *选择电源模式* /
sensor.accel_cfg.power = BMI160_ACCEL_SUSPEND_MODE; 
sensor.gyro_cfg.power = BMI160_GYRO_FASTSTARTUP_MODE; 

/ *  设置电源模式  * /
rslt = bmi160_set_power_mode（＆sensor）;

/ *选择电源模式* /
sensor.accel_cfg.power = BMI160_ACCEL_NORMAL_MODE;
sensor.gyro_cfg.power = BMI160_GYRO_NORMAL_MODE; 

/ *  设置电源模式  * /
rslt = bmi160_set_power_mode（＆sensor）;
```

### 读取传感器数据寄存器

#### 读取芯片地址示例

```c
int8_t rslt = BMI160_OK;
uint8_t reg_addr = BMI160_CHIP_ID_ADDR;
uint8_t数据；
uint16_t len = 1 ;
rslt = bmi160_get_regs（reg_addr，＆data，len，＆sensor）;
```

### 写入传感器数据寄存器

#### 将数据写入任何运动阈值寄存器的示例

```c
int8_t rslt = BMI160_OK;
uint8_t reg_addr = BMI160_INT_MOTION_1_ADDR;
uint8_t data = 20 ;
uint16_t len = 1 ;
rslt = bmi160_set_regs（reg_addr，＆data，len，＆sensor）;
```

### 使用软重置重置设备

#### 将软复位命令写入命令寄存器的示例

```c
int8_t rslt = BMI160_OK;
rslt = bmi160_soft_reset（＆sensor）;
```

### 配置传感器中断

要配置传感器中断，您首先需要创建一个中断结构。您可以通过创建结构bmi160_int_settg的实例来执行此操作。然后继续填写各种参数，如下所示

### 配置任意动作中断

#### 配置任意运动中断的示例

注意：-用户可以通过检查bmi160_dev结构的**any_sig_sel**来检查当前活动的中断（any-motion或sig-motion）。

```c
struct bmi160_int_settg int_config;

/ *选择中断通道/引脚* / 
int_config.int_channel = BMI160_INT_CHANNEL_1; //中断通道/引脚1

/ *选择中断类型* / 
int_config.int_type = BMI160_ACC_ANY_MOTION_INT; //选择任何运动中断
/ *选择中断通道/引脚设置* / 
int_config.int_pin_sett.output_en = BMI160_ENABLE; //使中断引脚充当输出引脚
int_config.int_pin_sett.output_mode = BMI160_DISABLE; //为中断引脚
int_config.int_pin_sett.output_type = BMI160_DISABLE;选择推挽模式 //选择低电平有效输出
int_config.int_pin_sett.edge_ctrl = BMI160_ENABLE; //选择边沿触发的输出
int_config.int_pin_sett.input_en = BMI160_DISABLE;//禁用中断引脚作为输入
int_config.int_pin_sett.latch_dur = BMI160_LATCH_DUR_NONE; //非锁定输出

/ *选择任意运动中断参数* / 
int_config.int_type_cfg.acc_any_motion_int.anymotion_en = BMI160_ENABLE; // 1-启用任意动作，0-禁用任意动作
int_config.int_type_cfg.acc_any_motion_int.anymotion_x = BMI160_ENABLE; //为任何运动中断启用x轴
int_config.int_type_cfg.acc_any_motion_int.anymotion_y = BMI160_ENABLE; //为任何运动中断启用y轴
int_config.int_type_cfg.acc_any_motion_int.anymotion_z = BMI160_ENABLE; //为任何运动中断启用z轴
int_config.int_type_cfg.acc_any_motion_int.anymotion_dur = 0 ; //任何运动持续时间
int_config.int_type_cfg.acc_any_motion_int.anymotion_thr = 20 ; //（2-g范围）->（slope_thr）* 3.91 mg，（4-g范围）->（slope_thr）* 7.81 mg，（8-g范围）->（slope_thr）* 15.63 mg，（16- g范围）->（slope_thr）* 31.25毫克

/ *设置任意运动中断* /
 bmi160_set_int_config（＆int_config，＆sensor）; / * sensor是结构bmi160_dev的实例  * /
```

### 配置平面中断

#### 配置平面中断的示例

```c
struct bmi160_int_settg int_config;

/ *选择中断通道/引脚* / 
int_config.int_channel = BMI160_INT_CHANNEL_1; //中断通道/引脚1

/ *选择中断类型* / 
int_config.int_type = BMI160_ACC_FLAT_INT; //选择平面中断
/ *选择中断通道/引脚设置* / 
int_config.int_pin_sett.output_en = BMI160_ENABLE; //使中断引脚充当输出引脚
int_config.int_pin_sett.output_mode = BMI160_DISABLE; //为中断引脚
int_config.int_pin_sett.output_type = BMI160_DISABLE;选择推挽模式 //选择低电平有效输出
int_config.int_pin_sett.edge_ctrl = BMI160_ENABLE; //选择边沿触发的输出
int_config.int_pin_sett.input_en = BMI160_DISABLE;//禁用中断引脚作为输入
int_config.int_pin_sett.latch_dur = BMI160_LATCH_DUR_NONE; //非锁定输出

/ *选择Flat中断参数* / 
int_config.int_type_cfg.acc_flat_int.flat_en = BMI160_ENABLE; // 1启用，0禁用平面中断
int_config.int_type_cfg.acc_flat_int.flat_theta = 8 ; //检测平面位置的阈值，范围为0°至44.8°。
int_config.int_type_cfg.acc_flat_int.flat_hy = 1 ; //迟滞
int_config.int_type_cfg.acc_flat_int.flat_hold_time = 1 ; //固定保持时间（0-> 0 ms，1-> 640 ms，2-> 1280 ms，3-> 2560 ms）

/ *设置Flat中断* /
 bmi160_set_int_config（＆int_config，＆sensor）; / * sensor是结构bmi160_dev的实例* /
```

### 配置步进检测器中断

#### 配置步进检测器中断的示例

```c
struct bmi160_int_settg int_config;

/ *选择中断通道/引脚* / 
int_config.int_channel = BMI160_INT_CHANNEL_1; //中断通道/引脚1

/ *选择中断类型* / 
int_config.int_type = BMI160_STEP_DETECT_INT; //选择步进检测器中断
/ *选择中断通道/引脚设置* / 
int_config.int_pin_sett.output_en = BMI160_ENABLE; //使中断引脚充当输出引脚
int_config.int_pin_sett.output_mode = BMI160_DISABLE; //为中断引脚
int_config.int_pin_sett.output_type = BMI160_ENABLE;选择推挽模式 //选择有效的高输出
int_config.int_pin_sett.edge_ctrl = BMI160_ENABLE; //选择边沿触发的输出
int_config.int_pin_sett.input_en = BMI160_DISABLE;//禁用中断引脚作为输入
int_config.int_pin_sett.latch_dur = BMI160_LATCH_DUR_NONE; //非锁定输出

/ *选择步骤检测器中断参数，请为步骤检测器使用推荐的设置* /
int_config.int_type_cfg.acc_step_detect_int.step_detector_mode = BMI160_STEP_DETECT_NORMAL;
int_config.int_type_cfg.acc_step_detect_int.step_detector_en = BMI160_ENABLE; // 1启用，0禁用步进检测器

/ *设置步进检测器中断* /
 bmi160_set_int_config（＆int_config，＆sensor）; / * sensor是结构bmi160_dev的实例* /
```

### 配置计步器

要配置步数计数器，用户需要按上节所述配置步幅检测器中断。配置步进检测器后，请参见下面的代码片段，了解用户空间和ISR

### 用户空间

```c
int8_t rslt = BMI160_OK;
uint8_t step_enable = 1 ; //启用计步器

rslt = bmi160_set_step_counter（step_enable，＆sensor）;
```

### 情监侦

```c
int8_t rslt = BMI160_OK;
uint16_t step_count = 0 ; //存储步数计数器值

rslt = bmi160_read_step_counter（＆step_count，＆sensor）;
```

### 取消映射中断

#### 取消映射步进检测器中断的示例

```c
struct bmi160_int_settg int_config;

/ *取消选择中断通道/引脚* /
int_config.int_channel = BMI160_INT_CHANNEL_NONE;
/ *选择中断类型* / 
int_config.int_type = BMI160_STEP_DETECT_INT; //选择步进检测器中断
/ *设置步进检测器中断* /
 bmi160_set_int_config（＆int_config，＆sensor）; / * sensor是结构bmi160_dev的实例* /
```

### 读取中断状态

#### 读取步进检测器中断状态的示例

```c
union bmi160_int_status中断;
枚举bmi160_int_status_sel int_status_sel;

/ *选择中断状态以读取所有中断* /
int_status_sel = BMI160_INT_STATUS_ALL;
rslt = bmi160_get_int_status（int_status_sel，＆interrupt，＆sensor）;
if（interrupt.bit.step）
	 printf（“发生步进检测器中断\ n ”）;
```

### 配置辅助传感器BMM150

假设bmi160的辅助接口具有外部上拉电阻，以便访问辅助传感器bmm150。

### 通过BMI160辅助接口使用BMM150 API访问辅助BMM150。

## 整合细节

- 将BMM150和BMI160的源代码集成到项目中。
- 如下所示在代码中包含bmi160.h和bmm150.h文件。
- 必须为主要接口和辅助传感器设置初始化bmi160设备结构。
- 创建两个包装函数，user_aux_read和user_aux_write，以匹配签名，如下所述。
- 调用“ bmi160_aux_init” API来初始化BMI160中的辅助接口。
- 调用“ bmm150_init” API来初始化BMM150传感器。
- 现在，我们可以使用BMM150传感器API通过BMI160访问BMM150。

```
/ * main.c文件* / 
＃包含 “ bmi160.h ” 
＃包含 “ bmm150.h ”
```

### 辅助传感器BMM150的初始化

```c
/* main.c file */
struct bmm150_dev bmm150;

/* function declaration */
int8_t user_aux_read(uint8_t id, uint8_t reg_addr, uint8_t *aux_data, uint16_t len);
int8_t user_aux_write(uint8_t id, uint8_t reg_addr, uint8_t *aux_data, uint16_t len);

/* Configure device structure for auxiliary sensor parameter */
sensor.aux_cfg.aux_sensor_enable = 1; // auxiliary sensor enable
sensor.aux_cfg.aux_i2c_addr = BMI160_AUX_BMM150_I2C_ADDR; // auxiliary sensor address
sensor.aux_cfg.manual_enable = 1; // setup mode enable
sensor.aux_cfg.aux_rd_burst_len = 2;// burst read of 2 byte

/* Configure the BMM150 device structure by 
mapping user_aux_read and user_aux_write */
bmm150.read = user_aux_read;
bmm150.write = user_aux_write;
bmm150.id = BMM150_DEFAULT_I2C_ADDRESS; 
/* Ensure that sensor.aux_cfg.aux_i2c_addr = bmm150.id
   for proper sensor operation */
bmm150.delay_ms = delay_ms;
bmm150.interface = BMM150_I2C_INTF;

/* Initialize the auxiliary sensor interface */
rslt = bmi160_aux_init(&sensor);

/* Auxiliary sensor is enabled and can be accessed from this point */

/* Configure the desired settings in auxiliary BMM150 sensor 
 * using the bmm150 APIs */

/* Initialising the bmm150 sensor */
rslt = bmm150_init(&bmm150);

/* Set the power mode and preset mode to enable Mag data sampling */
bmm150.settings.pwr_mode = BMM150_NORMAL_MODE;
rslt = bmm150_set_op_mode(&bmm150);

bmm150.settings.preset_mode= BMM150_PRESETMODE_LOWPOWER;
rslt = bmm150_set_presetmode(&bmm150);
```

### 包装功能

```
/*wrapper function to match the signature of bmm150.read */
int8_t user_aux_read(uint8_t id, uint8_t reg_addr, uint8_t *aux_data, uint16_t len)
{
	int8_t rslt;
	
	/* Discarding the parameter id as it is redundant*/
        rslt = bmi160_aux_read(reg_addr, aux_data, len, &bmi160);

	return rslt;
}

/*wrapper function to match the signature of bmm150.write */
int8_t user_aux_write(uint8_t id, uint8_t reg_addr, uint8_t *aux_data, uint16_t len)
{
	int8_t rslt;
	
	/* Discarding the parameter id as it is redundant */
	rslt = bmi160_aux_write(reg_addr, aux_data, len, &bmi160);

	return rslt;
}
```

### 在自动模式下初始化辅助BMM150

任何数据字节小于或等于8个字节的传感器都可以与BMI160同步，并可以从该实例的加速度计+陀螺仪+辅助传感器数据中读出，这有助于创建更少的延迟融合数据

```
/* Initialize the Auxiliary BMM150 following the above code 
 * until setting the power mode (Set the power mode as forced mode)
 * and preset mode */

	/* In BMM150 Mag data starts from register address 0x42 */
	uint8_t aux_addr = 0x42;
	/* Buffer to store the Mag data from 0x42 to 0x48 */	
	uint8_t mag_data[8] = {0};
	
	uint8_t index;
		
	/* Configure the Auxiliary sensor either in auto/manual modes and set the 
	polling frequency for the Auxiliary interface */	
	sensor.aux_cfg.aux_odr = 8; /* Represents polling rate in 100 Hz*/
	rslt = bmi160_config_aux_mode(&sensor)
	
	/* Set the auxiliary sensor to auto mode */
	rslt = bmi160_set_aux_auto_mode(&aux_addr, &sensor);

	/* Reading data from BMI160 data registers */
	rslt = bmi160_read_aux_data_auto_mode(mag_data, &sensor);

	printf("\n RAW DATA ");
	for(index = 0 ; index < 8 ; index++)
	{
		printf("\n MAG DATA[%d] : %d ", index, mag_data[index]);
	}
	
	/* Compensating the raw mag data available from the BMM150 API */
	rslt = bmm150_aux_mag_data(mag_data, &bmm150);
	
	printf("\n COMPENSATED DATA ");
	printf("\n MAG DATA X : %d Y : %d Z : %d", bmm150.data.x, bmm150.data.y, bmm150.data.z);
	
```

### 辅助FIFO数据解析

辅助传感器数据可以存储在FIFO中，在此我们演示使用Bosch磁力计传感器BMM150并将其数据存储在FIFO中的示例

```
/* Initialize the Aux BMM150 following the above 
 * code and by creating the Wrapper functions */

	int8_t rslt = 0;
	uint8_t aux_instance = 0;
	uint16_t fifo_cnt = 0;
	uint8_t auto_mode_addr;
	uint8_t i;

	/* Setup and configure the FIFO buffer */
	/* Declare memory to store the raw FIFO buffer information */
	uint8_t fifo_buff[1000] = {0};

	/* Modify the FIFO buffer instance and link to the device instance */
	struct bmi160_fifo_frame fifo_frame;
	fifo_frame.data = fifo_buff;
	fifo_frame.length = 1000;
	dev->fifo = &fifo_frame;

	/* Declare instances of the sensor data structure to store the parsed FIFO data */
	struct bmi160_aux_data aux_data[112]; //1000 / 9 bytes per frame ~ 111 data frames

	rslt = bmi160_init(dev);
	printf("\n BMI160 chip ID is : %d ",dev->chip_id);

	rslt = bmi160_aux_init(dev);

	rslt = bmm150_init(&bmm150);
	printf("\n BMM150 CHIP ID : %d",bmm150.chip_id);

	bmm150.settings.preset_mode = BMM150_PRESETMODE_LOWPOWER;
	rslt = bmm150_set_presetmode(&bmm150);

	bmm150.settings.pwr_mode = BMM150_FORCED_MODE;
	rslt = bmm150_set_op_mode(&bmm150);

	/* Enter the data register of BMM150 to "auto_mode_addr" here it is 0x42 */
	auto_mode_addr = 0x42;
	printf("\n ENTERING AUX. AUTO MODE ");
	dev->aux_cfg.aux_odr = BMI160_AUX_ODR_25HZ;
	rslt = bmi160_set_aux_auto_mode(&auto_mode_addr, dev);


	/* Disable other FIFO settings */
	rslt = bmi160_set_fifo_config(BMI160_FIFO_CONFIG_1_MASK , BMI160_DISABLE, dev);

	/* Enable the required FIFO settings */
	rslt = bmi160_set_fifo_config(BMI160_FIFO_AUX | BMI160_FIFO_HEADER, BMI160_ENABLE, dev);

	/* Delay for the FIFO to get filled */
	dev->delay_ms(400);


	printf("\n FIFO DATA REQUESTED (in bytes): %d",dev->fifo->length);
	rslt = bmi160_get_fifo_data(dev);
	printf("\n FIFO DATA AVAILABLE (in bytes): %d",dev->fifo->length);

	/* Print the raw FIFO data obtained */
	for(fifo_cnt = 0; fifo_cnt < dev->fifo->length ; fifo_cnt++) {
		printf("\n FIFO DATA [%d] IS : %x  ",fifo_cnt ,dev->fifo->data[fifo_cnt]);
	}

	printf("\n\n----------------------------------------------------\n");

	/* Set the number of required sensor data instances */
	aux_instance = 150;

	/* Extract the aux data , 1frame = 8 data bytes */
	printf("\n AUX DATA REQUESTED TO BE EXTRACTED (in frames): %d",aux_instance);
	rslt = bmi160_extract_aux(aux_data, &aux_instance, dev);
	printf("\n AUX DATA ACTUALLY EXTRACTED (in frames): %d",aux_instance);

	/* Printing the raw aux data */
	for (i = 0; i < aux_instance; i++) {
		printf("\n Aux data[%d] : %x",i,aux_data[i].data[0]);
		printf("\n Aux data[%d] : %x",i,aux_data[i].data[1]);
		printf("\n Aux data[%d] : %x",i,aux_data[i].data[2]);
		printf("\n Aux data[%d] : %x",i,aux_data[i].data[3]);
		printf("\n Aux data[%d] : %x",i,aux_data[i].data[4]);
		printf("\n Aux data[%d] : %x",i,aux_data[i].data[5]);
		printf("\n Aux data[%d] : %x",i,aux_data[i].data[6]);
		printf("\n Aux data[%d] : %x",i,aux_data[i].data[7]);
	}

	printf("\n\n----------------------------------------------------\n");

	/* Compensate the raw mag data using BMM150 API */
	for (i = 0; i < aux_instance; i++) {
		printf("\n----------------------------------------------------");
		printf("\n Aux data[%d] : %x , %x , %x , %x , %x , %x , %x , %x",i
					,aux_data[i].data[0],aux_data[i].data[1]
					,aux_data[i].data[2],aux_data[i].data[3]
					,aux_data[i].data[4],aux_data[i].data[5]
					,aux_data[i].data[6],aux_data[i].data[7]);

		/* Compensated mag data using BMM150 API */
		rslt = bmm150_aux_mag_data(&aux_data[i].data[0], &bmm150);

		/* Printing the  Compensated mag data */
		if (rslt == BMM150_OK) {
			printf("\n MAG DATA COMPENSATION USING BMM150 APIs");
			printf("\n COMPENSATED DATA ");
			printf("\n MAG DATA X : %d	Y : %d      Z : %d"
				, bmm150.data.x, bmm150.data.y, bmm150.data.z);

		} else {
			printf("\n MAG DATA COMPENSATION IN BMM150 API is FAILED ");
		}
		printf("\n----------------------------------------------------\n");
	}
```

## 自我测试

#### 进行加速自检的示例

```c
/* Call the "bmi160_init" API as a prerequisite before performing self test
 * since invoking self-test will reset the sensor */

	rslt = bmi160_perform_self_test(BMI160_ACCEL_ONLY, sen);
	/* Utilize the enum BMI160_GYRO_ONLY instead of BMI160_ACCEL_ONLY
	   to perform self test for gyro */
	if (rslt == BMI160_OK) {
		printf("\n ACCEL SELF TEST RESULT SUCCESS);
	} else {
		printf("\n ACCEL SELF TEST RESULT FAIL);
	}
```

## 先进先出

#### 在页眉模式下读取FIFO和提取陀螺仪数据的示例

```
/* An example to read the Gyro data in header mode along with sensor time (if available)
 * Configure the gyro sensor as prerequisite and follow the below example to read and
 * obtain the gyro data from FIFO */
int8_t fifo_gyro_header_time_data(struct bmi160_dev *dev)
{
	int8_t rslt = 0;

	/* Declare memory to store the raw FIFO buffer information */
	uint8_t fifo_buff[300];
	
	/* Modify the FIFO buffer instance and link to the device instance */
	struct bmi160_fifo_frame fifo_frame;
	fifo_frame.data = fifo_buff;
	fifo_frame.length = 300;
	dev->fifo = &fifo_frame;
	uint16_t index = 0;
	
	/* Declare instances of the sensor data structure to store the parsed FIFO data */
	struct bmi160_sensor_data gyro_data[42]; // 300 bytes / ~7bytes per frame ~ 42 data frames
	uint8_t gyro_frames_req = 42; 
	uint8_t gyro_index;
	
	/* Configure the sensor's FIFO settings */
	rslt = bmi160_set_fifo_config(BMI160_FIFO_GYRO | BMI160_FIFO_HEADER | BMI160_FIFO_TIME,
					BMI160_ENABLE, dev);
					
	if (rslt == BMI160_OK) {
		/* At ODR of 100 Hz ,1 frame gets updated in 1/100 = 0.01s
		i.e. for 42 frames we need 42 * 0.01 = 0.42s = 420ms delay */
		dev->delay_ms(420); 
	
		/* Read data from the sensor's FIFO and store it the FIFO buffer,"fifo_buff" */
		printf("\n USER REQUESTED FIFO LENGTH : %d\n",dev->fifo->length);
		rslt = bmi160_get_fifo_data(dev);

		if (rslt == BMI160_OK) {
			printf("\n AVAILABLE FIFO LENGTH : %d\n",dev->fifo->length);
			/* Print the raw FIFO data */
			for (index = 0; index < dev->fifo->length; index++) {
				printf("\n FIFO DATA INDEX[%d] = %d", index,
					dev->fifo->data[index]);
			}
			/* Parse the FIFO data to extract gyro data from the FIFO buffer */
			printf("\n REQUESTED GYRO DATA FRAMES : %d\n ",gyro_frames_req);
			rslt = bmi160_extract_gyro(gyro_data, &gyro_frames_req, dev);

			if (rslt == BMI160_OK) {
				printf("\n AVAILABLE GYRO DATA FRAMES : %d\n ",gyro_frames_req);
				
				/* Print the parsed gyro data from the FIFO buffer */
				for (gyro_index = 0; gyro_index < gyro_frames_req; gyro_index++) {
					printf("\nFIFO GYRO FRAME[%d]",gyro_index);
					printf("\nGYRO X-DATA : %d \t Y-DATA : %d \t Z-DATA : %d"
						,gyro_data[gyro_index].x ,gyro_data[gyro_index].y
						,gyro_data[gyro_index].z);
				}
				/* Print the special FIFO frame data like sensortime */
				printf("\n SENSOR TIME DATA : %d \n",dev->fifo->sensor_time);
				printf("SKIPPED FRAME COUNT : %d",dev->fifo->skipped_frame_count);
			} else {
				printf("\n Gyro data extraction failed");
			}
		} else {
			printf("\n Reading FIFO data failed");
		}
	} else {
		printf("\n Setting FIFO configuration failed");
	}

	return rslt;
}
```

## FOC和偏移补偿

> 低功耗模式下不应该使用FOC

#### 为加速和陀螺仪配置FOC的示例

```
/* An example for configuring FOC for accel and gyro data */
int8_t start_foc(struct bmi160_dev *dev)
{
	int8_t rslt = 0;
	/* FOC configuration structure */
	struct bmi160_foc_conf foc_conf;
	/* Structure to store the offsets */
	struct bmi160_offsets offsets;
	
	/* Enable FOC for accel with target values of z = 1g ; x,y as 0g */
	foc_conf.acc_off_en = BMI160_ENABLE;
	foc_conf.foc_acc_x  = BMI160_FOC_ACCEL_0G;
	foc_conf.foc_acc_y  = BMI160_FOC_ACCEL_0G;
	foc_conf.foc_acc_z  = BMI160_FOC_ACCEL_POSITIVE_G;
	
	/* Enable FOC for gyro */
	foc_conf.foc_gyr_en = BMI160_ENABLE;
	foc_conf.gyro_off_en = BMI160_ENABLE;

	rslt = bmi160_start_foc(&foc_conf, &offsets, sen);
	
	if (rslt == BMI160_OK) {
		printf("\n FOC DONE SUCCESSFULLY ");
		printf("\n OFFSET VALUES AFTER FOC : ");
		printf("\n OFFSET VALUES ACCEL X : %d ",offsets.off_acc_x);
		printf("\n OFFSET VALUES ACCEL Y : %d ",offsets.off_acc_y);
		printf("\n OFFSET VALUES ACCEL Z : %d ",offsets.off_acc_z);
		printf("\n OFFSET VALUES GYRO  X : %d ",offsets.off_gyro_x);
		printf("\n OFFSET VALUES GYRO  Y : %d ",offsets.off_gyro_y);
		printf("\n OFFSET VALUES GYRO  Z : %d ",offsets.off_gyro_z);	
	}
	
	/* After start of FOC offsets will be updated automatically and 
	 * the data will be very much close to the target values of measurement */

	return rslt;
}
```

#### 手动更新偏移量的示例

> 通过此方法设置的偏移量将在软重置/ POR时重置

```
/* An example for updating manual offsets to sensor */
int8_t write_offsets(struct bmi160_dev *dev)
{
	int8_t rslt = 0;
	/* FOC configuration structure */
	struct bmi160_foc_conf foc_conf;
	/* Structure to store the offsets */
	struct bmi160_offsets offsets;
	
	/* Enable offset update for accel */
	foc_conf.acc_off_en = BMI160_ENABLE;

	/* Enable offset update for gyro */
	foc_conf.gyro_off_en = BMI160_ENABLE;
	
	/* offset values set by user */
	offsets.off_acc_x = 0x10;
	offsets.off_acc_y = 0x10;
	offsets.off_acc_z = 0x10;
	offsets.off_gyro_x = 0x10;
	offsets.off_gyro_y = 0x10;
	offsets.off_gyro_z = 0x10;

	rslt = bmi160_set_offsets(&foc_conf, &offsets, sen);
	
	/* After offset setting the data read from the 
	 * sensor will have the corresponding offset */
	
	return rslt;
}
```

#### 将偏移量更新到NVM的示例

> 通过此方法设置的偏移量将存在于NVM中，并将在POR /软复位时恢复

```
/* An example for updating manual offsets to sensor */
int8_t write_offsets_nvm(struct bmi160_dev *dev)
{
	int8_t rslt = 0;
	/* FOC configuration structure */
	struct bmi160_foc_conf foc_conf;
	/* Structure to store the offsets */
	struct bmi160_offsets offsets;
	
	/* Enable offset update for accel */
	foc_conf.acc_off_en = BMI160_ENABLE;

	/* Enable offset update for gyro */
	foc_conf.gyro_off_en = BMI160_ENABLE;
	
	/* offset values set by user as per their reference 
	 * Resolution of accel = 3.9mg/LSB 
	 * Resolution of gyro  = (0.061degrees/second)/LSB */
	offsets.off_acc_x = 10;
	offsets.off_acc_y = -15;
	offsets.off_acc_z = 20;
	offsets.off_gyro_x = 30;
	offsets.off_gyro_y = -35;
	offsets.off_gyro_z = -40;

	rslt = bmi160_set_offsets(&foc_conf, &offsets, sen);
	 
	if (rslt == BMI160_OK) {
		/* Update the NVM */
		rslt = bmi160_update_nvm(dev);
	}
	
	/* After this procedure the offsets are written to 
	 * NVM and restored on POR/soft-reset 
	 * The set values can be removed to ideal case by 
	 * invoking the following APIs
	 *     - bmi160_start_foc()	 
	 *     - bmi160_update_nvm()
	 */

	return rslt;
}
```

## 版权所有（C）2016-2017 Bosch Sensortec GmbH