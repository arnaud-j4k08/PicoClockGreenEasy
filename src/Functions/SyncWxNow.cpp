#include "SyncWxNow.h"
#include "UiTexts.h"
#include "Clock.h"

void SyncWxNow::renderFrame(
    Bitmap &frame, int editedValueIndex, int blinkingCounter, bool fullRefresh)
{
    renderScrollingText(frame, fullRefresh, uiText(TextId::SyncWxNow));
}

void SyncWxNow::activate()
{
    clock().syncWxNow();

    if (m_nextFunction != nullptr)
        m_nextFunction->select();
}