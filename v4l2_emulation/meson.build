# Maxim P. DEMENTYEV, 2025
# Video for Linux ver 2 emulation

project('v4l2-emu', 'c', 'cpp', meson_version: '>=1.7.0', default_options: 'cpp_std=c++23')

sources = [
    'emulate_close.cpp',
    'emulate_fopen.cpp',
    'emulate_ioctl.cpp',
    'emulate_open.cpp',
    'emulate_openat.cpp',
    'emulate_stat.cpp',
    'emulate_syscall.cpp',
  ]

dl_dep = dependency('dl')

shared_library('v4l2_emu',
  sources,
  dependencies : [dl_dep],
  install : true)
