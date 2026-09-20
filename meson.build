project(
  'init', 'c',
  version: '1.6.2', meson_version: '>=1.3.0',
  default_options: ['sysconfdir=/etc']
)
add_project_arguments('-DHAVE_CONFIG_H', language: ['c', 'cpp'])
supported = ['linux', 'freebsd', 'netbsd']
if host_machine.system() not in supported
  error('Host OS type not supported by this project.')
endif
includes = include_directories('include')
config = configuration_data()
config.set_quoted(
  'S2_STOPSYS',
  get_option('prefix') / get_option('libexecdir') / 'stage2stopsys'
)
config.set_quoted('RC', get_option('sysconfdir') / 'rc')
config.set_quoted('RC_SHUT', get_option('sysconfdir') / 'rc.shutdown')
config.set_quoted('RC_CONS', get_option('sysconfdir') / 'rc.consoles')
config.set_quoted(
  'RC_FALL',
  get_option('prefix') / get_option('libdir') / 'rc/rc.fallback'
)
subdir('src')
subdir('scripts')
