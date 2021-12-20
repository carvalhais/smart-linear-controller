#include "BargraphBase.h"

class BargraphRfPower : public BargraphBase
{
public:
	void drawScale();
	void setValue(float value);

private:
	uint16_t _maxScale = 10;
	uint16_t _scaleValues[7] = {10, 50, 100, 200, 500, 750, 1000};
};
