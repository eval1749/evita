// Copyright (c) 2013 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/windows/window.h"
#include "base/macros.h"
#include "base/strings/string16.h"
#include "evita/dom/bindings/exception_state.h"
#include "evita/dom/public/view_delegate.h"
#include "evita/dom/public/view_event.h"
#include "evita/dom/script_host.h"
#include "evita/dom/testing/abstract_dom_test.h"
#include "evita/dom/testing/mock_view_impl.h"
#include "evita/v8_glue/constructor_template.h"
#include "evita/v8_glue/converter.h"
#include "gtest/gtest.h"

namespace dom {

using ::testing::Eq;

//////////////////////////////////////////////////////////////////////
//
// SampleWindow for JavaScript testing.
//
class SampleWindow final : public v8_glue::Scriptable<SampleWindow, Window> {
  DECLARE_SCRIPTABLE_OBJECT(SampleWindow);

 public:
  explicit SampleWindow(ScriptHost* script_host);
  ~SampleWindow() final = default;

  const base::string16& name() const { return name_; }
  void set_name(const base::string16& name) { name_ = name; }

 private:
  friend class SampleWindowClass;

  static SampleWindow* NewSampleWindow(ScriptHost* script_host);

  base::string16 name_;

  DISALLOW_COPY_AND_ASSIGN(SampleWindow);
};

SampleWindow::SampleWindow(ScriptHost* script_host) : Scriptable(script_host) {}

SampleWindow* SampleWindow::NewSampleWindow(ScriptHost* script_host) {
  return new SampleWindow(script_host);
}

//////////////////////////////////////////////////////////////////////
//
// SampleWindowClass
//
class SampleWindowClass final
    : public v8_glue::DerivedWrapperInfo<SampleWindow, Window> {
 public:
  explicit SampleWindowClass(const char* name) : BaseClass(name) {}
  ~SampleWindowClass() = default;

 private:
  static void ConstructSampleWindow(
      const v8::FunctionCallbackInfo<v8::Value>& info) {
    if (!v8_glue::internal::IsValidConstructCall(info))
      return;
    v8_glue::internal::FinishConstructCall(info, NewSampleWindow(info));
  }

  v8::Local<v8::FunctionTemplate> CreateConstructorTemplate(
      v8::Isolate* isolate) override {
    return v8::FunctionTemplate::New(isolate,
                                     &SampleWindowClass::ConstructSampleWindow);
  }

  static SampleWindow* NewSampleWindow(
      const v8::FunctionCallbackInfo<v8::Value>& info) {
    const auto runner = v8_glue::Runner::From(info.GetIsolate());
    const auto script_host = runner->user_data<ScriptHost>();
    return new SampleWindow(script_host);
  }

  v8::Local<v8::ObjectTemplate> SetupInstanceTemplate(
      v8::Isolate* isolate,
      v8::Local<v8::ObjectTemplate> base_templ) override {
    auto const templ = BaseClass::SetupInstanceTemplate(isolate, base_templ);
    templ->SetAccessorProperty(
        gin::StringToSymbol(isolate, "name"),
        v8::FunctionTemplate::New(isolate, &GetName),
        v8::FunctionTemplate::New(isolate, &SetName),
        static_cast<v8::PropertyAttribute>(v8::DontDelete | v8::DontEnum));
    return templ;
  }

  // |name| IDL attribute getter
  static void GetName(const v8::FunctionCallbackInfo<v8::Value>& info) {
    auto const isolate = info.GetIsolate();
    const auto context = isolate->GetCurrentContext();
    ExceptionState exception_state(ExceptionState::Situation::PropertyGet,
                                   context, "SampleWindow", "name");
    SampleWindow* impl = nullptr;
    if (!gin::ConvertFromV8(isolate, info.This(), &impl)) {
      exception_state.ThrowReceiverError(info.This());
      return;
    }
    auto value = impl->name();
    v8::Local<v8::Value> v8_value;
    if (!gin::TryConvertToV8(isolate, value, &v8_value))
      return;
    info.GetReturnValue().Set(v8_value);
  }

  // |name| IDL attribute setter
  static void SetName(const v8::FunctionCallbackInfo<v8::Value>& info) {
    auto const isolate = info.GetIsolate();
    const auto context = isolate->GetCurrentContext();
    ExceptionState exception_state(ExceptionState::Situation::PropertySet,
                                   context, "SampleWindow", "name");
    if (info.Length() != 1) {
      exception_state.ThrowArityError(1, 1, info.Length());
      return;
    }
    SampleWindow* impl = nullptr;
    if (!gin::ConvertFromV8(isolate, info.This(), &impl)) {
      exception_state.ThrowReceiverError(info.This());
      return;
    }
    base::string16 new_value;
    if (!gin::ConvertFromV8(isolate, info[0], &new_value)) {
      exception_state.ThrowArgumentError("string", info[0], 0);
      return;
    }
    impl->set_name(new_value);
  }

  DISALLOW_COPY_AND_ASSIGN(SampleWindowClass);
};

DEFINE_SCRIPTABLE_OBJECT(SampleWindow, SampleWindowClass);

//////////////////////////////////////////////////////////////////////
//
// WindowTest
//
class WindowTest : public AbstractDomTest {
 public:
  ~WindowTest() override = default;

 protected:
  WindowTest() = default;

 private:
  void PopulateGlobalTemplate(
      v8::Isolate* isolate,
      v8::Local<v8::ObjectTemplate> global_template) override {
    v8_glue::Installer<SampleWindow>::Run(isolate, global_template);
  }

  // testing::Test
  void SetUp() override {
    AbstractDomTest::SetUp();
    EXPECT_SCRIPT_VALID(
        "SampleWindow.handleEvent = function(event) {"
        "  Window.handleEvent.call(this, event);"
        "}");
  }

  DISALLOW_COPY_AND_ASSIGN(WindowTest);
};

TEST_F(WindowTest, Construction) {
  EXPECT_SCRIPT_VALID("var sample1 = new SampleWindow()");
  EXPECT_SCRIPT_EQ("1", "sample1.id");
  EXPECT_SCRIPT_VALID("sample1.name = 'test';");
  EXPECT_SCRIPT_EQ("test", "sample1.name");
  EXPECT_SCRIPT_EQ("notrealized", "sample1.state");
  EXPECT_SCRIPT_TRUE("sample1.firstChild === null");
  EXPECT_SCRIPT_TRUE("sample1.lastChild === null");
  EXPECT_SCRIPT_TRUE("sample1.nextSibling === null");
  EXPECT_SCRIPT_TRUE("sample1.previousSibling=== null");
}

TEST_F(WindowTest, Add) {
  EXPECT_CALL(*mock_view_impl(), AddWindow(Eq(1), Eq(2)));
  EXPECT_CALL(*mock_view_impl(), AddWindow(Eq(1), Eq(3)));
  EXPECT_CALL(*mock_view_impl(), RealizeWindow(Eq(1)));
  EXPECT_SCRIPT_VALID(
      "var parent = new SampleWindow();"
      "var child1 = new SampleWindow();"
      "var child2 = new SampleWindow();"
      "parent.appendChild(child1);"
      "parent.appendChild(child2);"
      "parent.realize();");
  EXPECT_SCRIPT_EQ("2", "parent.children.length");
  EXPECT_SCRIPT_TRUE("child1.parent === parent");
  EXPECT_SCRIPT_TRUE("child2.parent === parent");
  EXPECT_SCRIPT_TRUE("parent.firstChild === child1");
  EXPECT_SCRIPT_TRUE("parent.lastChild === child2");
  EXPECT_SCRIPT_TRUE("child1.nextSibling === child2");
  EXPECT_SCRIPT_TRUE("child1.previousSibling === null");
  EXPECT_SCRIPT_TRUE("child2.nextSibling === null");
  EXPECT_SCRIPT_TRUE("child2.previousSibling === child1");
  view_event_handler()->DidRealizeWidget(static_cast<domapi::WindowId>(1));
  view_event_handler()->DidDestroyWindow(static_cast<domapi::WindowId>(2));
  EXPECT_SCRIPT_EQ("1", "parent.children.length");
  EXPECT_SCRIPT_TRUE("parent.firstChild === child2");
  EXPECT_SCRIPT_TRUE("parent.lastChild === child2");
  EXPECT_SCRIPT_TRUE("child1.parent === null");
  EXPECT_SCRIPT_EQ("destroyed", "child1.state");
}

TEST_F(WindowTest, Destroy) {
  EXPECT_CALL(*mock_view_impl(), RealizeWindow(Eq(1)));
  EXPECT_CALL(*mock_view_impl(), AddWindow(Eq(1), Eq(2)));
  EXPECT_CALL(*mock_view_impl(), AddWindow(Eq(2), Eq(3)));
  EXPECT_CALL(*mock_view_impl(), AddWindow(Eq(2), Eq(4)));
  EXPECT_CALL(*mock_view_impl(), DestroyWindow(Eq(1)));
  EXPECT_SCRIPT_VALID(
      "var sample1 = new SampleWindow();"
      "var child1 = new SampleWindow();"
      "var child2 = new SampleWindow();"
      "var child3 = new SampleWindow();"
      "sample1.appendChild(child1);"
      "child1.appendChild(child2);"
      "child1.appendChild(child3);"
      "sample1.realize();"
      "sample1.destroy();");
  view_event_handler()->DidRealizeWidget(static_cast<domapi::WindowId>(1));
  EXPECT_SCRIPT_EQ("realized", "sample1.state");
  EXPECT_SCRIPT_EQ("destroying", "child1.state");
  EXPECT_SCRIPT_EQ("destroying", "child2.state");
  EXPECT_SCRIPT_EQ("destroying", "child3.state");
  view_event_handler()->DidDestroyWindow(static_cast<domapi::WindowId>(1));
  view_event_handler()->DidDestroyWindow(static_cast<domapi::WindowId>(2));
  view_event_handler()->DidDestroyWindow(static_cast<domapi::WindowId>(3));
  view_event_handler()->DidDestroyWindow(static_cast<domapi::WindowId>(4));
  EXPECT_SCRIPT_EQ("destroyed", "sample1.state");
  EXPECT_SCRIPT_EQ("destroyed", "child1.state");
  EXPECT_SCRIPT_EQ("destroyed", "child2.state");
  EXPECT_SCRIPT_EQ("destroyed", "child3.state");
}

TEST_F(WindowTest, changeParent) {
  EXPECT_CALL(*mock_view_impl(), AddWindow(Eq(1), Eq(2)));
  EXPECT_CALL(*mock_view_impl(), ChangeParentWindow(Eq(2), Eq(3)));
  EXPECT_SCRIPT_VALID(
      "var sample1 = new SampleWindow();"
      "var sample2 = new SampleWindow();"
      "var sample3 = new SampleWindow();"
      "sample1.appendChild(sample2)");
  EXPECT_SCRIPT_EQ(
      "Error: Failed to execute 'changeParent' on 'Window': Can't change "
      "parent of window(1) to window(2), because window(2)"
      " is descendant of window(1).",
      "sample1.changeParent(sample2)");
  EXPECT_SCRIPT_EQ(
      "Error: Failed to execute 'changeParent' on 'Window': Can't change "
      "parent to itself.",
      "sample2.changeParent(sample2)");
  EXPECT_SCRIPT_VALID("sample2.changeParent(sample1)");
  EXPECT_SCRIPT_VALID("sample2.changeParent(sample3)");
  EXPECT_SCRIPT_EQ("3", "sample2.parent.id");
}

TEST_F(WindowTest, DidResize) {
  EXPECT_SCRIPT_VALID("var sample = new SampleWindow();");
  EXPECT_SCRIPT_EQ("0", "sample.clientLeft");
  EXPECT_SCRIPT_EQ("0", "sample.clientTop");
  EXPECT_SCRIPT_EQ("0", "sample.clientWidth");
  EXPECT_SCRIPT_EQ("0", "sample.clientHeight");

  view_event_handler()->DidChangeWindowBounds(static_cast<domapi::WindowId>(1),
                                              11, 22, 33 + 11, 44 + 22);
  EXPECT_SCRIPT_EQ("11", "sample.clientLeft");
  EXPECT_SCRIPT_EQ("22", "sample.clientTop");
  EXPECT_SCRIPT_EQ("33", "sample.clientWidth");
  EXPECT_SCRIPT_EQ("44", "sample.clientHeight");
}

TEST_F(WindowTest, focus) {
  EXPECT_SCRIPT_VALID("var sample = new SampleWindow();");
  EXPECT_SCRIPT_EQ(
      "Error: Failed to execute 'focus' on 'Window': You can't focus "
      "unrealized window.",
      "sample.focus()");

  EXPECT_CALL(*mock_view_impl(), RealizeWindow(Eq(1)));
  EXPECT_SCRIPT_VALID("sample.realize();");
  view_event_handler()->DidRealizeWidget(static_cast<domapi::WindowId>(1));

  EXPECT_CALL(*mock_view_impl(), FocusWindow(Eq(1)));
  EXPECT_SCRIPT_VALID("sample.focus();");
}

TEST_F(WindowTest, focusTick_) {
  EXPECT_SCRIPT_VALID("var sample = new SampleWindow();");
  EXPECT_SCRIPT_EQ("0", "sample.focusTick_");
  EXPECT_TRUE("Window.focus === null");
  domapi::FocusEvent focus_event;
  focus_event.event_type = domapi::EventType::Focus;
  focus_event.target_id = 1;
  focus_event.related_target_id = domapi::kInvalidEventTargetId;
  view_event_handler()->DispatchFocusEvent(focus_event);
  EXPECT_SCRIPT_TRUE("Window.focus === sample");
  EXPECT_SCRIPT_EQ("1", "sample.focusTick_");
}

TEST_F(WindowTest, Properties) {
  EXPECT_SCRIPT_VALID("var sample1 = new SampleWindow()");
  EXPECT_SCRIPT_EQ("0", "sample1.children.length");
  EXPECT_SCRIPT_EQ("1", "sample1.id");
  EXPECT_SCRIPT_TRUE("sample1.parent === null");
}

TEST_F(WindowTest, Realize) {
  EXPECT_CALL(*mock_view_impl(), RealizeWindow(Eq(1)));
  EXPECT_SCRIPT_VALID("var sample1 = new SampleWindow(); sample1.realize()");
  EXPECT_SCRIPT_EQ("realizing", "sample1.state");
  EXPECT_SCRIPT_EQ(
      "Error: Failed to execute 'realize' on 'Window': This window is being "
      "realized.",
      "sample1.realize();");
  view_event_handler()->DidRealizeWidget(static_cast<domapi::WindowId>(1));
  EXPECT_SCRIPT_EQ("realized", "sample1.state");
}

TEST_F(WindowTest, show) {
  EXPECT_CALL(*mock_view_impl(), RealizeWindow(Eq(1)));
  EXPECT_SCRIPT_VALID("var sample1 = new SampleWindow(); sample1.realize()");

  EXPECT_CALL(*mock_view_impl(), HideWindow(Eq(1)));
  EXPECT_SCRIPT_VALID("sample1.hide()");

  EXPECT_CALL(*mock_view_impl(), ShowWindow(Eq(1)));
  EXPECT_SCRIPT_VALID("sample1.show()");
}

TEST_F(WindowTest, splitHorizontally) {
  EXPECT_CALL(*mock_view_impl(), AddWindow(Eq(1), Eq(4)));
  EXPECT_CALL(*mock_view_impl(), AddWindow(Eq(2), Eq(5)));
  EXPECT_CALL(*mock_view_impl(), RealizeWindow(Eq(1)));
  EXPECT_CALL(*mock_view_impl(), RealizeWindow(Eq(3)));
  EXPECT_CALL(*mock_view_impl(), SplitHorizontally(Eq(4), Eq(6)));
  EXPECT_SCRIPT_VALID(
      "var parent1 = new SampleWindow();"
      "var parent2 = new SampleWindow();"
      "var parent3 = new SampleWindow();"
      "var child1 = new SampleWindow();"
      "var child2 = new SampleWindow();"
      "var child3 = new SampleWindow();"
      "parent1.appendChild(child1);"
      "parent2.appendChild(child2);"
      "parent1.realize();"
      "parent3.realize();");
  view_event_handler()->DidRealizeWidget(static_cast<domapi::WindowId>(1));
  view_event_handler()->DidRealizeWidget(static_cast<domapi::WindowId>(3));
  view_event_handler()->DidRealizeWidget(static_cast<domapi::WindowId>(4));

  EXPECT_SCRIPT_EQ(
      "Error: Failed to execute 'splitHorizontally' on 'Window': Can't split "
      "top-level window.",
      "parent1.splitHorizontally(parent1);");
  EXPECT_SCRIPT_EQ(
      "Error: Failed to execute 'splitHorizontally' on 'Window': Can't split "
      "window with itself.",
      "child1.splitHorizontally(child1);");
  EXPECT_SCRIPT_EQ(
      "Error: Failed to execute 'splitHorizontally' on 'Window': Can't split "
      "unrealized window.",
      "child2.splitHorizontally(child3);");
  EXPECT_SCRIPT_EQ(
      "Error: Failed to execute 'splitHorizontally' on 'Window': Can't split "
      "with child window.",
      "child1.splitHorizontally(child2);");
  EXPECT_SCRIPT_EQ(
      "Error: Failed to execute 'splitHorizontally' on 'Window': Can't split "
      "with realized window.",
      "child1.splitHorizontally(parent3);");

  EXPECT_SCRIPT_VALID("child1.splitHorizontally(child3);");
  EXPECT_SCRIPT_TRUE("parent1.firstChild === child1");
  EXPECT_SCRIPT_TRUE("parent1.lastChild === child3");
  EXPECT_SCRIPT_TRUE("child1.nextSibling === child3");
  EXPECT_SCRIPT_TRUE("child1.previousSibling === null");
  EXPECT_SCRIPT_TRUE("child3.nextSibling === null");
  EXPECT_SCRIPT_TRUE("child3.previousSibling === child1");
}

TEST_F(WindowTest, splitVertically) {
  EXPECT_CALL(*mock_view_impl(), AddWindow(Eq(1), Eq(4)));
  EXPECT_CALL(*mock_view_impl(), AddWindow(Eq(2), Eq(5)));
  EXPECT_CALL(*mock_view_impl(), RealizeWindow(Eq(1)));
  EXPECT_CALL(*mock_view_impl(), RealizeWindow(Eq(3)));
  EXPECT_CALL(*mock_view_impl(), SplitVertically(Eq(4), Eq(6)));
  EXPECT_SCRIPT_VALID(
      "var parent1 = new SampleWindow();"
      "var parent2 = new SampleWindow();"
      "var parent3 = new SampleWindow();"
      "var child1 = new SampleWindow();"
      "var child2 = new SampleWindow();"
      "var child3 = new SampleWindow();"
      "parent1.appendChild(child1);"
      "parent2.appendChild(child2);"
      "parent1.realize();"
      "parent3.realize();");
  view_event_handler()->DidRealizeWidget(static_cast<domapi::WindowId>(1));
  view_event_handler()->DidRealizeWidget(static_cast<domapi::WindowId>(3));
  view_event_handler()->DidRealizeWidget(static_cast<domapi::WindowId>(4));

  EXPECT_SCRIPT_EQ(
      "Error: Failed to execute 'splitVertically' on 'Window': Can't split "
      "top-level window.",
      "parent1.splitVertically(parent1);");
  EXPECT_SCRIPT_EQ(
      "Error: Failed to execute 'splitVertically' on 'Window': Can't split "
      "window with itself.",
      "child1.splitVertically(child1);");
  EXPECT_SCRIPT_EQ(
      "Error: Failed to execute 'splitVertically' on 'Window': Can't split "
      "unrealized window.",
      "child2.splitVertically(child3);");
  EXPECT_SCRIPT_EQ(
      "Error: Failed to execute 'splitVertically' on 'Window': Can't split "
      "with child window.",
      "child1.splitVertically(child2);");
  EXPECT_SCRIPT_EQ(
      "Error: Failed to execute 'splitVertically' on 'Window': Can't split "
      "with realized window.",
      "child1.splitVertically(parent3);");

  EXPECT_SCRIPT_VALID("child1.splitVertically(child3);");
  EXPECT_SCRIPT_TRUE("parent1.firstChild === child1");
  EXPECT_SCRIPT_TRUE("parent1.lastChild === child3");
  EXPECT_SCRIPT_TRUE("child1.nextSibling === child3");
  EXPECT_SCRIPT_TRUE("child1.previousSibling === null");
  EXPECT_SCRIPT_TRUE("child3.nextSibling === null");
  EXPECT_SCRIPT_TRUE("child3.previousSibling === child1");
}

TEST_F(WindowTest, update) {
  EXPECT_CALL(*mock_view_impl(), RealizeWindow(Eq(1)));
  EXPECT_SCRIPT_VALID("var sample1 = new SampleWindow(); sample1.realize()");

  EXPECT_CALL(*mock_view_impl(), UpdateWindow(Eq(1)));
  EXPECT_SCRIPT_VALID("sample1.update()");
}

TEST_F(WindowTest, visible) {
  EXPECT_SCRIPT_VALID("var sample1 = new SampleWindow();");
  EXPECT_SCRIPT_TRUE("sample1.visible === false");
  view_event_handler()->DidChangeWindowVisibility(
      static_cast<domapi::WindowId>(1), domapi::Visibility::Visible);
  EXPECT_SCRIPT_TRUE("sample1.visible === true");
}

}  // namespace dom
