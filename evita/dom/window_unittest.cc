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
  RunScript("var sample1 = new SampleWindow()");
  EXPECT_EQ("1", RunScript("sample1.id"));
  RunScript("sample1.name = 'test';");
  EXPECT_EQ("test", RunScript("sample1.name"));
  EXPECT_EQ("notrealized", RunScript("sample1.state"));
}

TEST_F(WindowTest, Add) {
  EXPECT_CALL(*mock_view_impl(), AddWindow(Eq(1), Eq(2)));
  EXPECT_CALL(*mock_view_impl(), AddWindow(Eq(1), Eq(3)));
  EXPECT_CALL(*mock_view_impl(), RealizeWindow(Eq(1)));
  RunScript("var parent = new SampleWindow();"
            "var child1 = new SampleWindow();"
            "var child2 = new SampleWindow();"
            "parent.add(child1);"
            "parent.add(child2);"
            "parent.realize();");
  EXPECT_EQ("2", RunScript("parent.children.length"));
  EXPECT_EQ("1", RunScript("child1.parent.id"));
  EXPECT_EQ("1", RunScript("child2.parent.id"));
  dom::Window::DidRealizeWidget(static_cast<dom::WidgetId>(1));
  dom::Window::DidDestroyWidget(static_cast<dom::WidgetId>(2));
  EXPECT_EQ("1", RunScript("parent.children.length"));
  EXPECT_EQ("true", RunScript("child1.parent == null"));
  EXPECT_EQ("destroyed", RunScript("child1.state"));
}

TEST_F(WindowTest, Destroy) {
  EXPECT_CALL(*mock_view_impl(), RealizeWindow(Eq(1)));
  EXPECT_CALL(*mock_view_impl(), AddWindow(Eq(1), Eq(2)));
  EXPECT_CALL(*mock_view_impl(), AddWindow(Eq(2), Eq(3)));
  EXPECT_CALL(*mock_view_impl(), AddWindow(Eq(2), Eq(4)));
  RunScript("var sample1 = new SampleWindow();"
            "var child1 = new SampleWindow();"
            "var child2 = new SampleWindow();"
            "var child3 = new SampleWindow();"
            "sample1.add(child1);"
            "child1.add(child2);"
            "child1.add(child3);"
            "sample1.realize()");
  EXPECT_EQ("Error: You can't destroy unrealized window.",
            RunScript("sample1.destroy()"));
  view_event_handler()->DidRealizeWidget(static_cast<dom::WidgetId>(1));
  EXPECT_EQ("realized", RunScript("sample1.state"));
  EXPECT_CALL(*mock_view_impl(), DestroyWindow(Eq(1)));
  RunScript("sample1.destroy()");
  EXPECT_EQ("destroying", RunScript("sample1.state"));
  EXPECT_EQ("destroying", RunScript("child1.state"));
  EXPECT_EQ("destroying", RunScript("child2.state"));
  EXPECT_EQ("destroying", RunScript("child3.state"));
  view_event_handler()->DidDestroyWidget(static_cast<dom::WidgetId>(1));
  view_event_handler()->DidDestroyWidget(static_cast<dom::WidgetId>(2));
  view_event_handler()->DidDestroyWidget(static_cast<dom::WidgetId>(3));
  view_event_handler()->DidDestroyWidget(static_cast<dom::WidgetId>(4));
  EXPECT_EQ("destroyed", RunScript("sample1.state"));
  EXPECT_EQ("destroyed", RunScript("child1.state"));
  EXPECT_EQ("destroyed", RunScript("child2.state"));
  EXPECT_EQ("destroyed", RunScript("child3.state"));
}

TEST_F(WindowTest, Properties) {
  RunScript("var sample1 = new SampleWindow()");
  EXPECT_EQ("0", RunScript("sample1.children.length"));
  EXPECT_EQ("1", RunScript("sample1.id"));
  EXPECT_EQ("true", RunScript("sample1.parent == null"));
}

TEST_F(WindowTest, Realize) {
  EXPECT_CALL(*mock_view_impl(), RealizeWindow(Eq(1)));
  RunScript("var sample1 = new SampleWindow(); sample1.realize()");
  EXPECT_EQ("realizing", RunScript("sample1.state"));
  EXPECT_EQ("Error: This window is being realized.",
            RunScript("sample1.realize();"));
  view_event_handler()->DidRealizeWidget(static_cast<dom::WidgetId>(1));
  EXPECT_EQ("realized", RunScript("sample1.state"));
}

}  // namespace
