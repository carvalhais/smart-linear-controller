
#define BT_NAME "IC-705 Linear"
#define HEADER_TITLE "SMART HF/VHF LINEAR CONTROLLER"

#define DEBUG 1

#define PIN_INPUT_SWR_FWD 33
#define PIN_INPUT_SWR_REV 32

#define PIN_OUTPUT_SWR_FWD 34
#define PIN_OUTPUT_SWR_REV 35

#define PIN_HF_VHF_AMP 15
#define PIN_TX_VHF 2
#define PIN_TX_HF 4

#define PIN_LPF_160M 13
#define PIN_LPF_80M 12
#define PIN_LPF_60_40M 14
#define PIN_LPF_30_20M 27
#define PIN_LPF_17_15_12M 26
#define PIN_LPF_10_11M 25
#define PIN_LPF_6M 5

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


#ifdef DEBUG    //Macros are usually in all capital letters.
   #define DBG(...)    Serial.printf(__VA_ARGS__)
#else
   #define DBG(...)     //now defines a blank line
#endif