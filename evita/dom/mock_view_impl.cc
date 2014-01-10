// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/mock_view_impl.h"

namespace dom {

MockViewImpl::MockViewImpl() {
}

MockViewImpl::~MockViewImpl() {
}

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

void MockViewImpl::MessageBox(WindowId, const base::string16&,
                              const base::string16&, int flags,
                              MessageBoxCallback callback) {
  callback.Run(flags);
}

}  // namespace dom

