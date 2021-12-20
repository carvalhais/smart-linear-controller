#include "BargraphBase.h"

class BargraphSwr : public BargraphBase
{
public:
	void drawScale();
	void setValue(float value);

private:
	void dumpSwr(float ratio, int offset);
};
