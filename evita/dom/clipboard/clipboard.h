// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_dom_clipboard_clipboard_h)
#define INCLUDE_evita_dom_clipboard_clipboard_h

#include <windows.h>

#include <vector>

#include "base/basictypes.h"

#include "base/strings/string16.h"

namespace dom {

class DataTransferData;

class Clipboard {
  public: class Format {
    private: const uint32_t format_;
    private: const base::string16 mime_type_;

    protected: Format(const base::char16* name,
                      const base::string16& mime_type);
    protected: Format(uint32_t format, const base::string16& mime_type);
    public: virtual ~Format();

    public: uint32_t format() const { return format_; }
    public: const base::string16& mime_type() const { return mime_type_; }

    public: static const Format* Get(const base::string16& mime_type);
    public: static const Format* Get(uint32_t format);

    public: virtual DataTransferData* FromClipboard(HANDLE handle) const = 0;

    DISALLOW_COPY_AND_ASSIGN(Format);
  };

  private: bool opened_;

  public: Clipboard();
  public: ~Clipboard();

  public: bool opened() const { return opened_; }
  public: void Add(const Format* format, const DataTransferData* data);
  public: void Clear();
  public: DataTransferData* Get(const Format* format) const;

  DISALLOW_ASSIGN(Clipboard);
};

}  // namespace dom

#endif //!defined(INCLUDE_evita_dom_clipboard_clipboard_h)
