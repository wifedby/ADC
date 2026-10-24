#include "main.h"
#include "adc.h"
#include "debug_uart.h"
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
#define ADC_REPORT_PERIOD_MS       1000U
#define ADC_REFERENCE_MV           3300U

static HAL_StatusTypeDef ADC_ReadAverage(uint32_t *adc_value);
static void ADC_ReportValue(uint32_t adc_value);
static void LightControl_Update(uint32_t adc_value);

void SystemClock_Config(void);

int main(void)
{
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  Debug_UART_Init();
  MX_ADC1_Init();

  Debug_UART_WriteString("\r\n[SYS] Debug UART ready: USART2 TX=PA2, 115200 8N1\r\n");

  /* STM32F1 ADC 在首次使用前需要校准。 */
  if (HAL_ADCEx_Calibration_Start(&hadc1) != HAL_OK)
  {
    Debug_UART_WriteString("[ADC] calibration FAILED\r\n");
    Error_Handler();
  }
  Debug_UART_WriteString("[ADC] calibration OK\r\n");

  if (HAL_ADC_Start(&hadc1) != HAL_OK)
  {
    Debug_UART_WriteString("[ADC] start FAILED\r\n");
    Error_Handler();
  }
  Debug_UART_WriteString("[ADC] conversion started\r\n");

  while (1)
  {
    static uint32_t last_report_tick = 0U;
    static uint8_t first_report = 1U;
    uint32_t adc_value;

    if (ADC_ReadAverage(&adc_value) != HAL_OK)
    {
      Debug_UART_WriteString("[ADC] read FAILED: conversion timeout\r\n");
      Error_Handler();
    }

    LightControl_Update(adc_value);

    if ((first_report != 0U) ||
        ((HAL_GetTick() - last_report_tick) >= ADC_REPORT_PERIOD_MS))
    {
      ADC_ReportValue(adc_value);
      last_report_tick = HAL_GetTick();
      first_report = 0U;
    }

    HAL_Delay(CONTROL_PERIOD_MS);
  }
}

/**
 * @brief 读取多次 ADC 转换并取平均值，抑制采样噪声。
 */
static HAL_StatusTypeDef ADC_ReadAverage(uint32_t *adc_value)
{
  uint32_t sum = 0U;
  uint32_t i;

  if (adc_value == NULL)
  {
    return HAL_ERROR;
  }

  for (i = 0U; i < ADC_AVERAGE_SAMPLE_COUNT; ++i)
  {
    if (HAL_ADC_PollForConversion(&hadc1, ADC_POLL_TIMEOUT_MS) != HAL_OK)
    {
      return HAL_TIMEOUT;
    }

    sum += HAL_ADC_GetValue(&hadc1);
  }

  *adc_value = sum / ADC_AVERAGE_SAMPLE_COUNT;
  return HAL_OK;
}

/**
 * @brief 每秒输出一次 ADC 读取结果。
 */
static void ADC_ReportValue(uint32_t adc_value)
{
  uint32_t voltage_mv;

  voltage_mv = ((adc_value * ADC_REFERENCE_MV) + 2047U) / 4095U;

  Debug_UART_WriteString("[ADC] read OK: raw=");
  Debug_UART_WriteUInt(adc_value);
  Debug_UART_WriteString(", voltage=");
  Debug_UART_WriteUInt(voltage_mv);
  Debug_UART_WriteString(" mV\r\n");
}

/**
 * @brief 根据光线强弱控制 LED，并保留上一次状态以实现迟滞。
 */
static void LightControl_Update(uint32_t adc_value)
{
  static GPIO_PinState led_state = LED_OFF_STATE;
  static uint8_t state_initialized = 0U;
  GPIO_PinState new_state = led_state;

  if (adc_value >= ADC_DARK_ON_THRESHOLD)
  {
    new_state = LED_ON_STATE;
  }
  else if (adc_value <= ADC_LIGHT_OFF_THRESHOLD)
  {
    new_state = LED_OFF_STATE;
  }
  else
  {
    /* 迟滞区间内保持原状态。 */
  }

  if ((state_initialized == 0U) || (new_state != led_state))
  {
    led_state = new_state;
    HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, led_state);

    if (led_state == LED_ON_STATE)
    {
      Debug_UART_WriteString("[LIGHT] dark detected -> LED ON\r\n");
    }
    else
    {
      Debug_UART_WriteString("[LIGHT] bright detected -> LED OFF\r\n");
    }

    state_initialized = 1U;
  }
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
