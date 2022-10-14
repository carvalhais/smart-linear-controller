#pragma once

#include <Arduino.h>

enum LowPassFilter
{
    BAND_OTHER = 0,
    BAND_160M,
    BAND_80M,
    BAND_60_40M,
    BAND_30_20M,
    BAND_17_15M,
    BAND_12_10M,
    BAND_6M,
    BAND_2M,
    BAND_70CM,
};

enum Amplifier
{
    AMP_UNKNOWN,
    AMP_HF,
    AMP_VHF,
    AMP_UHF
};

enum Protection
{
    PROTECTION_NONE,
    SWR,
    GAIN,
    TEMPERATURE,
    CURRENT
};

enum TouchCmd {
    TOUCH_NONE,
    MAIN_PSU,
    MAIN_BYPASS,
    MAIN_STANDBY,
    PSU_BACK
};

struct TouchRegion {
    uint16_t x;
    uint16_t y;
    uint16_t w;
    uint16_t h;
    TouchCmd cmd;
};

struct Band
{
    uint32_t min;
    uint32_t max;
    char *name;
    LowPassFilter lpf;
    float interceptFwd;
    float interceptRev;
    float inputPowerFactor;
};

enum Screens
{
    STANDBY,
    DIAG,
    MAIN,
    PSU
};

struct Diag
{
    bool mainAdc;
    bool mainExpander;
    bool rfAdc;
    bool temperature;
    float temperatureCelsius;
};
