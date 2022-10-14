#include "BargraphBase.h"

class BargraphAmperes : public BargraphBase
{
public:
	void drawScale();
	void setValue(float value);

private:
	uint16_t _maxScale = 20;
};
