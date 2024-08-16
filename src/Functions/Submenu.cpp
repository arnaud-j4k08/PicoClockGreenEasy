#include "Submenu.h"
#include "Action.h"
#include "UiTexts.h"

void Submenu::activate()
{
    // Add "Exit" function just before the menu is going to be used, so that other menu functions are
    // present in the meantime.
    if (!m_exitAdded)
    {
        m_exitAdded = true;
        addFunction<Action>(clockUi(), uiText(TextId::Exit), std::bind(&Submenu::exit, this));
    }

    // Enter submenu
    setCurrentMenu(&m_functions);
}

void Submenu::renderFrame(
    Bitmap &frame, int editedValueIndex, int blinkingCounter, bool fullRefresh)
{
    renderScrollingText(frame, fullRefresh, m_name);
}

void Submenu::exit()
{
    setCurrentMenu(m_parentMenu, this);
}