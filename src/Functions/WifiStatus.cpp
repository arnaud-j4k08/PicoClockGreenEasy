#include "WifiStatus.h"
#include "UiTexts.h"
#include "PicoClockHw/Wifi.h"
#include <string>

bool WifiStatus::isAvailable() const
{
    return Wifi::linkStatus() != Wifi::NotAvailable;
}

void WifiStatus::renderFrame(Bitmap &frame, int editedValueIndex, int blinkingCounter, bool fullRefresh)
{
    std::string text = 
        uiText(TextId::WifiColon) + 
        uiText(static_cast<TextId>(static_cast<int>(TextId::Unknown) + static_cast<int>(Wifi::linkStatus())));
    renderScrollingText(frame, fullRefresh, text);
}
