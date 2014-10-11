// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/views/forms/form_window.h"

#include <algorithm>
#include <unordered_map>
#include <unordered_set>

#include "base/logging.h"
#include "common/tree/child_nodes.h"
#include "common/win/win32_verify.h"
#include "evita/editor/dom_lock.h"
#include "evita/gfx_base.h"
#include "evita/dom/forms/form.h"
#include "evita/dom/forms/form_control.h"
#include "evita/dom/forms/form_observer.h"
#include "evita/dom/forms/form_window.h"
#include "evita/dom/forms/button_control.h"
#include "evita/dom/forms/checkbox_control.h"
#include "evita/dom/forms/label_control.h"
#include "evita/dom/forms/radio_button_control.h"
#include "evita/dom/forms/text_field_control.h"
#include "evita/dom/forms/text_field_selection.h"
#include "evita/dom/lock.h"
#include "evita/editor/application.h"
#include "evita/ui/animation/animator.h"
#include "evita/ui/controls/button_control.h"
#include "evita/ui/controls/checkbox_control.h"
#include "evita/ui/controls/label_control.h"
#include "evita/ui/controls/radio_button_control.h"
#include "evita/ui/controls/text_field_control.h"
#include "evita/ui/system_metrics.h"
#include "evita/views/forms/form_control_controller.h"
#include "evita/views/switches.h"

#define DEBUG_PAINT 0

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

  protected: ui::Control::Style ComputeStyle() const;
  public: virtual ui::Control* CreateWidget() = 0;
  protected: void Update(const dom::FormControl* control, ui::Widget* widget);
  public: virtual void UpdateWidget(ui::Widget* widget) = 0;

  DISALLOW_COPY_AND_ASSIGN(ControlImporter);
};

ui::Control::Style ControlImporter::ComputeStyle() const {
  ui::CheckboxControl::Style style;
  style.bgcolor = ui::SystemMetrics::instance()->bgcolor();
  style.color = ui::SystemMetrics::instance()->color();
  style.font_family = ui::SystemMetrics::instance()->font_family();
  style.font_size = ui::SystemMetrics::instance()->font_size();
  style.gray_text = ui::SystemMetrics::instance()->gray_text();
  style.highlight = ui::SystemMetrics::instance()->highlight();
  style.hotlight = ui::SystemMetrics::instance()->hotlight();
  style.shadow = ui::SystemMetrics::instance()->shadow();
  return style;
}

void ControlImporter::Update(const dom::FormControl* control,
                             ui::Widget* widget) {
  gfx::Rect rect(
      gfx::Point(static_cast<int>(control->client_left()),
                 static_cast<int>(control->client_top())),
      gfx::Size(static_cast<int>(control->client_width()),
                static_cast<int>(control->client_height())));
  DCHECK(!rect.empty());
  widget->SetBounds(rect);

  if (auto const control_widget = widget->as<ui::Control>())
    control_widget->set_disabled(control->disabled());
}

//////////////////////////////////////////////////////////////////////
//
// ButtonImporter
//
class ButtonImporter : public ControlImporter {
  private: const dom::ButtonControl* const button_;

  public: ButtonImporter(const dom::ButtonControl* checkbox);
  public: virtual ~ButtonImporter() = default;

  // ControlImporter
  private: virtual ui::Control* CreateWidget() override;
  private: virtual void UpdateWidget(ui::Widget* widget) override;

  DISALLOW_COPY_AND_ASSIGN(ButtonImporter);
};

ButtonImporter::ButtonImporter(const dom::ButtonControl* button)
    : button_(button) {
}

ui::Control* ButtonImporter::CreateWidget() {
  auto const widget = new ui::ButtonControl(
      new FormControlController(button_->event_target_id()),
      button_->text(), ComputeStyle());
  Update(button_, widget);
  return widget;
}

void ButtonImporter::UpdateWidget(ui::Widget* widget) {
  auto const button_widget = widget->as<ui::ButtonControl>();
  button_widget->set_style(ComputeStyle());
  button_widget->set_text(button_->text());
  Update(button_, button_widget);
}

//////////////////////////////////////////////////////////////////////
//
// CheckboxImporter
//
class CheckboxImporter : public ControlImporter {
  private: const dom::CheckboxControl* const checkbox_;

  public: CheckboxImporter(const dom::CheckboxControl* checkbox);
  public: virtual ~CheckboxImporter() = default;

  // ControlImporter
  private: virtual ui::Control* CreateWidget() override;
  private: virtual void UpdateWidget(ui::Widget* widget) override;

  DISALLOW_COPY_AND_ASSIGN(CheckboxImporter);
};

CheckboxImporter::CheckboxImporter(const dom::CheckboxControl* checkbox)
    : checkbox_(checkbox) {
}

ui::Control* CheckboxImporter::CreateWidget() {
  auto const widget = new ui::CheckboxControl(
      new FormControlController(checkbox_->event_target_id()),
      checkbox_->checked(), ComputeStyle());
  Update(checkbox_, widget);
  return widget;
}

void CheckboxImporter::UpdateWidget(ui::Widget* widget) {
  auto const checkbox_widget = widget->as<ui::CheckboxControl>();
  checkbox_widget->set_checked(checkbox_->checked());
  checkbox_widget->set_style(ComputeStyle());
  Update(checkbox_, checkbox_widget);
}

//////////////////////////////////////////////////////////////////////
//
// LabelImporter
//
class LabelImporter : public ControlImporter {
  private: const dom::LabelControl* const label_;

  public: LabelImporter(const dom::LabelControl* label);
  public: virtual ~LabelImporter() = default;

  // ControlImporter
  private: virtual ui::Control* CreateWidget() override;
  private: virtual void UpdateWidget(ui::Widget* widget) override;

  DISALLOW_COPY_AND_ASSIGN(LabelImporter);
};

LabelImporter::LabelImporter(const dom::LabelControl* label)
    : label_(label) {
}

ui::Control* LabelImporter::CreateWidget() {
  auto const widget = new ui::LabelControl(
      new FormControlController(label_->event_target_id()),
      label_->text(), ComputeStyle());
  Update(label_, widget);
  return widget;
}

void LabelImporter::UpdateWidget(ui::Widget* widget) {
  auto const label_widget = widget->as<ui::LabelControl>();
  label_widget->set_style(ComputeStyle());
  label_widget->set_text(label_->text());
  Update(label_, label_widget);
}

//////////////////////////////////////////////////////////////////////
//
// RadioButtonImporter
//
class RadioButtonImporter : public ControlImporter {
  private: const dom::RadioButtonControl* const radio_button_;

  public: RadioButtonImporter(const dom::RadioButtonControl* radio_button);
  public: virtual ~RadioButtonImporter() = default;

  // ControlImporter
  private: virtual ui::Control* CreateWidget() override;
  private: virtual void UpdateWidget(ui::Widget* widget) override;

  DISALLOW_COPY_AND_ASSIGN(RadioButtonImporter);
};

RadioButtonImporter::RadioButtonImporter(
    const dom::RadioButtonControl* radio_button)
    : radio_button_(radio_button) {
}

ui::Control* RadioButtonImporter::CreateWidget() {
  auto const widget = new ui::RadioButtonControl(
      new FormControlController(radio_button_->event_target_id()),
      radio_button_->checked(), ComputeStyle());
  Update(radio_button_, widget);
  return widget;
}

void RadioButtonImporter::UpdateWidget(ui::Widget* widget) {
  auto const radio_button_widget = widget->as<ui::RadioButtonControl>();
  radio_button_widget->set_checked(radio_button_->checked());
  radio_button_widget->set_style(ComputeStyle());
  Update(radio_button_, radio_button_widget);
}

//////////////////////////////////////////////////////////////////////
//
// TextFieldImporter
//
class TextFieldImporter : public ControlImporter {
  private: const dom::TextFieldControl* const text_field_;

  public: TextFieldImporter(const dom::TextFieldControl* text_field);
  public: virtual ~TextFieldImporter() = default;

  private: ui::TextFieldControl::Selection ImportSelection() const;

  // ControlImporter
  private: virtual ui::Control* CreateWidget() override;
  private: virtual void UpdateWidget(ui::Widget* widget) override;

  DISALLOW_COPY_AND_ASSIGN(TextFieldImporter);
};

TextFieldImporter::TextFieldImporter(const dom::TextFieldControl* text_field)
    : text_field_(text_field) {
}

ui::TextFieldControl::Selection TextFieldImporter::ImportSelection() const {
  ui::TextFieldControl::Selection selection;
  auto const dom_selection = text_field_->selection();
  selection.anchor_offset= static_cast<size_t>(dom_selection->anchor_offset());
  selection.focus_offset= static_cast<size_t>(dom_selection->focus_offset());
  return selection;
}

ui::Control* TextFieldImporter::CreateWidget() {
  auto const controller = new FormControlController(
    text_field_->event_target_id());
  auto const widget = new ui::TextFieldControl(controller,
                                               ImportSelection(),
                                               text_field_->value(),
                                               ComputeStyle());
  widget->set_text_input_delegate(controller);
  Update(text_field_, widget);
  return widget;
}

void TextFieldImporter::UpdateWidget(ui::Widget* widget) {
  auto const text_field_widget = widget->as<ui::TextFieldControl>();
  text_field_widget->set_selection(ImportSelection());
  text_field_widget->set_style(ComputeStyle());
  text_field_widget->set_text(text_field_->value());
  Update(text_field_, text_field_widget);
}

//////////////////////////////////////////////////////////////////////
//
// ControlImporter
//
ControlImporter* ControlImporter::Create(const dom::FormControl* control) {
  if (auto const button = control->as<dom::ButtonControl>())
    return new ButtonImporter(button);
  if (auto const checkbox = control->as<dom::CheckboxControl>())
    return new CheckboxImporter(checkbox);
  if (auto const label = control->as<dom::LabelControl>())
    return new LabelImporter(label);
  if (auto const radio_button = control->as<dom::RadioButtonControl>())
    return new RadioButtonImporter(radio_button);
  if (auto const text_field = control->as<dom::TextFieldControl>())
    return new TextFieldImporter(text_field);
  NOTREACHED() << "Unsupported form control " <<
      control->wrapper_info()->class_name();
  return nullptr;
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// FormWindow::FormViewModel
//
class FormWindow::FormViewModel final : private dom::FormObserver  {
  private: bool dirty_;
  private: ui::Control* focus_control_;
  private: const dom::Form* const form_;
  private: std::unordered_map<domapi::EventTargetId, ui::Control*> map_;
  private: gfx::Size size_;
  private: base::string16 title_;
  private: FormWindow* const window_;

  public: FormViewModel(const dom::Form* form, FormWindow* window);
  public: virtual ~FormViewModel();

  public: bool dirty() const { return dirty_; }
  public: ui::Control* focus_control() const { return focus_control_; }
  public: const gfx::Size& size() const { return size_; }
  public: const base::string16& title() const { return title_; }

  // Update DOM form control map
  public: void Update();

  // dom::FormObserver
  private: virtual void DidChangeForm() override;

  DISALLOW_COPY_AND_ASSIGN(FormViewModel);
};

FormWindow::FormViewModel::FormViewModel(const dom::Form* form,
                                         FormWindow* window)
    : dirty_(true), focus_control_(nullptr), form_(form), window_(window) {
  form_->AddObserver(this);
}

FormWindow::FormViewModel::~FormViewModel() {
  form_->RemoveObserver(this);
}

void FormWindow::FormViewModel::Update() {
  UI_ASSERT_DOM_LOCKED();
  DCHECK(dirty_);

  // Window size should not be zero.
  auto const kMinHeight = 10;
  auto const kMinWidth = 10;
  size_ = gfx::Size(std::max(static_cast<int>(form_->width()), kMinWidth),
                    std::max(static_cast<int>(form_->height()), kMinHeight));
  title_ = form_->title();

  // Temporary remove controls from window.
  std::unordered_set<ui::Control*> controls_to_remove;
  auto current_focus = static_cast<ui::Control*>(nullptr);
  while (auto const child = window_->first_child()) {
    auto const control = child->as<ui::Control>();
    if (!control)
      continue;
    if (control->has_focus())
      current_focus = control;
    window_->RemoveChild(control);
    controls_to_remove.insert(control);
  }

  // Get or create control from DOM form.
  focus_control_ = static_cast<ui::Control*>(nullptr);
  auto focusable = static_cast<ui::Control*>(nullptr);
  for (auto control : form_->controls()) {
    std::unique_ptr<ControlImporter> importer(
        ControlImporter::Create(control));
    auto const it = map_.find(control->event_target_id());
    auto widget = static_cast<ui::Control*>(nullptr);
    if (it == map_.end()) {
      widget = importer->CreateWidget();
      map_[control->event_target_id()] = widget;
      widget->SetParentWidget(window_);
    } else {
      widget = it->second;
      window_->AppendChild(widget);
      importer->UpdateWidget(widget);
      window_->SchedulePaintInRect(widget->bounds());
      controls_to_remove.erase(widget);
    }
    DCHECK(!widget->bounds().empty());
    if (widget->focusable()) {
      if (!focusable)
        focusable = widget;
      if (control == form_->focus_control())
        focus_control_ = widget;
    }
  }

  // Move focus if needed
  if (!focus_control_)
    focus_control_ = focusable;
  if (focus_control_ && focus_control_ != current_focus &&
      (current_focus || window_->has_focus())) {
    focus_control_->RequestFocus();
  }

  // Destroy removed controls
  for (auto child : controls_to_remove) {

    window_->SchedulePaintInRect(child->bounds());
    child->DestroyWidget();
  }
  dirty_ = false;
}

// dom::FormObserver
void FormWindow::FormViewModel::DidChangeForm() {
  // TODO(eval1749) Schedule animation for FrameWindow.
  ASSERT_DOM_LOCKED();
  dirty_ = true;
}

//////////////////////////////////////////////////////////////////////
//
// FormWindow
//
FormWindow::FormWindow(dom::WindowId window_id, dom::Form* form,
                       Window* owner, gfx::Point offset)
    : views::Window(ui::NativeWindow::Create(this), window_id),
      model_(new FormViewModel(form, this)),
      offset_(offset), owner_(owner) {
}

FormWindow::FormWindow(dom::WindowId window_id, dom::Form* form)
    : FormWindow(window_id, form, nullptr, gfx::Point()) {
}

FormWindow::~FormWindow() {
}

void FormWindow::DoRealizeWidget() {
  UI_ASSERT_DOM_LOCKED();
  DCHECK(!is_realized());
  DCHECK(form_size_.empty());
  model_->Update();
  form_size_ = model_->size();
  title_ = model_->title();
  views::Window::RealizeWidget();
}

void FormWindow::TransferFocusIfNeeded() {
  auto const focus_control = model_->focus_control();
  if (!focus_control)
    return;
  if (!has_focus())
    return;
  focus_control->RequestFocus();
}

// ui::Animatable
void FormWindow::DidBeginAnimationFrame(base::Time) {
  DCHECK(is_realized());
  if (!visible())
    return;

  // TODO(eval1749) We should call |RequestAnimationFrame()| only if needed.
  RequestAnimationFrame();

  if (model_->dirty()) {
    UI_DOM_AUTO_TRY_LOCK_SCOPE(lock_scope);
    if (lock_scope.locked()) {
      model_->Update();

      if (form_size_ != model_->size()) {
        form_size_ = model_->size();
        RECT window_rect;
        ::GetWindowRect(AssociatedHwnd(), &window_rect);
        window_rect.right = window_rect.left + form_size_.width();
        window_rect.bottom = window_rect.top + form_size_.height();
        auto const extended_window_style = static_cast<DWORD>(
            ::GetWindowLong(AssociatedHwnd(), GWL_EXSTYLE));
        auto const window_style = static_cast<DWORD>(
            ::GetWindowLong(AssociatedHwnd(), GWL_STYLE));
        auto const has_menu = false;
        WIN32_VERIFY(::AdjustWindowRectEx(&window_rect, window_style, has_menu,
                                          extended_window_style));
        SetBounds(window_rect);
      }

      if (title_ != model_->title()) {
        title_ = model_->title();
        ::SetWindowTextW(AssociatedHwnd(), title_.c_str());
      }
    }
  }

  TransferFocusIfNeeded();
  gfx::Canvas::DrawingScope drawing_scope(canvas_.get());
  Window::OnDraw(canvas_.get());
}

// ui::SystemMetricsObserver
void FormWindow::DidChangeIconFont() {
  SchedulePaint();
}

void FormWindow::DidChangeSystemColor() {
  SchedulePaint();
}

void FormWindow::DidChangeSystemMetrics() {
  SchedulePaint();
}

// ui::Widget
void FormWindow::CreateNativeWindow() const {
  DCHECK(!form_size_.empty());

  struct Local {
    static gfx::Rect GetDefaultBounds(const gfx::Size& form_size) {
      return gfx::Rect(gfx::Point(CW_USEDEFAULT, CW_USEDEFAULT),
                       form_size);
    }

    // Compute form window bounds as center of foreground window if possible.
    static gfx::Rect ComputeFormWindowBounds(
        uint32_t extended_window_style, uint32_t window_style,
        const gfx::Size& form_size) {
      auto const foreground_hwnd = ::GetForegroundWindow();
      if (!foreground_hwnd)
        return GetDefaultBounds(form_size);
      RECT active_window_rect;
      if (!::GetWindowRect(foreground_hwnd, &active_window_rect))
        return GetDefaultBounds(form_size);
      gfx::Rect active_window_bounds(active_window_rect);
      auto window_bounds = gfx::Rect(
          active_window_bounds.origin() +
          ((active_window_bounds.size() - form_size) / 2),
          form_size);
      auto const has_menu = false;
      RECT window_rect(window_bounds);
      WIN32_VERIFY(::AdjustWindowRectEx(&window_rect, window_style,
                                        has_menu, extended_window_style));
      return gfx::Rect(window_rect);
    }
  };

  if (title_.empty()) {
    // Popup window
    DCHECK(owner_);
    auto const extended_window_style = WS_EX_LAYERED | WS_EX_TOOLWINDOW;
    auto const window_style = WS_POPUPWINDOW | WS_VISIBLE;
    auto const screen_point = owner_->MapToDesktopPoint(offset_);
    native_window()->CreateWindowEx(
      extended_window_style, window_style, L"popup",
      owner_->AssociatedHwnd(), screen_point, form_size_);
    return;
  }

  // Place dialog window at center of active window.
  auto const extended_window_style = WS_EX_LAYERED | WS_EX_TOOLWINDOW;
  auto const window_style = WS_CAPTION | WS_POPUPWINDOW | WS_VISIBLE;
  auto const form_window_bounds = Local::ComputeFormWindowBounds(
      extended_window_style, window_style, form_size_);
  native_window()->CreateWindowEx(
      extended_window_style, window_style, title_.c_str(), nullptr,
      form_window_bounds.origin(), form_window_bounds.size());
}

void FormWindow::DidChangeBounds() {
  canvas_->SetBounds(GetContentsBounds());
  {
    gfx::Canvas::DrawingScope drawing_scope(canvas_.get());
    canvas_->AddDirtyRect(GetContentsBounds());
    canvas_->Clear(ui::SystemMetrics::instance()->bgcolor());
  }
  SchedulePaint();
  Window::DidChangeBounds();
}

void FormWindow::DidDestroyWidget() {
  ui::SystemMetrics::instance()->RemoveObserver(this);
  Window::DidDestroyWidget();
}

void FormWindow::DidRealize() {
  // TODO(eval1749) We should get default value of form window transparency
  // from CSS.
  ::SetLayeredWindowAttributes(*native_window(), RGB(0, 0, 0), 80 * 255 / 100,
                               LWA_ALPHA);
  canvas_.reset(new gfx::CanvasForHwnd(*native_window()));
  ui::SystemMetrics::instance()->AddObserver(this);
  Window::DidRealize();
}

LRESULT FormWindow::OnMessage(uint32_t const message, WPARAM const wParam,
                              LPARAM const lParam) {
  if (message == WM_CLOSE) {
    // TODO(eval1749) Should we dispatch "close" event to JavaScript?
    Hide();
    return 0;
  }
  return Widget::OnMessage(message, wParam, lParam);
}

void FormWindow::RealizeWidget() {
  if (form_size_.empty()) {
    Application::instance()->RegisterTaskWithinDomLock(
        base::Bind(&FormWindow::DoRealizeWidget, base::Unretained(this)));
    return;
  }
  views::Window::RealizeWidget();
}

}  // namespace views
