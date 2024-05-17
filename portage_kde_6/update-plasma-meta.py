#!/usr/bin/env python3
#
# Copyright (C) <2024> <dememax@hotmail.com>
# Author: Maxim DEMENTYEV
#
# Update portage configs to kde 6
#

from subprocess import run, PIPE
import sys

MARKER_LINE = '!!! One of the following masked packages is required to complete your request:'
PREFIX='- '
SUFFIX='::gentoo (masked by: package.mask, ~amd64 keyword)'
LINE_AFTER='/usr/portage/profiles/package.mask:'

if __name__ == '__main__':
    while True:
        # run pretend
        res = run(['emerge', '-epv', 'world'], stdout=PIPE, stderr=PIPE)
        print('out', res.stdout)
        print('err', res.stderr)
        if not res.stderr:
            print('No more stderr output. Exiting...')
            sys.exit(0)

        # parse the output
        lines = res.stderr.decode().split('\n')
        print('lines', lines)
        if '!!! One of the following masked packages is required to complete your request:' not in lines:
            print('No marker line. Exiting...')
            sys.exit(8)
        index = lines.index(MARKER_LINE)
        if len(lines) <= index + 2:
            print(f'Error!!! len: {len(lines)}, index: {index} No space for package and line after! Exiting...')
            sys.exit(7)
        if lines[index + 2] != LINE_AFTER:
            print(f'Error!!! len: {len(lines)}, index: {index} No line after is detected! Exiting...')
            sys.exit(6)
        package_line = lines[index+1]
        if not package_line.startswith(PREFIX):
            print(f'Error!!! package_line: "{package_line}" No prefix is detected! Exiting...')
            sys.exit(5)
        if not package_line.endswith(SUFFIX):
            print(f'Error!!! package_line: "{package_line}" No suffix is detected! Exiting...')
            sys.exit(4)
        package = package_line[len(PREFIX):-len(SUFFIX)]
        print('package', package)
        hyph_index = package.rfind('-')
        if hyph_index < 1:
            print(f'Error!!! package_line: "{package_line}", hyph_index: {hyph_index} No hiphen 1 is detected! Exiting...')
            sys.exit(3)
        if hyph_index + 2 > len(package):
            print(f'Error!!! package_line: "{package_line}", hyph_index: {hyph_index} No hiphen 2 is detected! Exiting...')
            sys.exit(2)
        if package[hyph_index + 1] == 'r':
            hyph_index = package.rfind('-', 0, hyph_index)
            if hyph_index < 1 or hyph_index + 2 > len(package):
                print(f'Error!!! package_line: "{package_line}", hyph_index: {hyph_index} No hiphen 3 is detected! Exiting...')
                sys.exit(1)
        name = package[:hyph_index]
        print('package name', name)
        version = package[hyph_index+1:]
        print('package version', version)
        name_expr = name.replace('/', '\\/')
        expr = f"s/={name_expr}-[0-9]+(\\.[0-9]+)*(-r[0-9]+)?/={name_expr}-{version}/"
        print('expr', expr)
        for i in ['/etc/portage/package.accept_keywords/plasma-meta', '/etc/portage/package.unmask/plasma-meta']:
            res = run(['sed', '-E', '-e', expr, '--in-place=.bak', i])
