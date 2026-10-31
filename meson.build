# Root Meson build file, checks for specific programs and sets up config.h.

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

project(
  'init', 'c',
  version: '2.0.5', meson_version: '>=1.3.0',
  default_options: ['sysconfdir=/etc']
)
cc = meson.get_compiler('c')
add_project_arguments('-DHAVE_CONFIG_H', language: ['c', 'cpp'])
supported = ['linux', 'freebsd', 'netbsd', 'dragonfly']
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
  get_option('prefix') / get_option('datadir') / 'rc/rc.fallback'
)

cc.has_function('pipe2', required: true)
cc.has_function('dprintf', required: true)

if get_option('manpages')
  pod2man = find_program('pod2man', required: true)
  subdir('man')
endif

subdir('src')
subdir('scripts')
