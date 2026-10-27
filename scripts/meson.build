# Meson build file for all the scripts.

# SPDX-License-Identifier: AGPL-3.0-or-later
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
  'sysinit',
  install_dir: get_option('sysconfdir') / 'rc.d',
  install_mode: ['rwxr-xr-x', 0, 0]
)
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
