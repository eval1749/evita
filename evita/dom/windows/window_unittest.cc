// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#pragma warning(push)
#pragma warning(disable: 4365 4625 4626)
#include "gtest/gtest.h"
#pragma warning(pop)

#include "base/basictypes.h"
#include "base/strings/string16.h"
#include "evita/dom/abstract_dom_test.h"
#include "evita/dom/mock_view_impl.h"
#include "evita/dom/public/view_event.h"
#include "evita/dom/script_host.h"
#include "evita/dom/view_delegate.h"
#include "evita/dom/windows/window.h"
#include "evita/v8_glue/converter.h"

namespace {

using ::testing::Eq;

//////////////////////////////////////////////////////////////////////
//
// SampleWindow for JavaScript testing.
//
class SampleWindow : public v8_glue::Scriptable<SampleWindow, dom::Window> {
  DECLARE_SCRIPTABLE_OBJECT(SampleWindow);
  friend class SampleWindowClass;

  private: base::string16 name_;

  public: SampleWindow() = default;
  public: virtual ~SampleWindow() = default;

  public: const base::string16& name() const { return name_; }
  public: void set_name(const base::string16& name) { name_ = name; }

  private: static SampleWindow* NewSampleWindow() {
    return new SampleWindow();
  }

  DISALLOW_COPY_AND_ASSIGN(SampleWindow);
};

class SampleWindowClass :
    public v8_glue::DerivedWrapperInfo<SampleWindow, dom::Window> {

  public: explicit SampleWindowClass(const char* name)
      : BaseClass(name) {
  }
  public: ~SampleWindowClass() = default;

  private: virtual v8::Handle<v8::FunctionTemplate>
      CreateConstructorTemplate(v8::Isolate* isolate) override {
    return v8_glue::CreateConstructorTemplate(isolate,
        &SampleWindow::NewSampleWindow);
  }

  private: virtual void SetupInstanceTemplate(
      ObjectTemplateBuilder& builder) override {
    BaseClass::SetupInstanceTemplate(builder);
    builder
      .SetProperty("name", &SampleWindow::name, &SampleWindow::set_name);
  }
};

DEFINE_SCRIPTABLE_OBJECT(SampleWindow, SampleWindowClass);

//////////////////////////////////////////////////////////////////////
//
// WindowTest
//
class WindowTest : public dom::AbstractDomTest {
  protected: WindowTest() {
  }
  public: virtual ~WindowTest() {
  }

  private: void virtual PopulateGlobalTemplate(
      v8::Isolate* isolate,
      v8::Handle<v8::ObjectTemplate> global_template) override {
    v8_glue::Installer<SampleWindow>::Run(isolate, global_template);
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
  EXPECT_SCRIPT_TRUE("child1.parent == parent");
  EXPECT_SCRIPT_TRUE("child2.parent == parent");
  EXPECT_SCRIPT_TRUE("parent.firstChild == child1");
  EXPECT_SCRIPT_TRUE("parent.lastChild == child2");
  EXPECT_SCRIPT_TRUE("child1.nextSibling == child2");
  EXPECT_SCRIPT_TRUE("child1.previousSibling == null");
  EXPECT_SCRIPT_TRUE("child2.nextSibling == null");
  EXPECT_SCRIPT_TRUE("child2.previousSibling == child1");
  view_event_handler()->DidRealizeWidget(static_cast<dom::WindowId>(1));
  view_event_handler()->DidDestroyWidget(static_cast<dom::WindowId>(2));
  EXPECT_SCRIPT_EQ("1", "parent.children.length");
  EXPECT_SCRIPT_TRUE("parent.firstChild == child2");
  EXPECT_SCRIPT_TRUE("parent.lastChild == child2");
  EXPECT_SCRIPT_TRUE("child1.parent == null");
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
  view_event_handler()->DidRealizeWidget(static_cast<dom::WindowId>(1));
  EXPECT_SCRIPT_EQ("realized", "sample1.state");
  EXPECT_SCRIPT_EQ("destroying", "child1.state");
  EXPECT_SCRIPT_EQ("destroying", "child2.state");
  EXPECT_SCRIPT_EQ("destroying", "child3.state");
  view_event_handler()->DidDestroyWidget(static_cast<dom::WindowId>(1));
  view_event_handler()->DidDestroyWidget(static_cast<dom::WindowId>(2));
  view_event_handler()->DidDestroyWidget(static_cast<dom::WindowId>(3));
  view_event_handler()->DidDestroyWidget(static_cast<dom::WindowId>(4));
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
    "Error: Can't change parent of window(1) to window(2), becase window(2)"
    " is descendant of window(1).",
    "sample1.changeParent(sample2)");
  EXPECT_SCRIPT_EQ("Error: Can't change parent to itself.",
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

  view_event_handler()->DidChangeWindowBounds(static_cast<dom::WindowId>(1),
                                              11, 22, 33 + 11, 44 + 22);
  EXPECT_SCRIPT_EQ("11", "sample.clientLeft");
  EXPECT_SCRIPT_EQ("22", "sample.clientTop");
  EXPECT_SCRIPT_EQ("33", "sample.clientWidth");
  EXPECT_SCRIPT_EQ("44", "sample.clientHeight");
}

TEST_F(WindowTest, focus) {
  EXPECT_SCRIPT_VALID("var sample = new SampleWindow();");
  EXPECT_SCRIPT_EQ("Error: You can't focus unrealized window.",
                   "sample.focus()");

  EXPECT_CALL(*mock_view_impl(), RealizeWindow(Eq(1)));
  EXPECT_SCRIPT_VALID("sample.realize();");
  view_event_handler()->DidRealizeWidget(static_cast<dom::WindowId>(1));

  EXPECT_CALL(*mock_view_impl(), FocusWindow(Eq(1)));
  EXPECT_SCRIPT_VALID("sample.focus();");
}

TEST_F(WindowTest, focusTick_) {
  EXPECT_SCRIPT_VALID("var sample = new SampleWindow();");
  EXPECT_SCRIPT_EQ("0", "sample.focusTick_");
  EXPECT_TRUE("Window.focus == null");
  domapi::FocusEvent focus_event;
  focus_event.event_type = domapi::EventType::Focus;
  focus_event.target_id = 1;
  focus_event.related_target_id = domapi::kInvalidEventTargetId;
  view_event_handler()->DispatchFocusEvent(focus_event);
  EXPECT_SCRIPT_TRUE("Window.focus == sample");
  EXPECT_SCRIPT_EQ("1", "sample.focusTick_");
}

TEST_F(WindowTest, Properties) {
  EXPECT_SCRIPT_VALID("var sample1 = new SampleWindow()");
  EXPECT_SCRIPT_EQ("0", "sample1.children.length");
  EXPECT_SCRIPT_EQ("1", "sample1.id");
  EXPECT_SCRIPT_EQ("true", "sample1.parent == null");
}

TEST_F(WindowTest, Realize) {
  EXPECT_CALL(*mock_view_impl(), RealizeWindow(Eq(1)));
  EXPECT_SCRIPT_VALID("var sample1 = new SampleWindow(); sample1.realize()");
  EXPECT_SCRIPT_EQ("realizing", "sample1.state");
  EXPECT_SCRIPT_EQ("Error: This window is being realized.",
                   "sample1.realize();");
  view_event_handler()->DidRealizeWidget(static_cast<dom::WindowId>(1));
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
  view_event_handler()->DidRealizeWidget(static_cast<dom::WindowId>(1));
  view_event_handler()->DidRealizeWidget(static_cast<dom::WindowId>(3));
  view_event_handler()->DidRealizeWidget(static_cast<dom::WindowId>(4));

  EXPECT_SCRIPT_EQ("Error: Can't split top-level window.",
      "parent1.splitHorizontally(parent1);");
  EXPECT_SCRIPT_EQ("Error: Can't split window with itself.",
      "child1.splitHorizontally(child1);");
  EXPECT_SCRIPT_EQ("Error: Can't split unrealized window.",
      "child2.splitHorizontally(child3);");
  EXPECT_SCRIPT_EQ("Error: Can't split with child window.",
      "child1.splitHorizontally(child2);");
  EXPECT_SCRIPT_EQ("Error: Can't split with realized window.",
      "child1.splitHorizontally(parent3);");

  EXPECT_SCRIPT_VALID("child1.splitHorizontally(child3);");
  EXPECT_SCRIPT_TRUE("parent1.firstChild == child1");
  EXPECT_SCRIPT_TRUE("parent1.lastChild == child3");
  EXPECT_SCRIPT_TRUE("child1.nextSibling == child3");
  EXPECT_SCRIPT_TRUE("child1.previousSibling == null");
  EXPECT_SCRIPT_TRUE("child3.nextSibling == null");
  EXPECT_SCRIPT_TRUE("child3.previousSibling == child1");
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
  view_event_handler()->DidRealizeWidget(static_cast<dom::WindowId>(1));
  view_event_handler()->DidRealizeWidget(static_cast<dom::WindowId>(3));
  view_event_handler()->DidRealizeWidget(static_cast<dom::WindowId>(4));

  EXPECT_SCRIPT_EQ("Error: Can't split top-level window.",
      "parent1.splitVertically(parent1);");
  EXPECT_SCRIPT_EQ("Error: Can't split window with itself.",
      "child1.splitVertically(child1);");
  EXPECT_SCRIPT_EQ("Error: Can't split unrealized window.",
      "child2.splitVertically(child3);");
  EXPECT_SCRIPT_EQ("Error: Can't split with child window.",
      "child1.splitVertically(child2);");
  EXPECT_SCRIPT_EQ("Error: Can't split with realized window.",
      "child1.splitVertically(parent3);");

  EXPECT_SCRIPT_VALID("child1.splitVertically(child3);");
  EXPECT_SCRIPT_TRUE("parent1.firstChild == child1");
  EXPECT_SCRIPT_TRUE("parent1.lastChild == child3");
  EXPECT_SCRIPT_TRUE("child1.nextSibling == child3");
  EXPECT_SCRIPT_TRUE("child1.previousSibling == null");
  EXPECT_SCRIPT_TRUE("child3.nextSibling == null");
  EXPECT_SCRIPT_TRUE("child3.previousSibling == child1");
}

TEST_F(WindowTest, update) {
  EXPECT_CALL(*mock_view_impl(), RealizeWindow(Eq(1)));
  EXPECT_SCRIPT_VALID("var sample1 = new SampleWindow(); sample1.realize()");

  EXPECT_CALL(*mock_view_impl(), UpdateWindow(Eq(1)));
  EXPECT_SCRIPT_VALID("sample1.update()");
}

TEST_F(WindowTest, visible) {
  EXPECT_SCRIPT_VALID("var sample1 = new SampleWindow();");
  EXPECT_TRUE("sample1.visible === false");
  view_event_handler()->DidChangeWindowVisibility(static_cast<dom::WindowId>(1),
                                                  domapi::Visibility::Visible);
  EXPECT_TRUE("sample1.visible === true");
}

}  // namespace
