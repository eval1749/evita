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
#include "evita/dom/lock.h"
#include "evita/gfx/canvas.h"
#include "evita/gfx/rect_conversions.h"
#include "evita/editor/application.h"
#include "evita/editor/dom_lock.h"
#include "evita/dom/public/text_composition_data.h"
#include "evita/dom/public/view_event.h"
#include "evita/metrics/time_scope.h"
#include "evita/text/buffer.h"
#include "evita/text/marker_set.h"
#include "evita/text/range.h"
#include "evita/text/selection.h"
#include "evita/ui/animation/animator.h"
#include "evita/ui/base/ime/text_composition.h"
#include "evita/ui/base/ime/text_input_client.h"
#include "evita/ui/compositor/layer.h"
#include "evita/ui/controls/scroll_bar.h"
#include "evita/views/frame_list.h"
#include "evita/views/icon_cache.h"
#include "evita/views/text/render_selection.h"
#include "evita/views/text/text_renderer.h"
#include "evita/vi_EditPane.h"
#include "evita/vi_Frame.h"

using views::rendering::TextSelectionModel;

namespace {

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

TextSelectionModel::State GetTextSelectionState(TextEditWindow* window) {
  if (window->has_focus())
    return TextSelectionModel::State::HasFocus;

  if (!IsPopupWindow(::GetFocus()))
    return TextSelectionModel::State::Disabled;

  if (window->GetFrame() == views::FrameList::instance()->active_frame())
    return TextSelectionModel::State::Highlight;
  return TextSelectionModel::State::Disabled;
}

TextSelectionModel GetTextSelectionModel(
    TextEditWindow* window, const text::Selection& selection) {
  return TextSelectionModel(
      GetTextSelectionState(window),
      selection.anchor_offset(), selection.focus_offset());
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// TextEditWindow
//
TextEditWindow::TextEditWindow(views::WindowId window_id,
                               text::Selection* selection)
    : ContentWindow(window_id),
      canvas_(nullptr),
      m_lCaretPosn(-1),
      text_renderer_(new TextRenderer(selection->buffer())),
      selection_(selection),
      vertical_scroll_bar_(new ui::ScrollBar(ui::ScrollBar::Type::Vertical,
                                             this)) {
  AppendChild(vertical_scroll_bar_);
}

TextEditWindow::~TextEditWindow() {
}

text::Buffer* TextEditWindow::buffer() const {
  return text_renderer_->buffer();
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
        return text_renderer_->MapPointXToOffset(std::min(lGoal, lBufEnd), pt.x);
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

        return text_renderer_->MapPointXToOffset(std::max(lStart, lBufStart), pt.x);
      }
      return lPosn;

    case Unit_Screen: {
      // TODO(eval1749) We should not call |LargetScroll()| in |ComputeMotion|.
      if (LargeScroll(0, n))
        return MapPointToPosition(pt);
      if (n > 0)
        return std::min(GetEnd(), buffer()->GetEnd());
      if (n < 0)
        return GetStart();
      return lPosn;
    }

    case Unit_Window:
      if (n > 0)
        return std::max(std::min(GetEnd() - 1, buffer()->GetEnd()),
                        GetStart());
      if (n < 0)
        return GetStart();
      return lPosn;
  }

  LOG(ERROR) << "Unsupported unit " << eUnit;
  return lPosn;
}

Posn TextEditWindow::EndOfLine(text::Posn text_offset) {
  UI_ASSERT_DOM_LOCKED();
  return text_renderer_->EndOfLine(text_offset);
}

// For Selection.MoveDown Screen
Posn TextEditWindow::GetEnd() {
  UI_ASSERT_DOM_LOCKED();
  return text_renderer_->GetVisibleEnd();
}

//For Selection.MoveUp Screen
Posn TextEditWindow::GetStart() {
  UI_ASSERT_DOM_LOCKED();
  return text_renderer_->GetStart();
}
// Description:
// Maps position specified buffer position and returns height
// of caret, If specified buffer position isn't in window, this function
// returns 0.
gfx::RectF TextEditWindow::HitTestTextPosition(Posn text_offset) {
  DCHECK_GE(text_offset, 0);
  UI_ASSERT_DOM_LOCKED();
  return text_renderer_->HitTestTextPosition(text_offset);
}

bool TextEditWindow::LargeScroll(int, int iDy) {
  UI_ASSERT_DOM_LOCKED();

  auto scrolled = false;
  if (iDy < 0) {
    // Scroll Down -- place top line out of window.
    iDy = -iDy;

    auto const lBufStart = buffer()->GetStart();
    for (auto k = 0; k < iDy; ++k) {
      auto const lStart = text_renderer_->GetStart();
      if (lStart == lBufStart)
        break;

      // Scroll down until page start goes out to page.
      do {
        if (!text_renderer_->ScrollDown())
          break;
        scrolled = true;
      } while (text_renderer_->GetEnd() != lStart);
    }
  } else if (iDy > 0) {
    // Scroll Up -- format page from page end.
    const Posn lBufEnd = buffer()->GetEnd();
    for (auto k = 0; k < iDy; ++k) {
      auto const lStart = text_renderer_->GetEnd();
      if (lStart >= lBufEnd)
        break;
      text_renderer_->Format(lStart);
      scrolled = true;
    }
  }
  return scrolled;
}

Posn TextEditWindow::MapPointToPosition(const gfx::PointF pt) {
  return std::min(text_renderer_->MapPointToPosition(pt), buffer()->GetEnd());
}

void TextEditWindow::Redraw() {
  UI_ASSERT_DOM_LOCKED();

  if (!visible())
    return;

  auto const selection = GetTextSelectionModel(this, *selection_);
  Posn lCaretPosn;
  if (selection.disabled()) {
    auto const max_offset = buffer()->GetEnd();
    if (selection.start() == max_offset && selection.end() == max_offset)
      lCaretPosn = max_offset;
    else
      lCaretPosn = m_lCaretPosn == -1 ? selection.focus_offset() : m_lCaretPosn;
  } else {
    lCaretPosn = selection.focus_offset();
  }

  DCHECK_GE(lCaretPosn, 0);

  if (text_renderer_->FormatIfNeeded()) {
    if (m_lCaretPosn != lCaretPosn) {
      text_renderer_->ScrollToPosition(lCaretPosn);
      m_lCaretPosn = lCaretPosn;
    }
    Render(selection);
    return;
  }

  if (m_lCaretPosn != lCaretPosn) {
    m_lCaretPosn = lCaretPosn;
    if (text_renderer_->IsPositionFullyVisible(lCaretPosn)) {
      if (text_renderer_->ShouldRender()) {
        Render(selection);
      } else {
        gfx::Canvas::DrawingScope drawing_scope(canvas_.get());
        text_renderer_->RenderSelectionIfNeeded(canvas_.get(), selection);
        vertical_scroll_bar_->Render(canvas_.get());
      }
      return;
    }
    text_renderer_->ScrollToPosition(lCaretPosn);
    Render(selection);
    return;
  }

  if (text_renderer_->ShouldRender()) {
    Render(selection);
    return;
  }

  // The screen is clean.
  gfx::Canvas::DrawingScope drawing_scope(canvas_.get());
  text_renderer_->RenderSelectionIfNeeded(canvas_.get(), selection);
  vertical_scroll_bar_->Render(canvas_.get());
}

void TextEditWindow::Render(const TextSelectionModel& selection) {
  UI_ASSERT_DOM_LOCKED();
  if (!visible())
    return;

  gfx::Canvas::DrawingScope drawing_scope(canvas_.get());
  text_renderer_->Render(canvas_.get(), selection);

  // Update scroll bar
  {
    ui::ScrollBar::Data data;
    data.minimum = 0;
    data.thumb_size = text_renderer_->GetVisibleEnd() -
          text_renderer_->GetStart();
    data.thumb_value = text_renderer_->GetStart();
    data.maximum = buffer()->GetEnd() + 1;;
    vertical_scroll_bar_->SetData(data);
    vertical_scroll_bar_->Render(canvas_.get());
  }
}

void TextEditWindow::SetZoom(float new_zoom) {
  text_renderer_->SetZoom(new_zoom);
}

bool TextEditWindow::SmallScroll(int, int y_count) {
  UI_ASSERT_DOM_LOCKED();

  bool scrolled = false;
  if (y_count < 0) {
    for (auto k = y_count; k; ++k) {
      if (!text_renderer_->ScrollDown())
        break;
      scrolled = true;
    }
  } else if (y_count > 0) {
    for (auto k = 0; k < y_count; ++k) {
      if (!text_renderer_->ScrollUp())
        break;
      scrolled = true;
    }
 }

  return scrolled;
}

Posn TextEditWindow::StartOfLine(text::Posn text_offset) {
  UI_ASSERT_DOM_LOCKED();
  return text_renderer_->StartOfLine(text_offset);
}

void TextEditWindow::UpdateLayout() {
  DCHECK(!bounds().empty());
  auto const canvas_bounds = GetContentsBounds();
  if (canvas_)
    canvas_->SetBounds(canvas_bounds);

  auto const vertical_scroll_bar_width = static_cast<float>(
      ::GetSystemMetrics(SM_CXVSCROLL));

  auto const text_block_bounds = gfx::RectF(
      canvas_bounds.size() - gfx::SizeF(vertical_scroll_bar_width, 0.0f));

  auto const vertical_scroll_bar_bounds = gfx::RectF(
    gfx::PointF(text_block_bounds.right, text_block_bounds.top),
    gfx::SizeF(vertical_scroll_bar_width, text_block_bounds.height()));

  vertical_scroll_bar_->SetBounds(
      gfx::ToEnclosingRect(vertical_scroll_bar_bounds));

  text_renderer_->SetBounds(text_block_bounds);
}

// gfx::Canvas::Observer
void TextEditWindow::ShouldDiscardResources() {
  text_renderer_->DidLostCanvas();
}

// ui::Animatable
void TextEditWindow::Animate(base::Time) {
  if (!visible())
    return;
  ui::Animator::instance()->ScheduleAnimation(this);
  UI_DOM_AUTO_TRY_LOCK_SCOPE(lock_scope);
  if (lock_scope.locked())
    Redraw();
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
  text_renderer_->Format(value);
}

// ui::TextInputDelegate
void TextEditWindow::DidCommitComposition(
    const ui::TextComposition& composition) {
  DispatchTextCompositionEvent(domapi::EventType::TextCompositionCommit,
                               composition);
}

void TextEditWindow::DidFinishComposition() {
  DispatchTextCompositionEvent(domapi::EventType::TextCompositionEnd,
                               ui::TextComposition());
}
 
void TextEditWindow::DidStartComposition() {
  DispatchTextCompositionEvent(domapi::EventType::TextCompositionStart,
                               ui::TextComposition());
}
 
void TextEditWindow::DidUpdateComposition(
    const ui::TextComposition& composition) {
  DispatchTextCompositionEvent(domapi::EventType::TextCompositionUpdate,
                               composition);
}

ui::Widget* TextEditWindow::GetClientWindow() {
   return this;
}

// ui::Widget
void TextEditWindow::DidChangeBounds() {
  views::ContentWindow::DidChangeBounds();
  UpdateLayout();
}

void TextEditWindow::DidHide() {
  // Note: It is OK that hidden window have focus.
  views::ContentWindow::DidHide();
  canvas_.reset();
  vertical_scroll_bar_->Hide();
  text_renderer_->DidHide();
}

void TextEditWindow::DidKillFocus(ui::Widget* focused_widget) {
  views::ContentWindow::DidKillFocus(focused_widget);
  text_renderer_->DidKillFocus(canvas_.get());
  ui::TextInputClient::Get()->CommitComposition(this);
  ui::TextInputClient::Get()->CancelComposition(this);
  ui::TextInputClient::Get()->set_delegate(nullptr);
}

void TextEditWindow::DidRealize() {
  views::ContentWindow::DidRealize();
  if (bounds().empty())
    return;
  UpdateLayout();
}

void TextEditWindow::DidSetFocus(ui::Widget* last_focused) {
  ASSERT(has_focus());
  // Note: It is OK to set focus to hidden window.
  text_renderer_->DidSetFocus();
  ui::TextInputClient::Get()->set_delegate(this);
  views::ContentWindow::DidSetFocus(last_focused);
}

void TextEditWindow::DidShow() {
  views::ContentWindow::DidShow();
  DCHECK(!canvas_);
  canvas_.reset(layer()->CreateCanvas());
  vertical_scroll_bar_->Show();
}

HCURSOR TextEditWindow::GetCursorAt(const Point&) const {
  return ::LoadCursor(nullptr, IDC_IBEAM);
}

// views::ContentWindow
void TextEditWindow::MakeSelectionVisible() {
  m_lCaretPosn = -1;
}
