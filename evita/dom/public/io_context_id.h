// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_PUBLIC_IO_CONTEXT_ID_H_
#define EVITA_DOM_PUBLIC_IO_CONTEXT_ID_H_

#include <functional>

namespace domapi {

class IoContextId {
 public:
  IoContextId(const IoContextId& other);
  IoContextId();
  ~IoContextId();

  IoContextId& operator=(const IoContextId& other);

  bool operator==(const IoContextId& other) const;
  bool operator!=(const IoContextId& other) const;

  int value() const { return value_; }

  static IoContextId New();

 private:
  explicit IoContextId(int value);

  int value_;
};

class DirectoryId final : public IoContextId {
 public:
  explicit DirectoryId(IoContextId context_id);
  ~DirectoryId();
};

class FileId final : public IoContextId {
 public:
  explicit FileId(IoContextId context_id);
  ~FileId();
};

class ProcessId final : public IoContextId {
 public:
  explicit ProcessId(IoContextId context_id);
  ~ProcessId();
};

}  // namespace domapi

namespace std {
template <>
struct hash<domapi::IoContextId> {
  size_t operator()(const domapi::IoContextId& context_id) const;
};
}  // namespace std

#endif  // EVITA_DOM_PUBLIC_IO_CONTEXT_ID_H_
