# Copyright (c) 2016 Project Vogue. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import sys

module_path = os.path.dirname(os.path.realpath(__file__))
root_dir = os.path.normpath(os.path.join(
    module_path, os.pardir, os.pardir))
third_party_dir = os.path.normpath(
    os.path.join(root_dir, os.pardir, 'third_party'))
templates_dir = os.path.normpath(
    os.path.join(module_path, os.pardir, 'templates'))

# jinja2 is in chromium's third_party directory.
# Insert at 1 so at front to override system libraries, and
# after path[0] == invoking script dir
sys.path.insert(1, third_party_dir)
import jinja2


def generate(context, template_name):
    jinja_env = initialize_jinja_env(None)
    template = jinja_env.get_template(template_name)
    return template.render(context)


def initialize_jinja_env(cache_dir):
    jinja_env = jinja2.Environment(
        loader=jinja2.FileSystemLoader(templates_dir),
        # Bytecode cache is not concurrency-safe unless pre-cached:
        # if pre-cached this is read-only, but writing creates a race
        # condition.
        bytecode_cache=jinja2.FileSystemBytecodeCache(cache_dir),
        keep_trailing_newline=True,  # newline-terminate generated files
        lstrip_blocks=True,  # so can indent control flow tags
        trim_blocks=True)
    return jinja_env


def make_context(test_name, file_names):
    return {
        'Name': test_name.title(),
        'js_files': [make_js_file_context(file_name)
                     for file_name in file_names]
    }


def make_js_file_context(file_name):
    return {
        'name': os.path.basename(file_name).replace('_test.js', ''),
        'path_components': ['"%s"' % component
                            for component in file_name.split('/')],
    }


def main():
    if len(sys.argv) != 4:
        raise Exception('Usage: %s output_file test_name list_file' %
                        os.path.basename(sys.argv[0]))

    output_file_name = sys.argv[1]
    test_name = sys.argv[2]
    list_file_name = sys.argv[3]

    file_names = []
    with open(list_file_name) as list_file:
        file_names = list_file.read().split(' ')

    context = make_context(test_name, file_names)
    with open(output_file_name, 'wt') as output_file:
        output_file.write(generate(context, 'js_test.cc'))


if __name__ == '__main__':
    sys.exit(main())
