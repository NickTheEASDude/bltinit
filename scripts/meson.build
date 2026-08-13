install_data(
  'rcorder', 'stopsys',
  install_dir: get_option('sbindir'),
  install_mode: ['rwxr-xr-x', 0, 0]
)
links = ['reboot', 'halt', 'poweroff']
foreach link: links
  install_symlink(
    link,
    pointing_to: 'stopsys',
    install_dir: get_option('sbindir')
  )
endforeach
install_data(
  'rc', 'rc.shutdown',
  install_dir: get_option('sysconfdir'),
  install_mode: ['rwxr-xr-x', 0, 0]
)
install_data(
  'rc.subr',
  install_dir: get_option('sysconfdir'),
  install_mode: ['rw-r--r--', 0, 0]
)
install_data(
  'stage2stopsys',
  install_dir: get_option('libexecdir'),
  install_mode: ['rwxr-xr-x', 0, 0]
)
