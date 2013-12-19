//////////////////////////////////////////////////////////////////////////////
//
// Charset 
//
// Copyright (C) 1996-2012 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
// @(#)$Id: //proj/evcl3/mainline/listener/winapp/ed_Buffer.h#9 $
//
#if !defined(INCLUDE_charset_Charset_h)
#define INCLUDE_charset_Charset_h

namespace Charset {

// http://msdn.microsoft.com/en-us/goglobal/bb964653
// In WinNls.h
//  CP_ACP        0
//  CP_INSTALLED  1
//  CP_SUPPORTED  2
//  CP_OEMCP      1
//  CP_MACCP      2
//  CP_THREAD_ACP 3
//  CP_SYMBOL     42
//  CP_UTF7       65000
//  CP_UTF8       65001
enum CodePage {
  CP_GBK = 936, // Simplified Chinese, GB 2312-80
  CP_EUC_JP = 51932,
  CP_EUC_JP_OLD = 50932,
  CP_ISO_2022_JP = 50222,   // Japanese (JIS-Allow 1 byte kana - SO/SI)
  CP_JOHAB = 1361, // KS C-5601-1992
  CP_SHIFT_JIS = 932,
  CP_KOREA = 949, // KS C-5601-1987
  CP_BIG5 = 950, // Traditional Chinese, Big-5
  CP_UTF_8 = 65001,
};

} // Charset

#endif // !defined(INCLUDE_charset_Charset_h)
