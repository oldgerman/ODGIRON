// BSP mapping functions

#include "history.hpp"
#include "BSP.h"
#include "Model_Config.h"
#include "fusbpd.h"
#include "BSP_Power.h"
#include "Setup.h"
#include "main.h"
#include <IRQ.h>
#include "policy_engine.h"
#include "INA.h"

bool fusb302_process = 0;	//标记类型

volatile uint16_t PWMSafetyTimer = 0;
volatile uint8_t  pendingPWM     = 0;

const uint16_t       powerPWM         = 255;
static const uint8_t holdoffTicks     = 14; // delay of 8 ms
static const uint8_t tempMeasureTicks = 14;

uint16_t totalPWM; // htim2.Init.Period, the full PWM cycle

static bool fastPWM;

template<class T, uint8_t SIZE>
struct history {
	static const uint8_t size = SIZE;	//static const 类成员，为所有类的实例共享的数据，必须在类内对其初始化
	T buf[size];
	int32_t sum;	//T类型值的总和
	uint8_t loc;	//元素编号（location）

	void update(T const val) {
		// step backwards so i+1 is the previous value.
		//向后退一步，因此i + 1是前一个值。

		sum -= buf[loc];
		sum += val;
		buf[loc] = val;
		loc = (loc + 1) % size;	//loc0+8次1才会使loc=1？
	}

	//下标运算符 [] 重载
	T operator[](uint8_t i) const {
		// 0 = newest, size-1 = oldest.
		i = (i + loc) % size;
		return buf[i];
	}

	T average() const {
		return sum / size;
	}
};

// 2 second filter (ADC is PID_TIM_HZ Hz)
history<uint16_t, PID_TIM_HZ> rawTempFilter = {{0}, 0, 0};


void preRToSInit() {
  /* Reset of all peripherals, Initializes the Flash interface and the Systick.*/
  BSPInit();
  FRToSI2C1.FRToSInit();
  FRToSI2C2.FRToSInit();
}

// 初始化将在调度程序处于活动状态时执行
// Initialisation to be performed with scheduler active
// 仅在 doPOWTask()中调用
void postRToSInit() {
	osDelay(200);	//此延迟用于等待FUSB302上电初始化，先从PD取到5V电，再由I2C设置为取20V，不加可能会导致直接上电取电失败
#ifdef POW_PD
  if (usb_pd_detect() == true)
	{
	  fusb302_process = 1;
    //Spawn all of the USB-C processors
    fusb302_start_processing();
  }
#endif
}


void resetWatchdog() {
	HAL_IWDG_Refresh(&hiwdg);
}


uint16_t getHandleTemperature() {
  // We return the current handle temperature in X10 C
  // TMP36 in handle, 0.5V offset and then 10mV per deg C (0.75V @ 25C for
  // example) STM32 = 4096 count @ 3.3V input -> But We oversample by 32/(2^2) =（这个版本getADC改为16/2了）
  // 8 times oversampling Therefore 32768 is the 3.3V input, so 0.1007080078125
  // mV per count So we need to subtract an offset of 0.5V to center on 0C
  // (4964.8 counts)
  //
  int32_t result = getADC(0);
#ifdef TEMP_TMP36
  result -= 4965; // remove 0.5V offset
  // 10mV per C
  // 99.29 counts per Deg C above 0C. Tends to read a tad over across all of my sample units
  result *= 100;
  result /= 994;
#elif defined(TEMP_MCP9700AT)
  //usb_printf("result = %ld\r\n" , result);//室温在4000左右
  result = result/4*3300/4096 - 500;

#elif defined(TEMP_STM32)
  result = result*(3300/4096)/8;
  result = (1.43*1000-result)*10/43+25;
  #else
#error no Tref sensor defined
#endif
  return result;
}

#ifdef STM32F1
#define NUM_READINGS 8
#elif defined(STM32F4)
#define NUM_READINGS 4
#endif
uint16_t readingsInject[NUM_READINGS];

uint16_t getTipInstantTemperature() {
  uint16_t sum = 0; // 12 bit readingsInject * 8 -> 15 bits

  // Looking to reject the highest outlier readingsInject.
  // As on some hardware these samples can run into the op-amp recovery time
  // Once this time is up the signal stabilises quickly, so no need to reject minimums
//寻找拒绝最高的异常值读数。
//在某些硬件上，这些示例可能会在运算放大器恢复时间内运行
//一旦到了这个时间，信号就会迅速稳定下来，因此无需拒绝最小值//寻找拒绝最高的异常值读数。
//在某些硬件上，这些示例可能会在运算放大器恢复时间内运行
//一旦这个时间到了，信号就会迅速稳定下来，因此无需拒绝最小值
#ifdef STM32F1
  readingsInject[0] = hadc1.Instance->JDR1;	//inject channel要定时器触发
  readingsInject[1] = hadc1.Instance->JDR2;
  readingsInject[2] = hadc1.Instance->JDR3;	//inject channel要定时器触发
  readingsInject[3] = hadc1.Instance->JDR4;
  readingsInject[4] = hadc2.Instance->JDR1;	//inject channel要定时器触发
  readingsInject[5] = hadc2.Instance->JDR2;
  readingsInject[6] = hadc2.Instance->JDR3;	//inject channel要定时器触发
  readingsInject[7] = hadc2.Instance->JDR4;

  for (int i = 0; i < 8; i++) {
    sum += readingsInject[i];
  }
  //usb_printf("sum = %d\r\n", sum);
  return sum; // 8x over sample
#elif defined(STM32F4)
  readingsInject[0] = hadc1.Instance->JDR1;	//inject channel要定时器触发
  readingsInject[1] = hadc1.Instance->JDR2;
  readingsInject[2] = hadc1.Instance->JDR3;	//inject channel要定时器触发
  readingsInject[3] = hadc1.Instance->JDR4;

  for (int i = 0; i < 4; i++) {
    sum += readingsInject[i];
  }
  return sum*2; // 8x over sample
#endif
}


uint16_t getTipRawTemp(uint8_t refresh) {
  if (refresh) {
    uint16_t lastSample = getTipInstantTemperature();
    rawTempFilter.update(lastSample);
    return lastSample;	//返回8次过采样总和
  } else {
    return rawTempFilter.average();//8次 8次过采样 的平均值，滤波后的8次过采样总和
  }
}

/**
 * @brief	获取输入电压，X10就是函数内preFillneeded的值
 * @param  分压系数， 来自systemSettings.voltageDiv
 * @param  采样次数，若传0，只采样一次
 * @retval 输入电压的10倍
 */
uint16_t getInputVoltageX10() {
  //待使用INA226或FUSB302内置8bitADC实现。。
if(INA_Class::getDeviceCount())
  return busV/100;	//先返回5.0V
//else 使用FUSB302的期望电压
return PolicyEngine::getRequestedVoltage()/100;
}

void setTipPWM(uint8_t pulse) {
  //PWMSafetyTimer = 10; // This is decremented in the handler for PWM so that the tip pwm is
                       // disabled if the PID task is not scheduled often enough.
	PWMSafetyTimer = 20;

  pendingPWM = pulse;
}

static void switchToFastPWM(void) {
  fastPWM             = true;
  totalPWM            = powerPWM + tempMeasureTicks * 2 + holdoffTicks;//297
#if 0
  htim2.Instance->ARR = totalPWM;	//
  // ~3.5 Hz rate
  htim2.Instance->CCR1 = powerPWM + holdoffTicks * 2;	//乘以2？？
  // 2 MHz timer clock/2000 = 1 kHz tick rate
  htim2.Instance->PSC = 2000;
#else
  htim1.Instance->ARR = totalPWM;	//297
  htim1.Instance->CCR4 = powerPWM + holdoffTicks * 2;//283
  htim1.Instance->PSC = TIM1_PSC_FAST_PWM;	//每秒3次
#endif
}

static void switchToSlowPWM(void) {
  fastPWM             = false;
  totalPWM            = powerPWM + tempMeasureTicks + holdoffTicks;
#if 0
  htim2.Instance->ARR = totalPWM;
  // ~1.84 Hz rate
  htim2.Instance->CCR1 = powerPWM + holdoffTicks;
  // 2 MHz timer clock/4000 = 500 Hz tick rate
  htim2.Instance->PSC = 4000;
#else
  htim1.Instance->ARR = totalPWM;	//283
  htim1.Instance->CCR4 = powerPWM + holdoffTicks;//269
  htim1.Instance->PSC = TIM1_PSC_SLOW_PWM;//每秒2次
#endif
}

bool tryBetterPWM(uint8_t pwm) {
  if (fastPWM && pwm == powerPWM) {		//若pwm=255即全功率状态，且fastPWM是true，对应烙铁的加热过程
    // maximum power for fast PWM reached, need to go slower to get more
    switchToSlowPWM();					//启用慢速PWM模式，减少ADC测量次数，花费更多时间燃烧功率
    return true;
  } else if (!fastPWM && pwm < 230) {	//若慢速PWM状态下pwm<230了
    // 254 in fast PWM mode gives the same power as 239 in slow
    // allow for some reasonable hysteresis by switching only when it goes
    // below 230 (equivalent to 245 in fast mode)
	//快速PWM模式下的254与慢速模式下的239具有相同的功率<---这难道也在你的计算之中吗, JOJO!
	//仅在其出现时才进行切换，以提供一些合理的滞后
	//低于230（相当于快速模式下的245）
    switchToFastPWM();					//那就差不多达到预设温度了，切换到快速PMW模式以精准控温
    return true;
  }
  return false;
}


// (__weak重写)
void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim) {
// This was a when the PWM for the output has timed out
//tim1_cc1事件

	if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1) {
		//TIM3:36 ARR = 297 PSC = 35467 CCR1= 0 CCR4=283
		//TIM3:37 ARR = 297 PSC = 35467 CCR1= 0 CCR4=283
		//static uint32_t cnt_CC1 = 0;
		//usb_printf("Pul TIM3:%d ARR= %d PSC= %d CCR1= %d CCR4=%d\r\n", cnt_CC1++, htim->Instance->ARR, htim->Instance->PSC, htim->Instance->CCR1, htim->Instance->CCR4);
		HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_4);
	}
}



void unstick_I2C(I2C_HandleTypeDef * I2C_Handle) {
#if 1
  GPIO_InitTypeDef GPIO_InitStruct;
  int              timeout     = 100;
  int              timeout_cnt = 0;
  uint32_t SCL_Pin;
  uint32_t SDA_Pin;
#ifdef STM32F1
#include "stm32f103xb.h"
#elif defined(STM32F401xC)
#include "stm32f401xc.h"
#else
#endif
  GPIO_TypeDef  * SCL_GPIO_Port;
  GPIO_TypeDef  * SDA_GPIO_Port;

  // 1. Clear PE bit.
  I2C_Handle->Instance->CR1 &= ~(0x0001);
  /**I2C1 GPIO Configuration
   PB6     ------> I2C1_SCL
   PB7     ------> I2C1_SDA
   */
  //  2. Configure the SCL and SDA I/Os as General Purpose Output Open-Drain, High level (Write 1 to GPIOx_ODR).
  GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Pull  = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

  if(I2C_Handle == &hi2c1)
  {
	  SCL_Pin = SCL1_Pin;
	  SDA_Pin = SDA1_Pin;
	  SCL_GPIO_Port = SCL1_GPIO_Port;
	  SDA_GPIO_Port = SDA1_GPIO_Port;
  }
  else
  {
#ifdef STM32F1
	  SCL_Pin = SCL2_Pin;
	  SDA_Pin = SDA2_Pin;
	  SCL_GPIO_Port = SCL2_GPIO_Port;
	  SDA_GPIO_Port = SDA2_GPIO_Port;
#elif defined(STM32F4)
	  SCL_Pin = SCL3_Pin;
	  SDA_Pin = SDA3_Pin;
	  SCL_GPIO_Port = SCL3_GPIO_Port;
	  SDA_GPIO_Port = SDA3_GPIO_Port;
#else
#endif
  }

  GPIO_InitStruct.Pin = SCL_Pin;
  HAL_GPIO_Init(SCL_GPIO_Port, &GPIO_InitStruct);
  HAL_GPIO_WritePin(SCL_GPIO_Port, SCL_Pin, GPIO_PIN_SET);

  GPIO_InitStruct.Pin = SDA_Pin;
  HAL_GPIO_Init(SDA_GPIO_Port, &GPIO_InitStruct);
  HAL_GPIO_WritePin(SDA_GPIO_Port, SDA_Pin, GPIO_PIN_SET);

  while (GPIO_PIN_SET != HAL_GPIO_ReadPin(SDA_GPIO_Port, SDA_Pin)) {
    // Move clock to release I2C
    HAL_GPIO_WritePin(SCL_GPIO_Port, SCL_Pin, GPIO_PIN_RESET);
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    HAL_GPIO_WritePin(SCL_GPIO_Port, SCL_Pin, GPIO_PIN_SET);

    timeout_cnt++;
    if (timeout_cnt > timeout)
      return;
  }

  // 12. Configure the SCL and SDA I/Os as Alternate function Open-Drain.
  GPIO_InitStruct.Mode  = GPIO_MODE_AF_OD;
  GPIO_InitStruct.Pull  = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

  GPIO_InitStruct.Pin = SCL_Pin;
  HAL_GPIO_Init(SCL_GPIO_Port, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = SDA_Pin;
  HAL_GPIO_Init(SDA1_GPIO_Port, &GPIO_InitStruct);

  HAL_GPIO_WritePin(SCL_GPIO_Port, SCL_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(SDA_GPIO_Port, SDA_Pin, GPIO_PIN_SET);

  // 13. Set SWRST bit in I2Cx_CR1 register.
  I2C_Handle->Instance->CR1 |= 0x8000;

  asm("nop");

  // 14. Clear SWRST bit in I2Cx_CR1 register.
  I2C_Handle->Instance->CR1 &= ~0x8000;

  asm("nop");

  // 15. Enable the I2C peripheral by setting the PE bit in I2Cx_CR1 register
  I2C_Handle->Instance->CR1 |= 0x0001;

  // Call initialization function.
  HAL_I2C_Init(I2C_Handle);
#endif
}



void BSPInit(void) { switchToFastPWM(); }

void reboot() { NVIC_SystemReset(); }

void delay_ms(uint16_t count) { HAL_Delay(count); }



