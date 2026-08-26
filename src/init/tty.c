// Logic for console spawning and message broadcasting.

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

// NOTE: AI was used in the writing of this file and its corresponding header file.
// However, the author can assure you that this code has been thoroughly studied and tested by the author,
// and has proven to be fully functional without major bugs.
// Do note that further testing is encouraged for anybody who uses this program.

#include "tty.h"
#include <signal.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/ioctl.h>

console_t consoles[MAX_CONSOLES];
int nConsoles = 0;

void broadcast(const char *restrict format, ...) {
	va_list args;
	va_start(args, format);
	vprintf(format, args);
	va_end(args);
}

static void trimLF(char *s) {
	size_t n = strlen(s);
	if (n && s[n - 1] == '\n') s[n - 1] = '\0';
}
void addConsole(console_t console) {
	consoles[nConsoles] = console;
	nConsoles++;
}

int loadConsoles(const char *path) {
	FILE *f = fopen(path, "r");
	if (!f) return -1;

	char line[1024];
	int n = 0;

	while (fgets(line, sizeof(line), f) && n < MAX_CONSOLES) {
		trimLF(line);
		char *p = line;
		while (isspace((unsigned char)*p)) p++;
		if (*p == '\0' || *p == '#') continue;

		char device[64], isGetty[8], modeStr[8], prog[512];

		int matched = sscanf(p, "%63[^;];%7[^;];%7[^;];%511[^\n]",
			device, isGetty, modeStr, prog);
		if (matched != 4) {
			broadcast("rc.consoles: malformed line %s, skipping\n", p);
			continue;
		}

		console_t *c = &consoles[n];
		memset(c, 0, sizeof(*c));
		snprintf(c->device, sizeof(c->device), "%s", device);
		if (
			isGetty[0] == 'X' ||
			isGetty[0] == 'x' ||
			isGetty[0] == 'G' ||
			isGetty[0] == 'g'
		) snprintf(c->isGetty, sizeof(c->isGetty), "%s", isGetty);
		else {
			broadcast("rc.consoles: line %s column 2 incorrect, skipping\n", p);
			continue;
		}
		c->mode = (modeStr[0] == 'R' || modeStr[0] == 'r')
			    ? SPAWN_REPEAT :
			    (modeStr[0] == 'S' || modeStr[0] == 's')
			    ? SPAWN_ONCE :
			    (modeStr[0] == 'A' || modeStr[0] == 'a')
			    ? SPAWN_ASK : SPAWN_ERR;
		if (c->mode == SPAWN_ERR) {
			broadcast("rc.consoles: line %s column 3 incorrect, skipping\n", p);
			continue;
		}
		snprintf(c->prog, sizeof(c->prog), "%s", prog);
		c->pid = -1;
		n++;
	}

	fclose(f);
	nConsoles = n;
	return n;
}

bool execGetty(console_t *c) {
	int attempts = 0;
	pid_t childPID;
	for (;;) {
		childPID = fork();
		if (childPID >= 0)
			break;
		attempts++;
		if (attempts >= 5) {
			broadcast("getty fork failed after 5 attempts, giving up.\n");
			return false;
		}
		sleep(1);
	}
	if (childPID > 0) {
		c->pid = childPID;
		return true;
	} else {
		setsid();
		char prog[513];
		strncpy(prog, c->prog, 512);
		char *args[513];
		int count = 0;
		char *tokPtr;
		for (char *tok = strtok_r(prog, " ", &tokPtr); tok != NULL; tok = strtok_r(NULL, " ", &tokPtr)) args[count++] = tok;
		args[count] = NULL;
		if (strcmp(c->isGetty, "G") == 0 || strcmp(c->isGetty, "g") == 0) {
			execv(args[0], args);
			perror("init: console execv failed");
			_exit(1);
		}
		int fd = open(c->device, O_RDWR | O_NOCTTY);
		if (fd < 0) {
			perror("init: console open failed");
			_exit(1);
		}
		if (ioctl(fd, TIOCSCTTY, 0) < 0) {
			perror("init: console takeover failed");
			_exit(1);
		}
		dup2(fd, STDIN_FILENO);
		dup2(fd, STDOUT_FILENO);
		dup2(fd, STDERR_FILENO);
		if (fd > STDERR_FILENO) close(fd);
		if (c->mode == SPAWN_ASK) {
			write(STDOUT_FILENO, "Press enter to continue... ", 27);
			int userIn = getchar();
			while (userIn != '\n') {
				if (userIn < 0) {
					broadcast("\nEnd of file reached, continuing\n");
					break;
				}
				userIn = getchar();
			}
		}
		execv(args[0], args);
		perror("init: console execv failed");
		_exit(1);
	}
}

void spawnConsoles(void) {
	for (int i = 0; i < nConsoles; i++)
		execGetty(&consoles[i]);
}

void consoleExit(pid_t reapPID) {
	for (int i = 0; i < nConsoles; i++) {
		if (consoles[i].pid != reapPID) continue;
		consoles[i].pid = -1;
		if (consoles[i].mode == SPAWN_REPEAT || consoles[i].mode == SPAWN_ASK)
			execGetty(&consoles[i]);
		return;
	}
}
