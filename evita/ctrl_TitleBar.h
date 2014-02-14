// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_visual_control_TitleBar_h)
#define INCLUDE_visual_control_TitleBar_h

class TitleBar {
  private: int    m_cwch;
  private: HWND   m_hwnd;
  private: char16 m_wsz[100];

  public: TitleBar();
  public: ~TitleBar();

  public: bool IsEqual(const char16* title, int title_length) const;
  public: int Realize(HWND hwnd);
  public: int SetText(const char16* new_title, int new_title_length);
};

#endif //!defined(INCLUDE_visual_control_TitleBar_h)
