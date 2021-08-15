#include "HAL.h"
#include "Display/Display.h"

#define BATT_MIN_VOLTAGE            3300
#define BATT_MAX_VOLTAGE            4200
#define BATT_FULL_CHARGE_VOLTAGE    4100

#define POWER_ADC                   ADC1

/*��һ�β���ʱ��(ms)*/
static uint32_t Power_LastHandleTime = 0;

/*�Զ��ػ�ʱ��(��)*/
static uint16_t Power_AutoLowPowerTimeout = 60;

/*�Զ��ػ�����ʹ��*/
static bool Power_AutoLowPowerEnable = false;

static bool Power_IsShutdown = false;

static volatile uint16_t Power_ADCValue = 0;

static HAL::Power_CallbackFunction_t Power_EventCallback = NULL;

static void Power_ADC_Init()
{
    RCC_APB2PeriphClockCmd(RCC_APB2PERIPH_ADC1, ENABLE);
    RCC_ADCCLKConfig(RCC_APB2CLK_Div8);

    ADC_Reset(POWER_ADC);

    ADC_InitType ADC_InitStructure;
    ADC_StructInit(&ADC_InitStructure);
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_InitStructure.ADC_ScanMode = DISABLE;
    ADC_InitStructure.ADC_ContinuousMode = DISABLE;
    ADC_InitStructure.ADC_ExternalTrig = ADC_ExternalTrig_None;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NumOfChannel = 1;
    ADC_Init(POWER_ADC, &ADC_InitStructure);

    ADC_ClearFlag(POWER_ADC, ADC_FLAG_EC);

    ADC_INTConfig(POWER_ADC, ADC_INT_EC, ENABLE);
    NVIC_EnableIRQ(ADC1_2_IRQn);

    ADC_Ctrl(POWER_ADC, ENABLE);
    ADC_RstCalibration(POWER_ADC);
    while(ADC_GetResetCalibrationStatus(POWER_ADC));
    ADC_StartCalibration(POWER_ADC);
    while(ADC_GetCalibrationStatus(POWER_ADC));
}

static void Power_ADC_TrigUpdate()
{
    ADC_RegularChannelConfig(
        POWER_ADC,
        PIN_MAP[CONFIG_BAT_DET_PIN].ADC_Channel,
        1,
        ADC_SampleTime_41_5
    );
    ADC_SoftwareStartConvCtrl(POWER_ADC, ENABLE);
}

extern "C" {

    void ADC1_2_IRQHandler(void)
    {
        if(ADC_GetINTStatus(POWER_ADC, ADC_INT_EC) != RESET)
        {
            Power_ADCValue = ADC_GetConversionValue(POWER_ADC);
            ADC_ClearINTPendingBit(POWER_ADC, ADC_INT_EC);
        }
    }

}

/**
  * @brief  ��Դ��ʼ��
  * @param  ��
  * @retval ��
  */
void HAL::Power_Init()
{
    /*��Դʹ�ܱ���*/
    Serial.println("Power: Waiting...");
    pinMode(CONFIG_POWER_EN_PIN, OUTPUT);
    digitalWrite(CONFIG_POWER_EN_PIN, LOW);
    delay(1000);
    digitalWrite(CONFIG_POWER_EN_PIN, HIGH);
    Serial.println("Power: ON");

    /*��ؼ��*/
    Power_ADC_Init();
    pinMode(CONFIG_BAT_DET_PIN, INPUT_ANALOG);
    pinMode(CONFIG_BAT_CHG_DET_PIN, INPUT_PULLUP);

//    Power_SetAutoLowPowerTimeout(5 * 60);
//    Power_HandleTimeUpdate();
    Power_SetAutoLowPowerEnable(false);
}

/**
  * @brief  ���²���ʱ��
  * @param  ��
  * @retval ��
  */
void HAL::Power_HandleTimeUpdate()
{
    Power_LastHandleTime = millis();
}

/**
  * @brief  �����Զ��ػ�ʱ��
  * @param  sec:ʱ��(��)
  * @retval ��
  */
void HAL::Power_SetAutoLowPowerTimeout(uint16_t sec)
{
    Power_AutoLowPowerTimeout = sec;
}

/**
  * @brief  ��ȡ�Զ��ػ�ʱ��
  * @param  ��
  * @retval sec:ʱ��(��)
  */
uint16_t HAL::Power_GetAutoLowPowerTimeout()
{
    return Power_AutoLowPowerTimeout;
}

/**
  * @brief  �����Զ��ػ�����ʹ��
  * @param  en:ʹ��
  * @retval ��
  */
void HAL::Power_SetAutoLowPowerEnable(bool en)
{
    Power_AutoLowPowerEnable = en;
    Power_HandleTimeUpdate();
}

/**
  * @brief  ִ�йػ�
  * @param  ��
  * @retval ��
  */
void HAL::Power_Shutdown()
{
    Backlight_SetGradual(0, 500);
    digitalWrite(CONFIG_POWER_EN_PIN, LOW);
    Power_IsShutdown = true;
}

/**
  * @brief  �Զ��ػ����
  * @param  ��
  * @retval ��
  */
void HAL::Power_Update()
{
    __IntervalExecute(Power_ADC_TrigUpdate(), 1000);

    if(!Power_AutoLowPowerEnable)
        return;

    if(Power_AutoLowPowerTimeout == 0)
        return;

    if(millis() - Power_LastHandleTime >= (Power_AutoLowPowerTimeout * 1000))
    {
        Power_Shutdown();
    }
}

void HAL::Power_EventMonitor()
{
    if(Power_IsShutdown && Power_EventCallback)
    {
        Power_EventCallback();
    }
}

void HAL::Power_GetInfo(Power_Info_t* info)
{
    int voltage = map(
                      Power_ADCValue,
                      0, 4095,
                      0, 3300
                  );

    voltage *= 2;

    __LimitValue(voltage, BATT_MIN_VOLTAGE, BATT_MAX_VOLTAGE);

    int usage = map(
                    voltage,
                    BATT_MIN_VOLTAGE, BATT_FULL_CHARGE_VOLTAGE,
                    0, 100
                );

    __LimitValue(usage, 0, 100);

    info->usage = usage;
    info->isCharging = !digitalRead(CONFIG_BAT_CHG_DET_PIN);
    info->voltage = voltage;
}

void HAL::Power_SetEventCallback(Power_CallbackFunction_t callback)
{
    Power_EventCallback = callback;
}
