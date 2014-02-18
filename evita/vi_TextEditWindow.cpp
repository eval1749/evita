#include "precomp.h"
//////////////////////////////////////////////////////////////////////////////
//
// evcl - listener - winapp - Text Edit Window
// listener/winapp/vi_TextEditWindow.cpp
//
// Copyright (C) 1996-2007 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
// @(#)$Id: //proj/evcl3/mainline/listener/winapp/vi_TextEditWindow.cpp#3 $
//
#include "evita/vi_TextEditWindow.h"

#include <algorithm>
#include <vector>

#pragma warning(push)
#pragma warning(disable: 4625 4626)
#include "base/bind.h"
#pragma warning(pop)
#include "base/logging.h"
#include "base/strings/string16.h"
#include "base/strings/stringprintf.h"
#include "common/timer/timer.h"
#include "evita/text/modes/mode.h"
#include "evita/ed_Style.h"
#include "evita/gfx_base.h"
#include "evita/editor/application.h"
#include "evita/editor/dom_lock.h"
#include "evita/dom/document.h"
#include "evita/dom/buffer.h"
#include "evita/dom/range.h"
#include "evita/dom/selection.h"
#include "evita/dom/text_window.h"
#include "evita/text/range.h"
#include "evita/ui/events/event.h"
#include "evita/views/icon_cache.h"
#include "evita/vi_Caret.h"
#include "evita/vi_EditPane.h"
#include "evita/vi_Frame.h"
#include "evita/vi_Selection.h"

extern HWND g_hwndActiveDialog;

//////////////////////////////////////////////////////////////////////
//
// TextEditWindow::ScrollBar
//
TextEditWindow::ScrollBar::~ScrollBar() {
  if (m_hwnd)
    ::DestroyWindow(m_hwnd);
}

void TextEditWindow::ScrollBar::ShowWindow(int code) const {
  if (m_hwnd)
    ::ShowWindow(m_hwnd, code);
}

//////////////////////////////////////////////////////////////////////
//
// TextEditWindow
//
TextEditWindow::TextEditWindow(const dom::TextWindow& text_window)
    : ContentWindow(text_window.window_id()),
      caret_(std::move(std::make_unique<Caret>())),
      m_gfx(nullptr),
      m_lCaretPosn(-1),
      m_pPage(new Page()),
      selection_(text_window.view_selection()),
      #if SUPPORT_IME
        m_fImeTarget(false),
        m_lImeEnd(0),
        m_lImeStart(0),
      #endif // SUPPORT_IME
      m_pViewRange(text_window.view_range()->text_range()) {
  selection_->set_window(this);
}

TextEditWindow::~TextEditWindow() {
}

Posn TextEditWindow::computeGoalX(float xGoal, Posn lGoal) {
  if (xGoal < 0)
    return lGoal;

  Page::Line* pLine = nullptr;

  if (!m_pPage->IsDirty(rect(), *selection_))
    pLine = m_pPage->FindLine(lGoal);

  if (pLine)
    return pLine->MapXToPosn(*m_gfx, xGoal);

  auto lStart = GetBuffer()->ComputeStartOfLine(lGoal);
  Page oPage;
  gfx::RectF page_rect(rect());
  for (;;) {
    auto const pLine = oPage.FormatLine(*m_gfx, page_rect,
                                        *selection_, lStart);
    auto const lEnd = pLine->GetEnd();
    if (lGoal < lEnd)
      return pLine->MapXToPosn(*m_gfx, xGoal);
    lStart = lEnd;
  }
}

Count TextEditWindow::ComputeMotion(Unit eUnit, Count n,
                                    const gfx::PointF& pt,
                                    Posn* inout_lPosn)  {
  UI_ASSERT_DOM_LOCKED();
  switch (eUnit) {
    case Unit_WindowLine:
      if (n > 0) {
        auto const lBufEnd = GetBuffer()->GetEnd();
        auto lGoal = *inout_lPosn;
        auto k = 0;
        for (k = 0; k < n; ++k) {
          lGoal = EndOfLine(lGoal);
          if (lGoal >= lBufEnd)
            break;
          ++lGoal;
        }
        *inout_lPosn = computeGoalX(pt.x, std::min(lGoal, lBufEnd));
        return k;
      } else if (n < 0) {
        n = -n;

        auto const lBufStart = GetBuffer()->GetStart();
        auto lStart = *inout_lPosn;
        auto k = 0;
        for (k = 0; k < n; ++k) {
          lStart = StartOfLine(lStart);
          if (lStart <= lBufStart)
            break;
          --lStart;
        }

        *inout_lPosn = computeGoalX(pt.x, std::max(lStart, lBufStart));
        return k;
      }
      return 0;

    case Unit_Screen: {
      auto k = LargeScroll(0, n, false);
      if (k > 0) {
        auto const lStart = m_pPage->GetStart();
        m_pViewRange->SetRange(lStart, lStart);
        *inout_lPosn = MapPointToPosn(pt);
      } else if (n > 0) {
        *inout_lPosn = std::min(GetEnd(), GetBuffer()->GetEnd());
        k = 1;
      } else if (n < 0) {
        *inout_lPosn = GetStart();
        k = 1;
      }
      return k;
    }

    case Unit_Window:
      if (n > 0) {
        *inout_lPosn = std::min(GetEnd(), GetBuffer()->GetEnd());
        return 1;
      }
      if (n < 0) {
        *inout_lPosn = GetStart();
        return 1;
      }
      return 0;
  }

  LOG(ERROR) << "Unsupported unit " << eUnit;
  return 0;
}

void TextEditWindow::DidChangeHierarchy() {
  m_gfx = &frame().gfx();
  auto const parent_hwnd = AssociatedHwnd();
  if (auto const hwnd = m_oHoriScrollBar.GetHwnd())
    ::SetParent(hwnd, parent_hwnd);
  if (auto const hwnd = m_oVertScrollBar.GetHwnd())
    ::SetParent(hwnd, parent_hwnd);
}

void TextEditWindow::DidHide() {
  // Note: It is OK that hidden window have focus.
  m_oHoriScrollBar.ShowWindow(SW_HIDE);
  m_oVertScrollBar.ShowWindow(SW_HIDE);
}

void TextEditWindow::DidKillFocus() {
  ParentClass::DidKillFocus();
  caret_->Give();
}

void TextEditWindow::DidRealize() {
  ParentClass::DidRealize();
  auto const frame = Frame::FindFrame(*this);
  ASSERT(frame);
  m_gfx = &frame->gfx();
}

void TextEditWindow::DidSetFocus() {
  ASSERT(has_focus());
  // Note: It is OK to set focus to hidden window.
  caret_->Take(*m_gfx);
  ParentClass::DidSetFocus();
}

void TextEditWindow::DidShow() {
  m_oHoriScrollBar.ShowWindow(SW_SHOW);
  m_oVertScrollBar.ShowWindow(SW_SHOW);
}

Posn TextEditWindow::EndOfLine(Posn lPosn) {
  UI_ASSERT_DOM_LOCKED();
  if (!m_pPage->IsDirty(rect(), *selection_)) {
    auto const pLine = m_pPage->FindLine(lPosn);
    if (pLine)
      return pLine->GetEnd() - 1;
  }

  auto const lBufEnd = selection_->GetBuffer()->GetEnd();
  return lPosn >= lBufEnd ? lBufEnd : endOfLineAux(*m_gfx, lPosn);
}

Posn TextEditWindow::endOfLineAux(const gfx::Graphics& gfx, Posn lPosn) {
  UI_ASSERT_DOM_LOCKED();
  auto const lBufEnd = selection_->GetBuffer()->GetEnd();
  if (lPosn >= lBufEnd)
    return lBufEnd;

  Page oPage;
  gfx::RectF page_rect(rect());
  auto lStart = selection_->GetBuffer()->ComputeStartOfLine(lPosn);
  for (;;) {
    auto const pLine = oPage.FormatLine(gfx, page_rect, *selection_, lStart);
    lStart = pLine->GetEnd();
    if (lPosn < lStart)
      return lStart - 1;
  }
}

void TextEditWindow::format(const gfx::Graphics& gfx, Posn lStart) {
  m_pPage->Format(gfx, gfx::RectF(rect()), *selection_, lStart);
}

Buffer* TextEditWindow::GetBuffer() const {
  return GetSelection()->GetBuffer();
}

HCURSOR TextEditWindow::GetCursorAt(const Point&) const {
  return ::LoadCursor(nullptr, IDC_IBEAM);
}

// For Selection.MoveDown Screen
Posn TextEditWindow::GetEnd() {
  UI_ASSERT_DOM_LOCKED();
  updateScreen();
  return m_pPage->GetEnd();
}

HWND TextEditWindow::GetScrollBar(int which) const {
  DCHECK_EQ(SB_VERT, which);
  return m_oVertScrollBar.GetHwnd();
}

//For Selection.MoveUp Screen
Posn TextEditWindow::GetStart() {
  UI_ASSERT_DOM_LOCKED();
  updateScreen();
  return m_pPage->GetStart();
}

int TextEditWindow::LargeScroll(int, int iDy, bool fRender) {
  UI_ASSERT_DOM_LOCKED();
  updateScreen();

  auto k = 0;
  if (iDy < 0) {
    // Scroll Down -- place top line out of window.
    iDy = -iDy;

    auto const lBufStart = selection_->GetBuffer()->GetStart();
    for (k = 0; k < iDy; ++k) {
      auto const lStart = m_pPage->GetStart();
      if (lStart == lBufStart)
        break;

      // Scroll down until page start goes out to page.
      do {
        if (!m_pPage->ScrollDown(*m_gfx))
          break;
      } while (m_pPage->GetEnd() != lStart);
    }
  } else if (iDy > 0) {
    // Scroll Up -- format page from page end.
    const Posn lBufEnd = selection_->GetBuffer()->GetEnd();
    for (k = 0; k < iDy; ++k) {
      auto const lStart = m_pPage->GetEnd();
      if (lStart >= lBufEnd)
        break;
      format(*m_gfx, lStart);
    }
  }

  if (fRender && k > 0)
    Render();
  return k;
}

base::string16 TextEditWindow::GetTitle() const {
  return GetBuffer()->GetFileName().empty() || !GetBuffer()->IsModified() ?
      GetBuffer()->name(): GetBuffer()->name() + L"*";
}

void TextEditWindow::MakeSelectionVisible() {
  m_lCaretPosn = -1;
  Redraw();
}

Posn TextEditWindow::MapPointToPosn(const gfx::PointF pt) {
  updateScreen();
  return std::min(m_pPage->MapPointToPosn(*m_gfx, pt), GetBuffer()->GetEnd());
}

// Description:
// Maps position specified buffer position and returns height
// of caret, If specified buffer position isn't in window, this function
// returns 0.
gfx::RectF TextEditWindow::MapPosnToPoint(Posn lPosn) {
  DCHECK_GE(lPosn, 0);
  UI_ASSERT_DOM_LOCKED();
  updateScreen();
  for (;;) {
    if (auto rect = m_pPage->MapPosnToPoint(*m_gfx, lPosn))
      return rect;
    m_pPage->ScrollToPosn(*m_gfx, lPosn);
  }
}

void TextEditWindow::OnDraw(gfx::Graphics*) {
  UI_ASSERT_DOM_LOCKED();
  m_pPage->Reset();
  Redraw();
}

bool TextEditWindow::OnIdle(uint count) {
  auto const more = GetBuffer()->OnIdle(count);

  if (is_shown())
    Redraw();
  return more;
}

LRESULT TextEditWindow::OnMessage(uint uMsg, WPARAM wParam, LPARAM lParam) {
  switch (uMsg) {
    case WM_VSCROLL:
      onVScroll(LOWORD(wParam));
      return 0;

    #if SUPPORT_IME
    case WM_IME_COMPOSITION:
      onImeComposition(lParam);
      return 0;

    case WM_IME_ENDCOMPOSITION:
      m_fImeTarget = false;
      return 0;

    case WM_IME_REQUEST:
      if (IMR_RECONVERTSTRING == wParam) {
          UI_DOM_AUTO_LOCK_SCOPE();
          return static_cast<LRESULT>(setReconvert(
              reinterpret_cast<RECONVERTSTRING*>(lParam),
              GetSelection()->GetStart(),
              GetSelection()->GetEnd()));
      }
      break;

    case WM_IME_SETCONTEXT:
      // We draw composition string instead of IME. So, we don't
      // need default composition window.
      lParam &= ~ISC_SHOWUICOMPOSITIONWINDOW;
      break;

    case WM_IME_STARTCOMPOSITION:
      if (!m_fImeTarget) {
        UI_DOM_AUTO_LOCK_SCOPE();
        m_lImeStart = GetSelection()->GetStart();
        m_lImeEnd = m_lImeStart;
        m_fImeTarget = false;
      }
      return 0;
    #endif // SUPPORT_IME
  }
  return ParentClass::OnMessage(uMsg, wParam, lParam);
}

void TextEditWindow::OnMouseWheel(const ui::MouseWheelEvent& event) {
  UI_DOM_AUTO_LOCK_SCOPE();
  SmallScroll(0, event.delta() > 0 ? -2 : 2);
}

void TextEditWindow::onVScroll(uint nCode) {
  UI_DOM_AUTO_LOCK_SCOPE();

  switch (nCode) {
    case SB_ENDSCROLL: // 8
      break;

    case SB_LINEDOWN: // 1
      SmallScroll(0, 1);
      break;

    case SB_LINEUP: // 0
      SmallScroll(0, -1);
      break;

    case SB_PAGEDOWN: // 3
      LargeScroll(0, 1);
      break;

    case SB_PAGEUP: // 2
      LargeScroll(0, -1);
      break;

    case SB_THUMBPOSITION: // 4
      return;

    case SB_THUMBTRACK: { // 5
      SCROLLINFO oInfo;
      oInfo.cbSize = sizeof(oInfo);
      oInfo.fMask = SIF_ALL;
      if (m_oVertScrollBar.GetInfo(&oInfo)) {
        auto const lStart = startOfLineAux(*m_gfx, oInfo.nTrackPos);
        format(*m_gfx, lStart);
        Render();
      }
      break;
    }

    default:
      return;
  }
}

void TextEditWindow::Redraw() {
  auto fSelectionIsActive = has_focus();

  if (g_hwndActiveDialog) {
    auto const edit_pane = Application::instance()->GetActiveFrame()->
        GetActivePane()->as<EditPane>();
    if (edit_pane)
      fSelectionIsActive = edit_pane->GetActiveWindow() == this;
  }

  UI_ASSERT_DOM_LOCKED();

  auto const lSelStart = selection_->GetStart();
  auto const lSelEnd = selection_->GetEnd();

  Posn lCaretPosn;
  if (fSelectionIsActive) {
    lCaretPosn = selection_->IsStartActive() ? lSelStart : lSelEnd;

    // EvEdit 1.0's highlight color
    //selection_->SetColor(Color(0, 0, 0));
    //selection_->SetBackground(Color(0xCC, 0xCC, 0xFF));

    // We should not use GetSysColor. If we want to use here
    // default background must be obtained from GetSysColor.
    //selection_->SetColor(::GetSysColor(COLOR_HIGHLIGHTTEXT));
    //selection_->SetBackground(::GetSysColor(COLOR_HIGHLIGHT));

    // We use Vista's highlight color.
    selection_->SetColor(Color(255, 255, 255));
    selection_->SetBackground(Color(51, 153, 255));
  } else {
    lCaretPosn = m_lCaretPosn == -1 ? lSelStart : m_lCaretPosn;

    Posn lEnd = GetBuffer()->GetEnd();
    if (lSelStart == lEnd && lSelEnd == lEnd)
      lCaretPosn = lEnd;

     selection_->SetColor(Color(67, 78, 84));
     selection_->SetBackground(Color(191, 205, 219));
  }

  DCHECK_GE(lCaretPosn, 0);

  {
    auto const lStart = m_pViewRange->GetStart();
    if (m_pPage->IsDirty(rect(), *selection_, fSelectionIsActive)) {
      format(*m_gfx, startOfLineAux(*m_gfx, lStart));

      if (m_lCaretPosn != lCaretPosn) {
        // FIXME 2007-05-12 Fill the page with lines.
        m_pPage->ScrollToPosn(*m_gfx, lCaretPosn);
        m_lCaretPosn = lCaretPosn;
      }
    } else if (m_lCaretPosn != lCaretPosn) {
        m_pPage->ScrollToPosn(*m_gfx, lCaretPosn);
        m_lCaretPosn = lCaretPosn;
    } else if (m_pPage->GetStart() != lStart) {
        format(*m_gfx, startOfLineAux(*m_gfx, lStart));
    } else {
      // Page is clean.
      if (!m_fImeTarget)
        caret_->ShouldBlink();
      return;
    }
  }

  Render();
}

void TextEditWindow::Render() {
  if (!is_shown())
    return;

  gfx::Graphics::DrawingScope drawing_scope(*m_gfx);
  caret_->Hide();
  m_pPage->Render(*m_gfx);

  {
    auto const lStart = m_pPage->GetStart();
    m_pViewRange->SetRange(lStart, lStart);
    updateScrollBar();
  }

  const auto rect = m_pPage->MapPosnToPoint(*m_gfx, m_lCaretPosn);
  if (!rect) {
    caret_->Reset();
    return;
  }

  auto const caret_width = std::max(::GetSystemMetrics(SM_CXBORDER), 2);
  gfx::RectF caret_rect(rect.left_top(),
                        gfx::SizeF(static_cast<float>(caret_width),
                                   rect.height()));
  #if SUPPORT_IME
    if (m_fImeTarget) {
      POINT pt = {
        static_cast<int>(rect.left),
        static_cast<int>(rect.top)
      };

      SIZE size = {
        static_cast<int>(rect.width()),
        static_cast<int>(rect.height())
      };

      if (showImeCaret(size, pt))
        return;
    }
  #endif // SUPPORT_IME

  caret_->Update(caret_rect);
}

void TextEditWindow::SetScrollBar(HWND hwnd, int nBar) {
  DCHECK_EQ(SB_VERT, nBar);
  DCHECK(!m_oVertScrollBar.m_hwnd);
  m_oVertScrollBar.Set(hwnd, SB_CTL);
}

int TextEditWindow::SmallScroll(int, int iDy) {
  UI_ASSERT_DOM_LOCKED();
  updateScreen();

  if (iDy < 0) {
    iDy = -iDy;

    auto const lBufStart = selection_->GetBuffer()->GetStart();
    auto lStart = m_pPage->GetStart();
    int k;
    for (k = 0; k < iDy; ++k) {
      if (lStart == lBufStart)
        break;
        lStart = startOfLineAux(*m_gfx, lStart - 1);
    }

    if (k > 0) {
      format(*m_gfx, lStart);
      Render();
    }
    return k;
  }

  if (iDy > 0) {
    auto const lBufEnd = selection_->GetBuffer()->GetEnd();
    int k;
    for (k = 0; k < iDy; ++k) {
      if (m_pPage->GetEnd() >= lBufEnd) {
          // Make sure whole line of buffer end is visible.
          m_pPage->ScrollToPosn(*m_gfx, lBufEnd);
          ++k;
          break;
      }

      if (!m_pPage->ScrollUp(*m_gfx))
        break;
    }

    if (k > 0)
        Render();
    return k;
  }

  return 0;
}

Posn TextEditWindow::StartOfLine(Posn lPosn) {
  UI_ASSERT_DOM_LOCKED();
  return lPosn <= 0 ? 0 : startOfLineAux(*m_gfx, lPosn);
}

// See Also:
// EditPange::endOfLineAux
// Description:
// Returns start position of window line of specified position.
Posn TextEditWindow::startOfLineAux(const gfx::Graphics& gfx, Posn lPosn) {
  if (!m_pPage->IsDirty(rect(), *selection_)) {
    auto const pLine = m_pPage->FindLine(lPosn);
    if (pLine)
      return pLine->GetStart();
  }

  auto lStart = selection_->GetBuffer()->ComputeStartOfLine(lPosn);
  if (!lStart)
    return 0;

  Page oPage;
  gfx::RectF page_rect(rect());
  for (;;) {
    auto const pLine = oPage.FormatLine(gfx, page_rect, *selection_,
                                        lStart);

    auto const lEnd = pLine->GetEnd();
    if (lPosn < lEnd)
      return pLine->GetStart();
    lStart = lEnd;
  }
}

void TextEditWindow::updateScreen() {
  UI_ASSERT_DOM_LOCKED();
  if (!m_pPage->IsDirty(rect(), *selection_))
    return;

  Posn lStart = m_pViewRange->GetStart();
  lStart = startOfLineAux(*m_gfx, lStart);
  format(*m_gfx, lStart);
}

void TextEditWindow::updateScrollBar() {
  if (!m_pPage->GetBuffer())
    return;

  auto const lBufEnd = m_pPage->GetBuffer()->GetEnd() + 1;

  SCROLLINFO oInfo;
  oInfo.cbSize = sizeof(oInfo);
  oInfo.fMask = SIF_POS | SIF_RANGE | SIF_PAGE | SIF_DISABLENOSCROLL;
  oInfo.nPage = static_cast<uint>(m_pPage->GetEnd() - m_pPage->GetStart());
  oInfo.nMin = 0;
  oInfo.nMax = lBufEnd;
  oInfo.nPos = m_pPage->GetStart();

  if (static_cast<Count>(oInfo.nPage) >= lBufEnd) {
    // Current screen shows entire buffer. We disable scroll bar.
    oInfo.nMax = 0;
  }

  m_oVertScrollBar.SetInfo(&oInfo, true);
}

static std::vector<base::string16> ComposeStatusBarTexts(
    dom::Buffer* buffer, Selection* selection, bool has_focus) {
  static const char16* const k_rgwszNewline[4] = {
    L"--",
    L"LF",
    L"CR",
    L"CRLF",
  };

  UI_ASSERT_DOM_LOCKED();

  // FIXME 2007-07-18 yosi We should use lazy evaluation object for
  // computing line number of column or cache.
  text::Range::Information oInfo;
  selection->range()->GetInformation(&oInfo);

  return {
    buffer->IsNotReady() ? L"Busy" : has_focus ? L"Ready" : L"",
    buffer->GetMode()->GetName(),
    base::StringPrintf(L"CP%u", buffer->GetCodePage()),
    k_rgwszNewline[buffer->GetNewline()],
    base::StringPrintf(L"Ln %d%ls", oInfo.m_lLineNum,
                       oInfo.m_fLineNum ? L"" : L"+"),
    base::StringPrintf(L"Cn %d%ls", oInfo.m_lColumn,
                       oInfo.m_fColumn ? L"" : L"+"),
    base::StringPrintf(L"Ch %d", selection->IsStartActive() ?
        selection->GetStart() : selection->GetEnd()),
    // FIXME 2007-07-25 yosi@msn.com We need to show "OVR" if
    // we are in overwrite mode.
    base::StringPrintf(buffer->IsReadOnly() ? L"R/O" : L"INS"),
  };
}

void TextEditWindow::UpdateStatusBar() const {
  frame().SetStatusBar(ComposeStatusBarTexts(GetBuffer(), GetSelection(),
                                             has_focus()));
}

#if SUPPORT_IME

// See Also Caret::Show for moving candidate window.

#include <imm.h>
#pragma comment(lib, "imm32.lib")

extern StyleValues g_DefaultStyle;
StyleValues g_rgpImeStyleConverted[2];
StyleValues g_pImeStyleInput;
StyleValues g_pImeStyleTargetConverted;
StyleValues g_pImeStyleTargetNotConverted;

#define GCS_COMPSTRATTR (GCS_COMPSTR | GCS_COMPATTR | GCS_CURSORPOS)

class Imc {
  private: HWND m_hwnd;
  private: HIMC m_himc;

  public: Imc(HWND hwnd) : m_hwnd(hwnd), m_himc(::ImmGetContext(hwnd)) {}

  public: ~Imc() {
    if (m_himc)
      ::ImmReleaseContext(m_hwnd, m_himc);
  }

  public: operator HIMC() const { return m_himc; }
};

void TextEditWindow::onImeComposition(LPARAM lParam) {
  if (!editor::DomLock::instance()->locked()) {
    UI_DOM_AUTO_LOCK_SCOPE();
    onImeComposition(lParam);
    return;
  }

  Imc imc(AssociatedHwnd());
  if (!imc)
    return;

  UI_ASSERT_DOM_LOCKED();
  auto const range = GetSelection()->range();
  text::UndoBlock oUndo(range, L"IME");

  char16 rgwch[1024];
  // If IME has result string, we can insert it into buffer.
  if (lParam & GCS_RESULTSTR) {
    // Remove previous composition string. If user inputs "Space",
    // IME set GCS_RESULTSTR without composition.
    if (m_lImeStart != m_lImeEnd) {
      range->SetRange(m_lImeStart, m_lImeEnd);
      range->SetText(base::string16());
    }

    // Get result string
    auto const cwch = ::ImmGetCompositionString(
        imc, GCS_RESULTSTR, rgwch, sizeof(rgwch)) / sizeof(char16);

    // Insert result string into buffer
    if (cwch >= 1) {
      range->SetText(base::string16(rgwch, cwch));
      range->Collapse(Collapse_End);
      GetSelection()->SetStartIsActive(false);
      m_lImeEnd = GetSelection()->GetEnd();
      m_lImeStart = m_lImeEnd;
    }
  }

  // IME has composition string
  if ((lParam & GCS_COMPSTRATTR) == GCS_COMPSTRATTR) {
    // Remove previous composition string
    if (m_lImeStart != m_lImeEnd) {
        range->SetRange(m_lImeStart, m_lImeEnd);
        range->SetText(base::string16());
        m_lImeEnd = m_lImeStart;
    }

    // Get composition string
    auto const cwch = ::ImmGetCompositionString(
        imc, GCS_COMPSTR, rgwch, sizeof(rgwch)) / sizeof(char16);

    // Get composition attributes
    char rgbAttr[lengthof(rgwch)];
    auto const cbAttr = ::ImmGetCompositionString(
        imc, GCS_COMPATTR, rgbAttr, sizeof(rgbAttr));
    if (cbAttr != static_cast<int>(cwch)) {
      return;
    }

    auto const lCursor = ::ImmGetCompositionString(
          imc, GCS_CURSORPOS, nullptr, 0);
    if (lCursor < 0)
      return;

    uint32 rgnClause[100];
    ::ImmGetCompositionString(imc, GCS_COMPCLAUSE, rgnClause,
                              sizeof(rgnClause));

    range->SetText(base::string16(rgwch, cwch));
    range->Collapse(Collapse_End);
    GetSelection()->SetStartIsActive(false);
    m_lImeEnd = GetSelection()->GetEnd();
    range->SetRange(m_lImeStart + lCursor, m_lImeStart + lCursor);

    if (!g_pImeStyleInput.m_rgfMask) {
          // Converted[0]
          g_rgpImeStyleConverted[0].m_rgfMask =
              StyleValues::Mask_Decoration;

          g_rgpImeStyleConverted[0].m_eDecoration =
              TextDecoration_ImeInactiveA;

          // Converted[1]
          g_rgpImeStyleConverted[1].m_rgfMask =
              StyleValues::Mask_Decoration;

          g_rgpImeStyleConverted[1].m_eDecoration =
              TextDecoration_ImeInactiveB;

          // Input
          g_pImeStyleInput.m_rgfMask =
              StyleValues::Mask_Decoration;

          g_pImeStyleInput.m_eDecoration =
              TextDecoration_ImeInput;

          // Target Converted
          g_pImeStyleTargetConverted.m_rgfMask =
              StyleValues::Mask_Decoration;

          g_pImeStyleTargetConverted.m_eDecoration =
              TextDecoration_ImeActive;

          // Target Not Converted
          g_pImeStyleTargetNotConverted.m_rgfMask =
              StyleValues::Mask_Background |
              StyleValues::Mask_Color |
              StyleValues::Mask_Decoration;

          #if 0
              g_pImeStyleTargetNotConverted.m_crBackground =
                  g_DefaultStyle.GetColor();

              g_pImeStyleTargetNotConverted.m_crColor =
                  g_DefaultStyle.GetBackground();
          #else
              g_pImeStyleTargetNotConverted.m_crBackground =
                  GetSelection()->GetBackground();

              g_pImeStyleTargetNotConverted.m_crColor =
                  GetSelection()->GetColor();
          #endif

          g_pImeStyleTargetNotConverted.m_eDecoration =
              TextDecoration_None;
      }

      m_fImeTarget = false;
      Posn lEnd = static_cast<Posn>(m_lImeStart + cwch);
      Posn lPosn = m_lImeStart;
      int iClause = 0;
      int iConverted = 0;
      while (lPosn < lEnd) {
        StyleValues* pStyle;
        switch (rgbAttr[lPosn - m_lImeStart]) {
          case ATTR_INPUT:
          case ATTR_INPUT_ERROR:
            pStyle = &g_pImeStyleInput;
            iConverted = 0;
            break;

          case ATTR_TARGET_CONVERTED:
            pStyle = &g_pImeStyleTargetConverted;
            m_fImeTarget = true;
            iConverted = 0;
            break;

          case ATTR_TARGET_NOTCONVERTED:
            pStyle = &g_pImeStyleTargetNotConverted;
            m_fImeTarget = true;
            iConverted = 0;
            break;

          case ATTR_CONVERTED:
            pStyle = &g_rgpImeStyleConverted[iConverted];
            iConverted = 1 - iConverted;
            break;

          default:
            pStyle = &g_pImeStyleInput;
            break;
        }

        ++iClause;
        Posn lNext = static_cast<Posn>(m_lImeStart + rgnClause[iClause]);
        GetBuffer()->SetStyle(lPosn, lNext, pStyle);
        lPosn = lNext;
      }
  }

  ////////////////////////////////////////////////////////////
  //
  // We have already insert composed string. So, we don't
  // need WM_IME_CHAR and WM_CHAR messages to insert
  // composed string.
  if (lParam & GCS_RESULTSTR)
  {
      m_fImeTarget = false;
      return;
  }

  // Composition was canceled.
  if (0 == lParam)
  {
      m_fImeTarget = false;

      // Remove previous composition string
      range->SetRange(m_lImeStart, m_lImeEnd);
      range->SetText(base::string16());

      // if (m_fCancelButLeave)
      {
          auto const cwch = ::ImmGetCompositionString(
              imc,
              GCS_COMPSTR,
              rgwch,
              sizeof(rgwch)) / sizeof(char16);
          if (cwch >= 1)
          {
              range->SetText(base::string16(rgwch, cwch));
          }
      }

      m_lImeEnd = m_lImeStart;
  }
}

// Note:
// o IME2000 ignores string after newline.
// o We should limit number of characters to be reconverted.
//
void TextEditWindow::Reconvert(Posn lStart, Posn lEnd) {
  UI_ASSERT_DOM_LOCKED();

  BOOL fSucceeded;

  auto const cb = setReconvert(nullptr, lStart, lEnd);
  if (!cb)
    return;

  std::vector<uint8_t> buffer(cb);

  auto const p = reinterpret_cast<RECONVERTSTRING*>(&buffer[0]);
  setReconvert(p, lStart, lEnd);

  Imc imc(AssociatedHwnd());
  fSucceeded = ::ImmSetCompositionString(
      imc,
      SCS_QUERYRECONVERTSTRING,
      p,
      cb,
      nullptr,
      0);
  if (!fSucceeded) {
    DVLOG(0) << "ImmSetCompositionString SCS_QUERYRECONVERTSTRING failed";
    return;
  }

  m_lImeStart = static_cast<Posn>(lStart + p->dwCompStrOffset / 2);
  m_lImeEnd = static_cast<Posn>(m_lImeStart + p->dwCompStrLen);
  m_fImeTarget = true;

  fSucceeded = ::ImmSetCompositionString(
      imc,
      SCS_SETRECONVERTSTRING,
      p,
      cb,
      nullptr,
      0);
  if (!fSucceeded) {
    DVLOG(0) << "ImmSetCompositionString SCS_SETRECONVERTSTRING failed";
    return;
  }
}

size_t TextEditWindow::setReconvert(RECONVERTSTRING* p, Posn lStart,
                                  Posn lEnd) {
  ASSERT(lEnd >= lStart);
  UI_ASSERT_DOM_LOCKED();
  auto const cwch = lEnd - lStart;
  if (!cwch)
    return 0u;

  auto const cb = sizeof(RECONVERTSTRING) + sizeof(char16) * (cwch + 1);
  if (!p)
    return cb;
  p->dwSize = cb;
  p->dwVersion = 0;
  p->dwStrLen = static_cast<DWORD>(cwch);
  p->dwStrOffset = sizeof(RECONVERTSTRING);
  p->dwCompStrLen = static_cast<DWORD>(cwch); // # of characters
  p->dwCompStrOffset = 0; // byte offset
  p->dwTargetStrLen = p->dwCompStrLen;
  p->dwTargetStrOffset = p->dwCompStrOffset;

  auto const pwch = reinterpret_cast<char16*>(
      reinterpret_cast<char*>(p) + p->dwStrOffset);
  GetBuffer()->GetText(pwch, lStart, lEnd);
  pwch[cwch] = 0;
  return cb;
}

// Set left top coordinate of IME candiate window.
BOOL TextEditWindow::showImeCaret(SIZE sz, POINT pt) {
  Imc imc(AssociatedHwnd());
  if (!imc)
    return FALSE;

  CANDIDATEFORM oCF;
  oCF.dwIndex = 0;
  oCF.dwStyle = CFS_EXCLUDE;
  oCF.ptCurrentPos.x = pt.x;
  oCF.ptCurrentPos.y = pt.y + sz.cy;
  oCF.rcArea.left = pt.x;
  oCF.rcArea.top = pt.y;
  oCF.rcArea.right = pt.x;
  oCF.rcArea.bottom = pt.y + sz.cy;

  return ::ImmSetCandidateWindow(imc, &oCF);
}

#endif // SUPPORT_IME

// ContentWindow
int TextEditWindow::GetIconIndex() const {
  return views::IconCache::instance()->GetIconForFileName(
      GetBuffer()->GetName());
}
