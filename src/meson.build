# Meson build file for everything written in C.

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
