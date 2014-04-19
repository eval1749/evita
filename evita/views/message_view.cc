// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/views/message_view.h"

#include "base/logging.h"
#include "evita/ui/controls/status_bar.h"

namespace views {

MessageView::MessageView() : status_bar_(new ui::StatusBar()) {
}

MessageView::~MessageView() {
}

int MessageView::height() const {
  return status_bar_->height();
}

void MessageView::Realize(HWND hwnd_parent) {
  status_bar_->Realize(hwnd_parent);
}

void MessageView::SetBounds(const Rect& rect) {
  status_bar_->SetBounds(rect);
}

void MessageView::SetMessage(const base::string16& text) {
  message_time_ = base::Time::Now();
  if (message_text_ == text)
    return;
  message_text_ = text;
  if (status_bar_texts_.empty()) {
    status_bar_->Set(message_text_);
    return;
  }
  status_bar_texts_[0] = message_text_;
  status_bar_->Set(status_bar_texts_);
}

void MessageView::SetStatus(const std::vector<base::string16>& texts) {
  DCHECK(!texts.empty());
  status_bar_texts_ = texts;
  if (texts[0] == status_text_) {
    if (!message_text_.empty()) {
      if (base::Time::Now() - message_time_ < base::TimeDelta::FromSeconds(5))
        status_bar_texts_[0] = message_text_;
      else
        message_text_.clear();
    }
  } else {
    status_text_ = texts[0];
    message_text_.clear();
  }
  status_bar_->Set(status_bar_texts_);
}

}  // namespace views
