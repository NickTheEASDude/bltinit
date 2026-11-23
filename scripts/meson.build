# Meson build file for all the scripts.

# SPDX-License-Identifier: AGPL-3.0-or-later
# Copyright (C) 2026 BLT Sandwich

# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Affero General Public License for more details.

# You should have received a copy of the GNU Affero General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

script_config = configuration_data()
script_config.set('sysconfdir', get_option('sysconfdir'))

links = ['reboot', 'halt', 'poweroff']
foreach link: links
  install_symlink(
    link,
    pointing_to: 'stopsys',
    install_dir: get_option('sbindir')
  )
endforeach
if get_option('install_rc_framework')
  install_data(
    'rcorder', 'stopsys',
    install_dir: get_option('sbindir'),
    install_mode: ['rwxr-xr-x', 0, 0]
  )
  datafiles = {}
  foreach rc_gen: ['rc', 'rc.shutdown', 'rc.subr', 'sysinit']
    datafiles = datafiles + {
      rc_gen: configure_file(
        input: rc_gen + '.in',
        output: rc_gen,
        configuration: script_config
      )
    }
  endforeach
  install_data(
    datafiles['rc'], datafiles['rc.shutdown'],
    install_dir: get_option('sysconfdir'),
    install_mode: ['rwxr-xr-x', 0, 0]
  )
  install_data(
    datafiles['rc.subr'],
    install_dir: get_option('sysconfdir'),
    install_mode: ['rw-r--r--', 0, 0]
  )
  if get_option('install_sysinit')
    install_data(
     datafiles['sysinit'],
      install_dir: get_option('sysconfdir') / 'rc.d',
      install_mode: ['rwxr-xr-x', 0, 0]
    )
  endif
else
  warning('Not installing rc framework. Please remember to provide rc and rc.shutdown in ' + get_option('sysconfdir'))
endif
install_data(
  'stage2stopsys',
  install_dir: get_option('libexecdir'),
  install_mode: ['rwxr-xr-x', 0, 0]
)
install_data(
  'rc.fallback',
  install_dir: get_option('datadir') / 'rc',
  install_mode: ['rw-r--r--', 0, 0]
)
