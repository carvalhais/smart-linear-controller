#include "Frequency.h"

Frequency::Frequency()
{
}

void Frequency::gradientButton(
    uint16_t x,
    uint16_t y,
    uint8_t w,
    uint8_t h,
    uint8_t radius,
    uint16_t topColor,
    uint16_t bottomColor,
    uint16_t borderColor,
    bool dashed)
{

    for (uint16_t i = 0; i < h; ++i)
    {
        uint16_t start = x;
        uint16_t width = w;

        if (i < radius)
        {
            start += (radius - i);
            width -= (radius - i) * 2;
        }

        if (i > (h - radius - 1))
        {
            start += radius - (h - i) + 1;
            width -= (radius - (h - i) + 1) * 2;
        }

        uint8_t alpha = (255 * i) / h; // alpha is a value in the range 0-255
        uint16_t color = _spr->alphaBlend(alpha, bottomColor, topColor);
        _spr->drawFastHLine(start, y + i, width, color);
    }
    _spr->drawRoundRect(x, y, w, h, radius, borderColor);
    if (dashed)
    {
        uint8_t dashWidth = 2;
        uint8_t offset = radius + dashWidth;
        uint8_t numDashesH = (w - offset) / dashWidth / 2;
        uint8_t numDashesV = (h - offset) / dashWidth / 2;

        for (uint8_t i = 0; i < numDashesH; i++)
        {
            uint8_t pos = (i * 2) * dashWidth;
            _spr->drawFastHLine(x + offset + pos, y, dashWidth, TFT_BLACK);
            _spr->drawFastHLine(x + offset + pos, y + h - 1, dashWidth, TFT_BLACK);
        }
        for (uint8_t i = 0; i < numDashesV; i++)
        {
            uint8_t pos = (i * 2) * dashWidth;
            _spr->drawFastVLine(x, y + offset + pos, dashWidth, TFT_BLACK);
            _spr->drawFastVLine(x + w - 1, y + offset + pos, dashWidth, TFT_BLACK);
        }
    }
}

void Frequency::begin(uint32_t x, uint32_t y, uint32_t w, uint32_t h, TFT_eSPI *tft, const uint8_t smallFont[], const uint8_t mediumFont[], const uint8_t largeFont[])
{
    _x = x;
    _y = y;
    _w = w;
    _h = h;
    _tft = tft;
    _smallFont = (uint8_t *)smallFont;
    _mediumFont = (uint8_t *)mediumFont;
    _largeFont = (uint8_t *)largeFont;
    _spr = std::unique_ptr<TFT_eSprite>(new TFT_eSprite(tft));
    _spr->createSprite(_w, _h);
    _spr->drawFastHLine(0, _h, _w, TFT_DARKGREY);
    _spr->pushSprite(_x, _y);
}

void Frequency::end()
{
    if (_spr->created())
    {
        _spr->deleteSprite();
    }
}

void Frequency::frequencyChanged(uint32_t frequency, uint8_t modulation, uint8_t filter, bool txState, char *band, bool txEnabled)
{
    _frequency = frequency;
    _modulation = modulation;
    _filter = filter;
    _txState = txState;
    _band = band;
    _txEnabled = txEnabled;

    _freqChanged = true;
}

void Frequency::loop()
{
    if (_freqChanged && _spr != nullptr)
    {
        _freqChanged = false;

        // fill the sprite area with black
        _spr->fillRect(0, 0, _w, _h, TFT_BLACK);

        _spr->loadFont(_mediumFont);
        _spr->setTextDatum(TC_DATUM);

        // draw the background for the TX/RX "button"
        if (_txState)
        { //TX
            _spr->setTextColor(TFT_WHITE);
            gradientButton(6, 7, 30, 25, 2, 0xE8E3, 0x9000, 0x9A8A, !_txEnabled);
        }
        else
        { //RX
            _spr->setTextColor(TFT_RED);
            gradientButton(6, 7, 30, 25, 2, 0x2104, TFT_BLACK, TFT_RED, !_txEnabled);
        }
        _spr->drawCentreString("TX", 20, 12, 1);

        // draw the background for the MODE "button"
        gradientButton(42, 7, 40, 25, 2, 0x3A8F, 0x1169, 0x6C13, false); // light blue, dark blue, lighter blue

        _spr->setTextColor(TFT_WHITE);
        const char *modeText;
        if (_modulation < 9)
        {
            modeText = _modeText[_modulation];
        }
        else if (_modulation == 23)
        {
            modeText = (char *)"DV";
        }
        else
        {
            modeText = (char *)"-";
        }
        _spr->drawCentreString(modeText, 62, 12, 1);

        // draw the background for the BAND "button"
        gradientButton(88, 7, 40, 25, 2, TFT_DARKGREY, 0x2124, 0x4208, false); // light blue, dark blue, lighter blue
        _spr->setTextColor(TFT_WHITE);
        _spr->drawCentreString(_band, 108, 12, 1);

        _spr->unloadFont();
        _spr->loadFont(_largeFont);
        _spr->setTextColor(TFT_WHITE);

        char buffmain[7];
        char buffdigit[3];

        // draw the frequency digits one by one to mimic a fixed width font
        sprintf(buffmain, "%d", _frequency / 1000);
        sprintf(buffdigit, "%02d", (_frequency % 1000) / 10);

        uint16_t pos = 145;
        uint8_t charWidth = 20;
        String freqs = String(buffmain);
        uint8_t padCount = 6 - freqs.length();
        pos += (padCount * charWidth);
        uint8_t freqTop = 8;

        for (uint8_t i = 0; i < freqs.length(); i++)
        {
            _spr->drawCentreString(freqs.substring(i, i + 1), pos, freqTop, 1);
            pos += charWidth;

            if ((padCount + i + 1) % 3 == 0)
            {
                _spr->drawCentreString(".", pos - (charWidth * .3), freqTop, 1);
                pos += (charWidth * .4);
            }
        }

        for (uint8_t i = 0; i < 2; i++)
        {
            _spr->drawCentreString(String(buffdigit[i]), pos, freqTop, 1);
            pos += charWidth;
        }

        _spr->pushSprite(_x, _y);
        _spr->unloadFont();
    }
}
