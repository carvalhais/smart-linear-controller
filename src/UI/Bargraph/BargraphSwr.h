#include "BargraphRfPower.h"

class BargraphSwr : public BargraphRfPower
{
public:
	void drawScale();
	void setValue(float value);
	void drawScaleSwr();
	void setValueSwr(float value);
	void setReverseMode(ReversePowerMode mode);

private:
	ReversePowerMode _reverseMode = ReversePowerMode::MODE_SWR;
};
