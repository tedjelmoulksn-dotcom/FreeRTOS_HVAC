/*
 * main.c — Contrôleur HVAC sous FreeRTOS (STM32, HAL)
 *
 * Code RETRANSCRIT à la main à partir des six captures d'écran du compte rendu
 * « OS temps réel » (docs/compte_rendu_os_temps_reel.pdf). Les fichiers d'origine
 * du projet STM32CubeIDE n'ont pas été conservés.
 *
 * Le contenu est reproduit tel qu'il apparaît sur les captures, sans correction.
 * Seule l'accolade fermante de Control_Damper(), coupée entre deux captures,
 * a été ajoutée.
 *
 * Fichiers générés par CubeMX non fournis : main.h, SystemClock_Config(),
 * MX_GPIO_Init(), MX_ADC1_Init(), MX_TIM1_Init(), Error_Handler(), noyau FreeRTOS.
 * Ce fichier ne se compile donc pas seul.
 */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include <math.h>

/* USER CODE BEGIN Includes */
#include "core_cm4.h"  // Pour ITM
#include <stdio.h>
/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
TIM_HandleTypeDef htim1;

/* USER CODE BEGIN PV */
float temperature = 22.0f;
float co2_ppm = 0.0f;
uint8_t fad_pwm = 0, exad_pwm = 0;
uint8_t fh1_state = 0, fh2_state = 0;
/* USER CODE END PV */

/* USER CODE BEGIN PFP */
float Read_ADC_Channel(uint32_t channel);
void Set_PWM(TIM_HandleTypeDef *htim, uint32_t channel, uint8_t duty_cycle_percent);
void Read_CO2(void);
void Read_Temperature(void);
void Control_Damper(void);
void Control_Heater(void);
void Task_Sensors(void *argument);
void Task_Control(void *argument);
/* USER CODE END PFP */

/* USER CODE BEGIN 0 */
int _write(int file, char *ptr, int len)
{
    for (int i = 0; i < len; i++) {
        ITM_SendChar(*ptr++);
    }
    return len;
}

float Read_ADC_Channel(uint32_t channel)
{
    ADC_ChannelConfTypeDef sConfig = {0};
    sConfig.Channel = channel;
    sConfig.Rank = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;

    HAL_ADC_ConfigChannel(&hadc1, &sConfig);
    HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);
    float value = HAL_ADC_GetValue(&hadc1);
    HAL_ADC_Stop(&hadc1);

    return value / 4095.0f;  // Normalisé [0,1]
}

void Set_PWM(TIM_HandleTypeDef *htim, uint32_t channel, uint8_t duty_cycle_percent)
{
    uint32_t pulse = (__HAL_TIM_GET_AUTORELOAD(htim) * duty_cycle_percent) / 100;
    __HAL_TIM_SET_COMPARE(htim, channel, pulse);
}

void Read_CO2()
{
    float analog = Read_ADC_Channel(ADC_CHANNEL_1); // PA1
    co2_ppm = analog * 2000.0f;
}

void Read_Temperature()
{
    float analog = Read_ADC_Channel(ADC_CHANNEL_2); // PA2
    temperature = analog * 40.0f;
}

void Control_Damper()
{
    uint8_t co2_cmd = 0, vent_cmd = 0;

    if(co2_ppm < 600) co2_cmd = 0;
    else if(co2_ppm <= 700) co2_cmd = 25;
    else if(co2_ppm <= 1000) co2_cmd = 50;
    else if(co2_ppm <= 1500) co2_cmd = 75;
    else co2_cmd = 100;

    if(temperature < 19) vent_cmd = 0;
    else if(temperature < 21) vent_cmd = 0;
    else if(temperature < 23) vent_cmd = 50;
    else if(temperature < 25) vent_cmd = 30;
    else if(temperature < 30) vent_cmd = 10;
    else vent_cmd = 5;

    fad_pwm = exad_pwm = fmin(co2_cmd + vent_cmd, 100);

    Set_PWM(&htim1, TIM_CHANNEL_1, fad_pwm); // PA7
    Set_PWM(&htim1, TIM_CHANNEL_2, exad_pwm); // PA4
}

void Control_Heater()
{
    if (temperature < 14)
    {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET); // FH1 ON
        fh1_state = 1;

        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET); // FH2 ON
        fh2_state = 1;
    }
    else
    {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_RESET); // FH1 OFF
        fh1_state = 0;

        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5); // FH2 TOGGLE
        fh2_state = !fh2_state;
    }
}

void Task_Sensors(void *argument)
{
    for(;;)
    {
        Read_CO2();
        Read_Temperature();

        // Affichage ITM
        printf("[Sensors] Temp: %.2f °C | CO2: %.2f ppm\r\n", temperature, co2_ppm);

        vTaskDelay(pdMS_TO_TICKS(1000)); // 1 seconde
    }
}

void Task_Control(void *argument)
{
    for(;;)
    {
        Control_Damper();
        Control_Heater();

        // Affichage ITM
        printf("[Control] FAD_PWM: %d%% | EXAD_PWM: %d%% | FH1: %s | FH2: %s\r\n",
               fad_pwm, exad_pwm,
               fh1_state ? "ON" : "OFF",
               fh2_state ? "ON" : "OFF");

        vTaskDelay(pdMS_TO_TICKS(1000)); // 1 seconde
    }
}
/* USER CODE END 0 */

int main(void)
{
    /* MCU Configuration--------------------------------------------------------*/
    HAL_Init();
    SystemClock_Config();

    MX_GPIO_Init();
    MX_ADC1_Init();
    MX_TIM1_Init();

    /* USER CODE BEGIN 1 */
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1); // PA7
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2); // PA4
    /* USER CODE END 1 */

    /* USER CODE BEGIN 2 */
    xTaskCreate(Task_Sensors, "Sensors", 128, NULL, tskIDLE_PRIORITY + 1, NULL);
    xTaskCreate(Task_Control, "Control", 128, NULL, tskIDLE_PRIORITY + 1, NULL);
    vTaskStartScheduler();
    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    while (1)
    {
        // Ne doit jamais s'exécuter
    }
    /* USER CODE END WHILE */
}

/* USER CODE BEGIN 4 */
// Tu peux ajouter ici d'autres fonctions utilisateur
/* USER CODE END 4 */
