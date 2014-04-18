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
#include "evita/css/style.h"
#include "evita/gfx_base.h"
#include "evita/editor/application.h"
#include "evita/editor/dom_lock.h"
#include "evita/dom/text/document.h"
#include "evita/dom/text/range.h"
#include "evita/dom/windows/selection.h"
#include "evita/dom/windows/text_window.h"
#include "evita/metrics/time_scope.h"
#include "evita/text/buffer.h"
#include "evita/text/range.h"
#include "evita/ui/controls/scroll_bar.h"
#include "evita/views/frame_list.h"
#include "evita/views/icon_cache.h"
#include "evita/views/text/render_selection.h"
#include "evita/views/text/render_text_line.h"
#include "evita/views/text/text_renderer.h"
#include "evita/vi_Caret.h"
#include "evita/vi_EditPane.h"
#include "evita/vi_Frame.h"
#include "evita/vi_Selection.h"

namespace {
class RenderSelection : public views::rendering::Selection {
  public: RenderSelection(::Selection* selection, bool is_active);
  public: explicit RenderSelection(::Selection* selection);
  public: ~RenderSelection() = default;
};

RenderSelection::RenderSelection(::Selection* selection, bool is_active) {
  active = is_active;
  start = selection->GetStart();
  end = selection->GetEnd();
}

RenderSelection::RenderSelection(::Selection* selection)
    : RenderSelection(selection, false) {
}

bool IsPopupWindow(HWND hwnd) {
  while (hwnd) {
    auto const dwStyle = static_cast<DWORD>(::GetWindowLong(hwnd, GWL_STYLE));
    if (dwStyle & WS_POPUP)
      return true;
    if (!(dwStyle & WS_CHILD))
      return false;
    hwnd = ::GetParent(hwnd);
  }
  return false;
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// TextEditWindow
//
TextEditWindow::TextEditWindow(const dom::TextWindow& text_window)
    : ContentWindow(text_window.window_id()),
      caret_(new Caret()),
      m_gfx(nullptr),
      m_lCaretPosn(-1),
      text_renderer_(new TextRenderer(text_window.document()->buffer())),
      selection_(text_window.view_selection()),
      #if SUPPORT_IME
        m_fImeTarget(false),
        m_lImeEnd(0),
        m_lImeStart(0),
      #endif // SUPPORT_IME
      vertical_scroll_bar_(new ui::ScrollBar(ui::ScrollBar::Type::Vertical,
                                             this)),
      view_start_(0) {
  AppendChild(vertical_scroll_bar_);
}

TextEditWindow::~TextEditWindow() {
}

text::Buffer* TextEditWindow::buffer() const {
  return text_renderer_->GetBuffer();
}

bool TextEditWindow::is_selection_active() const {
  if (has_focus())
    return true;

  if (!IsPopupWindow(::GetFocus()))
    return false;

  auto const edit_pane = views::FrameList::instance()->active_frame()->
    GetActivePane()->as<EditPane>();
  return edit_pane && edit_pane->GetActiveWindow() == this;
}

Posn TextEditWindow::computeGoalX(float xGoal, Posn lGoal) {
  if (xGoal < 0)
    return lGoal;

  RenderSelection selection(selection_);
  if (!text_renderer_->ShouldFormat(selection)) {
    if (auto const line = text_renderer_->FindLine(lGoal))
      return line->MapXToPosn(*m_gfx, xGoal);
  }

  auto lStart = buffer()->ComputeStartOfLine(lGoal);
  for (;;) {
    auto const pLine = text_renderer_->FormatLine(lStart);
    auto const lEnd = pLine->GetEnd();
    if (lGoal < lEnd)
      return pLine->MapXToPosn(*m_gfx, xGoal);
    lStart = lEnd;
  }
}

text::Posn TextEditWindow::ComputeMotion(
    Unit eUnit, Count n, const gfx::PointF& pt, text::Posn lPosn) {
  UI_ASSERT_DOM_LOCKED();
  switch (eUnit) {
    case Unit_WindowLine:
      if (n > 0) {
        auto const lBufEnd = buffer()->GetEnd();
        auto lGoal = lPosn;
        auto k = 0;
        for (k = 0; k < n; ++k) {
          lGoal = EndOfLine(lGoal);
          if (lGoal >= lBufEnd)
            break;
          ++lGoal;
        }
        return computeGoalX(pt.x, std::min(lGoal, lBufEnd));
      }
      if (n < 0) {
        n = -n;
        auto const lBufStart = buffer()->GetStart();
        auto lStart = lPosn;
        auto k = 0;
        for (k = 0; k < n; ++k) {
          lStart = StartOfLine(lStart);
          if (lStart <= lBufStart)
            break;
          --lStart;
        }

        return computeGoalX(pt.x, std::max(lStart, lBufStart));
      }
      return lPosn;

    case Unit_Screen: {
      auto k = LargeScroll(0, n, false);
      if (k > 0) {
        auto const lStart = text_renderer_->GetStart();
        view_start_ = lStart;
        return MapPointToPosn(pt);
      }
      if (n > 0)
        return std::min(GetEnd(), buffer()->GetEnd());
      if (n < 0)
        return GetStart();
      return lPosn;
    }

    case Unit_Window:
      if (n > 0)
        return std::min(GetEnd(), buffer()->GetEnd());
      if (n < 0)
        return GetStart();
      return lPosn;
  }

  LOG(ERROR) << "Unsupported unit " << eUnit;
  return lPosn;
}

void TextEditWindow::DidChangeHierarchy() {
  ParentClass::DidChangeHierarchy();
  m_gfx = &frame().gfx();
  text_renderer_->SetGraphics(m_gfx);
}

void TextEditWindow::DidHide() {
  // Note: It is OK that hidden window have focus.
  vertical_scroll_bar_->Hide();
  text_renderer_->Reset();
}

void TextEditWindow::DidKillFocus(ui::Widget* focused_widget) {
  ParentClass::DidKillFocus(focused_widget);
  caret_->Give(this, m_gfx);
  text_renderer_->Reset();
}

void TextEditWindow::DidRealize() {
  ParentClass::DidRealize();
  auto const frame = Frame::FindFrame(*this);
  ASSERT(frame);
  m_gfx = &frame->gfx();
  text_renderer_->SetGraphics(m_gfx);
}

void TextEditWindow::DidSetFocus(ui::Widget* last_focused) {
  ASSERT(has_focus());
  // Note: It is OK to set focus to hidden window.
  caret_->Take(this);
  ParentClass::DidSetFocus(last_focused);
}

void TextEditWindow::DidShow() {
  vertical_scroll_bar_->Show();
}

Posn TextEditWindow::EndOfLine(Posn lPosn) {
  UI_ASSERT_DOM_LOCKED();
  RenderSelection selection(selection_);
  if (!text_renderer_->ShouldFormat(selection)) {
    auto const pLine = text_renderer_->FindLine(lPosn);
    if (pLine)
      return pLine->GetEnd() - 1;
  }

  auto const lBufEnd = buffer()->GetEnd();
  if (lPosn >= lBufEnd)
    return lBufEnd;

  auto lStart = buffer()->ComputeStartOfLine(lPosn);
  for (;;) {
    auto const pLine = text_renderer_->FormatLine(lStart);
    lStart = pLine->GetEnd();
    if (lPosn < lStart)
      return lStart - 1;
  }
}

HCURSOR TextEditWindow::GetCursorAt(const Point& point) const {
  if (vertical_scroll_bar_->rect().Contains(point))
    return ::LoadCursor(nullptr, IDC_ARROW);
  return ::LoadCursor(nullptr, IDC_IBEAM);
}

// For Selection.MoveDown Screen
Posn TextEditWindow::GetEnd() {
  UI_ASSERT_DOM_LOCKED();
  updateScreen();
  return text_renderer_->GetEnd();
}

//For Selection.MoveUp Screen
Posn TextEditWindow::GetStart() {
  UI_ASSERT_DOM_LOCKED();
  updateScreen();
  return text_renderer_->GetStart();
}
// Description:
// Maps position specified buffer position and returns height
// of caret, If specified buffer position isn't in window, this function
// returns 0.
gfx::RectF TextEditWindow::HitTestTextPosition(Posn lPosn) {
  DCHECK_GE(lPosn, 0);
  UI_ASSERT_DOM_LOCKED();
  updateScreen();
  for (;;) {
    if (auto rect = text_renderer_->HitTestTextPosition(lPosn))
      return rect;
    text_renderer_->ScrollToPosn(lPosn);
  }
}

int TextEditWindow::LargeScroll(int, int iDy, bool fRender) {
  UI_ASSERT_DOM_LOCKED();
  updateScreen();

  auto k = 0;
  if (iDy < 0) {
    // Scroll Down -- place top line out of window.
    iDy = -iDy;

    auto const lBufStart = buffer()->GetStart();
    for (k = 0; k < iDy; ++k) {
      auto const lStart = text_renderer_->GetStart();
      if (lStart == lBufStart)
        break;

      // Scroll down until page start goes out to page.
      do {
        if (!text_renderer_->ScrollDown())
          break;
      } while (text_renderer_->GetEnd() != lStart);
    }
  } else if (iDy > 0) {
    // Scroll Up -- format page from page end.
    const Posn lBufEnd = buffer()->GetEnd();
    for (k = 0; k < iDy; ++k) {
      auto const lStart = text_renderer_->GetEnd();
      if (lStart >= lBufEnd)
        break;
      text_renderer_->Format(lStart);
    }
  }

  if (fRender && k > 0)
    Render();
  return k;
}

void TextEditWindow::MakeSelectionVisible() {
  m_lCaretPosn = -1;
  Redraw();
}

Posn TextEditWindow::MapPointToPosn(const gfx::PointF pt) {
  updateScreen();
  return std::min(text_renderer_->MapPointToPosn(pt), buffer()->GetEnd());
}

void TextEditWindow::OnDraw(gfx::Graphics*) {
  UI_ASSERT_DOM_LOCKED();
  text_renderer_->Reset();
  Redraw();
}

LRESULT TextEditWindow::OnMessage(uint uMsg, WPARAM wParam, LPARAM lParam) {
  switch (uMsg) {
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

void TextEditWindow::Redraw() {
  auto const selection_is_active = is_selection_active();

  UI_ASSERT_DOM_LOCKED();

  RenderSelection selection (selection_, selection_is_active);

  auto const lSelStart = selection_->GetStart();
  auto const lSelEnd = selection_->GetEnd();

  Posn lCaretPosn;
  if (selection_is_active) {
    lCaretPosn = selection_->IsStartActive() ? lSelStart : lSelEnd;
  } else {
    lCaretPosn = m_lCaretPosn == -1 ? lSelStart : m_lCaretPosn;

    Posn lEnd = buffer()->GetEnd();
    if (lSelStart == lEnd && lSelEnd == lEnd)
      lCaretPosn = lEnd;
  }

  DCHECK_GE(lCaretPosn, 0);

  if (text_renderer_->ShouldFormat(selection, selection_is_active)) {
    text_renderer_->Prepare(selection);
    text_renderer_->Format(StartOfLine(view_start_));

    if (m_lCaretPosn != lCaretPosn) {
      // FIXME 2007-05-12 Fill the page with lines.
      text_renderer_->ScrollToPosn(lCaretPosn);
      m_lCaretPosn = lCaretPosn;
    }
  } else if (m_lCaretPosn != lCaretPosn) {
    text_renderer_->Prepare(selection);
    text_renderer_->ScrollToPosn(lCaretPosn);
    m_lCaretPosn = lCaretPosn;
  } else if (text_renderer_->GetStart() != view_start_) {
    text_renderer_->Prepare(selection);
    text_renderer_->Format(StartOfLine(view_start_));
  } else if (!text_renderer_->ShouldRender()) {
    // The screen is clean.
    if (!m_fImeTarget)
      caret_->Blink(m_gfx);
    return;
  }

  Render();
}

void TextEditWindow::Render() {
  if (!is_shown())
    return;

  gfx::Graphics::DrawingScope drawing_scope(*m_gfx);
  Caret::Updater caret_updater(caret_.get());
  m_gfx->set_dirty_rect(rect());
  text_renderer_->Render();

  view_start_ = text_renderer_->GetStart();
  updateScrollBar();

  const auto char_rect = text_renderer_->HitTestTextPosition(m_lCaretPosn);
  if (char_rect.empty()) {
    caret_updater.Clear();
    return;
  }

  auto const caret_width = std::max(::GetSystemMetrics(SM_CXBORDER), 2);
  gfx::RectF caret_rect(char_rect.left, char_rect.top,
                        std::min(char_rect.left + caret_width,
                                 static_cast<float>(rect().right)),
                        std::min(char_rect.bottom,
                                 static_cast<float>(rect().bottom)));
  #if SUPPORT_IME
    if (m_fImeTarget) {
      POINT pt = {
        static_cast<int>(caret_rect.left),
        static_cast<int>(caret_rect.top)
      };

      SIZE size = {
        static_cast<int>(caret_rect.width()),
        static_cast<int>(caret_rect.height())
      };

      if (showImeCaret(size, pt))
        return;
    }
  #endif // SUPPORT_IME

  caret_updater.Update(m_gfx, caret_rect);
}

int TextEditWindow::SmallScroll(int, int iDy) {
  UI_ASSERT_DOM_LOCKED();
  updateScreen();

  if (iDy < 0) {
    iDy = -iDy;

    auto const lBufStart = buffer()->GetStart();
    auto lStart = text_renderer_->GetStart();
    int k;
    for (k = 0; k < iDy; ++k) {
      if (lStart == lBufStart)
        break;
        lStart = StartOfLine(lStart - 1);
    }

    if (k > 0) {
      text_renderer_->Format(lStart);
      Render();
    }
    return k;
  }

  if (iDy > 0) {
    auto const lBufEnd = buffer()->GetEnd();
    int k;
    for (k = 0; k < iDy; ++k) {
      if (text_renderer_->GetEnd() >= lBufEnd) {
          // Make sure whole line of buffer end is visible.
          text_renderer_->ScrollToPosn(lBufEnd);
          ++k;
          break;
      }

      if (!text_renderer_->ScrollUp())
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
  if (lPosn <= 0 )
    return 0;

  RenderSelection selection(selection_);
  if (!text_renderer_->ShouldFormat(selection)) {
    auto const pLine = text_renderer_->FindLine(lPosn);
    if (pLine)
      return pLine->GetStart();
  }

  auto lStart = buffer()->ComputeStartOfLine(lPosn);
  if (!lStart)
    return 0;

  for (;;) {
    auto const pLine = text_renderer_->FormatLine(lStart);
    auto const lEnd = pLine->GetEnd();
    if (lPosn < lEnd)
      return pLine->GetStart();
    lStart = lEnd;
  }
}

void TextEditWindow::updateScreen() {
  UI_ASSERT_DOM_LOCKED();
  RenderSelection selection(selection_, is_selection_active());
  if (!text_renderer_->ShouldFormat(selection))
    return;
  text_renderer_->Prepare(selection);
  auto const line_start_ = StartOfLine(view_start_);
  text_renderer_->Format(line_start_);
}

void TextEditWindow::updateScrollBar() {
  auto const lBufEnd = buffer()->GetEnd() + 1;
  ui::ScrollBar::Data data;
  data.minimum = 0;
  data.thumb_size = text_renderer_->GetVisibleEnd() -
        text_renderer_->GetStart();
  data.thumb_value = text_renderer_->GetStart();
  // Current screen shows entire buffer. We disable scroll bar.
  data.maximum = data.thumb_size < lBufEnd ? lBufEnd : 0;
  vertical_scroll_bar_->SetData(data);
}

#if SUPPORT_IME

// See Also Caret::Show for moving candidate window.

#include <imm.h>
#pragma comment(lib, "imm32.lib")

css::Style* g_rgpImeStyleConverted[2];
css::Style* g_pImeStyleInput;
css::Style* g_pImeStyleTargetConverted;
css::Style* g_pImeStyleTargetNotConverted;

#define GCS_COMPSTRATTR (GCS_COMPSTR | GCS_COMPATTR | GCS_CURSORPOS)

namespace {

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

Posn ReplaceSelection(Selection* selection, const base::char16* text,
                      size_t length) {
  auto const range = selection->range();
  range->SetText(base::string16(text, length));
  auto const end = range->GetEnd();
  range->SetStart(end);
  selection->SetStartIsActive(false);
  return end;
}

}  // namespace

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
      m_lImeEnd = ReplaceSelection(GetSelection(), rgwch, cwch);
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
    m_lImeEnd = ReplaceSelection(GetSelection(), rgwch, cwch);
    range->SetRange(m_lImeStart + lCursor, m_lImeStart + lCursor);

    if (!g_pImeStyleInput) {
          // Converted[0]
          g_rgpImeStyleConverted[0] = new css::Style();
          g_rgpImeStyleConverted[0]->set_text_decoration(
              css::TextDecoration::ImeInactiveA);

          // Converted[1]
          g_rgpImeStyleConverted[1] = new css::Style();
          g_rgpImeStyleConverted[1]->set_text_decoration(
              css::TextDecoration::ImeInactiveB);

          // Input
          g_pImeStyleInput = new css::Style();
          g_pImeStyleInput->set_text_decoration(
              css::TextDecoration::ImeInput);

          // Target Converted
          g_pImeStyleTargetConverted = new css::Style();
          g_pImeStyleTargetConverted->set_text_decoration(
              css::TextDecoration::ImeActive);

          // Target Not Converted
          g_pImeStyleTargetNotConverted = new css::Style();
          g_pImeStyleTargetNotConverted->set_bgcolor(
              css::Color(51, 153, 255));
          g_pImeStyleTargetNotConverted->set_color(css::Color(255, 255, 255));
          g_pImeStyleTargetNotConverted->set_text_decoration(
              css::TextDecoration::None);
      }

      m_fImeTarget = false;
      Posn lEnd = static_cast<Posn>(m_lImeStart + cwch);
      Posn lPosn = m_lImeStart;
      int iClause = 0;
      int iConverted = 0;
      while (lPosn < lEnd) {
        const css::Style* pStyle;
        switch (rgbAttr[lPosn - m_lImeStart]) {
          case ATTR_INPUT:
          case ATTR_INPUT_ERROR:
            pStyle = g_pImeStyleInput;
            iConverted = 0;
            break;

          case ATTR_TARGET_CONVERTED:
            pStyle = g_pImeStyleTargetConverted;
            m_fImeTarget = true;
            iConverted = 0;
            break;

          case ATTR_TARGET_NOTCONVERTED:
            pStyle = g_pImeStyleTargetNotConverted;
            m_fImeTarget = true;
            iConverted = 0;
            break;

          case ATTR_CONVERTED:
            pStyle = g_rgpImeStyleConverted[iConverted];
            iConverted = 1 - iConverted;
            break;

          default:
            pStyle = g_pImeStyleInput;
            break;
        }

        ++iClause;
        Posn lNext = static_cast<Posn>(m_lImeStart + rgnClause[iClause]);
        buffer()->SetStyle(lPosn, lNext, *pStyle);
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
  buffer()->GetText(pwch, lStart, lEnd);
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

// text::BufferMutationObserver
void TextEditWindow::DidDeleteAt(text::Posn offset, size_t length) {
  if (view_start_ <= offset)
    return;
  view_start_ = std::max(static_cast<text::Posn>(view_start_ - length),
                         offset);
}

void TextEditWindow::DidInsertAt(text::Posn offset, size_t length) {
  if (view_start_ <= offset)
    return;
  view_start_ += length;
}

// ui::ScrollBarObserver
void TextEditWindow::DidClickLineDown() {
  UI_DOM_AUTO_LOCK_SCOPE();
  SmallScroll(0, 1);
}

void TextEditWindow::DidClickLineUp() {
  UI_DOM_AUTO_LOCK_SCOPE();
  SmallScroll(0, -1);
}

void TextEditWindow::DidClickPageDown() {
  UI_DOM_AUTO_LOCK_SCOPE();
  LargeScroll(0, 1);
}

void TextEditWindow::DidClickPageUp() {
  UI_DOM_AUTO_LOCK_SCOPE();
  LargeScroll(0, -1);
}

void TextEditWindow::DidMoveThumb(int value) {
  UI_DOM_AUTO_LOCK_SCOPE();
  auto const start = StartOfLine(value);
  text_renderer_->Format(start);
  Render();
}

// ui::Widget
void TextEditWindow::DidResize() {
  views::ContentWindow::DidResize();
  UI_DOM_AUTO_LOCK_SCOPE();
  auto const scroll_bar_width = ::GetSystemMetrics(SM_CXVSCROLL);
  auto text_block_rect = rect();
  text_block_rect.right -= scroll_bar_width;
  text_renderer_->SetRect(text_block_rect);

  auto scroll_bar_rect = rect();
  scroll_bar_rect.left = text_block_rect.right;
  vertical_scroll_bar_->ResizeTo(scroll_bar_rect);
}

// views::Window
bool TextEditWindow::OnIdle(int) {
  if (!is_shown())
    return false;
  Redraw();
  return false;
}
