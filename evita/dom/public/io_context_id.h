// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_dom_public_io_context_id_h)
#define INCLUDE_evita_dom_public_io_context_id_h

#include <functional>

namespace domapi {

class IoContextId {
  public: int value_;

  public: IoContextId(const IoContextId& other);
  private: explicit IoContextId(int value);
  public: IoContextId();
  public: ~IoContextId();

  public: IoContextId& operator=(const IoContextId& other);

  public: bool operator==(const IoContextId& other) const;
  public: bool operator!=(const IoContextId& other) const;

  public: int value() const { return value_; }

  public: static IoContextId New();
};

class FileId : public IoContextId {
  public: explicit FileId(IoContextId context_id);
  public: ~FileId();
};

class ProcessId : public IoContextId{
  public: explicit ProcessId(IoContextId context_id);
  public: ~ProcessId();
};

}  // namespace domapi

namespace std {
template<> struct hash<domapi::IoContextId> {
  size_t operator()(const domapi::IoContextId& context_id) const;
};
}  // namespace std

#endif //!defined(INCLUDE_evita_dom_public_io_context_id_h)
