// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_widgets_container_widget_h)
#define INCLUDE_widgets_container_widget_h

#include "base/tree/container_node.h"
#include "widgets/widget.h"

namespace widgets {

class ChildWidgets;
class ConstChildWidgets;

class ContainerWidget 
    : public base::tree::ContainerNode_<Widget, ContainerWidget,
                                        std::unique_ptr<NaitiveWindow>&&> {
  DECLARE_CASTABLE_CLASS(ContainerWidget, Widget);

  private: Widget* capture_widget_;
  private: Widget* focus_widget_;

  protected: explicit ContainerWidget(
      std::unique_ptr<NaitiveWindow>&& naitive_window);
  protected: ContainerWidget();
  public: virtual ~ContainerWidget();

  public: Widget* focus_widget() const { return focus_widget_; }
  public: virtual bool is_container() const override { return true; }

  // [D]
  public: virtual void DidAddChildWidget(const Widget& widget);
  protected: virtual void DidHide() override;
  public: virtual void DidRealizeChildWidget(const Widget& widget);
  public: virtual void DidRemoveChildWidget(const Widget& widget);
  protected: virtual void DidShow() override;
  private: void DispatchPaintMessage();

  // [G]
  private: ContainerWidget& GetHostContainer() const;
  private: Widget* GetWidgetAt(const gfx::Point& point) const;

  // [H]
  public: virtual void Hide();

  // [O]
  protected: virtual bool OnIdle(uint idle_count) override;

  // [R]
  public: void ReleaseCaptureFrom(const Widget&);

  // [S]
  public: void SetCaptureTo(const Widget&);
  private: bool SetCursor();
  public: void SetFocusTo(const Widget&);
  public: virtual void Show();

  // [T]
  public: virtual const ContainerWidget* ToContainer() const override {
    return this;
  }
  public: virtual ContainerWidget* ToContainer() override {
    return this;
  }

  // [W]
  public: virtual void WillDestroyChildWidget(const Widget&);
  protected: virtual LRESULT WindowProc(UINT uMsg, WPARAM  wParam,
                                        LPARAM lParam) override;
  public: virtual void WillRemoveChildWidget(const Widget& widget);

  DISALLOW_COPY_AND_ASSIGN(ContainerWidget);
};

} // namespace widgets

#endif //!defined(INCLUDE_widgets_container_widget_h)
