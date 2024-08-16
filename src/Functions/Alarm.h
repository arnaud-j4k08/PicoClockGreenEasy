#pragma once

#include "AbstractFunction.h"

class Bitmap;

class Alarm : public AbstractFunction
{
public:   
    enum AlarmId
    {
        Alarm1,
        Alarm2
    };

    Alarm(ClockUi *clockUi, AlarmId id) : AbstractFunction(clockUi), m_alarmId(id)
    {}

private:
    void renderFrame(
        Bitmap &frame, int editedValueIndex, int blinkingCounter, bool fullRefresh) override;
    int valueCount() const override;
    void startEditingValue(int valueIndex) override;
    void modifyValue(int valueIndex, Direction direction) override;
    void finishEditing() override;

    const Settings::Alarm &alarmSettings() const;
    Settings::Alarm &modifyAlarmSettings();

    enum EditableValue
    {
        NoEditing = 0,
        EditingAlarmMode,
        EditingAlarmHour,
        EditingAlarmMinute,
        EditingAlarmWeekDays,
        ValueCount 
    };

    const AlarmId m_alarmId;
    CyclicCounter m_editedAlarmWeekDay {7, 0};
};