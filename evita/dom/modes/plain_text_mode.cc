// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/modes/plain_text_mode.h"

#include "evita/dom/modes/char_syntax.h"
#include "evita/dom/modes/mode.h"

namespace text
{

// Cxx mode character syntax
static const uint
k_rgnPlainTextCharSyntax[0x80 - 0x20] =
{
    CharSyntax::Syntax_Whitespace,                  // 0x20
    CharSyntax::Syntax_Punctuation,                 // 0x21 !
    CharSyntax::Syntax_StringQuote | (0x22 << CharSyntax::Trait_PairShift),  // 0x22 "
    CharSyntax::Syntax_Punctuation,                 // 0x23 #
    CharSyntax::Syntax_Punctuation,                 // 0x24 $
    CharSyntax::Syntax_Punctuation,                 // 0x25 %
    CharSyntax::Syntax_Punctuation,                 // 0x26 &
    CharSyntax::Syntax_Punctuation,                 // 0x27 '
    CharSyntax::Syntax_OpenParen   | (0x29 << CharSyntax::Trait_PairShift),  // 0x28 (
    CharSyntax::Syntax_CloseParen  | (0x28 << CharSyntax::Trait_PairShift),  // 0x29 )
    CharSyntax::Syntax_Punctuation,                 // 0x2A *
    CharSyntax::Syntax_Punctuation,                 // 0x2B +
    CharSyntax::Syntax_Punctuation,                 // 0x2C ,
    CharSyntax::Syntax_Punctuation,                 // 0x2D -
    CharSyntax::Syntax_Punctuation,                 // 0x2E .
    CharSyntax::Syntax_Punctuation,                 // 0x2F /

    CharSyntax::Syntax_Word,                        // 0x30 0
    CharSyntax::Syntax_Word,                        // 0x31 1
    CharSyntax::Syntax_Word,                        // 0x32 2
    CharSyntax::Syntax_Word,                        // 0x33 3
    CharSyntax::Syntax_Word,                        // 0x34 4
    CharSyntax::Syntax_Word,                        // 0x35 5
    CharSyntax::Syntax_Word,                        // 0x36 6
    CharSyntax::Syntax_Word,                        // 0x37 7
    CharSyntax::Syntax_Word,                        // 0x38 8
    CharSyntax::Syntax_Word,                        // 0x39 9
    CharSyntax::Syntax_Punctuation,                 // 0x3A :
    CharSyntax::Syntax_Punctuation,                 // 0x3B ;
    CharSyntax::Syntax_Punctuation,                 // 0x3C <
    CharSyntax::Syntax_Punctuation,                 // 0x3D =
    CharSyntax::Syntax_Punctuation,                 // 0x3E >
    CharSyntax::Syntax_Punctuation,                 // 0x3F ?

    CharSyntax::Syntax_Punctuation,                 // 0x40 @
    CharSyntax::Syntax_Word,                        // 0x41 A
    CharSyntax::Syntax_Word,                        // 0x42 B
    CharSyntax::Syntax_Word,                        // 0x43 C
    CharSyntax::Syntax_Word,                        // 0x44 D
    CharSyntax::Syntax_Word,                        // 0x45 E
    CharSyntax::Syntax_Word,                        // 0x46 F
    CharSyntax::Syntax_Word,                        // 0x47 G
    CharSyntax::Syntax_Word,                        // 0x48 H
    CharSyntax::Syntax_Word,                        // 0x49 I
    CharSyntax::Syntax_Word,                        // 0x4A J
    CharSyntax::Syntax_Word,                        // 0x4B K
    CharSyntax::Syntax_Word,                        // 0x4C L
    CharSyntax::Syntax_Word,                        // 0x4D M
    CharSyntax::Syntax_Word,                        // 0x4E N
    CharSyntax::Syntax_Word,                        // 0x4F O

    CharSyntax::Syntax_Word,                        // 0x50 P
    CharSyntax::Syntax_Word,                        // 0x51 Q
    CharSyntax::Syntax_Word,                        // 0x52 R
    CharSyntax::Syntax_Word,                        // 0x53 S
    CharSyntax::Syntax_Word,                        // 0x54 T
    CharSyntax::Syntax_Word,                        // 0x55 U
    CharSyntax::Syntax_Word,                        // 0x56 V
    CharSyntax::Syntax_Word,                        // 0x57 W
    CharSyntax::Syntax_Word,                        // 0x58 X
    CharSyntax::Syntax_Word,                        // 0x59 Y
    CharSyntax::Syntax_Word,                        // 0x5A Z
    CharSyntax::Syntax_OpenParen  | (0x5D << CharSyntax::Trait_PairShift),   // 0x5B [
    CharSyntax::Syntax_Punctuation,                 // 0x5C backslash(\)
    CharSyntax::Syntax_CloseParen | (0x5B << CharSyntax::Trait_PairShift),   // 0x5D ]
    CharSyntax::Syntax_Punctuation,                 // 0x5E ^
    CharSyntax::Syntax_Punctuation,                 // 0x5F _

    CharSyntax::Syntax_Punctuation,                 // 0x60 `
    CharSyntax::Syntax_Word,                        // 0x61 a
    CharSyntax::Syntax_Word,                        // 0x62 b
    CharSyntax::Syntax_Word,                        // 0x63 c
    CharSyntax::Syntax_Word,                        // 0x64 d
    CharSyntax::Syntax_Word,                        // 0x65 e
    CharSyntax::Syntax_Word,                        // 0x66 f
    CharSyntax::Syntax_Word,                        // 0x67 g
    CharSyntax::Syntax_Word,                        // 0x68 h
    CharSyntax::Syntax_Word,                        // 0x69 i
    CharSyntax::Syntax_Word,                        // 0x6A j
    CharSyntax::Syntax_Word,                        // 0x6B k
    CharSyntax::Syntax_Word,                        // 0x6C l
    CharSyntax::Syntax_Word,                        // 0x6D m
    CharSyntax::Syntax_Word,                        // 0x6E n
    CharSyntax::Syntax_Word,                        // 0x6F o

    CharSyntax::Syntax_Word,                        // 0x70 p
    CharSyntax::Syntax_Word,                        // 0x71 q
    CharSyntax::Syntax_Word,                        // 0x72 r
    CharSyntax::Syntax_Word,                        // 0x73 s
    CharSyntax::Syntax_Word,                        // 0x74 t
    CharSyntax::Syntax_Word,                        // 0x75 u
    CharSyntax::Syntax_Word,                        // 0x76 v
    CharSyntax::Syntax_Word,                        // 0x77 w
    CharSyntax::Syntax_Word,                        // 0x78 x
    CharSyntax::Syntax_Word,                        // 0x79 y
    CharSyntax::Syntax_Word,                        // 0x7A z
    CharSyntax::Syntax_OpenParen  | (0x7D << CharSyntax::Trait_PairShift),   // 0x7B {
    CharSyntax::Syntax_Word,                        // 0x7C |
    CharSyntax::Syntax_CloseParen | (0x7B << CharSyntax::Trait_PairShift),   // 0x7D }
    CharSyntax::Syntax_Word,                        // 0x7E ^
    CharSyntax::Syntax_Control,                     // 0x7F DEL
}; // k_rgnPlainTextCharSyntax

/// <summary>
///   Construct PlainTextMode object
/// </summary>
PlainTextMode::PlainTextMode(ModeFactory* pFactory, Buffer* pBuffer) :
    Mode(pFactory, pBuffer) {}

/// <summary>
///  Construct PlainTextModeFactory object
/// </summary>
PlainTextModeFactory::PlainTextModeFactory() :
    ModeFactory(k_rgnPlainTextCharSyntax) {}

}  // namespace text
