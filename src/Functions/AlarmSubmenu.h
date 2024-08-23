#pragma once

#include "Submenu.h"

class AlarmSubmenu : public Submenu
{
public:
    AlarmSubmenu(
        ClockUi *clockUi,
        std::vector<std::unique_ptr<AbstractFunction>> *parentMenu);

private:
    void onSelect() override;
    void renderFrame(
        Bitmap &frame, int editedValueIndex, int blinkingCounter, bool fullRefresh) override;

    int m_nextAlarmWeekday = -1;
    bool m_nextAlarmOnMorning = false;
};