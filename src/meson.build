configure_file(
  output: 'config.h',
  configuration: config,
  macro_name: 'INIT_CONFIG_H'
)

executable(
  'init', 'init/main.c', 'init/rc.c', 'init/tty.c', 'init/shutdown.c',
  include_directories: includes,
  install: true,
  install_dir: get_option('sbindir')
)

executable(
  'rundaemon', 'rundaemon/main.c',
  include_directories: includes,
  install: true,
  install_dir: get_option('sbindir')
)
