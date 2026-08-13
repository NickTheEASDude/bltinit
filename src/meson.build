executable(
  'init', 'main.c', 'tty.c', 'rc.c',
  include_directories: includes,
  install: true,
  install_dir: get_option('sbindir')
)

executable(
  'rundaemon', 'rundaemon.c',
  include_directories: includes,
  install: true,
  install_dir: get_option('sbindir')
)
