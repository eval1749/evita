// Copyright 1996-2007 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef REGEX_REGEX_BYTECODES_H_
#define REGEX_REGEX_BYTECODES_H_

// Control operations
#define FOR_EACH_BYTE_CODE_CONTROL(VC) \
  VC(Capture, nth_start_end)           \
  VC(Continue, nextPc_posn)            \
  VC(Fail, none)                       \
  VC(PopInt, int)                      \
  VC(PopPosn, posn)                    \
  VC(PushInt, int)                     \
  VC(PushPosn, posn)                   \
  VC(Repeat_F, nextPc_posn_minPosn)    \
  VC(Repeat_B, nextPc_posn_maxPosn)    \
  VC(ResetCapture, nth)                \
  VC(SaveCxp, cxp_vsp)                 \
  VC(SavePosn, posn)

#define FOR_EACH_BYTE_CODE(V, VBF, vCBF, VC)                            \
  FOR_EACH_BYTE_CODE_CONTROL(VC)                                        \
  V(End, none)                                                          \
  /* [A] */                                                             \
  V(AfterNewline, none)     /* Matches start of line. "(?m:^)" */       \
  VBF(Any, none)            /* Matches any character. ".", "\p{ANY}" */ \
  V(AsciiBoundary, none)    /* \b */                                    \
  V(AsciiNotBoundary, none) /* \B */                                    \
  /* [B] */                                                             \
  V(BeforeNewline, none) /* (?m:$) Matches end of line. */              \
  /* [C] */                                                             \
  VBF(Capture, n)                                                       \
  V(CaptureIfNot, ln)                                                   \
  VCBF(CaptureEq, n)                                                    \
  V(CaptureMakeUnbound, n)                                              \
  VBF(CategoryEq, n) /* "\p{cat}" */                                    \
  VBF(CategoryNe, n)                                                    \
  VBF(CategoryLe, nn)                                                   \
  VBF(CategoryGt, nn)                                                   \
  VCBF(CharEq, c)                                                       \
  VCBF(CharNe, c)                                                       \
  VBF(CharSetEq, s)                                                     \
  VBF(CharSetNe, s)                                                     \
  /* [E] */                                                             \
  V(EndOfLine, none)   /* \Z Matches end of line */                     \
  V(EndOfString, none) /* \z Matches end of string */                   \
  /* [G] */                                                             \
  V(Go, l)                                                              \
  /* [L] */                                                             \
  VBF(Last, n)                                                          \
  /* [M] */                                                             \
  V(Max, ln)                                                            \
  V(Min, ln)                                                            \
  /* [N] */                                                             \
  V(Nulc, l)                                                            \
  V(Null, l)                                                            \
  /* O */                                                               \
  V(Or, l)                                                              \
  /* [P] */                                                             \
  V(Posn, none) /* \G Matches end of last match */                      \
  V(Push, l)                                                            \
  /* [R] */                                                             \
  VCBF(RangeEq, cc)                                                     \
  VCBF(RangeNe, cc) /* Bytecode for simple repetation (re*). */         \
  VBF(RepeatAny, n)                                                     \
  VCBF(RepeatCharEq, nc)                                                \
  VCBF(RepeatCharNe, nc)                                                \
  VBF(RepeatCharSetEq, ns)                                              \
  VBF(RepeatCharSetNe, ns)                                              \
  VCBF(RepeatRangeEq, ncc)                                              \
  VCBF(RepeatRangeNe, ncc)                                              \
  VBF(Rest, n)                                                          \
  V(RestoreCxp, none)                                                   \
  V(RestorePosn, none)                                                  \
  V(StartOfString, none) /* (?:s)^, (?-m)^, (?-s)^ */                   \
  VCBF(StringEq, s)                                                     \
  VCBF(StringNe, s)                                                     \
  V(Success, none)                                                      \
  /* [U] */                                                             \
  V(UnicodeBoundary, none)    /* \b */                                  \
  V(UnicodeNotBoundary, none) /* \B */                                  \
  /* Note: We must keep order of AsciiDigitCharEq to */                 \
  /* RepeatUnicodeCharNe for easy computing of repeat operation. */     \
  /* See computeRepeatOneWidthOp in regex_compile.cpp */                \
  VBF(AsciiDigitCharEq, none)      /* \d */                             \
  VBF(AsciiDigitCharNe, none)      /* \D */                             \
  VBF(AsciiSpaceCharEq, none)      /* \s */                             \
  VBF(AsciiSpaceCharNe, none)      /* \S */                             \
  VBF(AsciiWordCharEq, none)       /* \w */                             \
  VBF(AsciiWordCharNe, none)       /* \W */                             \
  VBF(UnicodeDigitCharEq, none)    /* \d */                             \
  VBF(UnicodeDigitCharNe, none)    /* \D */                             \
  VBF(UnicodeSpaceCharEq, none)    /* \s */                             \
  VBF(UnicodeSpaceCharNe, none)    /* \S */                             \
  VBF(UnicodeWordCharEq, none)     /* \w */                             \
  VBF(UnicodeWordCharNe, none)     /* \W */                             \
  VBF(RepeatAsciiDigitCharEq, n)   /* \d* */                            \
  VBF(RepeatAsciiDigitCharNe, n)   /* \D* */                            \
  VBF(RepeatAsciiSpaceCharEq, n)   /* \s* */                            \
  VBF(RepeatAsciiSpaceCharNe, n)   /* \S* */                            \
  VBF(RepeatAsciiWordCharEq, n)    /* \w* */                            \
  VBF(RepeatAsciiWordCharNe, n)    /* \W* */                            \
  VBF(RepeatUnicodeDigitCharEq, n) /* \d* */                            \
  VBF(RepeatUnicodeDigitCharNe, n) /* \D* */                            \
  VBF(RepeatUnicodeSpaceCharEq, n) /* \s* */                            \
  VBF(RepeatUnicodeSpaceCharNe, n) /* \S* */                            \
  VBF(RepeatUnicodeWordCharEq, n)  /* \w* */                            \
  VBF(RepeatUnicodeWordCharNe, n)  /* \W* */

#endif  // REGEX_REGEX_BYTECODES_H_
