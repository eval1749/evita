//////////////////////////////////////////////////////////////////////////////
//
// evcl - listener - The Listener Application class
// listener/winapp/vi_application.h
//
// Copyright (C) 1996-2007 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
// @(#)$Id: //proj/evcl3/mainline/listener/winapp/vi_Application.h#1 $
//
#if !defined(INCLUDE_visual_application_h)
#define INCLUDE_visual_application_h

#include "./cm_CmdProc.h"

#include "./vi_Frame.h"

class Buffer;
class IoManager;

//////////////////////////////////////////////////////////////////////
//
// Application
//
class Application : public Command::Processor
{
    protected: typedef DoubleLinkedList_<Frame>  Frames;
    protected: typedef DoubleLinkedList_<Buffer> Buffers;

    private: NewlineMode    m_eNewline;
    private: uint           m_nCodePage;
    private: Buffers        m_oBuffers;
    private: Frames   m_oFrames;
    private: Frame*   m_pActiveFrame;
    private: IoManager*     m_pIoManager;

    protected: static Application*  sm_pApplication;

    // ctor/dtor
    protected: Application();
    public:   ~Application();

    public: const Buffers& buffers() const { return m_oBuffers; }
    public: Buffers& buffers() { return m_oBuffers; }
    public: const Frames& frames() const { return m_oFrames; }
    public: Frames& frames() { return m_oFrames; }

    // FIXME 2007-08-19 yosi@msn.com We should NOT use InternalAddBufer,
    // this method is just for listener.
    public: void InternalAddBuffer(Buffer*);

    // [A]
    public: int Ask(uint, uint, ...);

    // [C]
    public: bool CanExit() const;
    public: Frame* CreateFrame();

    // [D]
    public: Frame* DeleteFrame(Frame*);

    // [E]
    public: void Exit(bool);

    // [F]
    public: Buffer*      FindBuffer(const char16*) const;
    public: Frame* FindFrame(HWND) const;
    public: Pane*        FindPane(HWND, POINT) const;

    // [G]
    public: static Application* Get() { return sm_pApplication; }

    public: Frame* GetActiveFrame() const { return m_pActiveFrame; }
    public: uint         GetCodePage()    const { return m_nCodePage; }

    public: Frame* GetFirstFrame() const
        { return m_oFrames.GetFirst(); }

    public: HIMAGELIST GetIconList() const;

    public: IoManager*   GetIoManager()   const { return m_pIoManager; }

    public: Frame* GetLastFrame() const
        { return m_oFrames.GetLast(); }

    public: NewlineMode GetNewline() const { return m_eNewline; }

    public: const char16* GetTitle() const
    {
        #if _DEBUG
            return L"Evita Debug";
        #else
            return L"Evita";
        #endif
    } // GetTitle

    // [H]
    public: bool HasMultipleFrames() const
        { return GetFirstFrame() != GetLastFrame(); }

    // [I]
    public: static void Init();

    // [K]
    public: bool KillBuffer(Buffer*, bool = false);

    // [L]
    public: Buffer* Load(const char16*);

    // [N]
    public: Buffer* NewBuffer(const char16*);

    // [O]
    public: bool OnIdle(uint);

    // [R]
    public: Buffer* RenameBuffer(Buffer*, const char16*);

    // [S]
    public: bool SaveBuffer(Frame*, Buffer*, bool = false);

    public: Frame* SetActiveFrame(Frame* pFrame)
    {
        return m_pActiveFrame = pFrame;
    } // SetActiveFrame

    public: void ShowMessage(MessageLevel, uint);
}; // Appliction

#endif //!defined(INCLUDE_visual_application_h)
