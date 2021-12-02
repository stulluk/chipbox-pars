#ifndef _TUNER_INIT
#define _TUNER_INIT

#define NUMBER_OF_TUNER      1

#define MAX_UNSIGNED_VAL     0xFFFFFFFF
#define MAX_UNSIGNED_DIGIT   10
#define MAX_HEX_DIGIT        8
#define MAX_LINE_LENGTH      256
#define ITEM_NAME_LENGTH     80
#define ITEM_VALUE_LENGTH    80

#define NUMBER_OF_TITLE         9

#define TITLE_INIT_DATA         "TunerInitData"
#define TITLE_OPENDATA          "TunerOpenData"
#define TITLE_DEMODULATOR_I2C   "DemodulatorI2c"
#define TITLE_PLL_I2C           "PllI2c"
#define TITLE_TUNER_RESET       "TunerResetGpio"
#define TITLE_LNB_POWER         "LnbPowerGpio"
#define TITLE_HOR_VER           "HorVerGpio"
#define TITLE_V12               "V12Gpio"
#define TITLE_SKEW              "SkewGpio"

#define ITEM_NUMBER_OF_TUNER    "NumberOfTuner"

#define ITEM_TUNER_NUMBER       "TunerNumber"
#define ITEM_PLL_REPEATE_MODE   "PllrepeateMode"
#define ITEM_OUTPUT_MODE        "TunerOutputMode"
#define ITEM_OUTPUT_CLK         "TunerOutputClock"
#define ITEM_TUNER_TYPE         "TunerType"
#define ITEM_DEMODULATOR_DEVICE "DemodulatorDevice"
#define ITEM_PLL_DEVICE         "PllDevice"

#define ITEM_I2C_NUMBER         "I2cNumber"
#define ITEM_I2C_ADDRESS        "I2cAddress"
#define ITEM_I2C_ADDRESS_LEN    "I2cSubLength"
#define ITEM_I2C_SPEED          "I2cSpeed"

#define ITEM_GPIO_PORT          "PortNumber"
#define ITEM_GPIO_BIT           "BitNumber"
#define ITEM_GPIO_MODE          "GpioMode"
#define ITEM_GPIO_INIT_VALUE    "InitialValue"

#define VALUE_ON                "ON"
#define VALUE_OFF               "OFF"
#define VALUE_STV0903           "STV0903"
#define VALUE_STV6110           "STV6110"
#define VALUE_AVL2108           "AVL2108"
#define VALUE_IX2564            "IX2564"
#define VALUE_TUNER_PARALLEL    "PARALLEL"
#define VALUE_TUNER_PARALLEL_CI "PARALLEL_CONT"
#define VALUE_TUNER_SERIAL      "SERIAL"
#define VALUE_TUNER_SERIAL_CONT "SERIAL_CONT"
#define VALUE_TUNER_CLK_FALLING "CLOCK_FALLING"
#define VALUE_TUNER_CLK_RISING  "CLOCK_RISING"
#define VALUE_TUNER_DVB_S       "DVB_S"
#define VALUE_TUNER_DVB_C       "DVB_C"
#define VALUE_TUNER_DVB_T       "DVB_T"
#define VALUE_GPIO_OUTPUT       "OUTPUT"
#define VALUE_GPIO_INPUT        "INPUT"
#define VALUE_GPIO_IO           "IO"

typedef enum{
	DATA_COMMENT,
	DATA_TITLE,
	DATA_ITEM,
	DATA_EOF,
	DATA_UNKNOWN
} ConfigDataType;

typedef struct
{
	U8   *Name;
	void *ConfigData;
	ConfigDataType (*Function)(FILE* cfgFile, void *data, U8 *nextTitle);
}ConfigData;

extern U32	*Tuner_HandleId;

int TunerInitial(void);
#endif

