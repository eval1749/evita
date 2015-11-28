// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

$define(global, 'testing', function($export) {
  function equals(object1, object2) {
    if (object1 === object2)
      return true;
    if (!Array.isArray(object1) || !Array.isArray(object2))
      return false;
    if (object1.length !== object2.length)
      return false;
    return object1.every((element1, index) => {
      return equals(element1, object2[index]);
    });
  }

// Note: As of v8:4.9.41, JSON.stringify yield nothing for symbols.
  const stringify = JSON.stringify.bind(JSON);

  //////////////////////////////////////////////////////////////////////
  //
  // Fixture
  //
  class Fixture {
    /**
     * @param {string} name
     * @param {!Object} location
     * @param {*} actual
     * @param {string=} opt_description
     */
    constructor(name, location, actual, opt_description) {
      this.actual_ = actual;
      this.description_ = opt_description ? opt_description : '';
      this.expected_ = undefined;
      this.location_ = location;
      this.name_ = name;
      this.succeeded_ = false;
    }

    get failed() { return !this.succeeded_; }
    get succeeded() { return this.succeeded_; }

    toBeFalsy() {
      this.succeeded_ = !!this.actual_;
      this.description_ += '; expected falsy value';
    }

    toBeTruthy() {
      this.succeeded_ = !!this.actual_;
      this.description_ += '; expected truth value';
    }

    toEqual(expected) {
      this.toStrictEqual(expected);
      if (this.succeeded_)
        return;
      if (!Array.isArray(this.actual_) || !Array.isArray(expected))
        return;
      if (this.actual_.length !== expected.length)
        return;
      this.succeeded_ = true;
      this.actual_.forEach((actual, index) => {
        if (equals(actual, expected[index]))
          return;
        this.succeeded_ = false;
        this.description_ += 'actual: ' + stringify(actual) +
            ' expected: ' + stringify(expected[index]);
      });
    }

    toStrictEqual(expected) {
      this.expected_ = expected;
      this.succeeded_ = this.actual_ === expected;
    }

    asPrettyString() {
      const outcome = this.succeeded_ ? 'SUCCEEDED' : 'FAIELD';
      const fileName = this.location_.fileName;
      const lineNumber = this.location_.lineNumber;
      let message = this.description_.length
          ? ` ${this.description_}` : '';
      if (this.expected_) {
        message += ' ; actual: ' + stringify(this.actual_) +
            ' expected: ' + stringify(this.expected_);
      }
      return `${outcome} "${fileName}"(${lineNumber})${message}`;
    }
  }

  function expect(fixtures, fixtureName, value, description) {
    const fixture = new Fixture(fixtureName, getLocation(), value, description);
    fixtures.push(fixture);
    return fixture;
  }

  function getLocation() {
    function getStack() {
      let stack;
      try {
        throw new Error('get line number');
      } catch (error) {
        stack = error.stack.split('\n').slice(1);
      }
      return stack
    }
    const locations = getStack();
    let index = locations.findIndex((line) => {
      return line.indexOf('at Object.test \u0028') > 0;
    });
    if (index > 0) {
        const match = new RegExp('at (.+?):(\\d+):(\\d+)$')
            .exec(locations[index - 1]);
        if (match)
          return {fileName: match[1], lineNumber: match[2], column: match[3]};
    }
    console.log(index, locations);
    return {fileName: '?', lineNumber: 0, column: 0};
  }

  function test(fixtureName, fixtureFunction) {
    const fixtures = [];
    const errors = [];
    const expectClosure = expect.bind(undefined, fixtures, fixtureName);
    const testModule = {expect: expectClosure};
    const startAt = Editor.performance.now();
    try {
      fixtureFunction.call(global, testModule);
    } catch (error) {
      console.log('test: fixture error', fixtureName, stringify(error));
      errors.push(error);
      throw error;
    }
    const endAt = Editor.performance.now();
    errors.forEach(error => console.log(error));
    fixtures.filter(fixture => fixture.failed).forEach(fixture => {
      console.log(' ', fixture.asPrettyString());
    });
    const fails = fixtures.filter(fixture => fixture.failed);
    const failed = fails.length || errors.length;
    const wallTime = Math.round((endAt - startAt) * 1000);
    console.log('/\u002F',
                failed ? 'FAILED' : 'SUCCEEDED',
                fixtureName,
                `(${wallTime}ms),`,
                `${fails.length}/${fixtures.length} tests failed.`);
  }

  $export({test});
});
