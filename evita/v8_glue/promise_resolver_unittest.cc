// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/abstract_dom_test.h"
#include "evita/v8_glue/converter.h"
#include "evita/v8_glue/promise_resolver.h"

namespace v8_glue {

namespace {

class PromiseResolverTest : public dom::AbstractDomTest {
  protected: PromiseResolverTest() {
  }
  public: virtual ~PromiseResolverTest() {
  }

  DISALLOW_COPY_AND_ASSIGN(PromiseResolverTest);
};

TEST_F(PromiseResolverTest, Reject) {
  v8::HandleScope handle_scope(isolate());
  PromiseResolver* resolver = new PromiseResolver();
  RunScript("var result = 'not called';"
            "function sample(promise) {"
            "  promise.then(function(x) { result = x; return 1; },"
            "               function() { result = 'rejected'; return 2;});"
            "}");
  Call("sample", {resolver->NewPromise(isolate())});
  resolver->Reject(isolate(), std::string("foo"));
  EXPECT_SCRIPT_EQ("rejected", "result");
}

TEST_F(PromiseResolverTest, Resolve) {
  v8::HandleScope handle_scope(isolate());
  PromiseResolver* resolver = new PromiseResolver();
  RunScript("var result = 'not called';"
            "function sample(promise) {"
            "  promise.then(function(x) { result = x; return 1; },"
            "               function() { result = 'rejected'; return 2;});"
            "}");
  Call("sample", {resolver->NewPromise(isolate())});
  resolver->Resolve(isolate(), std::string("foo"));
  EXPECT_SCRIPT_EQ("foo", "result");
}

}  // namespace

}  // namespace v8_glue
