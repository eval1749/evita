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

TEST_F(PromiseResolverTest, Identity) {
  v8::HandleScope handle_scope(isolate());
  RunScript("var resolver = new PromiseResolver();"
            "var promise = resolver.promise();");
  EXPECT_SCRIPT_TRUE("promise instanceof Promise");
}

TEST_F(PromiseResolverTest, Reject) {
  v8::HandleScope handle_scope(isolate());
  PromiseResolver* resolver = new PromiseResolver();
  RunScript("var result = 'not called';"
            "var promise; function setup(x) { promise = x; }");
  Call("setup", {resolver->NewPromise(isolate())});
  EXPECT_SCRIPT_TRUE("promise instanceof Promise");
  RunScript("promise.then(function(x) { result = 'resolved ' + x; },"
            "             function(x) { result = 'rejected ' + x;});");
  resolver->Reject(isolate(), std::string("foo"));
  EXPECT_SCRIPT_EQ("rejected foo", "result");
}

TEST_F(PromiseResolverTest, Resolve) {
  v8::HandleScope handle_scope(isolate());
  PromiseResolver* resolver = new PromiseResolver();
  RunScript("var result = 'not called';"
            "var promise; function setup(x) { promise = x; }");
  Call("setup", {resolver->NewPromise(isolate())});
  EXPECT_SCRIPT_TRUE("promise instanceof Promise");
  RunScript("promise.then(function(x) { result = 'resolved ' + x; },"
            "             function(x) { result = 'rejected ' + x;});");
  resolver->Resolve(isolate(), std::string("foo"));
  EXPECT_SCRIPT_EQ("resolved foo", "result");
}

}  // namespace

}  // namespace v8_glue
