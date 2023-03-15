
#define BT_NAME "IC-705 Linear"
#define HEADER_TITLE "SMART HF/VHF LINEAR CONTROLLER"

#define DIAG_TIMEOUT 5000

#define DEG2RAD 0.0174532925

#define PROTECTION_GAIN 30
#define PROTECTION_TEMPERATURE 50

#define IO_PA0 0
#define IO_PA1 1
#define IO_PA2 2
#define IO_PA3 3
#define IO_PA4 4
#define IO_PA5 5
#define IO_PA6 6
#define IO_PA7 7
#define IO_PB0 8
#define IO_PB1 9
#define IO_PB2 10
#define IO_PB3 11
#define IO_PB4 12
#define IO_PB5 13
#define IO_PB6 14
#define IO_PB7 15

#define IO_PIN_TX_RX_HF IO_PB0
#define IO_PIN_HF_VHF IO_PB1
#define IO_PIN_TX_RX_VHF IO_PB2
#define IO_PIN_PULSE_RX IO_PB3
#define IO_PIN_PULSE_TX IO_PB4
#define IO_PIN_BIAS_VHF IO_PB5
#define IO_PIN_BIAS_HF IO_PB6
#define IO_PIN_PSU_ON IO_PB7

#define ADDRESS_IO_EXPANDER 0x20
#define ADDRESS_ADC_OUTPUT_FWD 0x48
#define ADDRESS_ADC_OUTPUT_REV 0x49

#define ADC_RF_CHANNEL_HF_REV 0
#define ADC_RF_CHANNEL_HF_FWD 1
#define ADC_RF_CHANNEL_VHF_REV 3 // 2
#define ADC_RF_CHANNEL_VHF_FWD 2 // 3

#define ADC_SLOPE 40.0f // Slope of the AD8307 log output (Default = 40)

#define FREQ_VHF_AMP 60000

#define MODE_LSB 0
#define MODE_USB 1
#define MODE_AM 2
#define MODE_CW 3
#define MODE_FM 5

#define BARGRAPH_COLOR_ON 0x03E0   // TFT_GREEN;
#define BARGRAPH_COLOR_OFF 0xC618  // TFT_SILVER;
#define BARGRAPH_COLOR_FONT 0xC618 // TFT_SILVER;
#define TFT_GREY 0x5AEB

#define BUTTON_A 39
#define BUTTON_B 38
#define BUTTON_C 37

#define SCREEN_TIMEOUT 300000 // 5 minutos
#define DEFAULT_BACKLIGHT_LEVEL 40

#ifdef DEBUG
#define DBG(...) Serial.printf(__VA_ARGS__)
#else
#define DBG(...) // now defines a blank line
#endif

#define minimum(a, b) (((a) < (b)) ? (a) : (b))
#define maximum(a, b) (((a) > (b)) ? (a) : (b))

/* --- PRINTF_BYTE_TO_BINARY macro's --- */
#define PRINTF_BINARY_PATTERN_INT8 "%c%c%c%c%c%c%c%c"
#define PRINTF_BYTE_TO_BINARY_INT8(i) \
    (((i)&0x80ll) ? '1' : '0'),       \
        (((i)&0x40ll) ? '1' : '0'),   \
        (((i)&0x20ll) ? '1' : '0'),   \
        (((i)&0x10ll) ? '1' : '0'),   \
        (((i)&0x08ll) ? '1' : '0'),   \
        (((i)&0x04ll) ? '1' : '0'),   \
        (((i)&0x02ll) ? '1' : '0'),   \
        (((i)&0x01ll) ? '1' : '0')

#define PRINTF_BINARY_PATTERN_INT16 \
    PRINTF_BINARY_PATTERN_INT8 PRINTF_BINARY_PATTERN_INT8
#define PRINTF_BYTE_TO_BINARY_INT16(i) \
    PRINTF_BYTE_TO_BINARY_INT8((i) >> 8), PRINTF_BYTE_TO_BINARY_INT8(i)
#define PRINTF_BINARY_PATTERN_INT32 \
    PRINTF_BINARY_PATTERN_INT16 PRINTF_BINARY_PATTERN_INT16
#define PRINTF_BYTE_TO_BINARY_INT32(i) \
    PRINTF_BYTE_TO_BINARY_INT16((i) >> 16), PRINTF_BYTE_TO_BINARY_INT16(i)
#define PRINTF_BINARY_PATTERN_INT64 \
    PRINTF_BINARY_PATTERN_INT32 PRINTF_BINARY_PATTERN_INT32
#define PRINTF_BYTE_TO_BINARY_INT64(i) \
    PRINTF_BYTE_TO_BINARY_INT32((i) >> 32), PRINTF_BYTE_TO_BINARY_INT32(i)
/* --- end macros --- */

/*
u32 number = 0x1FFFFFE;
printf("My Flag "	PRINTF_BINARY_PATTERN_INT32 "\n", PRINTF_BYTE_TO_BINARY_INT32(number));
*/
