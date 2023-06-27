#pragma once
#include "BargraphBase.h"

class BargraphRfPower : public BargraphBase
{
public:
	void drawScale();
	void setValue(float value);
	void setInitialScale(uint16_t value);
	void showDecimal(bool value);

private:
	uint16_t _maxScale = 10;
	bool _showDecimal = true;
	uint16_t _scaleValues[9] = {5, 10, 20, 50, 100, 200, 500, 750, 1000};
};
