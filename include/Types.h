#ifndef TYPES_H
#define TYPES_H

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

struct Band
{
    uint32_t min;
    uint32_t max;
    char *name;
    LowPassFilter lpf;
};

#endif