// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_editor_application_proxy)
#define INCLUDE_evita_editor_application_proxy

#include <memory>

#include "base/strings/string16.h"
#include "common/memory/singleton.h"

namespace editor {

//////////////////////////////////////////////////////////////////////
//
// ApplicationProxy
//
class ApplicationProxy : public common::Singleton<ApplicationProxy> {
  DECLARE_SINGLETON_CLASS(ApplicationProxy);

  private: class Channel;
  private: class EventObject;
  private: class ShellHandler;

  private: std::unique_ptr<Channel> channel_;
  private: std::unique_ptr<EventObject> event_;
  private: std::unique_ptr<ShellHandler> shell_handler_;

  private: ApplicationProxy();
  public: ~ApplicationProxy();

  public: void DidCopyData(const COPYDATASTRUCT* data);
  public: void DidStartChannel(HWND channel_hwnd);
  public: int Run();
  public: void WillStartApplication();

  DISALLOW_COPY_AND_ASSIGN(ApplicationProxy);
};

}  // namespace editor

#endif //!defined(INCLUDE_evita_editor_application_proxy)
