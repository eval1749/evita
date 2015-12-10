// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/mock_view_impl.h"

#include "base/synchronization/waitable_event.h"
#include "evita/dom/text/document.h"

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

std::vector<int> MockViewImpl::GetTableRowStates(
    domapi::WindowId,
    const std::vector<base::string16>& keys) {
  std::vector<int> states;
  for (auto index = 0u; index < keys.size(); ++index) {
    states.push_back(index);
  }
  return std::move(states);
}

void MockViewImpl::MessageBox(domapi::WindowId,
                              const base::string16&,
                              const base::string16&,
                              int flags,
                              const MessageBoxResolver& resolver) {
  resolver.resolve.Run(flags);
}

}  // namespace dom
