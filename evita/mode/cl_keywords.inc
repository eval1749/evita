// -*- Mode: C++; -*-
// evcl - listner - Common Lisp Keyword Definitions
// listener/winapp/mode/cl_keywors.inc
//
// Copyright (C) 1996-2008 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
// @(#)$Id: //proj/evcl3/mainline/listener/winapp/cm_CmdProc.h#1 $
//
#define constant_name(x)    L ## #x,
#define macro_operator(x)   L ## #x,
#define special_operator(x) L ## #x,

constant_name(&allow-other-keys)
constant_name(&aux)
constant_name(&body)
constant_name(&environment)
constant_name(&key)
constant_name(&optional)
constant_name(&rest)
constant_name(&whole)

// special operators
special_operator(block)                  //  1
special_operator(catch)                  //  2
special_operator(eval-when)              //  3
special_operator(flet)                   //  4
special_operator(function)               //  5
special_operator(go)                     //  6
special_operator(if)                     //  7
special_operator(labels)                 //  8
special_operator(let)                    //  9
special_operator(let*)                   // 10
special_operator(load-time-value)        // 11
special_operator(locally)                // 12
special_operator(macrolet)               // 13
special_operator(multiple-value-call)    // 14
special_operator(multiple-value-prog1)   // 15
special_operator(progn)                  // 16
special_operator(progv)                  // 17
special_operator(quote)                  // 18
special_operator(return-from)            // 19
special_operator(setq)                   // 20
special_operator(symbol-macrolet)        // 21
special_operator(tagbody)                // 22
special_operator(the)                    // 23
special_operator(throw)                  // 24
special_operator(unwind-protect)         // 25

// Expressions
special_operator(declare)
special_operator(lambda)

// Declaration identifiers
special_operator(declaration)           // 1
special_operator(dynamic-extent)        // 2
special_operator(ftype)                 // 3
special_operator(ignorable)             // 4
special_operator(ignore)                // 5
special_operator(inline)                // 6
special_operator(notinline)             // 7
special_operator(optimize)              // 8
special_operator(special)               // 9
special_operator(type)                  // 10
special_operator(values)                // 11

// Optmization quality
special_operator(compilation-speed)
special_operator(debug)
special_operator(safty)
special_operator(space)
special_operator(speed)

// Macros
constant_name(array-dimension-limit)
constant_name(array-rank-limit)
constant_name(array-total-size-limit)
constant_name(boole-1)
constant_name(boole-2)
constant_name(boole-and)
constant_name(boole-andc1)
constant_name(boole-andc2)
constant_name(boole-c1)
constant_name(boole-c2)
constant_name(boole-clr)
constant_name(boole-eqv)
constant_name(boole-ior)
constant_name(boole-nand)
constant_name(boole-nor)
constant_name(boole-orc1)
constant_name(boole-orc2)
constant_name(boole-set)
constant_name(boole-xor)
constant_name(call-arguments-limit)
constant_name(char-code-limit)
constant_name(double-float-epsilon)
constant_name(double-float-negative-epsilon)
constant_name(internal-time-units-per-second)
constant_name(lambda-list-keywords)
constant_name(lambda-parameters-limit)
constant_name(least-negative-double-float)
constant_name(least-negative-long-float)
constant_name(least-negative-normalized-double-float)
constant_name(least-negative-normalized-long-float)
constant_name(least-negative-normalized-short-float)
constant_name(least-negative-normalized-single-float)
constant_name(least-negative-short-float)
constant_name(least-negative-single-float)
constant_name(least-positive-double-float)
constant_name(least-positive-long-float)
constant_name(least-positive-normalized-double-float)
constant_name(least-positive-normalized-long-float)
constant_name(least-positive-normalized-short-float)
constant_name(least-positive-normalized-single-float)
constant_name(least-positive-short-float)
constant_name(least-positive-single-float)
constant_name(long-float-epsilon)
constant_name(long-float-negative-epsilon)
constant_name(most-negative-double-float)
constant_name(most-negative-fixnum)
constant_name(most-negative-long-float)
constant_name(most-negative-short-float)
constant_name(most-negative-single-float)
constant_name(most-positive-double-float)
constant_name(most-positive-fixnum)
constant_name(most-positive-long-float)
constant_name(most-positive-short-float)
constant_name(most-positive-single-float)
constant_name(multiple-values-limit)
constant_name(nil)
constant_name(pi)
constant_name(short-float-epsilon)
constant_name(short-float-negative-epsilon)
constant_name(single-float-epsilon)
constant_name(single-float-negative-epsilon)
constant_name(t)

macro_operator(and)
macro_operator(assert)
macro_operator(case)
macro_operator(ccase)
macro_operator(check-type)
macro_operator(cond)
macro_operator(ctypecase)
macro_operator(decf)
macro_operator(declaim)
macro_operator(defclass)
macro_operator(defconstant)
macro_operator(defgeneric)
macro_operator(define-compiler-macro)
macro_operator(define-condition)
macro_operator(define-method-combination)
macro_operator(define-modify-macro)
macro_operator(define-setf-expander)
macro_operator(define-symbol-macro)
macro_operator(defmacro)
macro_operator(defmethod)
macro_operator(defpackage)
macro_operator(defparameter)
macro_operator(defsetf)
macro_operator(defstruct)
macro_operator(deftype)
macro_operator(defun)
macro_operator(defvar)
macro_operator(destructuring-bind)
macro_operator(do)
macro_operator(do*)
macro_operator(do-all-symbols)
macro_operator(do-external-symbols)
macro_operator(dolist)
macro_operator(do-symbols)
macro_operator(dotimes)
macro_operator(ecase)
macro_operator(etypecase)
macro_operator(formatter)
macro_operator(handler-bind)
macro_operator(handler-case)
macro_operator(ignore-errors)
macro_operator(incf)
macro_operator(in-package)
macro_operator(lambda)
macro_operator(loop)
macro_operator(loop-finish)
macro_operator(multiple-value-bind)
macro_operator(multiple-value-list)
macro_operator(multiple-value-setq)
macro_operator(nth-value)
macro_operator(or)
macro_operator(pop)
macro_operator(pprint-logical-block)
macro_operator(print-unreadable-object)
macro_operator(prog)
macro_operator(prog*)
macro_operator(prog1)
macro_operator(prog2)
macro_operator(psetf)
macro_operator(psetq)
macro_operator(push)
macro_operator(pushnew)
macro_operator(remf)
macro_operator(restart-bind)
macro_operator(restart-case)
macro_operator(return)
macro_operator(rotatef)
macro_operator(setf)
macro_operator(shiftf)
macro_operator(step)
macro_operator(time)
macro_operator(trace)
macro_operator(typecase)
macro_operator(unless)
macro_operator(untrace)
macro_operator(when)
macro_operator(with-accessors)
macro_operator(with-compilation-unit)
macro_operator(with-condition-restarts)
macro_operator(with-hash-table-iterator)
macro_operator(with-input-from-string)
macro_operator(with-open-file)
macro_operator(with-open-stream)
macro_operator(with-output-to-string)
macro_operator(with-package-iterator)
macro_operator(with-simple-restart)
macro_operator(with-slots)
macro_operator(with-standard-io-syntax)

// case and typecase
macro_operator(otherwise)

#undef constant_name
#undef macro_operator
#undef special_operator
