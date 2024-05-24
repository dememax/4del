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
MASK_SUFFIX='::gentoo (masked by: package.mask, ~amd64 keyword)'
ONLY_KEYWORD_SUFFIX='::gentoo (masked by: ~amd64 keyword)'
MASK_LINE_AFTER='/usr/portage/profiles/package.mask:'
CONFIG_FILES = ('/etc/portage/package.accept_keywords/plasma-meta', '/etc/portage/package.unmask/plasma-meta')

def update_for_cmd(cmd):
    while True:
        # run pretend
        res = run(cmd, stdout=PIPE, stderr=PIPE)
        print('out', res.stdout)
        print('err', res.stderr)
        if not res.stderr:
            print('No more stderr output. Exiting...')
            return 0

        # parse the output to find  the package to unmask
        # let's find the line with the package
        lines = res.stderr.decode().split('\n')
        print('lines', lines)
        if '!!! One of the following masked packages is required to complete your request:' not in lines:
            print('No marker line. Exiting...')
            return 0
        index = lines.index(MARKER_LINE)
        if len(lines) <= index + 2:
            print(f'Error!!! len: {len(lines)}, index: {index} No space for package and line after! Exiting...')
            return 7
        is_mask = True
        if lines[index + 2] != MASK_LINE_AFTER:
            is_mask = False
            print(f'Warning!!! len: {len(lines)}, index: {index} No mask line after is detected! Trying without...')
            # return 6

        # we found the line and its context
        # let's take the package info from this line
        package_line = lines[index+1]
        if not package_line.startswith(PREFIX):
            print(f'Error!!! package_line: "{package_line}" No prefix is detected! Exiting...')
            return 5
        actual_suffix = MASK_SUFFIX if is_mask else ONLY_KEYWORD_SUFFIX
        if not package_line.endswith(actual_suffix):
            print(f'Error!!! package_line: "{package_line}" No suffix "{actual_suffix}" is detected! Exiting...')
            return 4
        package = package_line[len(PREFIX):-len(actual_suffix)]

        # package is found, do configs contain it?
        print('package', package)
        hyph_index = package.rfind('-')
        if hyph_index < 1:
            print(f'Error!!! package_line: "{package_line}", hyph_index: {hyph_index} No hiphen 1 is detected! Exiting...')
            return 3
        if hyph_index + 2 > len(package):
            print(f'Error!!! package_line: "{package_line}", hyph_index: {hyph_index} No hiphen 2 is detected! Exiting...')
            return 2
        if package[hyph_index + 1] == 'r':
            hyph_index = package.rfind('-', 0, hyph_index)
            if hyph_index < 1 or hyph_index + 2 > len(package):
                print(f'Error!!! package_line: "{package_line}", hyph_index: {hyph_index} No hiphen 3 is detected! Exiting...')
                return 1
        name = package[:hyph_index]
        print('package name', name)
        version = package[hyph_index+1:]
        print('package version', version)
        package_regex = f"={name}-[0-9]+(\\.[0-9]+)*(-r[0-9]+)?"
        res = run(['grep', '-E', package_regex, CONFIG_FILES[0]])
        if res.returncode:
            print(f"Package not found (ret={res.returncode}) in configs, let's add it")
            for i in CONFIG_FILES:
                accept = " ~amd64"
                if "unmask" in i:
                    accept = ""
                    if not is_mask:
                        continue
                run(f'echo "={package}{accept}" >> {i}', shell=True)
            continue

        # package exists in configs, let's replace the version
        print("Replacing package version...")
        package_regex = package_regex.replace('/', '\\/')
        name_expr = name.replace('/', '\\/')
        sed_expr = f"s/{package_regex}/={name_expr}-{version}/"
        print('sed expr', sed_expr)
        for i in CONFIG_FILES:
            if "unmask" in i:
                if not is_mask:
                    continue
            run(['sed', '-E', '-e', sed_expr, '--in-place=.bak', i])

if __name__ == '__main__':
    e_world = ['emerge', '-epv', 'world']
    update_for = ('emerge', '-uND', '--with-bdeps=y', '--verbose-conflicts', '--backtrack=30', '-pv')
    plasma = list(update_for)
    plasma.append('=kde-plasma/plasma-meta-6.0.5')
    kdeutils = list(update_for)
    kdeutils.append('=kde-apps/kdeutils-meta-24.05.0')
    kdecore = list(update_for)
    kdecore.append('=kde-apps/kdecore-meta-24.05.0')
    kdegraphics = list(update_for)
    kdegraphics.append('=kde-apps/kdegraphics-meta-24.05.0')
    update_world = list(update_for)
    update_world.append('world')
    for cmd in (plasma, kdecore, kdeutils, kdegraphics, update_world, e_world):
        print("cmd:", cmd)
        r = update_for_cmd(cmd)
        if r:
            sys.exit(r)
