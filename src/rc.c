#include "rc.h"
#include <unistd.h>
#include <sys/wait.h>
#include <err.h>
#include <errno.h>

void startServices() {
retry:
	pid_t services = fork();
	if (services < 0) {
		warnx("Fork failed, retrying");
		goto retry;
	} else if (services > 0) {
		pid_t reapedServices;
		while ((reapedServices = waitpid(-1, NULL, 0)) != services) {
			if (reapedServices < 0) {
				if (errno == ECHILD)
					break;
				continue;
			}
		}
	} else {
		execl("/etc/rc", "/etc/rc", (char *) NULL);
		_exit(1);
	}
}

void stopServices() {
retry:
	pid_t services = fork();
	if (services < 0) {
		warnx("Fork failed, retrying");
		goto retry;
	} else if (services > 0) {
		pid_t reapedServices;
		while ((reapedServices = waitpid(-1, NULL, 0)) != services) {
			if (reapedServices < 0) {
				if (errno == ECHILD)
					break;
				continue;
			}
		}
	} else {
		execl("/etc/rc.shutdown", "/etc/rc.shutdown", (char *) NULL);
		_exit(1);
	}
}
