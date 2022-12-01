#include "sensor_bosch_bmx160.h"
#include "libraries/bmi160.h"
#include "libraries/bmm150.h"

#define DBG_ENABLE
#define DBG_LEVEL DBG_LOG
#define DBG_SECTION_NAME  "sensor.bosch.bmi160"
#define DBG_COLOR
#include <rtdbg.h>

#define FIFO_DATA_LEN   8

#if defined(BMI160_USING_ACCE) || defined(BMI160_USING_GYRO) || defined(BMX160_USING_MAG)

struct odr_node
{
    rt_uint16_t odr;
    rt_uint16_t mask;
};

static rt_uint16_t range_find_mask(const struct odr_node *tab, rt_uint16_t odr)
{
    const struct odr_node *node = tab;

    for (; node->odr && node->odr <= odr; node++);
    return node->odr && node != tab ? node[-1].mask : node->mask;
}

static void _delay_ms(uint32_t period)
{
    rt_thread_mdelay(period);
}

static int8_t rt_i2c_write_reg(void *bus, uint8_t addr, uint8_t reg, uint8_t *data, uint16_t len)
{
    rt_uint8_t tmp = reg;
    struct rt_i2c_msg msgs[2];

    msgs[0].addr  = addr;             /* Slave address */
    msgs[0].flags = RT_I2C_WR;        /* Write flag */
    msgs[0].buf   = &tmp;             /* Slave register address */
    msgs[0].len   = 1;                /* Number of bytes sent */

    msgs[1].addr  = addr;             /* Slave address */
    msgs[1].flags = RT_I2C_WR | RT_I2C_NO_START;        /* Read flag */
    msgs[1].buf   = data;             /* Read data pointer */
    msgs[1].len   = len;              /* Number of bytes read */

    if (rt_i2c_transfer(bus, msgs, 2) != 2)
    {
        return -RT_ERROR;
    }

    return RT_EOK;
}

static int8_t _i2c_read_reg(void *bus, uint8_t addr, uint8_t reg, uint8_t *data, uint16_t len)
{
    rt_uint8_t tmp = reg;
    struct rt_i2c_msg msgs[2];

    msgs[0].addr  = addr;             /* Slave address */
    msgs[0].flags = RT_I2C_WR;        /* Write flag */
    msgs[0].buf   = &tmp;             /* Slave register address */
    msgs[0].len   = 1;                /* Number of bytes sent */

    msgs[1].addr  = addr;             /* Slave address */
    msgs[1].flags = RT_I2C_RD;        /* Read flag */
    msgs[1].buf   = data;             /* Read data pointer */
    msgs[1].len   = len;              /* Number of bytes read */

    if (rt_i2c_transfer(bus, msgs, 2) != 2)
    {
        return -RT_ERROR;
    }

    return RT_EOK;
}

/*wrapper function to match the signature of bmm150.read */
static int8_t _aux_read(void *dev, uint8_t id, uint8_t reg_addr, uint8_t *aux_data, uint16_t len)
{
	int8_t rslt;
    struct bmm150_dev *bmm150 = dev;

	/* Discarding the parameter id as it is redundant*/
    rslt = bmi160_aux_read(reg_addr, aux_data, len, bmm150->parent);

	return rslt;
}

/*wrapper function to match the signature of bmm150.write */
static int8_t _aux_write(void *dev, uint8_t id, uint8_t reg_addr, uint8_t *aux_data, uint16_t len)
{
	int8_t rslt;
    struct bmm150_dev *bmm150 = dev;
	
	/* Discarding the parameter id as it is redundant */
	rslt = bmi160_aux_write(reg_addr, aux_data, len, bmm150->parent);

	return rslt;
}

static void _fifo_init(struct bmi160_dev *dev)
{
#define fifo_length FIFO_DATA_LEN * 7
	/* Modify the FIFO buffer instance and link to the device instance */
	struct bmi160_fifo_frame *fifo = rt_malloc(
            sizeof(struct bmi160_fifo_frame) + fifo_length * sizeof(uint8_t));

	fifo->data = (uint8_t *)(fifo + 1);
	fifo->length = fifo_length;
	dev->fifo = fifo;
#undef fifo_length
}

static void bmm150_create(struct bmi160_dev *bmi160)
{
    struct bmm150_dev *bmm150 = rt_calloc(1, sizeof(struct bmm150_dev));

    bmm150->parent = bmi160;
    bmi160->aux_dev = bmm150;

    /* Configure device structure for auxiliary sensor parameter */
    bmi160->aux_cfg.aux_sensor_enable = 1; // auxiliary sensor enable
    bmi160->aux_cfg.aux_i2c_addr = BMI160_AUX_BMM150_I2C_ADDR; // auxiliary sensor address
    bmi160->aux_cfg.manual_enable = 1; // setup mode enable
    bmi160->aux_cfg.aux_rd_burst_len = 2;// burst read of 2 byte

    /* Configure the BMM150 device structure by 
    mapping _aux_read and _aux_write */
    bmm150->read = _aux_read;
    bmm150->write = _aux_write;
    bmm150->dev_id = BMM150_DEFAULT_I2C_ADDRESS; 
    /* Ensure that sensor.aux_cfg.aux_i2c_addr = bmm150.id
       for proper sensor operation */
    bmm150->delay_ms = _delay_ms;
    bmm150->intf = BMM150_I2C_INTF;

    /* Initialize the auxiliary sensor interface */
    bmi160_aux_init(bmi160);

    /* Initialising the bmm150 sensor */
    bmm150_init(bmm150);

    /* Set the power mode and preset mode to enable Mag data sampling */
    bmm150->settings.pwr_mode = BMM150_NORMAL_MODE;
    bmm150_set_op_mode(bmm150);

    bmm150->settings.preset_mode= BMM150_PRESETMODE_LOWPOWER;
    bmm150_set_presetmode(bmm150);
}

static struct bmi160_dev *_bmi160_create(struct rt_sensor_intf *intf)
{
    struct bmi160_dev *hdev = RT_NULL;
    struct rt_i2c_bus_device *i2c_bus_dev = RT_NULL;
    int8_t rslt = BMI160_OK;

    i2c_bus_dev = (struct rt_i2c_bus_device *)rt_device_find(intf->dev_name);
    if (i2c_bus_dev == RT_NULL)
    {
        LOG_E("can not find device %s", intf->dev_name);
        return RT_NULL;
    }

    hdev = rt_calloc(1, sizeof(struct bmi160_dev));
    if (hdev == RT_NULL)
    {
        LOG_E("bmi160 dev memory allocation failed");
        return RT_NULL;
    }

    hdev->id = (uint32_t)intf->user_data; /* BMI160 I2C device address */
    hdev->interface = BMI160_I2C_INTF;
    hdev->bus = i2c_bus_dev;
    hdev->read = _i2c_read_reg;
    hdev->write = rt_i2c_write_reg;
    hdev->delay_ms = _delay_ms;

    rslt = bmi160_init(hdev);
    if (rslt == BMI160_OK)
    {
        rslt = bmi160_soft_reset(hdev);

        /* Select the Output data rate, range of accelerometer sensor */
        hdev->accel_cfg.odr = BMI160_ACCEL_ODR_100HZ;
        hdev->accel_cfg.range = BMI160_ACCEL_RANGE_2G;
        hdev->accel_cfg.bw = BMI160_ACCEL_BW_NORMAL_AVG4;

        /* Select the power mode of accelerometer sensor */
        hdev->accel_cfg.power = BMI160_ACCEL_NORMAL_MODE;

        /* Select the Output data rate, range of Gyroscope sensor */
        hdev->gyro_cfg.odr = BMI160_GYRO_ODR_100HZ;
        hdev->gyro_cfg.range = BMI160_GYRO_RANGE_2000_DPS;
        hdev->gyro_cfg.bw = BMI160_GYRO_BW_NORMAL_MODE;

        /* Select the power mode of Gyroscope sensor */
        hdev->gyro_cfg.power = BMI160_GYRO_NORMAL_MODE;

        /* Set the sensor configuration */
        rslt = bmi160_set_sens_conf(hdev);

        /* Select the power mode */
        hdev->accel_cfg.power = BMI160_ACCEL_SUSPEND_MODE;
        hdev->gyro_cfg.power = BMI160_GYRO_SUSPEND_MODE;

        bmi160_set_power_mode(hdev);

        _fifo_init(hdev);

        if (hdev->chip_id == BMX160_CHIP_ID) /* BMX160 */
        {
            bmm150_create(hdev);
        }

        return hdev;
    }
    else
    {
        LOG_E("bmi160 init failed");
        rt_free(hdev);
        return RT_NULL;
    }
}

static rt_err_t _set_odr(rt_sensor_t sensor, rt_uint16_t odr)
{
    struct bmi160_dev *hdev = sensor->parent.user_data;

    if (sensor->info.type == RT_SENSOR_CLASS_ACCE)
    {
        const static struct odr_node tab[] =
        {
            {   25, BMI160_ACCEL_ODR_25HZ   },
            {   50, BMI160_ACCEL_ODR_50HZ   },
            {  100, BMI160_ACCEL_ODR_100HZ  },
            {  200, BMI160_ACCEL_ODR_200HZ  },
            {  400, BMI160_ACCEL_ODR_400HZ  },
            {  800, BMI160_ACCEL_ODR_800HZ  },
            {    0, BMI160_ACCEL_ODR_1600HZ }
        };

        hdev->accel_cfg.odr = range_find_mask(tab, odr);
        /* Set the desired configurations to the sensor */
        bmi160_set_sens_conf(hdev);
        return RT_EOK;
    }

    if (sensor->info.type == RT_SENSOR_CLASS_GYRO)
    {
        const static struct odr_node tab[] =
        {
            {   25, BMI160_GYRO_ODR_25HZ   },
            {   50, BMI160_GYRO_ODR_50HZ   },
            {  100, BMI160_GYRO_ODR_100HZ  },
            {  200, BMI160_GYRO_ODR_200HZ  },
            {  400, BMI160_GYRO_ODR_400HZ  },
            {  800, BMI160_GYRO_ODR_800HZ  },
            { 1600, BMI160_GYRO_ODR_1600HZ },
            {    0, BMI160_GYRO_ODR_3200HZ }
        };

        hdev->gyro_cfg.odr = range_find_mask(tab, odr);
        /* Set the desired configurations to the sensor */
        bmi160_set_sens_conf(hdev);
        return RT_EOK;
    }

    if (sensor->info.type == RT_SENSOR_CLASS_MAG)
    {
        struct bmm150_dev *bmm150 = hdev->aux_dev;
        const static struct odr_node tab[] =
        {
            {    2, BMM150_DATA_RATE_02HZ  },
            {    6, BMM150_DATA_RATE_06HZ  },
            {    8, BMM150_DATA_RATE_08HZ  },
            {   10, BMM150_DATA_RATE_10HZ  },
            {   15, BMM150_DATA_RATE_15HZ  },
            {   20, BMM150_DATA_RATE_20HZ  },
            {   25, BMM150_DATA_RATE_25HZ  },
            {    0, BMM150_DATA_RATE_30HZ },
        };

        bmm150->settings.data_rate = range_find_mask(tab, odr);
        bmm150_set_sensor_settings(BMM150_DATA_RATE_SEL, bmm150);
    }

    return RT_EOK;
}

static rt_err_t _set_range(rt_sensor_t sensor, rt_uint16_t range)
{
    struct bmi160_dev *hdev = sensor->parent.user_data;

    if (sensor->info.type == RT_SENSOR_CLASS_ACCE)
    {
        const static struct odr_node tab[] =
        {
            { 2000, BMI160_ACCEL_RANGE_2G  },
            { 4000, BMI160_ACCEL_RANGE_4G  },
            { 8000, BMI160_ACCEL_RANGE_8G  },
            {    0, BMI160_ACCEL_RANGE_16G }
        };
    
        hdev->accel_cfg.range = range_find_mask(tab, range);
        /* Set the sensor configuration */
        bmi160_set_sens_conf(hdev);
        
        return RT_EOK;
    }
    else if (sensor->info.type == RT_SENSOR_CLASS_GYRO)
    {
        const static struct odr_node tab[] =
        {
            {  125, BMI160_GYRO_RANGE_125_DPS  },
            {  250, BMI160_GYRO_RANGE_250_DPS  },
            {  500, BMI160_GYRO_RANGE_500_DPS  },
            { 1000, BMI160_GYRO_RANGE_1000_DPS },
            {    0, BMI160_GYRO_RANGE_2000_DPS }
        };

        hdev->accel_cfg.range = range_find_mask(tab, range);
        /* Set the sensor configuration */
        bmi160_set_sens_conf(hdev);

        return RT_EOK;
    }

    return RT_ERROR;
}

static rt_err_t _set_power(rt_sensor_t sensor, rt_uint8_t power)
{
    int idx;
    struct bmi160_dev *hdev = sensor->parent.user_data;
    struct bmm150_dev *bmm150 = hdev->aux_dev;
    uint8_t * const pwr_reg[3] =
    {
        &hdev->accel_cfg.power,
        &hdev->gyro_cfg.power,
        &bmm150->settings.pwr_mode
    };
    const uint8_t pwr_cfg[3][3] =
    {
        { BMI160_ACCEL_SUSPEND_MODE, BMI160_GYRO_SUSPEND_MODE, BMM150_SUSPEND_MODE },
        { BMI160_ACCEL_NORMAL_MODE, BMI160_GYRO_NORMAL_MODE, BMM150_NORMAL_MODE },
        { BMI160_ACCEL_LOWPOWER_MODE, BMI160_GYRO_NORMAL_MODE, BMM150_FORCED_MODE },
    };

    switch (sensor->info.type)
    {
    case RT_SENSOR_CLASS_ACCE: idx = 0; break;
    case RT_SENSOR_CLASS_GYRO: idx = 1; break;
    case RT_SENSOR_CLASS_MAG: idx = 2; break;
    default:
        LOG_W("Unsupported sensor type, type is %d", sensor->info.type);
        return -RT_ERROR;
    }

    switch (power)
    {
    case RT_SENSOR_POWER_DOWN: /* power down */
        *pwr_reg[idx] = pwr_cfg[0][idx];
        break;
    case RT_SENSOR_POWER_NORMAL: /* power normal */
        *pwr_reg[idx] = pwr_cfg[1][idx];
        break;
    case RT_SENSOR_POWER_LOW: /* power low */
        *pwr_reg[idx] = pwr_cfg[2][idx];
        break;
    default:
        LOG_W("Unsupported mode, code is %d", power);
        return -RT_ERROR;
    }

    bmi160_set_power_mode(hdev);
    bmm150_set_op_mode(bmm150);
    return  RT_EOK;
}

static void _get_mag_config_fifo(struct bmi160_dev *hdev)
{
    /* Enter the data register of BMM150 to "auto_mode_addr" here it is 0x42 */
	uint8_t auto_mode_addr = 0x42;
	hdev->aux_cfg.aux_odr = BMI160_AUX_ODR_25HZ;
	bmi160_set_aux_auto_mode(&auto_mode_addr, hdev);

    /* Disable other FIFO settings */
	bmi160_set_fifo_config(BMI160_FIFO_CONFIG_1_MASK , BMI160_DISABLE, hdev);

	/* Enable the required FIFO settings */
	bmi160_set_fifo_config(BMI160_FIFO_AUX | BMI160_FIFO_HEADER, BMI160_ENABLE, hdev);
}

static rt_err_t _select_mode(struct rt_sensor_device *sensor, rt_uint32_t mode)
{
    int use_int = 0;
    struct bmi160_int_settg int_config;
    struct bmi160_dev *hdev = sensor->parent.user_data;

    if (mode == RT_SENSOR_MODE_INT)
    {
        use_int = 1;
        /* Select the Interrupt type */
        int_config.int_type = BMI160_ACC_GYRO_DATA_RDY_INT; // Choosing Gyro and Acce Data Ready interrupt
    }
    else if (mode == RT_SENSOR_MODE_FIFO)
    {
        use_int = 1;
        /* Select the Interrupt channel/pin */
        int_config.int_channel = BMI160_INT_CHANNEL_1; // Interrupt channel/pin 1
        /* Select the Interrupt type */
        int_config.int_type = BMI160_ACC_GYRO_FIFO_FULL_INT; // Choosing Gyro and Acce FIFO Full interrupt

        if (sensor->info.type == RT_SENSOR_CLASS_MAG)
        {
            _get_mag_config_fifo(hdev);
        }
        else
        {
            bmi160_set_fifo_config(BMI160_FIFO_GYRO | BMI160_FIFO_ACCEL
                | BMI160_FIFO_HEADER | BMI160_FIFO_TIME, BMI160_ENABLE, hdev);
        }
    }
    if (use_int)
    {
        /* Select the Interrupt channel/pin */
        int_config.int_channel = BMI160_INT_CHANNEL_1; // Interrupt channel/pin 1

        /* Select the interrupt channel/pin settings */
        int_config.int_pin_settg.output_en = BMI160_ENABLE; // Enabling interrupt pins to act as output pin
        int_config.int_pin_settg.output_mode = BMI160_DISABLE; // Choosing push-pull mode for interrupt pin
        int_config.int_pin_settg.output_type = BMI160_ENABLE; // Choosing active high output
        int_config.int_pin_settg.edge_ctrl = BMI160_ENABLE; // Choosing edge triggered output
        int_config.int_pin_settg.input_en = BMI160_DISABLE; // Disabling interrupt pin to act as input
        int_config.int_pin_settg.latch_dur = BMI160_LATCH_DUR_NONE; // non-latched output

        /* Set the interrupt mode */
        bmi160_set_int_config(&int_config, hdev); /* sensor is an instance of the structure bmi160_dev */
    }
    return RT_EOK;
}

static void _copy_data(struct rt_sensor_data *dst, struct bmi160_sensor_data *src)
{
    dst->data.acce.x = src->x;
    dst->data.acce.y = src->y;
    dst->data.acce.z = src->z;
    dst->timestamp = rt_sensor_get_ts();
}

static void _get_mag_data(struct bmi160_dev *hdev, struct rt_sensor_data *data)
{
    struct bmm150_dev *bmm150 = hdev->aux_dev;

    bmm150_read_mag_data(bmm150);
    data->data.acce.x = bmm150->data.x;
    data->data.acce.y = bmm150->data.y;
    data->data.acce.z = bmm150->data.z;
    data->timestamp = rt_sensor_get_ts();
}

static int _get_data(struct rt_sensor_device *sensor, void *buf)
{
    struct bmi160_dev *hdev = sensor->parent.user_data;
    struct rt_sensor_data *data = buf;
    struct bmi160_sensor_data acce_data, gyro_data;

    if (sensor->info.type == RT_SENSOR_CLASS_MAG)
    {
        if (hdev->chip_id == BMX160_CHIP_ID) /* BMX160 */
        {
            _get_mag_data(hdev, data);
            return 1;
        }
        return 0;
    }
    bmi160_get_sensor_data(BMI160_ACCEL_SEL | BMI160_GYRO_SEL, &acce_data, &gyro_data, hdev);
    if (sensor->info.type == RT_SENSOR_CLASS_ACCE)
    {
        data->type = RT_SENSOR_CLASS_ACCE;
        _copy_data(data, &acce_data);
    }
    else if (sensor->info.type == RT_SENSOR_CLASS_GYRO)
    {
        data->type = RT_SENSOR_CLASS_GYRO;
        _copy_data(data, &gyro_data);
    }
    return 1;
}

static void _extract_mag(struct bmi160_dev *hdev,
    struct bmi160_aux_data *aux_data, struct bmi160_sensor_data *data, uint8_t len)
{
    int i;
    struct bmm150_dev *bmm150 = hdev->aux_dev;

    for (i = 0; i < len; i++)
    {
		/* Compensated mag data using BMM150 API */
		int sre = bmm150_aux_mag_data(aux_data[i].data, bmm150);

		/* Copy the  Compensated mag data */
		if (sre == BMM150_OK) {
            data[i].x = bmm150->data.x;
            data[i].y = bmm150->data.y;
            data[i].z = bmm150->data.z;
		}
	}
}

static int _get_fifo_data(struct rt_sensor_device *sensor, void *buf, rt_size_t len)
{
	int i;
    uint8_t frame_len = FIFO_DATA_LEN;
    struct rt_sensor_data *data = buf;
    struct bmi160_sensor_data buffer[FIFO_DATA_LEN]; // 32 data frames
    struct bmi160_dev *hdev = sensor->parent.user_data;
    rt_uint8_t data_type;

    if (bmi160_get_fifo_data(hdev) != BMI160_OK)
        return 0;

    if (sensor->info.type == RT_SENSOR_CLASS_ACCE)
    {
        data_type = RT_SENSOR_CLASS_ACCE;
        bmi160_extract_accel(buffer, &frame_len, hdev);
    }
    else if(sensor->info.type == RT_SENSOR_CLASS_GYRO)
    {
        data_type = RT_SENSOR_CLASS_GYRO;
        bmi160_extract_gyro(buffer, &frame_len, hdev);
    }
    else if (sensor->info.type == RT_SENSOR_CLASS_MAG)
    {
        struct bmi160_aux_data aux_data[FIFO_DATA_LEN];
        data_type = RT_SENSOR_CLASS_MAG;
        bmi160_extract_aux(aux_data, &frame_len, hdev);
        _extract_mag(hdev, aux_data, buffer, frame_len);
    }
    len = len < frame_len ? len : frame_len;
    for (i = 0; i < len; i++)
    {
        data[i].type = data_type;
        _copy_data(data + i, buffer + i);
    }
    return len;
}

static rt_size_t _fetch_data(struct rt_sensor_device *sensor, void *buf, rt_size_t len)
{
    if (sensor->config.mode == RT_SENSOR_MODE_FIFO)
        return _get_fifo_data(sensor, buf, len);
    return _get_data(sensor, buf);
}

static rt_err_t _control(struct rt_sensor_device *sensor, int cmd, void *args)
{
    struct bmi160_dev *hdev = sensor->parent.user_data;
    rt_err_t result = RT_EOK;

    switch (cmd)
    {
    case RT_SENSOR_CTRL_GET_ID:
        *(rt_uint8_t *)args = hdev->chip_id;
        break;
    case RT_SENSOR_CTRL_SET_ODR:
        result = _set_odr(sensor, (rt_uint32_t)args & 0xffff);
        break;
    case RT_SENSOR_CTRL_SET_RANGE:
        result = _set_range(sensor, (rt_uint32_t)args);
        break;
    case RT_SENSOR_CTRL_SET_POWER:
        result = _set_power(sensor, (rt_uint32_t)args & 0xff);
        break;
    case RT_SENSOR_CTRL_SET_MODE:
        result = _select_mode(sensor, (rt_uint32_t)args);
        break;
    case RT_SENSOR_CTRL_SELF_TEST:
        /* TODO */
        result = -RT_EINVAL;
        break;
    default:
        return -RT_EINVAL;
    }
    return result;
}

static struct rt_sensor_ops sensor_ops =
{
    _fetch_data,
    _control
};

#endif

#ifdef BMI160_USING_ACCE
static void * _acce_init(const char *name, struct rt_sensor_config *cfg, void *hdev)
{
    rt_int8_t result;
    rt_sensor_t sensor_acce = RT_NULL;

    /* accelerometer sensor register */
    sensor_acce = rt_calloc(1, sizeof(struct rt_sensor_device));
    if (sensor_acce == RT_NULL)
        return RT_NULL;

    sensor_acce->info.type       = RT_SENSOR_CLASS_ACCE;
    sensor_acce->info.vendor     = RT_SENSOR_VENDOR_BOSCH;
    sensor_acce->info.model      = "bmi160_acce";
    sensor_acce->info.unit       = RT_SENSOR_UNIT_PA;
    sensor_acce->info.intf_type  = RT_SENSOR_INTF_I2C;
    sensor_acce->info.range_max  = 16000;
    sensor_acce->info.range_min  = 2000;
    sensor_acce->info.period_min = 100;
    sensor_acce->info.fifo_max   = FIFO_DATA_LEN;

    rt_memcpy(&sensor_acce->config, cfg, sizeof(struct rt_sensor_config));
    sensor_acce->ops = &sensor_ops;

    result = rt_hw_sensor_register(sensor_acce, name, RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX | RT_DEVICE_FLAG_FIFO_RX, hdev);
    if (result != RT_EOK)
    {
        LOG_E("device register err code: %d", result);
        rt_free(sensor_acce);
        return RT_NULL;
    }
    return sensor_acce;
}
#endif

#ifdef BMI160_USING_GYRO
static void * _gyro_init(const char *name, struct rt_sensor_config *cfg, void *hdev)
{
    rt_int8_t result;
    rt_sensor_t sensor_gyro = RT_NULL;

    /* gyroscope sensor register */
    sensor_gyro = rt_calloc(1, sizeof(struct rt_sensor_device));
    if (sensor_gyro == RT_NULL)
        return RT_NULL;

    sensor_gyro->info.type       = RT_SENSOR_CLASS_GYRO;
    sensor_gyro->info.vendor     = RT_SENSOR_VENDOR_BOSCH;
    sensor_gyro->info.model      = "bmi160_gyro";
    sensor_gyro->info.unit       = RT_SENSOR_UNIT_MDPS;
    sensor_gyro->info.intf_type  = RT_SENSOR_INTF_I2C;
    sensor_gyro->info.range_max  = 2000;
    sensor_gyro->info.range_min  = 125;
    sensor_gyro->info.period_min = 100;
    sensor_gyro->info.fifo_max   = FIFO_DATA_LEN;

    rt_memcpy(&sensor_gyro->config, cfg, sizeof(struct rt_sensor_config));
    sensor_gyro->ops = &sensor_ops;

    result = rt_hw_sensor_register(sensor_gyro, name, RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX | RT_DEVICE_FLAG_FIFO_RX, hdev);
    if (result != RT_EOK)
    {
        LOG_E("device register err code: %d", result);
        rt_free(sensor_gyro);
        return RT_NULL;
    }
    return sensor_gyro;
}
#endif

#ifdef BMX160_USING_MAG
static void * _mag_init(const char *name, struct rt_sensor_config *cfg, void *hdev)
{
    rt_int8_t result;
    rt_sensor_t sensor_mag = RT_NULL;

    sensor_mag = rt_calloc(1, sizeof(struct rt_sensor_device));
    if (sensor_mag == RT_NULL)
        return RT_NULL;

    sensor_mag->info.type       = RT_SENSOR_CLASS_MAG;
    sensor_mag->info.vendor     = RT_SENSOR_VENDOR_BOSCH;
    sensor_mag->info.model      = "bmi160_mag";
    sensor_mag->info.unit       = RT_SENSOR_UNIT_MGAUSS;
    sensor_mag->info.intf_type  = RT_SENSOR_INTF_I2C;
    sensor_mag->info.range_max  = 2000;
    sensor_mag->info.range_min  = 125;
    sensor_mag->info.period_min = 100;
    sensor_mag->info.fifo_max   = FIFO_DATA_LEN;

    rt_memcpy(&sensor_mag->config, cfg, sizeof(struct rt_sensor_config));
    sensor_mag->ops = &sensor_ops;

    result = rt_hw_sensor_register(sensor_mag, name, RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX | RT_DEVICE_FLAG_FIFO_RX, hdev);
    if (result != RT_EOK)
    {
        LOG_E("device register err code: %d", result);
        rt_free(sensor_mag);
        rt_device_unregister(&sensor_mag->parent);
        return RT_NULL;
    }

    return sensor_mag;
}
#endif

int rt_hw_bmx160_init(const char *name, struct rt_sensor_config *cfg)
{
#if defined(BMI160_USING_ACCE) || defined(BMI160_USING_GYRO) || defined(BMX160_USING_MAG)
    struct bmi160_dev *hdev = RT_NULL;

    hdev = _bmi160_create(&cfg->intf);
    if (hdev == RT_NULL)
    {
        LOG_E("sensor create failed");
        return -RT_ERROR;
    }
#endif

#ifdef BMI160_USING_ACCE
    _acce_init(name, cfg, hdev);
#endif
#ifdef BMI160_USING_GYRO
    _gyro_init(name, cfg, hdev);
#endif
#ifdef BMX160_USING_MAG
    if (hdev->chip_id == BMX160_CHIP_ID)
    {
        _mag_init(name, cfg, hdev);
    }
#endif

    return RT_EOK;
}
