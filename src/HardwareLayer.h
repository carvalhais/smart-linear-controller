#include <Arduino.h>
#include <memory>
#include <functional>
#include "Defines.h"
#include "Types.h"
#include "Button.h"
#include "LatchingRelay.h"

typedef std::function<void(float forwardMv, float reverseMv)> SwrCb;
typedef std::function<void(int button, bool longPress)> ButtonPressedCb;

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
    void onButtonPressedCallback(ButtonPressedCb cb);

    void begin();
    void loop();

private:

    SwrCb _inputSwrCallback;
    SwrCb _outputSwrCallback;
    ButtonPressedCb _buttonPressed;

    float readVpp(uint8_t pin);
    float readPowerSwr(uint8_t pinForward, uint8_t pinReverse, SwrCb callback, float lastRead);

    volatile time_t _timer1;

    float _lastInputSwr;
    float _lastOutputSwr;

    LatchingRelay _vhfRelay;

    LowPassFilter _lpf;
    Amplifier _amp;

    Button _buttonA;
    Button _buttonB;
    Button _buttonC;

};