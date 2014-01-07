// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/mock_view_impl.h"

namespace dom {

MockViewImpl::MockViewImpl() {
}

MockViewImpl::~MockViewImpl() {
}

void MockViewImpl::GetFilenameForLoad(WidgetId,
                                      const base::string16& dir_path,
                                      GetFilenameForLoadCallback callback) {
  callback.Run(dir_path + L"/foo.bar");
}

void MockViewImpl::GetFilenameForSave(WidgetId,
                                      const base::string16& dir_path,
                                      GetFilenameForSaveCallback callback) {
  callback.Run(dir_path + L"/foo.bar");
}

}  // namespace dom

