//
// Created by Andre on 7/10/2018.
//

#ifndef PINPUT_HPP_INCLUDED
#define PINPUT_HPP_INCLUDED


#include <queue>
#include <set>
#include <thread>
#include <mutex>
#include "KInputCtrl.hpp"

/**
 * Java constants
 */

typedef enum Events
{
    KEY_TYPED = 400,
    KEY_PRESSED = 401,
    KEY_RELEASED = 402
} Event;

typedef enum KeyEvents
{
    CHAR_UNDEFINED = 65535,
    KEY_LOCATION_LEFT = 2,
    KEY_LOCATION_NUMPAD = 4,
    KEY_LOCATION_RIGHT = 3,
    KEY_LOCATION_STANDARD = 1,
    KEY_LOCATION_UNKNOWN = 0,
    VK_ALT = 18,
} KeyEvent;

typedef enum InputEvents
{
    ALT_MASK = 8,
    SHIFT_DOWN_MASK = 64,
    SHIFT_MASK = 1,
    BUTTON1_DOWN_MASK = 1024,
    BUTTON2_DOWN_MASK = 2048,
    BUTTON3_DOWN_MASK = 4096,
    META_DOWN_MASK = 256,
    CTRL_MASK = 2,
} InputEvent;

typedef enum FocusEvents
{
    FOCUS_GAINED = 1004,
    FOCUS_LOST = 1005,
} FocusEvent;

typedef enum MouseEvents
{
    BUTTON1 = 1,
    BUTTON2 = 2,
    BUTTON3 = 3,
    MOUSE_PRESSED = 501,
    MOUSE_DRAGGED = 506,
    MOUSE_CLICKED = 500,
    MOUSE_MOVEDD = 503,
    MOUSE_RELEASED = 502,
    MOUSE_WHEEL = 507,
} MouseEvent;

typedef enum MouseWheelEvents
{
    WHEEL_UNIT_SCROLL = 0,
} MouseWheelEvent;

class HeldKey
{
private:
    std::int32_t KeyCode;
    std::int64_t SentTime;
public:
    HeldKey(int32_t KeyCode, int64_t SentTime) : KeyCode(KeyCode), SentTime(SentTime)
    {}

    explicit HeldKey(int32_t KeyCode) : HeldKey(KeyCode, 0)
    {}

    bool operator<(const HeldKey &rhs) const
    {
        return getKeyCode() < rhs.getKeyCode();
    }

    bool operator>(const HeldKey &rhs) const
    {
        return rhs < *this;
    }

    bool operator<=(const HeldKey &rhs) const
    {
        return !(rhs < *this);
    }

    bool operator>=(const HeldKey &rhs) const
    {
        return !(*this < rhs);
    }

    int32_t getKeyCode() const
    {
        return KeyCode;
    }

    int64_t getSentTime() const
    {
        return SentTime;
    }
};

class PInput
{
private:
    KInputCtrl *KInput;

    // Keyboard
    std::set<HeldKey> KeysHeld;
    std::mutex KeysHeldLock;
    std::thread KeySenderThread;
    bool RunningKeySender;
    bool ShiftDown;

    // Focus
    bool Focused;

    // Mouse
    bool LeftDown;
    bool MidDown;
    bool RightDown;
    std::int32_t X, Y;
private:
    void SetKeyDown(HeldKey Key, bool Held);

    void KeySenderRun();

public:
    PInput(DWORD PID, std::string &Path);

    // Keyboard
    bool PressKey(std::int32_t KeyCode);

    bool IsKeyDown(std::int32_t KeyCode);

    bool KeyUp(std::int32_t KeyCode);

    bool KeyDown(std::int32_t KeyCode);

    bool SendKeys(std::string Text, std::int32_t KeyWait, std::int32_t KeyModWait);

    // Focus
    bool IsFocused();

    bool GainFocus();

    bool LoseFocus(bool AltTab);

    // Mouse
    bool GetMousePos(std::int32_t *X, std::int32_t *Y);

    bool IsMouseButtonDown(std::int32_t ClickType);

    bool IsDragging();

    bool MoveMouse(std::int32_t X, std::int32_t Y);

    bool HoldMouse(std::int32_t X, std::int32_t Y, std::int32_t ClickType);

    bool ReleaseMouse(std::int32_t X, std::int32_t Y, std::int32_t ClickType);

    bool ClickMouse(std::int32_t X, std::int32_t Y, std::int32_t ClickType);

    bool ScrollMouse(std::int32_t X, std::int32_t Y, std::int32_t Lines);

    ~PInput();
};

#endif //PINPUT_HPP_INCLUDED