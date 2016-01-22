// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/mock_view_impl.h"

#include "base/synchronization/waitable_event.h"
#include "evita/dom/text/text_document.h"
#include "evita/visuals/display/public/display_item_list.h"

namespace dom {

MockViewImpl::MockViewImpl() : check_spelling_result_(false) {}

MockViewImpl::~MockViewImpl() {}

// domapi::ViewDelegate
void MockViewImpl::GetFileNameForLoad(
    domapi::WindowId,
    const base::string16& dir_path,
    const GetFileNameForLoadResolver& resolver) {
  resolver.resolve.Run(dir_path + L"/foo.bar");
}

void MockViewImpl::GetFileNameForSave(
    domapi::WindowId,
    const base::string16& dir_path,
    const GetFileNameForSaveResolver& resolver) {
  resolver.resolve.Run(dir_path + L"/foo.bar");
}

void MockViewImpl::MessageBox(domapi::WindowId,
                              const base::string16&,
                              const base::string16&,
                              int flags,
                              const MessageBoxResolver& resolver) {
  resolver.resolve.Run(flags);
}

void MockViewImpl::PaintVisualDocument(
    domapi::WindowId window_id,
    std::unique_ptr<visuals::DisplayItemList> display_item_list) {}

}  // namespace dom
