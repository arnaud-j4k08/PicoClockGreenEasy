#pragma once

#include "PicoClockHw/Button.h"
#include "PicoClockHw/Display.h"
#include "PicoClockHw/gpio.h"
#include "PicoClockHw/Buzzer.h"
#include "Bitmap.h"
#include "Clock.h"
#include "Settings.h"
#include "Functions/AbstractFunction.h"

class Countdown;
class Stopwatch;

class ClockUi
{
    // Give access to all members to AbstractFunction so that it can implement methods that
    // provide access to the UI infrastructure to classes derived from it.
    friend AbstractFunction;

public:
    ClockUi();
    void startNtpRequest()
    {
        m_clock.startSyncFromNtp();
    }

private:
    enum EditValue
    {
        NoEditing = 0
    };

    Clock m_clock;
    bool m_forceRefresh = true;
    CyclicCounter m_blinkingCounter {Display::FRAME_RATE, -1};
    int m_editedValueIndex = 0;
    Bitmap m_frameBuffer; 
    Display m_display{m_frameBuffer.buffer(), std::bind(&ClockUi::onFrameCallback, this)};
    Button m_setButton{K2};
    Button m_upButton{K1};
    Button m_downButton{K0};
    Buzzer m_buzzer;
    Settings m_settings;
    int m_secondsWithoutUserInput = 0;
    bool m_dayLight = false;
    int m_tickCountInDark = 0;
    Settings::AlarmMode m_alarmRinging = Settings::AlarmMode::Off;
    int m_ringingForSecs = 0;

    // Menu and functions
    std::vector<std::unique_ptr<AbstractFunction>> *m_currentMenu = &m_rootMenu;
    std::vector<std::unique_ptr<AbstractFunction>> m_rootMenu;
    int m_curFuncIdx = 0;
    int m_dateFuncIdx = 0;
    int m_temperatureFuncIdx = 0;
    Stopwatch *m_stopwatchFunc = nullptr;
    Countdown *m_countdownFunc = nullptr;

    // Remember the last used time function to scroll to it when auto scroll is enabled.
    int m_lastUsedTimeFunction = 0;
    
    // Horizontal Scrolling.
    enum HorizScrollPhase
    {
        NoHorizScrolling,
        LeftPause,
        MovingRight,
        RightPause,
        MovingLeft,
        RightPauseAfterValueChange
    } m_horizScrollPhase = NoHorizScrolling;
    int m_horizScrollPos = 0;
    int m_horizScrollPauseCounter = 0;
    CyclicCounter m_horizScrollFrameCounter {Display::FRAME_RATE / 25}; // To slow down scrolling by moving every N frames

    // Vertical scrolling
    int m_vertScrollPos = 0;
    int m_vertScrollDir = 0;
    CyclicCounter m_vertScrollFrameCounter {Display::FRAME_RATE / 25};

    template <class FunctionType, typename... CtorParams>
    int addFunction(CtorParams... ctorParams);

    template <class FunctionType, typename... CtorParams>
    FunctionType *addFunctionAndReturnPtr(CtorParams... ctorParams);

    void onFrameCallback();
    void renderFrame();
    void onSetButtonPressed();
    void onUpOrDownButtonPressed(AbstractFunction::Direction direction);
    bool onAnyButtonTouched(); // Return true if nothing else shall be done
    void editValues();
    void renderHorizScrollingText(
        Bitmap &frame,
        bool fullRefresh,
        const std::string &leftText, 
        const std::string &editedValue = "", 
        const std::string &rightText = "");
    void initHorizScrolling();
    void updateHorizScrolling(int scrollTextWidth);
    void bringHorizScrollingToRight();
    void startVertScrolling(int dir);
    bool hourlyChimeActive() const;
    void adjustBrightness();
    void handleControlFromConsole();
    void renderIndicators();
};