#!/usr/bin/env python

import os
import sys


class ExternsSource:

    def __init__(self, name, file_name, lines):
        self._file_name = file_name
        self._lines = lines
        self._name = name

    @property
    def file_name(self):
        return self._file_name

    @property
    def lines(self):
        return self._lines

    @property
    def name(self):
        return self._name


def escape_as_cc_string(text):
    return text.replace('\\', '\\\\').replace('"', '\\"').replace('\n', '\\n')


def internal_file_name_of(module_name, file_name):
    return '//externs/%s/%s' % (module_name, file_name)


def generate_cc_file(module_file_name, module_name, sources):
    lines = [
        '#include "joana/checker/externs_module.h"',
        ''
        'namespace joana {',
        'namespace {',
    ]

    for source in sources:
        lines += ['', 'const char k%sContent[] = {' % source.name.title()]
        for line in source.lines:
            lines += [to_digits(line)]
        lines += ['};']

    lines += [
        '',
        '}  // namespace',
        '',
        'ExternsModule Get%sExtens() {' % module_name.title(),
        '  std::vector<ExternsFile> externs_files{',
    ]

    for source in sources:
        lines += ['    ExternsFile{"%s", sizeof(k%sContent), k%sContent},' % (
            internal_file_name_of(module_name, source.file_name),
            source.name.title(),
            source.name.title())]

    lines += [
        '  };',
        '  return ExternsModule{"%s", externs_files};' % module_name,
        '}',
        '',
        '}  // namespace joana'
    ]
    with open('%s_externs.cc' % module_file_name, 'wt') as output:
        output.write('\n'.join(lines))


def make_source(path_name):
    file_name = os.path.basename(path_name)
    return ExternsSource(os.path.splitext(file_name)[0],
                 file_name,
                 open(path_name, 'rt').readlines())


def to_digits(text):
    digits = []
    for ch in text:
        digits += ['%d' % ord(ch)]
    return '%s,' % ', '.join(digits)


def main():
    if len(sys.argv) < 3:
        sys.stderr.write('Usage: %s output_dir module_name source_file...\n' %
                         sys.argv[0])
        return 1
    output_directory = sys.argv[1]
    module_name = sys.argv[2]

    sources = []
    for file_name in sys.argv[3:]:
        sources.append(make_source(file_name))

    module_file_name = os.path.join(output_directory, module_name.lower())
    generate_cc_file(module_file_name, module_name, sources)

if __name__ == "__main__":
    main()
