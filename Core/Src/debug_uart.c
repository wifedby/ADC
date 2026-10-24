#include "debug_uart.h"

#define DEBUG_UART_BAUD_RATE  115200U

static void Debug_UART_WriteChar(char character);

void Debug_UART_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  uint32_t peripheral_clock;

  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_USART2_CLK_ENABLE();

  GPIO_InitStruct.Pin = GPIO_PIN_2;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  peripheral_clock = HAL_RCC_GetPCLK1Freq();

  USART2->CR1 = 0U;
  USART2->CR2 = 0U;
  USART2->CR3 = 0U;
  USART2->BRR = (peripheral_clock + (DEBUG_UART_BAUD_RATE / 2U)) /
                DEBUG_UART_BAUD_RATE;
  USART2->CR1 = USART_CR1_TE | USART_CR1_UE;
}

void Debug_UART_WriteString(const char *text)
{
  if (text == NULL)
  {
    return;
  }

  while (*text != '\0')
  {
    Debug_UART_WriteChar(*text);
    ++text;
  }
}

void Debug_UART_WriteUInt(uint32_t value)
{
  char digits[10];
  uint32_t digit_count = 0U;

  do
  {
    digits[digit_count] = (char)('0' + (value % 10U));
    value /= 10U;
    ++digit_count;
  } while (value != 0U);

  while (digit_count > 0U)
  {
    --digit_count;
    Debug_UART_WriteChar(digits[digit_count]);
  }
}

static void Debug_UART_WriteChar(char character)
{
  while ((USART2->SR & USART_SR_TXE) == 0U)
  {
  }

  USART2->DR = (uint16_t)(uint8_t)character;
}
