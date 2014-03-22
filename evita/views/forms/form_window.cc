// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/views/forms/form_window.h"

#include <unordered_map>
#include <unordered_set>

#include "base/logging.h"
#include "common/tree/child_nodes.h"
#include "evita/editor/dom_lock.h"
#include "evita/gfx_base.h"
#include "evita/dom/forms/form.h"
#include "evita/dom/forms/form_control.h"
#include "evita/dom/forms/form_observer.h"
#include "evita/dom/forms/checkbox_control.h"
#include "evita/dom/forms/label_control.h"
#include "evita/dom/forms/radio_button_control.h"
#include "evita/dom/lock.h"
#include "evita/ui/controls/checkbox_control.h"
#include "evita/ui/controls/label_control.h"
#include "evita/ui/controls/radio_button_control.h"
#include "evita/ui/root_widget.h"

#define WIN32_VERIFY(exp) { \
  if (!(exp)) { \
    auto const last_error = ::GetLastError(); \
    DVLOG(0) << "Win32 API failure: " #exp << " error=" << last_error; \
  } \
}

namespace views {

namespace {
//////////////////////////////////////////////////////////////////////
//
// ControlImporter
//
class ControlImporter {
  protected: ControlImporter() = default;
  public: virtual ~ControlImporter() = default;

  public: static ControlImporter* Create(const dom::FormControl* control);

  public: virtual ui::Widget* CreateWidget() = 0;
  protected: void SetRect(const dom::FormControl* control, ui::Widget* widget);
  public: virtual void UpdateWidget(ui::Widget* widget) = 0;

  DISALLOW_COPY_AND_ASSIGN(ControlImporter);
};

void ControlImporter::SetRect(const dom::FormControl* control,
                              ui::Widget* widget) {
  gfx::Rect rect(
      gfx::Point(static_cast<int>(control->client_left()),
                 static_cast<int>(control->client_top())),
      gfx::Size(static_cast<int>(control->client_width()),
                static_cast<int>(control->client_height())));
  widget->ResizeTo(rect);
}

//////////////////////////////////////////////////////////////////////
//
// CheckboxImporter
//
class CheckboxImporter : public ControlImporter {
  private: const dom::CheckboxControl* const checkbox_;

  public: CheckboxImporter(const dom::CheckboxControl* checkbox);
  public: virtual ~CheckboxImporter() = default;

  private: ui::CheckboxControl::Style ComputeStyle() const;

  // ControlImporter
  private: virtual ui::Widget* CreateWidget() override;
  private: virtual void UpdateWidget(ui::Widget* widget) override;

  DISALLOW_COPY_AND_ASSIGN(CheckboxImporter);
};

CheckboxImporter::CheckboxImporter(const dom::CheckboxControl* checkbox)
    : checkbox_(checkbox) {
}

ui::CheckboxControl::Style
    CheckboxImporter::ComputeStyle() const {
  // TODO(yosi) We should get checkbox style from |dom::FormControl|.
  ui::CheckboxControl::Style style;
  style.bgcolor = gfx::ColorF(1, 1, 1);
  style.color = gfx::ColorF(gfx::ColorF::LightGray); // #D3D3D3
  return style;
}

ui::Widget* CheckboxImporter::CreateWidget() {
  auto const widget = new ui::CheckboxControl(checkbox_->checked(),
                                              ComputeStyle());
  SetRect(checkbox_, widget);
  return widget;
}

void CheckboxImporter::UpdateWidget(ui::Widget* widget) {
  auto const checkbox_widget = widget->as<ui::CheckboxControl>();
  checkbox_widget->set_style(ComputeStyle());
  checkbox_widget->set_checked(checkbox_->checked());
  SetRect(checkbox_, checkbox_widget);
}

//////////////////////////////////////////////////////////////////////
//
// LabelImporter
//
class LabelImporter : public ControlImporter {
  private: const dom::LabelControl* const label_;

  public: LabelImporter(const dom::LabelControl* label);
  public: virtual ~LabelImporter() = default;

  private: ui::LabelControl::LabelStyle ComputeLabelStyle() const;

  // ControlImporter
  private: virtual ui::Widget* CreateWidget() override;
  private: virtual void UpdateWidget(ui::Widget* widget) override;

  DISALLOW_COPY_AND_ASSIGN(LabelImporter);
};

LabelImporter::LabelImporter(const dom::LabelControl* label)
    : label_(label) {
}

ui::LabelControl::LabelStyle LabelImporter::ComputeLabelStyle() const {
  // TODO(yosi) We should get label style from |dom::FormControl|.
  ui::LabelControl::LabelStyle style;
  style.bgcolor = gfx::ColorF(1, 1, 1);
  style.color = gfx::ColorF(0, 0, 0);
  style.font_family = L"MS Shell Dlg 2";
  style.font_size = 13;
  return style;
}

ui::Widget* LabelImporter::CreateWidget() {
  auto const widget = new ui::LabelControl(label_->text(), ComputeLabelStyle());
  SetRect(label_, widget);
  return widget;
}

void LabelImporter::UpdateWidget(ui::Widget* widget) {
  auto const label_widget = widget->as<ui::LabelControl>();
  label_widget->set_style(ComputeLabelStyle());
  label_widget->set_text(label_->text());
  SetRect(label_, label_widget);
}

//////////////////////////////////////////////////////////////////////
//
// RadioButtonImporter
//
class RadioButtonImporter : public ControlImporter {
  private: const dom::RadioButtonControl* const radio_button_;

  public: RadioButtonImporter(const dom::RadioButtonControl* radio_button);
  public: virtual ~RadioButtonImporter() = default;

  private: ui::RadioButtonControl::Style ComputeStyle() const;

  // ControlImporter
  private: virtual ui::Widget* CreateWidget() override;
  private: virtual void UpdateWidget(ui::Widget* widget) override;

  DISALLOW_COPY_AND_ASSIGN(RadioButtonImporter);
};

RadioButtonImporter::RadioButtonImporter(
    const dom::RadioButtonControl* radio_button)
    : radio_button_(radio_button) {
}

ui::RadioButtonControl::Style
    RadioButtonImporter::ComputeStyle() const {
  // TODO(yosi) We should get radio_button style from |dom::FormControl|.
  ui::RadioButtonControl::Style style;
  style.bgcolor = gfx::ColorF(1, 1, 1);
  style.color = gfx::ColorF(gfx::ColorF::LightGray); // #D3D3D3
  return style;
}

ui::Widget* RadioButtonImporter::CreateWidget() {
  auto const widget = new ui::RadioButtonControl(radio_button_->checked(),
                                                 ComputeStyle());
  SetRect(radio_button_, widget);
  return widget;
}

void RadioButtonImporter::UpdateWidget(ui::Widget* widget) {
  auto const radio_button_widget = widget->as<ui::RadioButtonControl>();
  radio_button_widget->set_style(ComputeStyle());
  radio_button_widget->set_checked(radio_button_->checked());
  SetRect(radio_button_, radio_button_widget);
}

ControlImporter* ControlImporter::Create(const dom::FormControl* control) {
  if (auto const checkbox = control->as<dom::CheckboxControl>())
    return new CheckboxImporter(checkbox);
  if (auto const label = control->as<dom::LabelControl>())
    return new LabelImporter(label);
  if (auto const radio_button = control->as<dom::RadioButtonControl>())
    return new RadioButtonImporter(radio_button);
  NOTREACHED();
  return nullptr;
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// FormWindow::FormViewModel
//
class FormWindow::FormViewModel : public dom::FormObserver {
  private: bool dirty_;
  private: const dom::Form* const form_;
  private: std::unordered_map<dom::EventTargetId, ui::Widget*> map_;
  private: FormWindow* const window_;

  public: FormViewModel(const dom::Form* form, FormWindow* window);
  public: virtual ~FormViewModel();

  public: bool dirty() const { return dirty_; }

  public: void Update();

  // dom::FormObserver
  private: virtual void DidChangeForm() override;

  DISALLOW_COPY_AND_ASSIGN(FormViewModel);
};

FormWindow::FormViewModel::FormViewModel(const dom::Form* form,
                                         FormWindow* window)
    : dirty_(true), form_(form), window_(window) {
  form_->AddObserver(this);
}

FormWindow::FormViewModel::~FormViewModel() {
  form_->RemoveObserver(this);
}

void FormWindow::FormViewModel::Update() {
  UI_ASSERT_DOM_LOCKED();
  if (!dirty_)
    return;
  std::unordered_set<ui::Widget*> children_to_remove;
  while (auto const child = window_->first_child()) {
    window_->RemoveChild(child);
    children_to_remove.insert(child);
  }
  for (auto control : form_->controls()) {
    std::unique_ptr<ControlImporter> importer(
        ControlImporter::Create(control));
    auto const it = map_.find(control->event_target_id());
    if (it == map_.end()) {
      auto const widget = importer->CreateWidget();
      map_[control->event_target_id()] = widget;
      window_->AppendChild(widget);
      continue;
    }
    auto const widget = it->second;
    importer->UpdateWidget(widget);
    window_->AppendChild(widget);
    window_->SchedulePaintInRect(widget->rect());
    children_to_remove.erase(widget);
  }
  for (auto child : children_to_remove) {
    window_->SchedulePaintInRect(child->rect());
    child->DestroyWidget();
  }
  dirty_ = false;
}

// dom::FormObserver
void FormWindow::FormViewModel::DidChangeForm() {
  ASSERT_DOM_LOCKED();
  dirty_ = true;
}

//////////////////////////////////////////////////////////////////////
//
// FormWindow
//
FormWindow::FormWindow(WindowId window_id, const dom::Form* form)
    : views::Window(ui::NativeWindow::Create(*this), window_id),
      form_(form), gfx_(new gfx::Graphics()),
      model_(new FormViewModel(form, this)) {
}

FormWindow::~FormWindow() {
}

bool FormWindow::OnIdle(int) {
  if (!is_realized())
    return false;

  if (!pending_update_rect_.empty()) {
    gfx::Rect rect;
    std::swap(pending_update_rect_, rect);
    SchedulePaintInRect(rect);
  }

  if (!model_->dirty())
    return false;

  UI_DOM_AUTO_TRY_LOCK_SCOPE(lock_scope);
  if (!lock_scope.locked())
    return true;

  model_->Update();
  return false;
}

// ui::Widget
void FormWindow::CreateNativeWindow() const {
  auto const dwExStyle = WS_EX_LAYERED | WS_EX_TOOLWINDOW;
  auto const dwStyle = WS_POPUP | WS_CAPTION | WS_SYSMENU |WS_VISIBLE;
  // TODO(yosi) We should retrieve size of |FormWindow| from |form|.
  Rect contentRect(0, 0, 400, 240);
  Rect windowRect(contentRect);
  bool has_menu = false;
  WIN32_VERIFY(::AdjustWindowRectEx(&windowRect, dwStyle, has_menu, dwExStyle));

  native_window()->CreateWindowEx(
      // TODO(yosi) We should specify window title of|FormWindow|.
      dwExStyle, dwStyle, L"FormWindow", nullptr,
      gfx::Point(CW_USEDEFAULT, CW_USEDEFAULT),
      windowRect.size());
}

void FormWindow::DidCreateNativeWindow() {
 // TODO(yosi) We should get default value of form window transparency from
 //CSS.
  ::SetLayeredWindowAttributes(*native_window(), RGB(0, 0, 0), 80 * 255 / 100,
                               LWA_ALPHA);
  gfx_->Init(*native_window());
  Widget::DidCreateNativeWindow();
}

void FormWindow::DidResize() {
  gfx_->Resize(rect());
  gfx::Graphics::DrawingScope drawing_scope(*gfx_);
  (*gfx_)->Clear(gfx::ColorF(gfx::ColorF::White));
}

bool FormWindow::DoIdle(int hint) {
  bool more = false;
  for (auto widget : ui::RootWidget::instance()->child_nodes()) {
    if (auto form_window = widget->as<views::FormWindow>())
      more |= form_window->OnIdle(hint);
  }
  return more;
}

LRESULT FormWindow::OnMessage(uint32_t const uMsg, WPARAM const wParam,
                              LPARAM const lParam) {
  return Widget::OnMessage(uMsg, wParam, lParam);
}

void FormWindow::OnPaint(const gfx::Rect rect) {
  if (!editor::DomLock::instance()->locked()) {
    UI_DOM_AUTO_TRY_LOCK_SCOPE(lock_scope);
    if (lock_scope.locked()) {
      OnPaint(rect);
    } else {
      // TODO(yosi) Should we have list of dirty rectangles rather than
      // bounding dirty rectangles?
      pending_update_rect_.Unite(rect);
    }
    return;
  }

  gfx::Graphics::DrawingScope drawing_scope(*gfx_);
  for (auto child : child_nodes()) {
    child->OnDraw(&*gfx_);
  }
  gfx_->FillRectangle(gfx::Brush(*gfx_, gfx::ColorF(0.0f, 0.0f, 1.0f, 0.1f)),
                      rect);
  gfx_->DrawRectangle(gfx::Brush(*gfx_, gfx::ColorF(0.0f, 0.0f, 1.0f, 0.5f)),
                      rect, 2.0f);
}

}  // namespace views
