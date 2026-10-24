#include "main.h"
#include "adc.h"
#include "gpio.h"

/*
 * PA0 上的光敏分压电路在环境变暗时输出升高。
 * 开启与关闭阈值特意分开，避免临界亮度下 LED 频繁闪烁。
 */
#define ADC_DARK_ON_THRESHOLD      1920U
#define ADC_LIGHT_OFF_THRESHOLD    1800U
#define ADC_AVERAGE_SAMPLE_COUNT     16U
#define ADC_POLL_TIMEOUT_MS          10U
#define CONTROL_PERIOD_MS            20U

static uint32_t ADC_ReadAverage(void);
static void LightControl_Update(uint32_t adc_value);

void SystemClock_Config(void);

int main(void)
{
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_ADC1_Init();

  /* STM32F1 ADC 在首次使用前需要校准。 */
  if (HAL_ADCEx_Calibration_Start(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  if (HAL_ADC_Start(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  while (1)
  {
    LightControl_Update(ADC_ReadAverage());
    HAL_Delay(CONTROL_PERIOD_MS);
  }
}

/**
 * @brief 读取多次 ADC 转换并取平均值，抑制采样噪声。
 */
static uint32_t ADC_ReadAverage(void)
{
  uint32_t sum = 0U;
  uint32_t i;

  for (i = 0U; i < ADC_AVERAGE_SAMPLE_COUNT; ++i)
  {
    if (HAL_ADC_PollForConversion(&hadc1, ADC_POLL_TIMEOUT_MS) != HAL_OK)
    {
      Error_Handler();
    }

    sum += HAL_ADC_GetValue(&hadc1);
  }

  return sum / ADC_AVERAGE_SAMPLE_COUNT;
}

/**
 * @brief 根据光线强弱控制 LED，并保留上一次状态以实现迟滞。
 */
static void LightControl_Update(uint32_t adc_value)
{
  static GPIO_PinState led_state = LED_OFF_STATE;

  if (adc_value >= ADC_DARK_ON_THRESHOLD)
  {
    led_state = LED_ON_STATE;
  }
  else if (adc_value <= ADC_LIGHT_OFF_THRESHOLD)
  {
    led_state = LED_OFF_STATE;
  }
  else
  {
    /* 迟滞区间内保持原状态。 */
  }

  HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, led_state);
}

void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                                RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV2;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

void Error_Handler(void)
{
  __disable_irq();
  while (1)
  {
  }
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
  (void)file;
  (void)line;
}
#endif /* USE_FULL_ASSERT */
