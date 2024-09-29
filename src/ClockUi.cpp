#include "ClockUi.h"
#include "Utils/Trace.h"
#include "UiTexts.h"

#include "PicoClockHw/Display.h"
#include "PicoClockHw/Platform.h"

#include "Functions/Action.h"
#include "Functions/Alarm.h"
#include "Functions/Countdown.h"
#include "Functions/Date.h"
#include "Functions/Options.h"
#include "Functions/AlarmSubmenu.h"
#include "Functions/SkipNextAlarm.h"
#include "Functions/Stopwatch.h"
#include "Functions/Submenu.h"
#include "Functions/Time.h"
#include "Functions/Temperature.h"
#include "Functions/WifiStatus.h"

#include <iostream>
#include <iomanip>

namespace
{
    const int BUTTON_REPEAT_DELAY = 500;
    const int DIM_AMBIENT_LIGHT = 10; // As percentage
    const int BRIGHTNESS_BOOST_AFTER_USER_INPUT_FOR_SEC = 5;
    const float BRIGHTNESS_BOOST_PERCENT = 20;
    const int STOP_RINGING_AFTER_SEC = 60 * 5; // Stop ringing after 5 minutes
    const int AUTO_SCROLL_DELAY_SEC = 20;
}

// Make m_clock tick at the display frame rate, so that calculations are simpler.
ClockUi::ClockUi() : m_clock(Display::FRAME_RATE)
{
    TRACE << "Constructor";

    // Bind buttons callbacks to handlers
    m_setButton.setPressedCallback(std::bind(&ClockUi::onSetButtonPressed, this));
    m_upButton.setPressedCallback(std::bind(&ClockUi::onUpOrDownButtonPressed, this, AbstractFunction::Up));
    m_upButton.setRepeatCallback(
        std::bind(&ClockUi::onUpOrDownButtonPressed, this, AbstractFunction::RepeatedUp), BUTTON_REPEAT_DELAY);
    m_downButton.setPressedCallback(std::bind(&ClockUi::onUpOrDownButtonPressed, this, AbstractFunction::Down));
    m_downButton.setRepeatCallback(
        std::bind(&ClockUi::onUpOrDownButtonPressed, this, AbstractFunction::RepeatedDown), BUTTON_REPEAT_DELAY);

    // Consider settings that were just read from the flash memory
    m_curFuncIdx = m_settings.get().function;
    initHorizScrolling(); // If the selected function needs to scroll
    m_clock.setAlarm(Clock::Alarm1, m_settings.get().alarm1);
    m_clock.setAlarm(Clock::Alarm2, m_settings.get().alarm2);

    TRACE << "Add root level functions";
    addFunction<Time>(Time::HourMinSec);
    int hourMinBarFuncIdx = addFunction<Time>(Time::HourMinBar);
    addFunction<Time>(Time::HourMin);
    m_dateFuncIdx = addFunction<Date>();
    m_temperatureFuncIdx = addFunction<Temperature>();
    Submenu *alarmSubmenu =
        addFunctionAndReturnPtr<AlarmSubmenu>(&m_rootMenu);
    Submenu *countdownSubmenu = 
        addFunctionAndReturnPtr<Submenu>(uiText(TextId::Countdown), &m_rootMenu);
    Submenu *stopwatchSubmenu = 
        addFunctionAndReturnPtr<Submenu>(uiText(TextId::Stopwatch), &m_rootMenu);
    addFunction<WifiStatus>();
    addFunction<Options>();

    TRACE << "Add functions of the alarm submenu";
    alarmSubmenu->addFunction<SkipNextAlarm>(this);
    alarmSubmenu->addFunction<Alarm>(this, Alarm::Alarm1);
    alarmSubmenu->addFunction<Alarm>(this, Alarm::Alarm2);

    TRACE << "Add functions of the countdown submenu";
    m_countdownFunc = countdownSubmenu->addFunction<Countdown>(this, countdownSubmenu->menu());
    countdownSubmenu->addFunction<Action>(
        this, uiText(TextId::Set), std::bind(&Countdown::set, m_countdownFunc));

    TRACE << "Add functions of the stopwatch submenu";
    m_stopwatchFunc = stopwatchSubmenu->addFunction<Stopwatch>(this);
    stopwatchSubmenu->addFunction<Action>(
        this, uiText(TextId::Reset), std::bind(&Stopwatch::reset, m_stopwatchFunc));

    // Remember the last used time function in case auto scroll is enabled.
    if (m_currentMenu->at(m_curFuncIdx)->isTimeFunction())
        m_lastUsedTimeFunction = m_curFuncIdx;
    else
        m_lastUsedTimeFunction = hourMinBarFuncIdx; // Default to hour:min:bar

    m_currentMenu->at(m_curFuncIdx)->onSelect();

    // Setup display and turn it on
    m_frameBuffer.setDrawOrigin(Display::MATRIX_LEFT, Display::MATRIX_TOP);
    adjustBrightness();

    TRACE << "Constructed";
}

template <class FunctionType, typename... CtorParams>
int ClockUi::addFunction(CtorParams... ctorParams)
{
    m_rootMenu.push_back(std::make_unique<FunctionType>(this, ctorParams...));
    
    // Return the index of the element that was just added.
    return m_rootMenu.size() - 1;
}

template <class FunctionType, typename... CtorParams>
FunctionType *ClockUi::addFunctionAndReturnPtr(CtorParams... ctorParams)
{
    auto uniquePtr = std::make_unique<FunctionType>(this, ctorParams...);
    FunctionType *rawPtr = uniquePtr.get();

    m_rootMenu.push_back(std::move(uniquePtr));

    return rawPtr;
}

void ClockUi::onFrameCallback()
{
    // Make the clock and some functions tick
    bool clockAdjusted = false;
    Clock::AlarmId reachedAlarm = Clock::NoAlarm;
    m_clock.tick(clockAdjusted, reachedAlarm, m_settings);
    m_countdownFunc->tick();
    m_stopwatchFunc->tick();

    // Make the display get refreshed if time did not just advance normally.
    if (clockAdjusted)
        m_forceRefresh = true;
    
    // Start ringing if an alarm was reached.
    if (reachedAlarm != Clock::NoAlarm)
    {
        m_alarmRinging = 
            reachedAlarm == Clock::Alarm1 ? 
            m_settings.get().alarm1.mode : m_settings.get().alarm2.mode;
        m_ringingForSecs = 0;
    }

    adjustBrightness();

    if (m_clock.tickCount() == 0)
    {
        if (m_alarmRinging != Settings::AlarmMode::Off)
        {
            m_ringingForSecs++;
            
            if (m_alarmRinging == Settings::AlarmMode::Gradual)
                m_buzzer.beepForMs(m_ringingForSecs);
            else
                m_buzzer.beepForMs(500);

            if (m_ringingForSecs >= STOP_RINGING_AFTER_SEC)
                m_alarmRinging = Settings::AlarmMode::Off;
        }
        else if (hourlyChimeActive() && m_clock.get().tm_min == 0 && m_clock.get().tm_sec == 0)
        {
            TRACE << "Hourly chime";
            m_buzzer.beepForMs(100);
        }

        // Autoscroll if enabled, not editing something, the user is in the root menu and has not 
        // touched any button for a while.
        m_secondsWithoutUserInput++;
        if (m_settings.get().autoScroll && 
            m_editedValueIndex == NoEditing && 
            m_currentMenu == &m_rootMenu &&
            m_secondsWithoutUserInput >= AUTO_SCROLL_DELAY_SEC)
        {
            switch(m_clock.get().tm_sec)
            {
                case 0:
                    m_curFuncIdx = m_lastUsedTimeFunction;
                    startVertScrolling(-1);
                    break;

                case AUTO_SCROLL_DELAY_SEC:
                    m_curFuncIdx = m_dateFuncIdx;
                    startVertScrolling(-1);
                    break;

                case AUTO_SCROLL_DELAY_SEC * 2:
                    // Show the temperature if the RTC is available, otherwise go back to time.
                    if (m_clock.rtc() != nullptr)
                       m_curFuncIdx = m_temperatureFuncIdx;
                    else
                        m_curFuncIdx = m_lastUsedTimeFunction;

                    startVertScrolling(-1);
                    break;
            }
        }
    }

    handleControlFromConsole();
    renderFrame();
}

void ClockUi::renderFrame()
{
    // If vertically scroling, update m_vertScrollPos at the right frequency.
    if (m_vertScrollDir != 0 && m_vertScrollFrameCounter.increment())
    {
        m_vertScrollPos += m_vertScrollDir;
        if (m_vertScrollPos > Display::MATRIX_HEIGHT || m_vertScrollPos < -Display::MATRIX_HEIGHT)
        {
            // Vertical scrolling finished as the new function is reached.
            m_vertScrollPos = 0;
            m_vertScrollDir = 0;
            m_forceRefresh = true;
        }
    }

    // Scroll the display down or up if a vertical scrolling is ongoing.
    AbstractFunction &curFunc = *m_currentMenu->at(m_curFuncIdx);
    if (m_vertScrollPos > 0)
    {
        if (m_vertScrollFrameCounter == 0)
        {
            // Move the view of the previous function down
            m_frameBuffer.moveRectangle(
                0, 
                m_vertScrollPos - 1, 
                Display::MATRIX_WIDTH - 1, 
                Display::MATRIX_HEIGHT - 2,
                1);

            // Draw the empty line between functions
            m_frameBuffer.drawRectangle(
                0, m_vertScrollPos - 1, Display::MATRIX_WIDTH - 1, m_vertScrollPos - 1, false);

            // Render the new function into a temporary bitmap and partially copy it to the frame buffer
            Bitmap b;
            b.setDrawOrigin(Display::MATRIX_LEFT, Display::MATRIX_TOP);
            curFunc.renderFrame(b, m_editedValueIndex, m_blinkingCounter, true);
            b.copyRectangle(
                0, 
                Display::MATRIX_HEIGHT - m_vertScrollPos + 1,
                Display::MATRIX_WIDTH - 1, 
                Display::MATRIX_HEIGHT -1,
                m_frameBuffer,
                0
            );
        }
    } 
    else if (m_vertScrollPos < 0)
    {
        if (m_vertScrollFrameCounter == 0)
        {
            // Move the view of the previous function up
            m_frameBuffer.moveRectangle(
                0, 
                1, 
                Display::MATRIX_WIDTH - 1, 
                Display::MATRIX_HEIGHT + m_vertScrollPos,
                -1);

            // Draw the empty line between functions
            m_frameBuffer.drawRectangle(
                0, 
                Display::MATRIX_HEIGHT + m_vertScrollPos, 
                Display::MATRIX_WIDTH - 1, 
                Display::MATRIX_HEIGHT + m_vertScrollPos, 
                false);

            // Render the new function into a temporary bitmap and partially copy it to the frame buffer
            Bitmap b;
            b.setDrawOrigin(Display::MATRIX_LEFT, Display::MATRIX_TOP);
            curFunc.renderFrame(b, m_editedValueIndex, m_blinkingCounter, true);
            b.copyRectangle(
                0, 
                0,
                Display::MATRIX_WIDTH - 1, 
                -m_vertScrollPos - 2,
                m_frameBuffer,
                Display::MATRIX_HEIGHT + 1 + m_vertScrollPos
            );
        }
    }
    else
    {
        // Render directly to the frame buffer if no vertical scrolling is ongoing.
        if (m_vertScrollDir == 0)
            curFunc.renderFrame(m_frameBuffer, m_editedValueIndex, m_blinkingCounter, m_forceRefresh);
    }

    renderIndicators();

    if (m_editedValueIndex != NoEditing)
        m_blinkingCounter.increment();

    m_forceRefresh = false;
}

void ClockUi::renderIndicators()
{
    // Render simple indicators
    m_frameBuffer.putIndicator(Bitmap::MoveOn, m_settings.get().autoScroll);
    m_frameBuffer.putIndicator(Bitmap::Hourly, hourlyChimeActive());
    m_frameBuffer.putIndicator(Bitmap::AutoLight, m_settings.get().autoLight);

    // Render "Alarm on" indicator, blinking slowly if "Skip next alarm" is active
    bool alarmOnIndicator;
    if (m_clock.isAlarmOn())
    {
        if (m_settings.get().skipNextAlarm)
            alarmOnIndicator = m_clock.get().tm_sec % 2 != 0;
        else
            alarmOnIndicator = true;
    } else
        alarmOnIndicator = false;
    m_frameBuffer.putIndicator(Bitmap::AlarmOn, alarmOnIndicator);
}

void ClockUi::handleControlFromConsole()
{
    // This method allows interacting with the program via the serial I/O, for example using Tera Term.
    // It even works if the Pico is not in the Pico Clock Green device.

    // Enable this section to display all pixels on the standard output in ASCII.
#if 0
    if (m_clock.tickCount() % 4 == 0)
    {
        for (int y = 0; y < Display::HEIGHT; y++)
        {
            std::string s;
            for (int x = 0; x < Display::WIDTH; x++)
            {
                if (m_frameBuffer.pixel(x, y))
                    s += "#";
                else
                    s += " ";
            }
            std::cout << s <<std::endl;
        }
        std::cout << std::endl;
    }
#endif

    // Enable this section to simulate the three buttons using the standard input. Enter triggers SET
    // and the arrow keys trigger UP and DOWN.
#ifdef SIMULATE_BUTTONS_FROM_STDIO
    int c = Platform::getCharNonBlocking();
    switch (c)
    {
        case 66: // Upwards Arrow
            onUpOrDownButtonPressed(AbstractFunction::Down);
            break;
        case 65: // Downwards Arrow
            onUpOrDownButtonPressed(AbstractFunction::Up);
            break;
        case 13: // Enter
            onSetButtonPressed();
            break;
    }
#endif
}

bool ClockUi::hourlyChimeActive() const
{
    switch (m_settings.get().hourlyChime)
    {
        case Settings::HourlyChimeMode::Off:
            return false;
        case Settings::HourlyChimeMode::On:
            return true;
        case Settings::HourlyChimeMode::OnDayLight:
            return m_dayLight;
    }
    return false;
}

void ClockUi::adjustBrightness()
{
    float ambientLight = m_display.ambientLight();

    m_dayLight = ambientLight >= DIM_AMBIENT_LIGHT;

    if (m_settings.get().autoLight)
    {
        float brightness = 0;
        if (m_dayLight)
        {
            // Interpolate between "brightness dim" and "brightness bright"
            brightness =
                m_settings.get().brightnessDim + (ambientLight - DIM_AMBIENT_LIGHT) * (m_settings.get().brightnessBright - m_settings.get().brightnessDim) / (100 - DIM_AMBIENT_LIGHT);
        } else
        {
            // Interpolate between "brightness dark" and "brightness dim"
            brightness = 
                m_settings.get().brightnessDark + ambientLight * (m_settings.get().brightnessDim - m_settings.get().brightnessDark) / DIM_AMBIENT_LIGHT;

            if (
                m_secondsWithoutUserInput < BRIGHTNESS_BOOST_AFTER_USER_INPUT_FOR_SEC &&
                m_currentMenu->at(m_curFuncIdx)->allowsBrightnessBoost(m_editedValueIndex))
            {
                // Temporarily increase brightness after user input
                brightness = 
                    std::min(brightness + BRIGHTNESS_BOOST_PERCENT, static_cast<float>(m_settings.get().brightnessDim));
            }
        }
            
        m_display.setBrightness(brightness);
    } else
    {
        // TODO: do not continuously set the brightness
        m_display.setBrightness(m_settings.get().manualBrightness);
    }
}

void ClockUi::renderHorizScrollingText(
        Bitmap &frame,
        bool fullRefresh,
        const std::string &leftText, 
        const std::string &editedValue, 
        const std::string &rightText)
{
    frame.setFont(&narrowFont);

    int scrollTextWidth = frame.textWidth(leftText + editedValue + rightText);

    // Move to the right after the value has changed so that the user can read it
    if (m_horizScrollPhase == RightPauseAfterValueChange)
    {
        if (frame.textWidth(editedValue) <= Display::MATRIX_WIDTH)
        {
            // If the edited value fits on the display, scroll to the end of the text
            m_horizScrollPos = 
                std::max(m_horizScrollPos, scrollTextWidth - (Display::MATRIX_WIDTH));
        } else
        {
            // If the edit value is wider than the display, scroll to its beginning and change state
            // to leftpause so that scrolling will continue to the right.
            m_horizScrollPos = frame.textWidth(leftText);
            m_horizScrollPhase = LeftPause;
        }
        
        m_horizScrollFrameCounter = 0;

        // Reset the scroll pause counter so that the user has time to read.
        m_horizScrollPauseCounter = 0;
    }

    if (m_horizScrollPhase == RightPauseAfterValueChange)
        m_horizScrollPhase = RightPause;

    if (fullRefresh ||
        (m_horizScrollPhase == MovingRight || m_horizScrollPhase == MovingLeft) && m_horizScrollFrameCounter == 0 ||
        m_horizScrollPauseCounter == 0 ||
        m_blinkingCounter == 0 ||
        m_blinkingCounter == AbstractFunction::BLINKING_DISAPPEAR_FRAME)
    {
        if (fullRefresh)
            frame.clear();
        else
        {
            // Clear the frame buffer, but not the first row (week days), as the DMA controller may 
            // be tranferring this row at the same time, which would cause glitches on it when 
            // editing alarm weekdays.
            frame.drawRectangle(
                0, 0, Display::MATRIX_WIDTH, Display::MATRIX_HEIGHT, false);
        }

        if (editedValue.empty() || m_blinkingCounter < AbstractFunction::BLINKING_DISAPPEAR_FRAME)
        {
            TRACE << "Draw the scrolling text:" << leftText + editedValue + rightText;
            frame.drawText(-m_horizScrollPos, 0, leftText + editedValue + rightText);
        } else
        {
            int x = -m_horizScrollPos;
            x += frame.drawText(x, 0, leftText);
            
            // Skip the value which is currently not visible due to blinking.
            x += frame.textWidth(editedValue) + 2;

            frame.drawText(x, 0, rightText);
        }

        // Clear indicators on the left as the scrolled text may go into this area.
        frame.drawRectangle(-2, 0, -1, 6, false);
    }

    updateHorizScrolling(scrollTextWidth);
}

void ClockUi::updateHorizScrolling(int scrollTextWidth)
{
    switch (m_horizScrollPhase)
    {
        case LeftPause:
            m_horizScrollPauseCounter++;
            if (m_horizScrollPauseCounter >= Display::FRAME_RATE * 2)
                m_horizScrollPhase = MovingRight;

            break;
        case MovingRight:
            if (m_horizScrollFrameCounter.increment())
            {
                if (m_horizScrollPos < scrollTextWidth - (Display::MATRIX_WIDTH))
                    m_horizScrollPos++;
                else
                {
                    m_horizScrollPhase = RightPause;
                    m_horizScrollPauseCounter = 0;
                }
            }
            break;
        case RightPause:
            m_horizScrollPauseCounter++;
            if (m_horizScrollPauseCounter >= Display::FRAME_RATE * 2)
                m_horizScrollPhase = MovingLeft;
            break;
        case MovingLeft:
            if (m_horizScrollFrameCounter.increment())
            {
                if (m_horizScrollPos > 0)
                    m_horizScrollPos--;
                else
                {
                    m_horizScrollPhase = LeftPause;
                    m_horizScrollPauseCounter = 0;
                }                
            }
            break;
    }
}

void ClockUi::onUpOrDownButtonPressed(AbstractFunction::Direction direction)
{
    if (onAnyButtonTouched())
        return;

    if (m_editedValueIndex == NoEditing)
    {
        do
        {
            // Navigate up and down through the menu
            if (direction == AbstractFunction::Up || direction ==AbstractFunction:: RepeatedUp)
            {
                if (m_curFuncIdx <= 0)
                    m_curFuncIdx = m_currentMenu->size() - 1;
                else
                    m_curFuncIdx--;

                startVertScrolling(1);
            }
            else if (direction == AbstractFunction::Down || direction == AbstractFunction::RepeatedDown)
            {
                if (m_curFuncIdx >= m_currentMenu->size() - 1)
                    m_curFuncIdx = 0;
                else
                    m_curFuncIdx++;

                startVertScrolling(-1);
            }
        } // Skip currently unavailable functions
        while (!m_currentMenu->at(m_curFuncIdx)->isAvailable());

        // If we are in the root menu, save the current function in the settings so that it will be 
        // restored on the next power up.
        if (m_currentMenu == &m_rootMenu)
        {
            m_settings.modify().function = m_curFuncIdx;

            // Also remember the last used time function is case auto scroll is enabled
            if (m_currentMenu->at(m_curFuncIdx)->isTimeFunction())
                m_lastUsedTimeFunction = m_curFuncIdx;
        }

        // Initialize the scrolling for functions that use it. For other functions, it has no effect.
        initHorizScrolling();

        m_currentMenu->at(m_curFuncIdx)->onSelect();
    } 
    else
    {
        m_currentMenu->at(m_curFuncIdx)->modifyValue(m_editedValueIndex, direction);

        // Reset blinking to make the changed value appear immediately.
        m_blinkingCounter = 0;

        bringHorizScrollingToRight();
    }
}

void ClockUi::initHorizScrolling()
{
    m_horizScrollPhase = LeftPause;
    m_horizScrollPauseCounter = 0;
    m_horizScrollFrameCounter = 0;
    m_horizScrollPos = 0;
}

void ClockUi::startVertScrolling(int dir)
{
    // Adapt scroll position if the scrolling direction is changed while currently scrolling.
    if (m_vertScrollDir < 0 && dir > 0)
        m_vertScrollPos += Display::MATRIX_HEIGHT + 1;
    if (m_vertScrollDir > 0 && dir < 0)
        m_vertScrollPos -= Display::MATRIX_HEIGHT + 1;

    m_vertScrollDir = dir;
}

void ClockUi::onSetButtonPressed()
{
    if (onAnyButtonTouched())
        return;

    auto &curFunc = *m_currentMenu->at(m_curFuncIdx);
    curFunc.activate(); // May call editValues
}

void ClockUi::editValues()
{
    m_editedValueIndex++;
    auto &curFunc = *m_currentMenu->at(m_curFuncIdx);
    if (m_editedValueIndex < curFunc.valueCount())
    {
        initHorizScrolling(); // In case something will scroll
        curFunc.startEditingValue(m_editedValueIndex);
    }
    else
    {
        initHorizScrolling(); // In case something will scroll
        m_editedValueIndex = 0;
        m_blinkingCounter = -1; // Stop blinking and prevent unnecessary rendering
        curFunc.finishEditing();
    }
    m_forceRefresh = true;
}

void ClockUi::bringHorizScrollingToRight()
{
    // Bring or keep the scrolling to the right so that the user sees the 
    // value that just changed.
    m_horizScrollPhase = RightPauseAfterValueChange;
}

bool ClockUi::onAnyButtonTouched()
{
    m_secondsWithoutUserInput = 0;

    if (m_alarmRinging != Settings::AlarmMode::Off)
    {
        m_alarmRinging = Settings::AlarmMode::Off;
        return true; // Request not to do anything else
    }

    if (m_countdownFunc->stopRinging())
        return true; // Request not to do anything else

    return false;
}
