// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_views_message_view_h)
#define INCLUDE_evita_views_message_view_h

#include <memory>
#include <vector>

#include "base/strings/string16.h"
#include "evita/ui/animation/animatable.h"
#include "evita/ui/widget.h"

namespace gfx {
class Canvas;
}

namespace views {

//////////////////////////////////////////////////////////////////////
//
// MessageView
//
class MessageView final : public ui::Animatable, public ui::Widget {
  private: class Model;

  private: std::unique_ptr<gfx::Canvas> canvas_;
  private: std::unique_ptr<Model> model_;

  public: MessageView();
  public: virtual ~MessageView();

  public: void SetMessage(const base::string16& text);
  public: void SetStatus(const std::vector<base::string16>& texts);

  // ui::Animatable
  private: virtual void Animate(base::Time time) override;

  // ui::Widget
  private: virtual void DidChangeBounds() override;
  private: virtual void DidRealize() override;
  private: virtual gfx::Size GetPreferredSize() const override;
  private: virtual void WillDestroyWidget() override;

  DISALLOW_COPY_AND_ASSIGN(MessageView);
};

}  // namespace views

#endif //!defined(INCLUDE_evita_views_message_view_h)
