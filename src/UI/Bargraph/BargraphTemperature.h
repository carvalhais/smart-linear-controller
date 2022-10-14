#include "BargraphBase.h"

class BargraphTemperature : public BargraphBase
{
public:
	void drawScale();
	void setValue(float value);

private:
	uint16_t _maxScale = 100;
};
