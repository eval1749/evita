// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/io/io_thread_proxy.h"

#include "base/bind.h"
#include "base/callback.h"
#include "base/message_loop/message_loop.h"
#include "base/threading/thread.h"

namespace io {

IoThreadProxy::IoThreadProxy(domapi::IoDelegate* delegate, base::Thread* thread)
    : delegate_(delegate), thread_(thread) {}

IoThreadProxy::~IoThreadProxy() {}

// IoDelegate
#define DEFINE_DELEGATE_2(name, type1, type2)                                \
  void IoThreadProxy::name(type1 p1, type2 p2) {                             \
    thread_->task_runner()->PostTask(                                        \
        FROM_HERE,                                                           \
        base::Bind(&IoDelegate::name, base::Unretained(delegate_), p1, p2)); \
  }

#define DEFINE_DELEGATE_3(name, type1, type2, type3)                          \
  void IoThreadProxy::name(type1 p1, type2 p2, type3 p3) {                    \
    thread_->task_runner()->PostTask(                                         \
        FROM_HERE, base::Bind(&IoDelegate::name, base::Unretained(delegate_), \
                              p1, p2, p3));                                   \
  }

#define DEFINE_DELEGATE_4(name, type1, type2, type3, type4)                   \
  void IoThreadProxy::name(type1 p1, type2 p2, type3 p3, type4 p4) {          \
    thread_->task_runner()->PostTask(                                         \
        FROM_HERE, base::Bind(&IoDelegate::name, base::Unretained(delegate_), \
                              p1, p2, p3, p4));                               \
  }

#define DEFINE_DELEGATE_6(name, ty1, ty2, ty3, ty4, ty5, ty6)                 \
  void IoThreadProxy::name(ty1 p1, ty2 p2, ty3 p3, ty4 p4, ty5 p5, ty6 p6) {  \
    thread_->task_runner()->PostTask(                                         \
        FROM_HERE, base::Bind(&IoDelegate::name, base::Unretained(delegate_), \
                              p1, p2, p3, p4, p5, p6));                       \
  }

DEFINE_DELEGATE_2(CheckSpelling,
                  const base::string16&,
                  const CheckSpellingResolver&)
DEFINE_DELEGATE_2(CloseContext,
                  const domapi::IoContextId&,
                  const domapi::IoIntPromise&)
DEFINE_DELEGATE_2(ComputeFullPathName,
                  const base::string16&,
                  const ComputeFullPathNamePromise&)
DEFINE_DELEGATE_3(GetWinResourceNames,
                  const domapi::WinResourceId&,
                  const base::string16&,
                  const GetWinResourceNamessPromise&)
DEFINE_DELEGATE_2(GetSpellingSuggestions,
                  const base::string16&,
                  const GetSpellingSuggestionsResolver&)
DEFINE_DELEGATE_6(LoadWinResource,
                  const domapi::WinResourceId&,
                  const base::string16&,
                  const base::string16&,
                  uint8_t*,
                  size_t,
                  const domapi::IoIntPromise&)
DEFINE_DELEGATE_3(MakeTempFileName,
                  const base::string16&,
                  const base::string16&,
                  const domapi::MakeTempFileNamePromise&)
DEFINE_DELEGATE_4(MoveFile,
                  const base::string16&,
                  const base::string16&,
                  const domapi::MoveFileOptions&,
                  const domapi::IoBoolPromise&)
DEFINE_DELEGATE_2(OpenDirectory,
                  const base::string16&,
                  const domapi::OpenDirectoryPromise&)
DEFINE_DELEGATE_3(OpenFile,
                  const base::string16&,
                  const base::string16&,
                  const domapi::OpenFilePromise&)
DEFINE_DELEGATE_2(OpenProcess,
                  const base::string16&,
                  const domapi::OpenProcessPromise&)
DEFINE_DELEGATE_2(OpenWinResource,
                  const base::string16&,
                  const domapi::OpenWinResourcePromise&)
DEFINE_DELEGATE_2(QueryFileStatus,
                  const base::string16&,
                  const domapi::QueryFileStatusPromise&)
DEFINE_DELEGATE_3(ReadDirectory,
                  domapi::IoContextId,
                  size_t,
                  const domapi::ReadDirectoryPromise&)
DEFINE_DELEGATE_4(ReadFile,
                  domapi::IoContextId,
                  void*,
                  size_t,
                  const domapi::IoIntPromise&)
DEFINE_DELEGATE_2(RemoveFile,
                  const base::string16&,
                  const domapi::IoBoolPromise&)
DEFINE_DELEGATE_4(WriteFile,
                  domapi::IoContextId,
                  void*,
                  size_t,
                  const domapi::IoIntPromise&)

}  // namespace io
