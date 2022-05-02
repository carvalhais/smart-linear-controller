#include <Arduino.h>
#include <functional>
#include "Defines.h"
#include "Types.h"

typedef std::function<void(float forwardMv, float reverseMv)> SwrCb;

class HardwareLayer
{
public:
    HardwareLayer();

    void onInputSwrCallback(SwrCb cb);
    void onOutputSwrCallback(SwrCb cb);

    void onAmplifierChanged(Amplifier amp);
    void onLowPassFilterChanged(LowPassFilter lpf);
    void onTransmitChanged(bool state);
    void onPowerSupplyChanged(bool state);

    void begin();
    void loop();

private:
    SwrCb _inputSwrCallback;
    SwrCb _outputSwrCallback;

    float readVpp(uint8_t pin);
    float readPowerSwr(uint8_t pinForward, uint8_t pinReverse, SwrCb callback, float lastRead);

    volatile time_t _timer1;

    float _lastInputSwr;
    float _lastOutputSwr;

    uint8_t _lpfPin;
    LowPassFilter _lpf;
    Amplifier _amp;
};