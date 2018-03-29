#include "KInput.hpp"
#include <windows.h>
#include <vector>
#include <string>
#include <iostream>

static BOOL CALLBACK GetHWNDCurrentPID(HWND WindowHandle, LPARAM lParam)
{
    std::vector<HWND>* Temp = reinterpret_cast<std::vector<HWND>*>(lParam);
    DWORD PID = GetCurrentProcessId();
    DWORD HWNDPID;
    GetWindowThreadProcessId(WindowHandle, &HWNDPID);
    if (PID == HWNDPID)
        Temp->push_back(WindowHandle);
    return TRUE;
}

HWND GetCanvasHWND()
{
    HWND CanvasHandle = 0;
    std::vector<HWND> OSRSWindows;
    EnumWindows(GetHWNDCurrentPID, reinterpret_cast<LPARAM>(&OSRSWindows));
    if (OSRSWindows.size() > 0)
    {
        for (auto const& Window : OSRSWindows)
        {
            char ClassName[128];
            GetClassName(Window, ClassName, sizeof(ClassName));
            std::string WindowClassName(ClassName);
            if (WindowClassName == "SunAwtFrame")
            {
                CanvasHandle = Window;
                break;
            }
        }
        HWND Temp = CanvasHandle;
        CanvasHandle = nullptr;
        int C = 0;
        while (Temp)
        {
            Temp = GetWindow(Temp, GW_CHILD);
            if (Temp)
            {
                char ClassName[128];
                GetClassName(Temp, ClassName, sizeof(ClassName));
                std::string ChildClassName(ClassName);
                if (ChildClassName == "SunAwtCanvas")
                {
                    CanvasHandle = Temp;
                    C++;
                }
            }
        }
        if (C != 3)
            CanvasHandle = nullptr;
    }
    return CanvasHandle;
}

KInput::KInput()
{
    this->Initialized = false;
    this->JVM = nullptr;
    this->Thread = nullptr;
    this->Client = nullptr;
    this->Canvas = nullptr;
    this->Canvas_Class = nullptr;
    this->Canvas_DispatchEvent = nullptr;
    this->FocusEvent_Class = nullptr;
    this->FocusEvent_Init = nullptr;
    this->KeyEvent_Class = nullptr;
    this->KeyEvent_Init = nullptr;
    this->MouseEvent_Class = nullptr;
    this->MouseEvent_Init = nullptr;
    this->MouseWheelEvent_Class = nullptr;
    this->MouseWheelEvent_Init = nullptr;
    HMODULE JVMDLL = GetModuleHandle("jvm.dll");
    if (!JVMDLL)
        return;
    typedef int (*ptr_GCJavaVMs)(JavaVM **vmBuf, jsize bufLen, jsize * nVMs);
    ptr_GCJavaVMs GetJVMs = (ptr_GCJavaVMs)GetProcAddress(JVMDLL, "JNI_GetCreatedJavaVMs");
    GetJVMs(&(this->JVM), 1, nullptr);
    if (this->JVM)
    {
        HMODULE JAWTDLL = GetModuleHandle("jawt.dll");
        typedef jboolean (JNICALL *ptr_GetAWT)(JNIEnv* env, JAWT* awt);
        ptr_GetAWT GetAWT = (ptr_GetAWT)GetProcAddress(JAWTDLL, "_JAWT_GetAWT@8");
        if (this->AttachThread())
        {
            memset(&(this->Toolkit), 0, sizeof(this->Toolkit));
            this->Toolkit.version = JAWT_VERSION_1_4;
            GetAWT(this->Thread, &(this->Toolkit));
            HWND CanvasHWND = GetCanvasHWND();
            if (CanvasHWND)
            {
                jobject TempCanvas = this->Toolkit.GetComponent(this->Thread, (void*)CanvasHWND);
                if (TempCanvas)
                {
                    this->Canvas = this->Thread->NewGlobalRef(TempCanvas);
                    if (this->Canvas)
                    {
                        jclass CanvasClass = this->Thread->GetObjectClass(this->Canvas);
                        if (CanvasClass)
                        {
                            jmethodID Canvas_getParent = this->Thread->GetMethodID(CanvasClass, "getParent", "()Ljava/awt/Container;");
                            jobject TempClient = (jstring)this->Thread->CallObjectMethod(this->Canvas, Canvas_getParent);
                            if (TempClient)
                            {
                                this->Client = this->Thread->NewGlobalRef(TempClient);
                                this->Initialized = true;
                                this->Thread->DeleteLocalRef(TempClient);
                            }
                            this->Thread->DeleteLocalRef(CanvasClass);
                        }
                    }
                    this->Thread->DeleteLocalRef(TempCanvas);
                }
            }
        }
    }
}

bool KInput::AttachThread()
{
    this->Thread = nullptr;
    if (this->JVM)
        if (this->JVM->GetEnv((void**)&(this->Thread), JNI_VERSION_1_6) == JNI_EDETACHED)
            this->JVM->AttachCurrentThreadAsDaemon((void**)&(this->Thread), nullptr);
    return (this->Thread);
}

bool KInput::DispatchEvent(jobject Event)
{
    if (this->AttachThread())
    {
        if (!this->Canvas_Class)
        {
            jclass Temp = this->Thread->FindClass("Ljava/awt/Canvas;");
            if (Temp)
            {
                this->Canvas_Class = (jclass)this->Thread->NewGlobalRef(Temp);
                this->Thread->DeleteLocalRef(Temp);
            }
        }
        if (this->Canvas_Class)
        {
            if (!this->Canvas_DispatchEvent)
                this->Canvas_DispatchEvent = this->Thread->GetMethodID(this->Canvas_Class, "dispatchEvent", "(Ljava/awt/AWTEvent;)V");
            if (this->Canvas_DispatchEvent)
            {
                this->Thread->CallVoidMethod(this->Canvas, this->Canvas_DispatchEvent, Event);
                return true;
            }
        }
    }
    return false;
}

bool KInput::FocusEvent(std::int32_t ID)
{
    if (this->AttachThread())
    {
        if (!this->FocusEvent_Class)
        {
            jclass Temp = this->Thread->FindClass("Ljava/awt/event/FocusEvent;");
            if (Temp)
            {
                this->FocusEvent_Class = (jclass)this->Thread->NewGlobalRef(Temp);
                this->Thread->DeleteLocalRef(Temp);
            }
        }
        if (this->FocusEvent_Class)
        {
            if (!this->FocusEvent_Init)
                this->FocusEvent_Init = this->Thread->GetMethodID(this->FocusEvent_Class, "<init>", "(Ljava/awt/Component;I)V");
            if (this->FocusEvent_Init)
            {
                jobject FocusEvent_Object = this->Thread->NewObject(this->FocusEvent_Class, this->FocusEvent_Init, this->Canvas, ID);
                if (FocusEvent_Object)
                {
                    bool Result = this->DispatchEvent(FocusEvent_Object);
                    this->Thread->DeleteLocalRef(FocusEvent_Object);
                    return Result;
                }
            }
        }
    }
    return false;
}

bool KInput::KeyEvent(std::int32_t ID, std::int64_t When, std::int32_t Modifiers, std::int32_t KeyCode,
              std::uint16_t KeyChar, std::int32_t KeyLocation)
{
    if (this->AttachThread())
    {
        if (!this->KeyEvent_Class)
        {
            jclass Temp = this->Thread->FindClass("Ljava/awt/event/KeyEvent;");
            if (Temp)
            {
                this->KeyEvent_Class = (jclass)this->Thread->NewGlobalRef(Temp);
                this->Thread->DeleteLocalRef(Temp);
            }
        }
        if (this->KeyEvent_Class)
        {
            if (!this->KeyEvent_Init)
                this->KeyEvent_Init = this->Thread->GetMethodID(this->KeyEvent_Class, "<init>", "(Ljava/awt/Component;IJIICI)V");
            if (this->KeyEvent_Init)
            {
                jobject KeyEvent_Object = this->Thread->NewObject(this->KeyEvent_Class, this->KeyEvent_Init, this->Canvas, ID, When,
                                                                  Modifiers, KeyCode, KeyChar, KeyLocation);
                if (KeyEvent_Object)
                {
                    bool Result = this->DispatchEvent(KeyEvent_Object);
                    this->Thread->DeleteLocalRef(KeyEvent_Object);
                    return Result;
                }
            }
        }
    }
    return false;
}

bool KInput::MouseEvent(std::int32_t ID, std::int64_t When, std::int32_t Modifiers, std::int32_t X,
                std::int32_t Y, std::int32_t ClickCount, bool PopupTrigger, std::int32_t Button)
{
    if (this->AttachThread())
    {
        if (!this->MouseEvent_Class)
        {
            jclass Temp = this->Thread->FindClass("Ljava/awt/event/MouseEvent;");
            if (Temp)
            {
                this->MouseEvent_Class = (jclass)this->Thread->NewGlobalRef(Temp);
                this->Thread->DeleteLocalRef(Temp);
            }
        }
        if (this->MouseEvent_Class)
        {
            if (!this->MouseEvent_Init)
                this->MouseEvent_Init = this->Thread->GetMethodID(this->MouseEvent_Class, "<init>", "(Ljava/awt/Component;IJIIIIZI)V");
            if (this->MouseEvent_Init)
            {
                jobject MouseEvent_Object = this->Thread->NewObject(this->MouseEvent_Class, this->MouseEvent_Init, this->Client, ID, When,
                                                                    Modifiers, X, Y, ClickCount,
                                                                    PopupTrigger, Button);
                if (MouseEvent_Object)
                {
                    bool Result = this->DispatchEvent(MouseEvent_Object);
                    this->Thread->DeleteLocalRef(MouseEvent_Object);
                    return Result;
                }
            }
        }
    }
    return false;
}

bool KInput::MouseWheelEvent(std::int32_t ID, std::int64_t When, std::int32_t Modifiers, std::int32_t X,
                     std::int32_t Y, std::int32_t ClickCount, bool PopupTrigger, std::int32_t ScrollType,
                     std::int32_t ScrollAmount, std::int32_t WheelRotation)
{
    if (this->AttachThread())
    {
        if (!this->MouseWheelEvent_Class)
        {
            jclass Temp = this->Thread->FindClass("Ljava/awt/event/MouseWheelEvent;");
            if (Temp)
            {
                this->MouseWheelEvent_Class = (jclass)this->Thread->NewGlobalRef(Temp);
                this->Thread->DeleteLocalRef(Temp);
            }
        }
        if (this->MouseWheelEvent_Class)
        {
            if (!this->MouseWheelEvent_Init)
                this->MouseWheelEvent_Init = this->Thread->GetMethodID(this->MouseWheelEvent_Class, "<init>", "(Ljava/awt/Component;IJIIIIZIII)V");
            if (this->MouseWheelEvent_Init)
            {
                jobject MouseWheelEvent_Object = this->Thread->NewObject(this->MouseWheelEvent_Class, this->MouseWheelEvent_Init, this->Client, ID,
                                                                         When, Modifiers, X, Y,
                                                                         ClickCount, PopupTrigger, ScrollType, ScrollAmount,
                                                                         WheelRotation);
                if (MouseWheelEvent_Object)
                {
                    bool Result = this->DispatchEvent(MouseWheelEvent_Object);
                    this->Thread->DeleteLocalRef(MouseWheelEvent_Object);
                    return Result;
                }
            }
        }
    }
    return false;
}

KInput::~KInput()
{
    if (this->AttachThread())
    {
        if (this->Canvas)
        {
            this->Thread->DeleteGlobalRef(this->Canvas);
            this->Canvas = nullptr;
        }
        if (this->Client)
        {
            this->Thread->DeleteGlobalRef(this->Client);
            this->Client = nullptr;
        }
        if (this->Canvas_Class)
        {
            this->Thread->DeleteGlobalRef(this->Canvas_Class);
            this->Canvas_Class = nullptr;
        }
        if (this->FocusEvent_Class)
        {
            this->Thread->DeleteGlobalRef(this->FocusEvent_Class);
            this->FocusEvent_Class = nullptr;
        }
        if (this->KeyEvent_Class)
        {
            this->Thread->DeleteGlobalRef(this->KeyEvent_Class);
            this->KeyEvent_Class = nullptr;
        }
        if (this->MouseEvent_Class)
        {
            this->Thread->DeleteGlobalRef(this->MouseEvent_Class);
            this->MouseEvent_Class = nullptr;
        }
        if (this->MouseWheelEvent_Class)
        {
            this->Thread->DeleteGlobalRef(this->MouseWheelEvent_Class);
            this->MouseWheelEvent_Class = nullptr;
        }
    }
}
