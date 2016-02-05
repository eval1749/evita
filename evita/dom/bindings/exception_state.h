// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_BINDINGS_EXCEPTION_STATE_H_
#define EVITA_DOM_BINDINGS_EXCEPTION_STATE_H_

#include <iosfwd>
#include <string>

#include "base/macros.h"
#include "base/strings/string_piece.h"
#include "evita/ginx/v8.h"

namespace dom {

#define FOR_EACH_EXCEPTION_STATE_SITUATION(V) \
  V(Construction)                             \
  V(DispatchEvent)                            \
  V(MethodCall)                               \
  V(PropertyGet)                              \
  V(PropertySet)

//////////////////////////////////////////////////////////////////////
//
// ExceptionState
//
class ExceptionState final {
 public:
  enum class Situation {
#define V(name) name,
    FOR_EACH_EXCEPTION_STATE_SITUATION(V)
#undef V
  };

  ExceptionState(Situation situation,
                 v8::Local<v8::Context> context,
                 base::StringPiece interface_name,
                 base::StringPiece property_name);

  ExceptionState(Situation situation,
                 v8::Local<v8::Context> context,
                 base::StringPiece interface_name);

  ~ExceptionState();

  bool is_thrown() const { return is_thrown_; }

  // Throw arity error in generated code from IDL.
  void ThrowArityError(int min_arity, int max_arity, int actual_arity);
  // Throw argument type error in generated code from IDL.
  void ThrowArgumentError(const char* expected_type,
                          v8::Local<v8::Value> value,
                          int index);
  void ThrowError(base::StringPiece detail);
  void ThrowException(v8::Local<v8::Value> exception);
  void ThrowPlatformError(base::StringPiece api_name, int error_code);
  void ThrowRangeError(base::StringPiece detail);
  // Throw receiver type error in generated code from IDL.
  void ThrowReceiverError(v8::Local<v8::Value> value);
  void ThrowTypeError(base::StringPiece detail);

 private:
  std::string ComposeMessage(const std::string& detail) const;
  std::string ComposeMessage(base::StringPiece detail) const;

  v8::Local<v8::Context> context_;
  const base::StringPiece interface_name_;
  bool is_thrown_ = false;
  const base::StringPiece property_name_;
  const Situation situation_;

  DISALLOW_COPY_AND_ASSIGN(ExceptionState);
};

std::ostream& operator<<(std::ostream& ostream,
                         ExceptionState::Situation situation);

}  // namespace dom

#endif  // EVITA_DOM_BINDINGS_EXCEPTION_STATE_H_
