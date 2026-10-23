// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 BLT Sandwich

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.*

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#ifndef INIT_SCRIPTS_H
#define INIT_SCRIPTS_H

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifndef RC
# define RC "/etc/rc"
#endif

#ifndef RC_CONS
# define RC_CONS "/etc/rc.consoles"
#endif

#ifndef RC_SHUT
# define RC_SHUT "/etc/rc.shutdown"
#endif

#ifndef RC_FALL
# define RC_FALL "/usr/share/rc/rc.fallback"
#endif

#ifndef S2_STOPSYS
# define S2_STOPSYS "/usr/libexec/stage2stopsys"
#endif

#endif
