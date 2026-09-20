project('init', 'c', version: '1.6.0')
includes = include_directories('include')
supported = ['linux', 'freebsd', 'netbsd']
if host_machine.system() not in supported
  error('Host OS type not supported by this project.')
endif
subdir('src')
subdir('scripts')
