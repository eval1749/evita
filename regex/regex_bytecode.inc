// -*- Mode: C++; -*-

#if !defined(DefByteCode)
    #define DefByteCode(mp_mnemonic, mp_operands)
#endif

#if !defined(DefByteCodeBF)
    #define DefByteCodeBF(mp_mnemonic, mp_operands) \
        DefByteCode(mp_mnemonic ## _B, mp_operands) \
        DefByteCode(mp_mnemonic ## _F, mp_operands)
#endif

#if !defined(DefByteCodeCBF)
    #define DefByteCodeCBF(mp_mnemonic, mp_operands) \
        DefByteCodeBF(mp_mnemonic ## _Ci, mp_operands) \
        DefByteCodeBF(mp_mnemonic ## _Cs, mp_operands)
#endif

#if !defined(DefControl)
    #define DefControl(mp_name, mp_operands)
#endif 

    DefByteCode(End, none)

    // [A]
    DefByteCode(AfterNewline, none) // Matches start of line. "(?m:^)"
    DefByteCodeBF(Any, none)        // Matches any character. ".", "\p{ANY}"

    DefByteCode(AsciiBoundary, none)        // \b
    DefByteCode(AsciiNotBoundary, none)     // \B

    // [B]
    DefByteCode(BeforeNewline, none) // (?m:$) Matches end of line. 

    // [C]
    DefByteCodeBF(Capture, n)
    DefByteCode(CaptureIfNot, ln)
    DefByteCodeCBF(CaptureEq, n)
    DefByteCode(CaptureMakeUnbound, n)

    DefByteCodeBF(CategoryEq, n)    // "\p{cat}"
    DefByteCodeBF(CategoryNe, n)
    DefByteCodeBF(CategoryLe, nn)
    DefByteCodeBF(CategoryGt, nn)

    DefByteCodeCBF(CharEq, c)
    DefByteCodeCBF(CharNe, c)

    DefByteCodeBF(CharSetEq, s)
    DefByteCodeBF(CharSetNe, s)

    // [E]
    DefByteCode(EndOfLine,   none)  // \Z Matches end of line
    DefByteCode(EndOfString, none)  // \z Matches end of string

    // [F]
    DefByteCode(Fail, none)

    // [G]
    DefByteCode(Go, l)

    // [M]
    DefByteCode(Max, ln)
    DefByteCode(Min, ln)

    DefByteCodeBF(Last, n)

    DefByteCode(Nulc, l)
    DefByteCode(Null, l)

    DefByteCode(Or, l)

    // [P]
    DefByteCode(Posn, none) // \G Matches end of last match
    DefByteCode(Push, l)
    DefByteCode(PushInt, n)
    DefByteCode(PushPosn, none)

    // [R]
    DefByteCodeCBF(RangeEq, cc)
    DefByteCodeCBF(RangeNe, cc)

    // Bytecode for simple repetation (re*).
    DefByteCodeBF(RepeatAny, n)

    DefByteCodeCBF(RepeatCharEq, nc)
    DefByteCodeCBF(RepeatCharNe, nc)

    DefByteCodeBF(RepeatCharSetEq, ns)
    DefByteCodeBF(RepeatCharSetNe, ns)

    DefByteCodeCBF(RepeatRangeEq, ncc)
    DefByteCodeCBF(RepeatRangeNe, ncc)

    DefByteCodeBF(Rest, n)

    DefByteCode(RestoreCxp, none)
    DefByteCode(RestorePosn, none)

    // [S]
    DefByteCode(SaveCxp, none)
    DefByteCode(SavePosn, none)

    DefByteCode(StartOfString, none)    // (?:s)^, (?-m)^, (?-s)^

    DefByteCodeCBF(StringEq, s)
    DefByteCodeCBF(StringNe, s)

    DefByteCode(Success, none)

    // [U]
    DefByteCode(UnicodeBoundary, none)      // \b
    DefByteCode(UnicodeNotBoundary, none)   // \B

    // Note: We must keep order of AsciiDigitCharEq to
    // RepeatUnicodeCharNe for easy computing of repeat operation.
    // See computeRepeatOneWidthOp in regex_compile.cpp

    DefByteCodeBF(AsciiDigitCharEq, none)               // \d
    DefByteCodeBF(AsciiDigitCharNe, none)               // \D
    DefByteCodeBF(AsciiSpaceCharEq, none)               // \s
    DefByteCodeBF(AsciiSpaceCharNe, none)               // \S
    DefByteCodeBF(AsciiWordCharEq, none)                // \w
    DefByteCodeBF(AsciiWordCharNe, none)                // \W

    DefByteCodeBF(UnicodeDigitCharEq, none)             // \d
    DefByteCodeBF(UnicodeDigitCharNe, none)             // \D
    DefByteCodeBF(UnicodeSpaceCharEq, none)             // \s
    DefByteCodeBF(UnicodeSpaceCharNe, none)             // \S
    DefByteCodeBF(UnicodeWordCharEq, none)              // \w
    DefByteCodeBF(UnicodeWordCharNe, none)              // \W

    DefByteCodeBF(RepeatAsciiDigitCharEq, n)            // \d*
    DefByteCodeBF(RepeatAsciiDigitCharNe, n)            // \D*
    DefByteCodeBF(RepeatAsciiSpaceCharEq, n)            // \s*
    DefByteCodeBF(RepeatAsciiSpaceCharNe, n)            // \S*
    DefByteCodeBF(RepeatAsciiWordCharEq, n)             // \w*
    DefByteCodeBF(RepeatAsciiWordCharNe, n)             // \W*

    DefByteCodeBF(RepeatUnicodeDigitCharEq, n)          // \d*
    DefByteCodeBF(RepeatUnicodeDigitCharNe, n)          // \D*
    DefByteCodeBF(RepeatUnicodeSpaceCharEq, n)          // \s*
    DefByteCodeBF(RepeatUnicodeSpaceCharNe, n)          // \S*
    DefByteCodeBF(RepeatUnicodeWordCharEq, n)           // \w*
    DefByteCodeBF(RepeatUnicodeWordCharNe, n)           // \W*


    ////////////////////////////////////////////////////////////
    //
    // Control operations
    //
    DefControl(Capture,         nth_start_end)
    DefControl(Continue,        nextPc_posn)
    DefControl(Fail,            none)
    DefControl(PopInt,          int)
    DefControl(PopPosn,         posn)
    DefControl(PushInt,         int)
    DefControl(PushPosn,        posn)
    DefControl(Repeat_F,        nextPc_posn_minPosn)
    DefControl(Repeat_B,        nextPc_posn_maxPosn)
    DefControl(ResetCapture,    nth)
    DefControl(SaveCxp,         cxp_vsp)
    DefControl(SavePosn,        posn)

#undef DefByteCode
#undef DefControl
// EOF
