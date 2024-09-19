#pragma once

#include "Settings.h"
#include "Bitmap.h"
#include "PicoClockHw/Display.h"

#include <string>
#include <vector>
#include <memory>

class Clock;
class Bitmap;
class ClockUi;
class Buzzer;

class AbstractFunction
{
public:
    // Note: The blinking is delayed by one frame to prevent digits from disappearing just before the
    // button repetition will make it appear again.
    static const int BLINKING_DISAPPEAR_FRAME = Display::FRAME_RATE / 2 + 1;

    enum Direction
    {
        Up,
        Down,
        RepeatedUp,
        RepeatedDown
    };

    AbstractFunction(ClockUi *clockUi) : m_clockUi(clockUi)
    {}

    virtual void renderFrame(
        Bitmap &frame, int editedValueIndex, int blinkingCounter, bool fullRefresh) = 0;
    virtual bool isTimeFunction() const { return false; }
    virtual bool isAvailable() const { return true; }
    
    // Called when pressing the "Set" button. The default behaviour is to cycle through 
    // editable values.
    virtual void activate() 
    {
        editValues();
    }

    virtual int valueCount() const
    {
        return 1; // For the value that cannot be edited
    }

    // Called when a function was just selected on the display
    virtual void onSelect()
    {}
    
    virtual void startEditingValue(int valueIndex) {}
    virtual void modifyValue(int valueIndex, Direction direction) {}
    virtual void finishEditing() {}

    virtual bool allowsBrightnessBoost(int valueIndex) const
    {
        return true;
    }

protected:
    enum Field
    {
        Year,
        Month,
        Day,
        Hour,
        Minute,
        Alarm1Hour,
        Alarm1Minute,
        Alarm2Hour,
        Alarm2Minute,
        CountdownStartMinute,
        CountdownStartSecond,
        ManualBrightness,
        BrightnessDark,
        BrightnessDim,
        BrightnessBright,
    };

    // Method to access ClockUi
    const Settings::Values &settings() const;
    Settings::Values &modifySettings();
    const Clock &clock() const;
    Clock &clock();
    void convertHour(int hour24, int &displayedHour, bool &morning) const;
    void editValues();
    int editedValueIndex() const;
    void adjustField(Field f, Direction d);
    template <typename Enum>
    void adjustEnum(Enum &value, AbstractFunction::Direction dir);
    void putAmPmIndicators(Bitmap &frame, bool morning);
    void putWeekDay(Bitmap &frame);
    void setBlinkingCounter(int counter);
    void forceRefresh();
    void bringScrollingToRight();
    void renderScrollingText(
        Bitmap &frame,
        bool fullRefresh,
        const std::string &leftText, 
        const std::string &editedValue = "", 
        const std::string &rightText = "");
    ClockUi *clockUi()
    {
        return m_clockUi;
    }
    void setCurrentMenu(
        std::vector<std::unique_ptr<AbstractFunction>> *menu, 
        AbstractFunction *function = nullptr);
    void select(); // Select the function if it is in the current menu
    Buzzer &buzzer();

private:
    ClockUi *m_clockUi = nullptr;
};

template <typename Enum>
void AbstractFunction::adjustEnum(Enum &enumValue, AbstractFunction::Direction dir)
{
    int value = static_cast<int>(enumValue);
    
    if (dir == Up || dir == RepeatedUp)
        value++;
    else if (dir == Down || dir == RepeatedDown)
        value--;
    
    if (value < 0 )
        value = static_cast<int>(Enum::Count) - 1;
    else if (value >= static_cast<int>(Enum::Count))
        value = 0;
    
    enumValue = static_cast<Enum>(value);
}