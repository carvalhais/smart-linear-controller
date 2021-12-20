#include "UI.h"

UI::UI()
{
}

void UI::drawHeader()
{
    uint16_t width = SCREEN_WIDTH / 2;
    for (uint16_t i = 0; i <= width; i++)
    {
        uint8_t alpha = (255 * i) / width; // alpha is a value in the range 0-255
        uint16_t color = _spr->alphaBlend(alpha, 0x4A69, TFT_BLACK);
        _spr->drawFastVLine(i, 0, HEADER_HEIGHT - 1, color);
        _spr->drawFastVLine(SCREEN_WIDTH - i, 0, HEADER_HEIGHT - 1, color);
    }
    _spr->drawFastHLine(0, HEADER_HEIGHT - 1, SCREEN_WIDTH, TFT_DARKGREY);
    _spr->setTextDatum(MC_DATUM);
    _spr->loadFont(_smallFont);
    _spr->setTextColor(TFT_WHITE);
    _spr->drawString(HEADER_TITLE, 160, 12, 1);
    _spr->unloadFont();
    _spr->pushSprite(0, 0);
}

void UI::clearScreen()
{
    _tft->drawRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, TFT_DARKGREY);
    _tft->fillRect(1, HEADER_HEIGHT + 1, SCREEN_WIDTH - 2, SCREEN_HEIGHT - HEADER_HEIGHT - 2, TFT_BLACK);
    _tft->setTextDatum(MC_DATUM);
    _tft->loadFont(_smallFont);
    _tft->setTextColor(TFT_WHITE, TFT_BLACK);
    _tft->drawString("WAITING FOR BLUETOOTH...", 160, 120, 1);
    _tft->unloadFont();
}

void UI::begin(TFT_eSPI *tft, const uint8_t smallFont[])
{
    _tft = tft;
    _smallFont = (uint8_t *)smallFont;
    _spr = std::unique_ptr<TFT_eSprite>(new TFT_eSprite(_tft));
    _spr->createSprite(SCREEN_WIDTH, HEADER_HEIGHT);
}

void UI::end()
{
    if (_spr != nullptr && _spr->created())
    {
        _spr->deleteSprite();
    }
}