#ifndef _BARGRAPH_CLASS_
#define _BARGRAPH_CLASS_

#include "TFT_eSPI.h"
#include "Defines.h"

class BargraphBase
{
public:
	BargraphBase();
	virtual ~BargraphBase();
	void begin(uint32_t x, uint32_t y, uint32_t w, uint32_t h, TFT_eSPI *tft, const uint8_t smallFont[], const uint8_t mediumFont[]);
	void end();
	void setColor(uint32_t on, uint32_t off, uint32_t font);
	void loop();
	virtual void drawScale();

protected:
	void drawScaleItem(float value, char *label);
	void drawInfiniteSymbol(float value);
	void drawBar(uint8_t position, uint32_t color);
	void setValueLabel(float value, char *valueText);

	uint8_t *_smallFont;
	uint8_t *_mediumFont;
	uint32_t _fontColor;
	uint32_t _barColorOn;
	uint32_t _barColorOff;
	uint32_t _x;
	uint32_t _y;
	uint32_t _w;
	uint32_t _h;
	uint32_t _laneW;
	uint32_t _laneH;
	uint32_t _laneX;
	uint32_t _laneY;

	uint8_t _paddingLeft = 6;
	uint8_t _paddingTop = 5;
	uint8_t _paddingRight = 40;
	uint8_t _paddingBottom = 16;

	float _value;
	float _lastValue;
	float _peakValue;
	uint8_t _lastValuePosition;
	uint8_t _lastPeakPosition;
	uint8_t _peakIncrement;
	uint8_t _peakSpeed;
	uint8_t _numBars;
	uint8_t _barWidth;
	char *_valueText;
	TFT_eSPI *_tft;
	std::unique_ptr<TFT_eSprite> _spr;

private:
	void drawLabelValue();
	
};

#endif