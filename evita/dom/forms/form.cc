// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/forms/form.h"

#include "evita/dom/forms/form_control.h"
#include "evita/dom/forms/form_observer.h"
#include "evita/dom/script_host.h"
#include "evita/dom/view_delegate.h"
#include "evita/v8_glue/converter.h"
#include "evita/v8_glue/wrapper_info.h"

namespace dom {

namespace {
//////////////////////////////////////////////////////////////////////
//
// FormClass
//
class FormClass :
    public v8_glue::DerivedWrapperInfo<Form, ViewEventTarget> {

  public: FormClass(const char* name)
      : BaseClass(name) {
  }
  public: ~FormClass() = default;

  private: virtual v8::Handle<v8::FunctionTemplate>
      CreateConstructorTemplate(v8::Isolate* isolate) override {
    return v8_glue::CreateConstructorTemplate(isolate,
        &FormClass::NewForm);
  }

  private: static Form* NewForm(const base::string16& name) {
    return new Form(name);
  }

  private: virtual void SetupInstanceTemplate(
      ObjectTemplateBuilder& builder) override {
    builder
        .SetProperty("controls", &Form::controls)
        .SetProperty("focusControl", &Form::focus_control,
                                     &Form::set_focus_control)
        .SetProperty("height", &Form::height, &Form::set_height)
        .SetProperty("name", &Form::name)
        .SetProperty("title", &Form::title, &Form::set_title)
        .SetProperty("width", &Form::width, &Form::set_width)
        .SetMethod("add", &Form::AddFormControl)
        .SetMethod("control", &Form::control)
        .SetMethod("realize", &Form::Realize)
        .SetMethod("show", &Form::Show);
  }

  DISALLOW_COPY_AND_ASSIGN(FormClass);
};
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// Form
//
DEFINE_SCRIPTABLE_OBJECT(Form, FormClass);

Form::Form(const base::string16& name)
    : height_(0.0f), name_(name), width_(0.0f) {
  if (name == L"FindDialogBox")
    ScriptHost::instance()->view_delegate()->CreateFindDialogBox(this);
}

Form::~Form() {
}

FormControl* Form::control(int control_id) const {
  auto const it = controls_.find(control_id);
  return it == controls_.end() ? nullptr : it->second;
}

std::vector<FormControl*> Form::controls() const {
  std::vector<FormControl*> controls(controls_.size());
  controls.resize(0);
  for (const auto& it : controls_) {
    controls.push_back(it.second);
  }
  return std::move(controls);
}

void Form::set_focus_control(
    v8_glue::Nullable<FormControl> new_focus_control) {
  if (focus_control_ == new_focus_control)
    return;
  focus_control_ = new_focus_control;
  FOR_EACH_OBSERVER(FormObserver, observers_, DidChangeForm());
}

void Form::set_height(float new_height) {
  if (height_ == new_height)
    return;
  height_ = new_height;
  FOR_EACH_OBSERVER(FormObserver, observers_, DidChangeForm());
}

void Form::set_title(const base::string16& new_title) {
  if (title_ == new_title)
    return;
  title_ = new_title;
  FOR_EACH_OBSERVER(FormObserver, observers_, DidChangeForm());
}

void Form::set_width(float new_width) {
  if (width_ == new_width)
    return;
  width_ = new_width;
  FOR_EACH_OBSERVER(FormObserver, observers_, DidChangeForm());
}

void Form::AddFormControl(FormControl* control) {
  // TODO(yosi) Check |control| isn't in |controls_|.
  controls_[control->control_id()] = control;
  control->form_ = this;
  FOR_EACH_OBSERVER(FormObserver, observers_, DidChangeForm());
}

void Form::AddObserver(FormObserver* observer) const {
  observers_.AddObserver(observer);
}

void Form::DidChangeFormControl(FormControl*) {
  if (name_ == L"FindDialogBox") {
    ScriptHost::instance()->view_delegate()->DidChangeFormContents(
        dialog_box_id());
    return;
  }
  FOR_EACH_OBSERVER(FormObserver, observers_, DidChangeForm());
}

void Form::Realize() {
  if (name_ != L"FindDialogBox") {
    ScriptHost::instance()->ThrowError("Requires Find form");
    return;
  }
  ScriptHost::instance()->view_delegate()->RealizeDialogBox(
      dialog_box_id());
}

void Form::RemoveObserver(FormObserver* observer) const {
  observers_.RemoveObserver(observer);
}

void Form::Show() {
  if (name_ != L"FindDialogBox") {
    ScriptHost::instance()->ThrowError("Requires Find form");
    return;
  }
  ScriptHost::instance()->view_delegate()->ShowDialogBox(
      dialog_box_id());
}

}  // namespace dom
