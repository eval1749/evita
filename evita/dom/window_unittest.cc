// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#include "gtest/gtest.h"

#include "base/basictypes.h"
#include "base/strings/string16.h"
#include "evita/dom/abstract_dom_test.h"
#include "evita/dom/mock_view_impl.h"
#include "evita/dom/script_controller.h"
#include "evita/dom/view_delegate.h"
#include "evita/dom/window.h"
#include "evita/v8_glue/converter.h"

namespace {

using ::testing::Eq;

//////////////////////////////////////////////////////////////////////
//
// SampleWindow for JavaScript testing.
//
class SampleWindow : public v8_glue::Scriptable<SampleWindow, dom::Window> {
  class WrapperInfo : public v8_glue::WrapperInfo {
    public: WrapperInfo() : v8_glue::WrapperInfo("SampleWindow") {
    }
    public: ~WrapperInfo() = default;
  
    private: virtual v8_glue::WrapperInfo* inherit_from() const override {
      return dom::Window::static_wrapper_info();
    }
  
    private: virtual v8::Handle<v8::FunctionTemplate>
        CreateConstructorTemplate(v8::Isolate* isolate) override {
      return v8_glue::CreateConstructorTemplate(isolate,
          &SampleWindow::NewSampleWindow);
    }

    private: virtual void SetupInstanceTemplate(
        ObjectTemplateBuilder& builder) override {
      v8_glue::WrapperInfo::SetupInstanceTemplate(builder);
      builder
        .SetProperty("name", &SampleWindow::name, &SampleWindow::set_name);
    }
  };
  friend class WrapperInfo;

  private: base::string16 name_;

  public: SampleWindow() = default;
  public: virtual ~SampleWindow() = default;

  public: const base::string16& name() const { return name_; }
  public: void set_name(const base::string16& name) { name_ = name; }

  public: static v8_glue::WrapperInfo* static_wrapper_info() {
    DEFINE_STATIC_LOCAL(WrapperInfo, wrapper_info, ());
    return &wrapper_info;
  }

  private: static SampleWindow* NewSampleWindow() {
    return new SampleWindow();
  }

  DISALLOW_COPY_AND_ASSIGN(SampleWindow);
};

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
  EXPECT_VALID_SCRIPT("var sample1 = new SampleWindow()");
  EXPECT_SCRIPT_EQ("1", "sample1.id");
  EXPECT_VALID_SCRIPT("sample1.name = 'test';");
  EXPECT_SCRIPT_EQ("test", "sample1.name");
  EXPECT_SCRIPT_EQ("notrealized", "sample1.state");
}

TEST_F(WindowTest, Add) {
  EXPECT_CALL(*mock_view_impl(), AddWindow(Eq(1), Eq(2)));
  EXPECT_CALL(*mock_view_impl(), AddWindow(Eq(1), Eq(3)));
  EXPECT_CALL(*mock_view_impl(), RealizeWindow(Eq(1)));
  EXPECT_VALID_SCRIPT(
      "var parent = new SampleWindow();"
      "var child1 = new SampleWindow();"
      "var child2 = new SampleWindow();"
      "parent.add(child1);"
      "parent.add(child2);"
      "parent.realize();");
  EXPECT_SCRIPT_EQ("2", "parent.children.length");
  EXPECT_SCRIPT_EQ("1", "child1.parent.id");
  EXPECT_SCRIPT_EQ("1", "child2.parent.id");
  dom::Window::DidRealizeWidget(static_cast<dom::WindowId>(1));
  dom::Window::DidDestroyWidget(static_cast<dom::WindowId>(2));
  EXPECT_SCRIPT_EQ("1", "parent.children.length");
  EXPECT_SCRIPT_EQ("true", "child1.parent == null");
  EXPECT_SCRIPT_EQ("destroyed", "child1.state");
}

TEST_F(WindowTest, Destroy) {
  EXPECT_CALL(*mock_view_impl(), RealizeWindow(Eq(1)));
  EXPECT_CALL(*mock_view_impl(), AddWindow(Eq(1), Eq(2)));
  EXPECT_CALL(*mock_view_impl(), AddWindow(Eq(2), Eq(3)));
  EXPECT_CALL(*mock_view_impl(), AddWindow(Eq(2), Eq(4)));
  EXPECT_CALL(*mock_view_impl(), DestroyWindow(Eq(1)));
  EXPECT_VALID_SCRIPT(
      "var sample1 = new SampleWindow();"
      "var child1 = new SampleWindow();"
      "var child2 = new SampleWindow();"
      "var child3 = new SampleWindow();"
      "sample1.add(child1);"
      "child1.add(child2);"
      "child1.add(child3);"
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

TEST_F(WindowTest, focus) {
  EXPECT_VALID_SCRIPT("var sample = new SampleWindow();");
  EXPECT_SCRIPT_EQ("Error: You can't focus unrealized window.",
                   "sample.focus()");

  EXPECT_CALL(*mock_view_impl(), RealizeWindow(Eq(1)));
  EXPECT_VALID_SCRIPT("sample.realize();");
  view_event_handler()->DidRealizeWidget(static_cast<dom::WindowId>(1));

  EXPECT_CALL(*mock_view_impl(), FocusWindow(Eq(1)));
  EXPECT_VALID_SCRIPT("sample.focus();");
}

TEST_F(WindowTest, Properties) {
  EXPECT_VALID_SCRIPT("var sample1 = new SampleWindow()");
  EXPECT_SCRIPT_EQ("0", "sample1.children.length");
  EXPECT_SCRIPT_EQ("1", "sample1.id");
  EXPECT_SCRIPT_EQ("true", "sample1.parent == null");
}

TEST_F(WindowTest, Realize) {
  EXPECT_CALL(*mock_view_impl(), RealizeWindow(Eq(1)));
  EXPECT_VALID_SCRIPT("var sample1 = new SampleWindow(); sample1.realize()");
  EXPECT_SCRIPT_EQ("realizing", "sample1.state");
  EXPECT_SCRIPT_EQ("Error: This window is being realized.",
                   "sample1.realize();");
  view_event_handler()->DidRealizeWidget(static_cast<dom::WindowId>(1));
  EXPECT_SCRIPT_EQ("realized", "sample1.state");
}

}  // namespace
