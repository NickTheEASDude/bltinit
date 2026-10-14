# Meson build file for generating the manpages.

# Copyright (C) 2026 BLT Sandwich

# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

version = meson.project_version()
date = run_command('date', '+%Y-%m-%d', check: true).stdout().strip()
man_pages = [
  ['init.pod', 8],
  ['rc.pod', 8], 
  ['rundaemon.pod', 8],
  ['rcorder.pod', 8],
  ['stopsys.pod', 8],
  ['rc.subr.pod', 8],
  ['rc.conf.pod', 5],
  ['rc.consoles.pod', 5]
]
man_centers = [
  'General Commands Manual',
  'System Calls Manual',
  'Library Functions Manual',
  'Device Drivers Manual',
  'File Formats Manual',
  'Games Manual',
  'Miscellaneous Information Manual',
  'System Manager\'s Manual'
]
rc8_symlinks = ['rc.d.8', 'rc.shutdown.8']
stopsys8_symlinks = ['halt.8', 'poweroff.8', 'reboot.8']

foreach page: man_pages
  page_source = page[0]
  page_num = page[1]
  custom_target(f'manpage_@page_source@',
    input: page_source,
    output: page_source.replace('.pod', f'.@page_num@'),
    command: [
      pod2man,
      f'--section=@page_num@',
      '--center=@0@'.format(man_centers[page_num - 1]),
      f'--release=init @version@',
      f'--date=@date@',
      '@INPUT@',
      '@OUTPUT@'
    ],
    install: true,
    install_dir: get_option('mandir') / f'man@page_num@'
  )
endforeach

foreach link: rc8_symlinks
  install_symlink(
    link,
    pointing_to: 'rc.8',
    install_dir: get_option('mandir') / 'man8'
  )
endforeach

foreach link: stopsys8_symlinks
  install_symlink(
    link,
    pointing_to: 'stopsys.8',
    install_dir: get_option('mandir') / 'man8'
  )
endforeach
