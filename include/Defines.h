
#define BT_NAME "IC-705 Linear"
#define HEADER_TITLE "SMART HF/VHF LINEAR CONTROLLER"

#define DEBUG 1

#define DEG2RAD 0.0174532925

#define PIN_INPUT_SWR_FWD 36
#define PIN_INPUT_SWR_REV 32

#define PIN_OUTPUT_SWR_FWD 34
#define PIN_OUTPUT_SWR_REV 35

#define PIN_HF_VHF_AMP 2 // OK
#define PIN_TX_VHF 0 // OK
#define PIN_TX_HF 13 // OK

#define PIN_LPF_A 17 // OK
#define PIN_LPF_B 26 // OK
#define PIN_LPF_C 16 // OK

#define PIN_PULSE_RX 5 // OK
#define PIN_PULSE_TX 22 // OK

#define PIN_PSU_CONTROL 21

#define FREQ_VHF_AMP 60000

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240
#define HEADER_HEIGHT 20

#define MODE_LSB 0
#define MODE_USB 1
#define MODE_AM 2
#define MODE_CW 3
#define MODE_FM 5

#define BARGRAPH_COLOR_ON 0x000F   //TFT_NAVY;
#define BARGRAPH_COLOR_OFF 0xC618  //TFT_SILVER;
#define BARGRAPH_COLOR_FONT 0xC618 //TFT_SILVER;
#define TFT_GREY 0x5AEB

#define BLK_PWM_CHANNEL 7
#define TFT_BL 32

#define BUTTON_A 39
#define BUTTON_B 38
#define BUTTON_C 37

#ifdef DEBUG    //Macros are usually in all capital letters.
   #define DBG(...)    Serial.printf(__VA_ARGS__)
#else
   #define DBG(...)     //now defines a blank line
#endif

#define minimum(a,b)     (((a) < (b)) ? (a) : (b))