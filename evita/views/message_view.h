// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_views_message_view_h)
#define INCLUDE_evita_views_message_view_h

#include <memory>
#include <vector>

#include "base/strings/string16.h"
#include "base/time/time.h"
#include "common/win/rect.h"

namespace ui {
class StatusBar;
}

namespace views {

using common::win::Rect;

class MessageView {
  private: std::unique_ptr<ui::StatusBar> status_bar_;
  private: std::vector<base::string16> status_bar_texts_;
  private: base::string16 message_text_;
  private: base::Time message_time_;
  private: base::string16 status_text_;

  public: MessageView();
  public: ~MessageView();

  public: int height() const;

  public: void Realize(HWND hwnd_parent);
  public: void ResizeTo(const Rect& rect);
  public: void SetMessage(const base::string16& text);
  public: void SetStatus(const std::vector<base::string16>& texts);

  DISALLOW_COPY_AND_ASSIGN(MessageView);
};

}  // namespace views

#endif //!defined(INCLUDE_evita_views_message_view_h)
