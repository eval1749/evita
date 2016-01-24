// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_CLIPBOARD_CLIPBOARD_H_
#define EVITA_DOM_CLIPBOARD_CLIPBOARD_H_

#include <windows.h>

#include <vector>

#include "base/macros.h"
#include "base/strings/string16.h"

namespace dom {

class DataTransferData;
class ExceptionState;

//////////////////////////////////////////////////////////////////////
//
// Clipboard
//
class Clipboard final {
 public:
  class Format {
   public:
    virtual ~Format();

    uint32_t format() const { return format_; }
    const base::string16& mime_type() const { return mime_type_; }

    static const Format* Get(const base::string16& mime_type);
    static const Format* Get(uint32_t format);

    virtual DataTransferData* FromClipboard(HANDLE handle) const = 0;

   protected:
    Format(const base::char16* name, const base::string16& mime_type);
    Format(uint32_t format, const base::string16& mime_type);

   private:
    const uint32_t format_;
    const base::string16 mime_type_;

    DISALLOW_COPY_AND_ASSIGN(Format);
  };

  explicit Clipboard(ExceptionState* exception_state);
  ~Clipboard();

  bool opened() const { return opened_; }
  void Add(const Format* format,
           const DataTransferData* data,
           ExceptionState* exception_state);
  void Clear();
  DataTransferData* Get(const Format* format) const;

 private:
  bool opened_;

  DISALLOW_ASSIGN(Clipboard);
};

}  // namespace dom

#endif  // EVITA_DOM_CLIPBOARD_CLIPBOARD_H_
