#include "precomp.h"
//////////////////////////////////////////////////////////////////////////////
//
// evcl - listener - winapp - winmain
// listener/winapp/winmain.cpp
//
// Copyright (C) 1996-2007 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
// @(#)$Id: //proj/evcl3/mainline/listener/winapp/vi_Application.cpp#1 $
//
#include "./vi_Application.h"

#include "./vi_Buffer.h"
#include "./vi_EditPane.h"
#include "./vi_FileDialogBox.h"
#include "./vi_IoManager.h"

Application* Application::sm_pApplication;
uint g_nDropTargetMsg;

static const char16 Evita__DropTarget[] = L"Evita.DropTarget";

//////////////////////////////////////////////////////////////////////
//
// Application ctor
//
Application::Application() :
    m_eNewline(NewlineMode_CrLf),
    m_hIconList(::ImageList_Create(16, 16, ILC_COLOR32, 10, 10)),
    m_nCodePage(932),
    m_pActiveFrame(NULL),
    m_pIoManager(NULL)
{
    // nothing to do
} // Application::Application

//////////////////////////////////////////////////////////////////////
//
// Application dtor
//
Application::~Application()
{
    if (NULL == m_hIconList)
    {
        ::ImageList_Destroy(m_hIconList);
    }
} // Application::~Application

//////////////////////////////////////////////////////////////////////
//
// Application::AddIcon
//
int Application::AddIcon(HICON hIcon)
{
    return ::ImageList_ReplaceIcon(m_hIconList, -1, hIcon);
} // Application::AddIcon

//////////////////////////////////////////////////////////////////////
//
// Application::Ask
//
int Application::Ask(uint nFlags, uint nFormatId, ...)
{
    char16 wszFormat[1024];
    ::LoadString(g_hResource, nFormatId, wszFormat, lengthof(wszFormat));

    char16 wsz[1024];
    va_list args;
    va_start(args, nFormatId);
    ::wvsprintf(wsz, wszFormat, args);
    va_end(args);

    return ::MessageBox(
        *GetActiveFrame(),
        wsz,
        GetTitle(),
        nFlags);
} //Application::Ask

//////////////////////////////////////////////////////////////////////
//
// Application::CanExit
//
bool Application::CanExit() const
{
    for (auto& buffer: m_oBuffers) {
      // TODO: We should make Buffer::CanKill() const.
      if (!const_cast<Buffer&>(buffer).CanKill())
        return false;
    }

    return true;
} // Application::CanExit

//////////////////////////////////////////////////////////////////////
//
// Application::CreateFrame
//
Frame* Application::CreateFrame()
{
    Frame* pFrame = new Frame;
    return m_oFrames.Append(pFrame);
} // Application::CreateFrame

//////////////////////////////////////////////////////////////////////
//
// Application::DeleteFrame
//
Frame* Application::DeleteFrame(Frame* pFrame)
{
    m_oFrames.Delete(pFrame);

    if (m_oFrames.IsEmpty())
    {
        ::PostQuitMessage(0);
    } // if

    return pFrame;
} // Application::DeleteFrame

//////////////////////////////////////////////////////////////////////
//
// Application::Exit
//
void Application::Exit(bool fForce)
{
    while (NULL != m_oBuffers.GetFirst())
    {
        if (! KillBuffer(m_oBuffers.GetFirst(), fForce))
        {
            return;
        }
    } // while

    while (NULL != m_oFrames.GetFirst())
    {
        ::DestroyWindow(*m_oFrames.GetFirst());
    } // while
} // Application::Exit

//////////////////////////////////////////////////////////////////////
//
// Application::FindBuffer
//
Buffer* Application::FindBuffer(const char16* pwszName) const
{
    for (auto& buffer: m_oBuffers) {
      if (!::lstrcmpiW(buffer.GetName(), pwszName))
        return const_cast<Buffer*>(&buffer);
    }
    return nullptr;
} // Application::FindBuffer

//////////////////////////////////////////////////////////////////////
//
// Application::FindFrame
//
Frame* Application::FindFrame(HWND hwnd) const
{
    for (auto& frame: m_oFrames) {
        if (frame == hwnd)
            return const_cast<Frame*>(&frame);
    }
    return NULL;
} // Application::FindFrame

//////////////////////////////////////////////////////////////////////
//
// Application::FindPane
//
Pane* Application::FindPane(HWND hwndMouse, POINT pt) const
{
    unless (::ClientToScreen(hwndMouse, &pt)) return NULL;
    HWND hwnd = ::WindowFromPoint(pt);
    when (NULL == hwnd) return NULL;

    if (0 == g_nDropTargetMsg)
    {
        g_nDropTargetMsg = ::RegisterWindowMessage(Evita__DropTarget);
        when (0 == g_nDropTargetMsg) return NULL;
    }

    do
    {
        LRESULT iAnswer = ::SendMessage(hwnd, g_nDropTargetMsg, 0, 0);
        when (iAnswer) return reinterpret_cast<Pane*>(iAnswer);
        hwnd = ::GetParent(hwnd);
    } while (NULL != hwnd);

    return NULL;
} // Application::FindPane

//////////////////////////////////////////////////////////////////////
//
// Application::Init
//
void Application::Init()
{
    Command::Processor::GlobalInit();
    sm_pApplication = new Application;
    sm_pApplication->m_pIoManager = new IoManager;
    sm_pApplication->m_pIoManager->Realize();
} // Application::Init


//////////////////////////////////////////////////////////////////////
//
// Application::Load
//
Buffer* Application::Load(
    const char16*   pwszFileName )
{
    char16 wszFileName[MAX_PATH+1];
    char16* pwszFile;
    ::GetFullPathName(
        pwszFileName,
        lengthof(wszFileName),
        wszFileName,
        &pwszFile );

    for (auto& buffer: m_oBuffers) {
      if (!::lstrcmpiW(buffer.GetFileName(), wszFileName))
        return &buffer;
    }

    Buffer* pBuffer = NewBuffer(pwszFile);
    pBuffer->Load(wszFileName);

    return pBuffer;
} // Application::Load


//////////////////////////////////////////////////////////////////////
//
// Application::KillBuffer
//
bool Application::KillBuffer(Buffer* pBuffer, bool fForce)
{
    if (! fForce && ! pBuffer->CanKill())
    {
        return false;
    }

    for (;;)
    {
        Buffer::Window* pWindow = pBuffer->GetWindow();
        when (NULL == pWindow) break;
        pWindow->Destroy();
    } // for each window

    m_oBuffers.Delete(pBuffer);

    delete pBuffer;

    return true;
} // Application::KillBuffer


//////////////////////////////////////////////////////////////////////
//
// Application::NewBuffer
//
Buffer* Application::NewBuffer(const char16* pwszName)
{
    Buffer* pBuffer = new Buffer(pwszName);
    RenameBuffer(pBuffer, pwszName);
    return m_oBuffers.Append(pBuffer);
} // Application::NewBuffer


//////////////////////////////////////////////////////////////////////
//
// Application::OnIdle
//
bool Application::OnIdle(uint nCount)
{
    bool fMore = false;
    for (auto& frame: m_oFrames) {
        if (frame.OnIdle(nCount))
            fMore = true;
    } // for each Frame
    return fMore;
} // Application::OnIdle


//////////////////////////////////////////////////////////////////////
//
// Application::RenameBuffer
//
Buffer* Application::RenameBuffer(
    Buffer*         pBuffer,
    const char16*   pwszName )
{
    Buffer* pPresent = FindBuffer(pwszName);
    if (pBuffer == pPresent)
    {
        return pBuffer;
    }

    if (NULL == pPresent)
    {
        pBuffer->SetName(pwszName);
        return pBuffer;
    }

    char16 wsz[MAX_PATH + 1];
    char16* pwszTail;
    const char16* pwszDot = lstrrchrW(pwszName, '.');
    if (NULL == pwszDot)
    {
        pwszTail = wsz + lstrlenW(pwszName);
        pwszDot  = L"";
    }
    else
    {
        pwszTail = wsz + (pwszDot - pwszName);
    }

    lstrcpyW(wsz, pwszName);

    uint n = 1;
    while (NULL != FindBuffer(wsz))
    {
        n += 1;
        ::wsprintf(pwszTail, L"<%d>%s", n, pwszDot);
    } // while

    pBuffer->SetName(wsz);
    return pBuffer;
} // Buffer* Application::RenameBuffer


//////////////////////////////////////////////////////////////////////
//
// SaveBuffer
//
bool Application::SaveBuffer(
    Frame*    pFrame,
    Buffer*         pBuffer,
    bool            fSaveAs )
{
    NewlineMode eNewline = pBuffer->GetNewline();
    if (NewlineMode_Detect == eNewline)
    {
        eNewline = GetNewline();
    } // if

    uint nCodePage = pBuffer->GetCodePage();
    if (0 == nCodePage)
    {
        nCodePage = GetCodePage();
    } // if

    FileDialogBox::Param oParam;
    oParam.m_hwndOwner = *pFrame;

    oParam.m_wsz[0] = 0;

    if (fSaveAs || 0 == pBuffer->GetFileName()[0])
    {
        ::lstrcpyW(oParam.m_wsz, pBuffer->GetFileName());
        FileDialogBox oDialog;
        if (! oDialog.GetSaveFileName(&oParam))
        {
            return true;
        }

        const char16* pwszName = ::lstrrchrW(oParam.m_wsz, '\\');
        if (NULL == pwszName)
        {
            pwszName = ::lstrrchrW(oParam.m_wsz, '/');
            if (NULL == pwszName)            {
                pwszName = oParam.m_wsz + 1;
            }
        }

        // Skip slash(/)
        pwszName++;

        RenameBuffer(pBuffer, pwszName);
    } // if

    return pBuffer->Save(oParam.m_wsz, nCodePage, eNewline);
} // SaveBuffer


//////////////////////////////////////////////////////////////////////
//
// Application::ShowMessage
//
void Application::ShowMessage(MessageLevel iLevel, uint nFormatId)
{
    GetActiveFrame()->ShowMessage(iLevel, nFormatId);
} // Application::ShowMessage
