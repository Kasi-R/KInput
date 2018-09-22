#ifndef KINPUT_HPP_INCLUDED
#define KINPUT_HPP_INCLUDED

#include <jni.h>
#include <cstdint>

class KInput
{
    private:
        bool Initialized;
        JavaVM* JVM;
        JNIEnv* Thread;
        jobject Client;
        jobject Canvas;

        jclass Canvas_Class;
        jmethodID Canvas_DispatchEvent;

        jclass FocusEvent_Class;
        jmethodID FocusEvent_Init;

        jclass KeyEvent_Class;
        jmethodID KeyEvent_Init;

        jclass MouseEvent_Class;
        jmethodID MouseEvent_Init;

        jclass MouseWheelEvent_Class;
        jmethodID MouseWheelEvent_Init;

        bool AttachThread();
        void GrabCanvas();
    public:
        KInput();

        bool DispatchEvent(jobject Event);
        bool FocusEvent(std::int32_t ID);
        bool KeyEvent(std::int32_t ID, std::int64_t When, std::int32_t Modifiers, std::int32_t KeyCode,
                      std::uint16_t KeyChar, std::int32_t KeyLocation);
        bool MouseEvent(std::int32_t ID, std::int64_t When, std::int32_t Modifiers, std::int32_t X,
                        std::int32_t Y, std::int32_t ClickCount, bool PopupTrigger, std::int32_t Button);
        bool MouseWheelEvent(std::int32_t ID, std::int64_t When, std::int32_t Modifiers, std::int32_t X,
                             std::int32_t Y, std::int32_t ClickCount, bool PopupTrigger, std::int32_t ScrollType,
                             std::int32_t ScrollAmount, std::int32_t WheelRotation);
        ~KInput();
};

#endif // KINPUT_HPP_INCLUDED
