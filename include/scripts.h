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
# define RC_FALL "/usr/lib/rc/rc.fallback"
#endif

#ifndef S2_STOPSYS
# define S2_STOPSYS "/usr/libexec/stage2stopsys"
#endif

#endif // INIT_SCRIPTS_H

