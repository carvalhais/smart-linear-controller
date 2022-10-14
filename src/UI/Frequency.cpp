#include "Frequency.h"

Frequency::Frequency()
{
}

void Frequency::begin(uint16_t x, uint16_t y, uint16_t w, uint16_t h, TFT_eSPI *tft, const uint8_t smallFont[], const uint8_t mediumFont[], const uint8_t largeFont[])
{
    _x = x;
    _y = y;
    _w = w;
    _h = h;
    _tft = tft;

    _smallFont = (uint8_t *)smallFont;
    _mediumFont = (uint8_t *)mediumFont;
    _largeFont = (uint8_t *)largeFont;

    _offsetTop = (_h / 2) - 12;

    _btnTx.begin(_tft, _mediumFont, _x + 6, _y + _offsetTop, 30, 25, 2);
    _btnTx.setBackColors(0x2104, TFT_BLACK, TFT_RED);
    _btnTx.setText(TFT_RED, "TX");

    _btnMode.begin(_tft, _mediumFont, _x + 42, _y + _offsetTop, 40, 25, 2);
    _btnMode.setBackColors(0x3A8F, 0x1169, 0x6C13);

    _btnBand.begin(_tft, _mediumFont, _x + 88, _y + _offsetTop, 40, 25, 2);
    _btnBand.setBackColors(TFT_DARKGREY, 0x2124, 0x4208);

    _spr = new TFT_eSprite(tft);
    _spr->setColorDepth(8);
    _spr->createSprite(_charWidth * 9, 25);
    _spr->setTextDatum(TL_DATUM);
    _spr->loadFont(_largeFont);
    _spr->setTextColor(TFT_WHITE, TFT_BLACK);

    _bandChanged = true;
    _txStateChanged = true; // draw the button once;
    _modChanged = true;
    _freqChanged = true;

    _updated = true;
}

void Frequency::end()
{
    if (_spr != nullptr && _spr->created())
    {
        _spr->deleteSprite();
        delete _spr;
        _spr = nullptr;
        _btnTx.end();
        _btnMode.end();
        _btnBand.end();
    }
}
void Frequency::setTransmitEnabled(bool state)
{
    _txEnabled = state;
    _txStateChanged = true;
    _updated = true;
}

void Frequency::frequencyChanged(uint32_t frequency, uint8_t modulation, uint8_t filter, bool txState, char *band, bool txEnabled)
{
    if (_frequency != frequency)
    {
        _frequency = frequency;
        _freqChanged = true;
    }

    if (_modulation != modulation)
    {
        _modulation = modulation;
        _modChanged = true;
    }

    _filter = filter;

    if (_band != band)
    {
        _band = band;
        _bandChanged = true;
    }

    if (_txEnabled != txEnabled || _txState != txState)
    {
        _txState = txState;
        _txEnabled = txEnabled;
        _txStateChanged = true;
    }

    _updated = true;
}

void Frequency::loop()
{
    if (_updated && _spr != nullptr)
    {
        _updated = false;

        if (_txStateChanged)
        {
            _btnTx.setDashed(!_txEnabled);
            if (_txEnabled)
            {
                if (_txState)
                { // TX
                    _btnTx.setBackColors(0xE8E3, 0x9000, 0x9A8A);
                    _btnTx.setText(TFT_WHITE, "TX");
                }
                else
                { // RX
                    _btnTx.setBackColors(0x2104, TFT_BLACK, TFT_RED);
                    _btnTx.setText(TFT_RED, "TX");
                }
            }
            _btnTx.update();
            _txStateChanged = false;
        }

        if (_modChanged)
        {
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
            _btnMode.setText(TFT_WHITE, modeText);
            _btnMode.update();
            _modChanged = false;
        }

        if (_bandChanged)
        {
            _btnBand.setText(TFT_WHITE, _band);
            _btnBand.update();
            _bandChanged = false;
        }

        if (_freqChanged)
        {
            char buffmain[7];
            char buffdigit[3];
            // draw the frequency digits one by one to mimic a fixed width font
            sprintf(buffmain, "%d", _frequency / 1000);
            sprintf(buffdigit, "%02d", (_frequency % 1000) / 10);

            String freqs = String(buffmain);
            uint8_t padCount = 6 - freqs.length();
            uint16_t pos = (padCount * _charWidth);

            _spr->fillSprite(TFT_BLACK);

            for (uint8_t i = 0; i < freqs.length(); i++)
            {
                _spr->drawString(freqs.substring(i, i + 1), pos, 0, 1);
                pos += _charWidth;

                if ((padCount + i + 1) % 3 == 0)
                {
                    _spr->drawString(".", pos, 0, 1);
                    pos += (_charWidth * .4);
                }
            }

            for (uint8_t i = 0; i < 2; i++)
            {
                _spr->drawString(String(buffdigit[i]), pos, 0, 1);
                pos += _charWidth;
            }
            _freqChanged = false;
            _spr->pushSprite(_x + _w - _spr->width(), _y + _offsetTop);
        }
    }
}
