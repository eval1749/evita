;;;; -*- Mode: Lisp; -*-
;;; Regex Facility Test Driver
;;; retest.lisp
;;;
;;; Copyright (C) 1996-2008 by Project Vogue.
;;; Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
;;;
;;; $Id: //proj/evedit2/mainline/regex/retest.lisp#1 $
;;;
;
(in-package :cl-user)

(defun backslash (string)
    (declare (values simple-string))
    (declare (type string string))
  (with-input-from-string (in string)
  (with-output-to-string (out)
    (let ((acc 0)
          (count 0)
          (state :normal) )
      (loop
        (let ((ch (read-char in nil)))
          (unless ch (return))
          (ecase state
            ((:normal)
              (if (char= ch #\\)
                  (setq state :backslash)
                (write-char ch out) ) )
            ((:backslash)
              (case ch
                ((#\n)
                  (write-char #\Newline out)
                  (setq state :normal) )
                ((#\t)
                  (write-char #\Tab out)
                  (setq state :normal) )
                ((#\u)
                  (setq acc 0)
                  (setq count 0)
                  (setq state :unicode) )
                (otherwise
                  (write-char #\\ out)
                  (write-char ch out)
                  (setq state :normal) )) )
            ((:unicode)
              (let ((digit (digit-char-p ch 16)))
                (unless digit (error "Bad \\u"))
                (setq acc (logior (ash acc 4) digit))
                (incf count)
                (when (eql count 4)
                  (write-char (code-char acc) out)
                  (setq state :normal) ) ) )) )) ) ) ) )

(defun test-case (id pat txt opts expect)
    (declare (type simple-string id))
    (declare (type string pat))
    (declare (type list opts))
    (declare (type (or list string) expect))
    (declare (values t))
  (labels (
    (collect (match)
        (declare (values list))
        (declare (type si::regex-match match))
      (when (si::matched-p match)
        (let ((results '()))
          (dotimes (nth (si::match-group-count match) (nreverse results))
            (push (si::match-string match nth) results) ) )) )

    (process ()
      (multiple-value-bind (re cond)
          (ignore-errors (apply #'si::compile-regex pat opts))
        (if cond
            (format nil "~A" cond)
          (collect (si::string-match re txt)) ) ) )
    )
    (let* ((result (process))
           (succeeded (equal expect result)) )
      (if succeeded
          (format t "SUCCEEDED ~S~%" id)
        (error "FAILED ~S expect=~S result=~S~%" id expect result) )
      succeeded ) ) )

#|
(defun test-case (id pat txt opts expect)
  (labels (
    (commafy (list &optional (separator ", "))
      (with-output-to-string (out)
        (let ((comma ""))
          (dolist (x list)
            (write-string comma out)
            (setq comma separator)
            (write-string x out) ) ) ) )
    (escape (in)
      (with-output-to-string (out)
        (write-char #\" out)
        (dotimes (i (length in))
          (let ((ch (char in i)))
            (cond
              ((char= ch #\Newline)
                (write-string "\\n" out) )
              ((char< ch #\Space)
                (format out "\\x~2,'0X" (char-code ch)) )
              ((or (char= ch #\\) (char= ch #\"))
                (write-char #\\ out)
                (write-char ch out))
              (t (write-char (char in i) out))) ) )
        (write-char #\" out)) )
    (parse-options (options)
      (let ((results '()))
        (when (getf options :from-end) (push "Regex::Option_Backward" results))
        (when (getf options :multiple-line) (push "Regex::Option_Multiline" results))
        (and results (list (commafy results))) ) )
    )
  (format t "  EXPECT_EQ(Result(~A), Execute(~A)); // ~A~%"
    (commafy (mapcar #'escape (if (listp expect) expect (list expect))))
    (commafy (list* (escape pat) (escape txt) (parse-options opts)))
    id) ) )

|#

(test-case "fixed-001" "foo" "foobar" nil '("foo"))
(test-case "fixed-002" "foo" "abcfoobar" nil '("foo"))
(test-case "quant-001" "foo.*" "foo" nil '("foo"))
(test-case "quant-002" "foo.*" "foobar" nil '("foobar"))
(test-case "quant-003" "foo.*b" "foobarbaz" nil '("foobarb"))
(test-case "capture-001" "foo(.*)b" "foobarbaz" nil '("foobarb" "bar"))

(test-case "syntax-error-001" "{foo}" "foo" nil "Regex compile failed at 1")
(test-case "syntax-error-002" "*foo*" "foo" nil "Regex compile failed at 1")
(test-case "syntax-error-003" "+foo+" "foo" nil "Regex compile failed at 1")
(test-case "syntax-error-004" "?foo?" "foo" nil "Regex compile failed at 1")
(test-case "syntax-error-005" "foo{}" "foo" nil "Regex compile failed at 5")
(test-case "syntax-error-006" "foo{bar}" "foo" nil "Regex compile failed at 5")
(test-case "syntax-error-007" "foo**" "foo" nil "Regex compile failed at 5")
(test-case "syntax-error-008" "(foo" "foo" nil "Regex compile failed at 0")
(test-case "syntax-error-009" "foo)" "foo" nil "Regex compile failed at 4")

