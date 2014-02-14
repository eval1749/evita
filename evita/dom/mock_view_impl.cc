// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/mock_view_impl.h"

#include "base/synchronization/waitable_event.h"
#include "evita/dom/document.h"

namespace dom {

MockViewImpl::MockViewImpl() {
}

MockViewImpl::~MockViewImpl() {
}

void MockViewImpl::SetLoadFileCallbackData(
    const domapi::LoadFileCallbackData& data) {
  load_file_callback_data_ = data;
}

void MockViewImpl::SetSaveFileCallbackData(
    const domapi::SaveFileCallbackData& data) {
  save_file_callback_data_ = data;
}

// dom::ViewDelegate
void MockViewImpl::GetFilenameForLoad(WindowId,
                                      const base::string16& dir_path,
                                      GetFilenameForLoadCallback callback) {
  callback.Run(dir_path + L"/foo.bar");
}

void MockViewImpl::GetFilenameForSave(WindowId,
                                      const base::string16& dir_path,
                                      GetFilenameForSaveCallback callback) {
  callback.Run(dir_path + L"/foo.bar");
}

void MockViewImpl::GetTableRowStates(WindowId,
    const std::vector<base::string16>& keys, int* states,
    base::WaitableEvent*) {
  for (auto index = 0u; index < keys.size(); ++index) {
    *states = index;
    ++states;
  }
}

void MockViewImpl::LoadFile(Document*, const base::string16&,
                            LoadFileCallback callback) {
  callback.Run(load_file_callback_data_);
}

void MockViewImpl::MessageBox(WindowId, const base::string16&,
                              const base::string16&, int flags,
                              MessageBoxCallback callback) {
  callback.Run(flags);
}

void MockViewImpl::SaveFile(Document*, const base::string16&,
                            const SaveFileCallback& callback) {
  callback.Run(save_file_callback_data_);
}

}  // namespace dom
