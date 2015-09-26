// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_EDITOR_APPLICATION_PROXY_H_
#define EVITA_EDITOR_APPLICATION_PROXY_H_

#include <memory>

#include "base/strings/string16.h"
#include "common/memory/singleton.h"

namespace editor {

//////////////////////////////////////////////////////////////////////
//
// ApplicationProxy
//
class ApplicationProxy final : public common::Singleton<ApplicationProxy> {
  DECLARE_SINGLETON_CLASS(ApplicationProxy);

 public:
  ~ApplicationProxy() final;

  void DidCopyData(const COPYDATASTRUCT* data);
  int Run();
  void WillStartApplication();

 private:
  class Channel;
  class EventObject;
  class ShellHandler;

  ApplicationProxy();

  void StartChannel(HWND channel_hwnd);

  std::unique_ptr<Channel> channel_;
  std::unique_ptr<EventObject> event_;
  std::unique_ptr<ShellHandler> shell_handler_;

  DISALLOW_COPY_AND_ASSIGN(ApplicationProxy);
};

}  // namespace editor

#endif  // EVITA_EDITOR_APPLICATION_PROXY_H_
