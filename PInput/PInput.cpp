/*
    Copyright (C) <2018>  <Andre>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <chrono>
#include <random>
#include <algorithm>
#include <iostream>
#include "PInput.hpp"

/**
 * Constants
 */

static std::default_random_engine generator;

/**
 * Generated using the following Python code:
 *
 * def PrintTypeable():
 *     Typeable = [0] * 0xff
 *     Typeable[32] = 32
 *     for c in xrange(ord('A'), ord('Z') + 1):
 *         Typeable[c] = c
 *     for c in xrange(ord('0'), ord('9') + 1):
 *         Typeable[c] = c
 *     Typeable[186] = ord(';')  #  ;:
 *     Typeable[187] = ord('=')  #  =+
 *     Typeable[188] = ord(',')  # hack: ,
 *     Typeable[189] = ord('-')  #  -_
 *     Typeable[190] = ord('.')  #  .>
 *     Typeable[191] = ord('/')  #  /?
 *     Typeable[192] = ord('`')  #  `~
 *     Typeable[219] = ord('[')  #  [{
 *     Typeable[220] = ord('\\') #  # \|
 *     Typeable[221] = ord(']')  #  ]}
 *     Typeable[222] = ord('\'') #  '"
 *     Typeable[226] = ord(',')  # hack: <
 *     print Typeable
 *
 * Based of https://github.com/BenLand100/SMART/blob/master/src/EventNazi.java#L432
 */
static std::uint32_t TypableKeyCodes[] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 32, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 0, 0, 0, 0, 0, 0, 0, 65, 66, 67, 68,
        69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 59, 61, 44, 45, 46, 47, 96, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 91, 92, 93, 39, 0, 0, 0, 44, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

/**
 * Translates the Simba mouse position to Mouse events
 */
static std::int32_t MouseTranslation[] = {3, 1, 2};

/**
 * Utilities
 */

/**
 * Returns the current time millis.
 * @return Current time millis.
 */
std::uint64_t CurrentTimeMillis()
{
    return static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count());
}

/**
 * Generates a random integer between Min and Max.
 * @param Min The minimum bound.
 * @param Max The maximum bound.
 * @return The random integer.
 */
std::int32_t Random(std::int32_t Min, std::int32_t Max)
{
    std::uniform_int_distribution<int> distribution(Min, Max);
    return distribution(generator);
}

void Wait(std::int32_t Amount)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(Amount));
}

void Wait(std::int32_t Min, std::int32_t Max)
{
    Wait(Random(Min, Max));
}

/**
 * Converts a char into a KeyCode value for KeyEvent
 * @param c Char to convert
 * @return c's KeyCode
 */
std::int32_t ToKeyCode(char c)
{
    std::string special = "~!@#$%^&*()_+|{}:\"<>?";
    std::string normal = "`1234567890-=\\[];',./";
    std::int64_t index = special.find(c);
    return std::toupper(index == std::string::npos ? c : normal[index]);
}

/**
 * Converts a vk code into a char
 * @param code KeyCode to convert
 * @return the char
 */
std::uint16_t ToChar(std::int32_t KeyCode, bool Shift)
{
    if (KeyCode == VK_SHIFT)
    {
        return CHAR_UNDEFINED;
    }
    uint32_t code = TypableKeyCodes[KeyCode];
    std::string special = "~!@#$%^&*()_+|{}:\"<>?";
    std::string normal = "`1234567890-=\\[];',./";
    std::int64_t index = normal.find((char) code);
    if (index == std::string::npos)
    {
        return static_cast<uint16_t>(static_cast<int16_t>(Shift ? std::toupper((char) code) : std::tolower((char) code)));
    }
    else
    {
        return static_cast<uint16_t>(Shift ? special[index] : code);
    }
}

/**
 * Returns true if the key code is type-able
 */
bool IsKeyCodeTypeable(std::int32_t KeyCode)
{
    return KeyCode < 0xff && TypableKeyCodes[KeyCode] != 0;
}

/**
 * Tests if a character requires the shift key to be pressed.
 * @param c Char to check for.
 * @return True if shift is required.
 */
bool RequiresShift(char c)
{
    std::string special = "~!@#$%^&*()_+|{}:\"<>?";
    return special.find(c) != std::string::npos || (c - 'A' >= 0 && c - 'A' <= 25);
}

void ReplaceJavaEnter(std::int32_t *KeyCode)
{
    // Java uses VK_ENTER = 10
    if (*KeyCode == 13)
    {
        *KeyCode = 10;
    }
}

/**
 * Class definitions
 */

PInput::PInput(DWORD PID, std::string &Path)
{
    this->KInput = new KInputCtrl(PID, Path);
    this->KeysHeld = std::set<HeldKey>();
    this->RunningKeySender = true;
    this->KeySenderThread = std::thread(&PInput::KeySenderRun, this);
    this->ShiftDown = false;
    std::mutex KeysHeldLock;
    this->Focused = false;
    this->LeftDown = false;
    this->MidDown = false;
    this->RightDown = false;
    this->X = -1;
    this->Y = -1;
}

bool PInput::IsKeyDown(std::int32_t KeyCode)
{
    ReplaceJavaEnter(&KeyCode);
    KeysHeldLock.lock();
    auto result = (KeysHeld.find(HeldKey(KeyCode)) != KeysHeld.end());
    KeysHeldLock.unlock();
    return result;
}

/**
 * Keyboard
 */

void PInput::SetKeyDown(HeldKey Key, bool Held)
{
    KeysHeldLock.lock();
    if (Held)
    {
        KeysHeld.insert(Key);
    }
    else
    {
        KeysHeld.erase(Key);
    }
    KeysHeldLock.unlock();
}

bool PInput::KeyDown(std::int32_t KeyCode)
{
    ReplaceJavaEnter(&KeyCode);
    if (!IsFocused())
    {
        GainFocus();
    }
    std::uint64_t StartTime = CurrentTimeMillis();
    HeldKey KeyHeld(KeyCode, StartTime & 0xFFFFFFFF);
    if (!this->IsKeyDown(KeyCode))
    {
        if (VK_SHIFT == KeyCode)
        {
            this->ShiftDown = true;
        }

        KInput->KeyEvent(KEY_PRESSED, StartTime, ShiftDown ? SHIFT_DOWN_MASK : 0, KeyCode, ToChar(KeyCode, ShiftDown), KEY_LOCATION_STANDARD);
        if (IsKeyCodeTypeable(KeyCode))
        {
            KInput->KeyEvent(KEY_TYPED, StartTime, ShiftDown ? SHIFT_DOWN_MASK : 0, 0, ToChar(KeyCode, ShiftDown), KEY_LOCATION_UNKNOWN);
        }
        this->SetKeyDown(KeyHeld, true);
        return true;
    }
    return false;
}

bool PInput::KeyUp(std::int32_t KeyCode)
{
    ReplaceJavaEnter(&KeyCode);
    if (!IsFocused())
    {
        GainFocus();
    }
    HeldKey KeyHeld(KeyCode);
    if (IsKeyDown(KeyCode))
    {
        SetKeyDown(KeyHeld, false);
        KInput->KeyEvent(KEY_RELEASED, CurrentTimeMillis(), ShiftDown ? SHIFT_DOWN_MASK : 0, KeyCode, ToChar(KeyCode, ShiftDown), KEY_LOCATION_STANDARD);
        if (VK_SHIFT == KeyCode)
        {
            ShiftDown = false;
        }
        return true;
    }
    return false;
}

void PInput::KeySenderRun()
{
    while (RunningKeySender)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(30 + Random(0, 5)));
        auto RepeatDelay = static_cast<uint32_t>((CurrentTimeMillis() & 0xFFFFFFFF) - 500);
        KeysHeldLock.lock();
        for (auto KeyHeld : KeysHeld)
        {
            if (KeyHeld.getSentTime() < RepeatDelay)
            {
                KInput->KeyEvent(KEY_PRESSED, CurrentTimeMillis(), ShiftDown ? SHIFT_DOWN_MASK : 0, KeyHeld.getKeyCode(), ToChar(KeyHeld.getKeyCode(), ShiftDown), KEY_LOCATION_STANDARD);
                if (IsKeyCodeTypeable(KeyHeld.getKeyCode()))
                {
                    KInput->KeyEvent(KEY_TYPED, CurrentTimeMillis(), ShiftDown ? SHIFT_DOWN_MASK : 0, 0, ToChar(KeyHeld.getKeyCode(), ShiftDown), KEY_LOCATION_UNKNOWN);
                }
            }
        }
        KeysHeldLock.unlock();
        std::this_thread::yield();
    }
    std::cout << "Killed Key Sender Gracefully!" << std::endl;
}


bool PInput::PressKey(std::int32_t KeyCode)
{
    if (!IsFocused())
    {
        GainFocus();
    }
    ReplaceJavaEnter(&KeyCode);
    // Craft the order of events
    this->KInput->KeyEvent(KEY_PRESSED, CurrentTimeMillis(), ShiftDown ? SHIFT_DOWN_MASK : 0, KeyCode, ToChar(KeyCode, ShiftDown), KEY_LOCATION_STANDARD);
    // 0 between the pressed and typed
    this->KInput->KeyEvent(KEY_TYPED, CurrentTimeMillis(), ShiftDown ? SHIFT_DOWN_MASK : 0, 0, ToChar(KeyCode, ShiftDown), KEY_LOCATION_UNKNOWN);
    // Random human release time
    std::this_thread::sleep_for(std::chrono::milliseconds(Random(45, 96)));
    this->KInput->KeyEvent(KEY_RELEASED, CurrentTimeMillis(), ShiftDown ? SHIFT_DOWN_MASK : 0, KeyCode, ToChar(KeyCode, ShiftDown), KEY_LOCATION_STANDARD);
    return true;
}

std::int32_t WeirdRandom(std::int32_t Base)
{
    std::uniform_real_distribution<double> distribution(0, 1);
    return static_cast<std::int32_t>(((distribution(generator) * 0.1 + 1) * Base));
}

bool PInput::SendKeys(std::string Text, std::int32_t KeyWait, std::int32_t KeyModWait)
{
    if (!IsFocused())
    {
        GainFocus();
    }
    std::uniform_real_distribution<double> distribution(0, 1);
    for (auto Char : Text)
    {
        auto KeyCode = ToKeyCode(Char);
        ReplaceJavaEnter(&KeyCode);
        auto KeyLocation = isdigit(Char) ? (distribution(generator) > 0.5 ? KEY_LOCATION_NUMPAD : KEY_LOCATION_STANDARD) : KEY_LOCATION_STANDARD;
        if (RequiresShift(Char))
        {
            auto ShiftLocation = distribution(generator) > 0.5 ? KEY_LOCATION_RIGHT : KEY_LOCATION_LEFT;
            KInput->KeyEvent(KEY_PRESSED, CurrentTimeMillis(), SHIFT_MASK, VK_SHIFT, CHAR_UNDEFINED, ShiftLocation);
            std::this_thread::sleep_for(std::chrono::milliseconds(WeirdRandom(KeyModWait)));
            auto Time = CurrentTimeMillis();
            KInput->KeyEvent(KEY_PRESSED, Time, SHIFT_MASK, KeyCode, static_cast<uint16_t>(Char), KeyLocation);
            KInput->KeyEvent(KEY_TYPED, Time, SHIFT_MASK, 0, static_cast<uint16_t>(Char), KEY_LOCATION_UNKNOWN);
            std::this_thread::sleep_for(std::chrono::milliseconds(WeirdRandom(KeyWait)));
            KInput->KeyEvent(KEY_RELEASED, CurrentTimeMillis(), SHIFT_MASK, KeyCode, static_cast<uint16_t>(Char), KeyLocation);
            std::this_thread::sleep_for(std::chrono::milliseconds(WeirdRandom(KeyModWait)));
            KInput->KeyEvent(KEY_RELEASED, CurrentTimeMillis(), 0, VK_SHIFT, CHAR_UNDEFINED, ShiftLocation);
        }
        else
        {
            auto Time = CurrentTimeMillis();
            KInput->KeyEvent(KEY_PRESSED, Time, 0, KeyCode, static_cast<uint16_t>(Char), KeyLocation);
            KInput->KeyEvent(KEY_TYPED, Time, 0, 0, static_cast<uint16_t>(Char), KEY_LOCATION_UNKNOWN);
            std::this_thread::sleep_for(std::chrono::milliseconds(WeirdRandom(KeyWait)));
            KInput->KeyEvent(KEY_RELEASED, CurrentTimeMillis(), 0, KeyCode, static_cast<uint16_t>(Char), KeyLocation);
        }
    }
    return true;
}

/**
 * Focus
 */

bool PInput::IsFocused()
{
    return Focused;
}

bool PInput::GainFocus()
{
    if (IsFocused())
    {
        return false;
    }
    KInput->FocusEvent(FOCUS_GAINED);
    Focused = true;
    std::this_thread::sleep_for(std::chrono::milliseconds(Random(100, 200)));
    return true;
}

bool PInput::LoseFocus(bool AltTab)
{
    if (!IsFocused())
    {
        return false;
    }
    if (AltTab)
    {
        KInput->KeyEvent(KEY_PRESSED, CurrentTimeMillis(), ALT_MASK, VK_ALT, CHAR_UNDEFINED, KEY_LOCATION_LEFT);
        std::this_thread::sleep_for(std::chrono::milliseconds(Random(10, 50)));
    }
    KInput->FocusEvent(FOCUS_LOST);
    KInput->FocusEvent(FOCUS_LOST);
    Focused = false;
    std::this_thread::sleep_for(std::chrono::milliseconds(Random(100, 200)));
    return true;
}

/**
 * Mouse
 */

bool PInput::GetMousePos(std::int32_t *X, std::int32_t *Y)
{
    *X = this->X;
    *Y = this->Y;
    return true;
}

bool PInput::IsMouseButtonDown(std::int32_t ClickType)
{
    ClickType = MouseTranslation[ClickType];
    switch (ClickType)
    {
        case 1:
            return LeftDown;
        case 2:
            return MidDown;
        case 3:
            return RightDown;
    }
    return false;
}

bool PInput::IsDragging()
{
    return LeftDown || MidDown || RightDown;
}

bool PInput::MoveMouse(std::int32_t X, std::int32_t Y)
{
    int ButtonMask = (LeftDown ? BUTTON1_DOWN_MASK : 0) | (RightDown ? (BUTTON3_DOWN_MASK | META_DOWN_MASK) : 0);
    if (IsDragging())
    {
        KInput->MouseEvent(MOUSE_DRAGGED, CurrentTimeMillis(), ButtonMask, X, Y, 0, false, 0);
    }
    else
    {
        KInput->MouseEvent(MOUSE_MOVEDD, CurrentTimeMillis(), ButtonMask, X, Y, 0, false, 0);
    }
    this->X = X;
    this->Y = Y;
    return true;
}

bool PInput::HoldMouse(std::int32_t X, std::int32_t Y, std::int32_t ClickType)
{
    if (!IsMouseButtonDown(ClickType))
    {
        // Since IsMouseButtonDown performs the translation as well we can only translate in here
        ClickType = MouseTranslation[ClickType];
        int ButtonMask = ((LeftDown || ClickType == 1) ? BUTTON1_DOWN_MASK : 0) | ((MidDown || ClickType == 2) ? (BUTTON2_DOWN_MASK | META_DOWN_MASK) : 0) | ((RightDown || ClickType == 3) ? (BUTTON3_DOWN_MASK | META_DOWN_MASK) : 0);
        int Button = 0;
        switch (ClickType)
        {
            case 1:
                Button = BUTTON1;
                break;
            case 2:
                Button = BUTTON2;
                break;
            case 3:
                Button = BUTTON3;
                break;
        }
        MoveMouse(X, Y);
        KInput->MouseEvent(MOUSE_PRESSED, CurrentTimeMillis(), ButtonMask, this->X, this->Y, 1, false, Button);
        if (!IsFocused())
        {
            Wait(25, 50);
            GainFocus();
        }
        switch (ClickType)
        {
            case 1:
                LeftDown = true;
                break;
            case 2:
                MidDown = true;
                break;
            case 3:
                RightDown = true;
                break;
        }
        return true;
    }
    return false;
}

bool PInput::ReleaseMouse(std::int32_t X, std::int32_t Y, std::int32_t ClickType)
{
    if (IsMouseButtonDown(ClickType))
    {
        // Since IsMouseButtonDown performs the translation as well we can only translate in here
        ClickType = MouseTranslation[ClickType];
        int ButtonMask = ((LeftDown || ClickType == 1) ? BUTTON1_DOWN_MASK : 0) | ((MidDown || ClickType == 2) ? (BUTTON2_DOWN_MASK | META_DOWN_MASK) : 0) | ((RightDown || ClickType == 3) ? (BUTTON3_DOWN_MASK | META_DOWN_MASK) : 0);
        int Button = 0;
        switch (ClickType)
        {
            case 1:
                Button = BUTTON1;
                break;
            case 2:
                Button = BUTTON2;
                break;
            case 3:
                Button = BUTTON3;
                break;
        }
        MoveMouse(X, Y);
        std::uint64_t Time = CurrentTimeMillis();
        KInput->MouseEvent(MOUSE_RELEASED, Time, ButtonMask, this->X, this->Y, 1, false, Button);
        KInput->MouseEvent(MOUSE_CLICKED, Time, ButtonMask, this->X, this->Y, 1, false, Button);
        switch (ClickType)
        {
            case 1:
                LeftDown = false;
                break;
            case 2:
                MidDown = false;
                break;
            case 3:
                RightDown = false;
                break;
        }
        return true;
    }
    return false;
}

bool PInput::ClickMouse(std::int32_t X, std::int32_t Y, std::int32_t ClickType)
{
    if (!IsMouseButtonDown(ClickType))
    {
        // Since IsMouseButtonDown performs the translation as well we can only translate in here
        ClickType = MouseTranslation[ClickType];
        int ButtonMask = ((LeftDown || ClickType == 1) ? BUTTON1_DOWN_MASK : 0) | ((MidDown || ClickType == 2) ? (BUTTON2_DOWN_MASK | META_DOWN_MASK) : 0) | ((RightDown || ClickType == 3) ? (BUTTON3_DOWN_MASK | META_DOWN_MASK) : 0);
        int Button = 0;
        switch (ClickType)
        {
            case 1:
                Button = BUTTON1;
                break;
            case 2:
                Button = BUTTON2;
                break;
            case 3:
                Button = BUTTON3;
                break;
        }
        MoveMouse(X, Y);
        KInput->MouseEvent(MOUSE_PRESSED, CurrentTimeMillis(), ButtonMask, this->X, this->Y, 1, false, Button);
        switch (ClickType)
        {
            case 1:
                LeftDown = true;
                break;
            case 2:
                MidDown = true;
                break;
            case 3:
                RightDown = true;
                break;
        }
        if (!Focused)
        {
            Wait(25, 50);
            GainFocus();
        }
        std::uniform_real_distribution<double> distribution(0, 1);
        Wait(static_cast<std::int32_t>(distribution(generator) * 56 + 90));
        std::uint64_t Time = CurrentTimeMillis();
        KInput->MouseEvent(MOUSE_RELEASED, Time, ButtonMask, this->X, this->Y, 1, false, Button);
        KInput->MouseEvent(MOUSE_CLICKED, Time, ButtonMask, this->X, this->Y, 1, false, Button);
        switch (ClickType)
        {
            case 1:
                LeftDown = false;
                break;
            case 2:
                MidDown = false;
                break;
            case 3:
                RightDown = false;
                break;
        }
        return true;
    }
    return false;
}

bool PInput::ScrollMouse(std::int32_t X, std::int32_t Y, std::int32_t Lines)
{
    int ButtonMask = (IsKeyDown(VK_SHIFT) ? SHIFT_MASK : 0) | (IsKeyDown(VK_ALT) ? ALT_MASK : 0) | (IsKeyDown(VK_CONTROL) ? CTRL_MASK : 0);
    MoveMouse(X, Y);
    KInput->MouseWheelEvent(MOUSE_WHEEL, CurrentTimeMillis(), ButtonMask, X, Y, 0, false, WHEEL_UNIT_SCROLL, abs(Lines), Lines < 0 ? -1 : 1);
    return true;
}

PInput::~PInput()
{
    delete this->KInput;
    RunningKeySender = false;
    KeySenderThread.join();
}
