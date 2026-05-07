/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include <string.h>
#include <stdio.h>

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */

uint8_t txData[8];
uint8_t rxData[9];

uint16_t reg1;
uint16_t reg2;

uint32_t combined;

float value = 0.0f;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);

/* USER CODE BEGIN PFP */

uint16_t ModRTU_CRC(uint8_t buf[], int len);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {

	/* USER CODE BEGIN 1 */

	/* USER CODE END 1 */

	/* MCU Configuration--------------------------------------------------------*/

	HAL_Init();

	SystemClock_Config();

	MX_GPIO_Init();
	MX_USART2_UART_Init();

	/* USER CODE BEGIN 2 */

	/*
	 * Modbus RTU Request Frame
	 *
	 * Slave ID      = 1
	 * Function Code = 03
	 * Start Address = 0000
	 * Quantity      = 0002
	 */

	txData[0] = 0x01; // Slave address
	txData[1] = 0x03; // Function code
	txData[2] = 0x00; // Start Address High Byte (where to start reading)
	txData[3] = 0x00; // Start Address Low Byte
	txData[4] = 0x00; // Quantity High Byte
	txData[5] = 0x02; // Quantity low Byte
//	txData[5] = 0x02;


	uint16_t crc = ModRTU_CRC(txData, 6);

	// here we are reversing least byte to first
	txData[6] = crc & 0xFF;
	txData[7] = (crc >> 8) & 0xFF;

	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1) {
		uint16_t received_crc;
		uint16_t calculated_crc;


		uint8_t rxData[9] = {0};

		/*
		 * Send Modbus Request
		 */
		HAL_UART_Transmit(&huart2, txData, 8, 100);

		/*
		 * Receive Response
		 *
		 * Expected:
		 * 01 03 04 XX XX XX XX CRC CRC
		 */
		HAL_StatusTypeDef status;

		status =  HAL_UART_Receive(&huart2, rxData, 9, 1000);

		if(status == HAL_OK){
		received_crc = rxData[7] | (rxData[8] << 8);

		calculated_crc = ModRTU_CRC(rxData, 7);

		if(received_crc == calculated_crc)
		{
//			 uint32_t raw_data;
//		    raw_data = ((uint32_t)rxData[3] << 24) |
//		               ((uint32_t)rxData[4] << 16) |
//		               ((uint32_t)rxData[5] << 8)  |
//		               ((uint32_t)rxData[6]);
//
//		    memcpy(&value, &raw_data, sizeof(value));
//
//		    printf("Raw HEX     = 0x%08lX\r\n", raw_data);
//
//		    printf("Float Value = %.4f\r\n", value);
		    reg1 = (rxData[3] << 8) | rxData[4];

		    reg2 = (rxData[5] << 8) | rxData[6];

		    combined = ((uint32_t)reg1 << 16) | reg2;

		    value = *(float*)&combined;

		    printf("Float Value = %.4f\r\n", value);
		}
		else
		{
		    printf("CRC ERROR\r\n");
		}
		}
		else if(status == HAL_TIMEOUT)
		{
		    printf("NO RESPONSE FROM SLAVE\r\n");
		}
		else
		{
		    printf("UART ERROR\r\n");
		}

//		/*
//		 * Extract Register Values
//		 */
//		reg1 = (rxData[3] << 8) | rxData[4];
//
//		reg2 = (rxData[5] << 8) | rxData[6];
//
//		/*
//		 * Combine Registers
//		 */
//		combined = ((uint32_t) reg1 << 16) | reg2;
//
//		/*
//		 * Convert to Float
//		 */
//		memcpy(&value, &combined, sizeof(value));
//
//		printf("Float Value = %.4f\r\n", value);

		HAL_Delay(1000);

		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */
	}

	/* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

	__HAL_RCC_PWR_CLK_ENABLE();

	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;

	RCC_OscInitStruct.HSIState = RCC_HSI_ON;

	RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;

	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;

	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;

	RCC_OscInitStruct.PLL.PLLM = 8;

	RCC_OscInitStruct.PLL.PLLN = 50;

	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;

	RCC_OscInitStruct.PLL.PLLQ = 7;

	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}

	RCC_ClkInitStruct.ClockType =
	RCC_CLOCKTYPE_HCLK |
	RCC_CLOCKTYPE_SYSCLK |
	RCC_CLOCKTYPE_PCLK1 |
	RCC_CLOCKTYPE_PCLK2;

	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;

	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;

	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;

	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK) {
		Error_Handler();
	}
}

/**
 * @brief USART2 Initialization Function
 * @param None
 * @retval None
 */
static void MX_USART2_UART_Init(void) {

	huart2.Instance = USART2;

	huart2.Init.BaudRate = 9600;

	huart2.Init.WordLength = UART_WORDLENGTH_8B;

	huart2.Init.StopBits = UART_STOPBITS_1;

	huart2.Init.Parity = UART_PARITY_NONE;

	huart2.Init.Mode = UART_MODE_TX_RX;

	huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;

	huart2.Init.OverSampling = UART_OVERSAMPLING_16;

	if (HAL_UART_Init(&huart2) != HAL_OK) {
		Error_Handler();
	}
}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void MX_GPIO_Init(void) {
	GPIO_InitTypeDef GPIO_InitStruct = { 0 };

	__HAL_RCC_GPIOE_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOH_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();

	HAL_GPIO_WritePin(GPIOD,
	LD4_Pin | LD3_Pin | LD5_Pin | LD6_Pin, GPIO_PIN_RESET);

	GPIO_InitStruct.Pin =
	LD4_Pin |
	LD3_Pin |
	LD5_Pin |
	LD6_Pin;

	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;

	GPIO_InitStruct.Pull = GPIO_NOPULL;

	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

	HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
}

/* USER CODE BEGIN 4 */

int __io_putchar(int ch) {
	ITM_SendChar(ch);
	return ch;
}

/**
 Modbus CRC16
 Cyclic Redundancy Check
*/
uint16_t ModRTU_CRC(uint8_t buf[], int len) {
	uint16_t crc = 0xFFFF;

	for (int pos = 0; pos < len; pos++) {
		crc ^= (uint16_t) buf[pos];

		for (int i = 8; i != 0; i--) {
			if ((crc & 0x0001) != 0) {
				crc >>= 1;
				crc ^= 0xA001;
			} else {
				crc >>= 1;
			}
		}
	}

	return crc;
}

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
	__disable_irq();

	while (1) {
	}
}

#ifdef USE_FULL_ASSERT

void assert_failed(uint8_t *file, uint32_t line)
{
}

#endif
