#
# Pre-commit check
#

import pipes
import sys

BLACK_LIST = [
    'base/',
    'build/',
    'buildtools/',
    'gin/',
    'evita/dom/bindings/templates',
    'testing/',
    'tools/',
    'third_party/',
]


def should_check(path):
    for dir in BLACK_LIST:
        if path.startswith(dir):
            return False
    return True


def lint_command_of(fileName):
    if fileName.endswith(".cc") or fileName.endswith(
            ".h") or fileName.endswith(".cpp"):
        return 'cpplint %(name)s 2>&1' % {'name': fileName}
    if fileName.endswith(".py"):
        return 'pylint -f msvs %(name)s 2>&1' % {'name': fileName}
    return ''


def main():
    git_pipe = pipes.Template()
    git_pipe.prepend('git diff --cached --name-status', '.-')
    diff_output = git_pipe.open('files', 'r')
    lines = diff_output.readlines()
    exit_code = 0
    for line in lines:
        line = line.rstrip()
        if len(line) == 0:
            continue
        words = line.split()
        if words[0] == 'D':
            continue
        cpplint_pipe = pipes.Template()
        if not should_check(words[1]):
            print >> sys.stderr, 'Ignore', words[1]
            continue
        command_line = lint_command_of(words[1])
        if command_line == '':
            print >> sys.stderr, 'Skip', words[1]
            continue
        cpplint_pipe.prepend(command_line, '.-')
        outputs = cpplint_pipe.open('files', 'r').readlines()
        if len(outputs) == 0:
            continue
        if outputs[len(outputs) - 1] == 'Total errors found: 0\n':
            continue
        exit_code = 1
        for output in outputs:
            output = output.rstrip()
            print output
    diff_output.close()
    sys.exit(exit_code)

if __name__ == '__main__':
    main()
