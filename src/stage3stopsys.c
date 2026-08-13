// Calls reboot(2) based on argv input (or poweroff if none is given).

// Copyright (C) 2026 BLT Sandwich

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#include <sys/reboot.h>
#include <linux/reboot.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char *argv[]) {
	if (getpid() != 1) {
		write(STDERR_FILENO, "This program must be run as a continuation of init.\n", 52);
		return 1;
	}
	reboot(LINUX_REBOOT_CMD_CAD_ON);
	char *stopType = NULL;
	if (argc > 1)
		stopType = argv[1];
	else
		stopType = "shutdown";

	if (strcmp(stopType, "reboot") == 0) {
		write(STDOUT_FILENO, "Now rebooting system\n", 21);
		reboot(LINUX_REBOOT_CMD_RESTART);
	} else if (strcmp(stopType, "halt") == 0) {
		write(STDOUT_FILENO, "Now halting system\n", 19);
		reboot(LINUX_REBOOT_CMD_HALT);
	} else if (strcmp(stopType, "shutdown") == 0) {
		write(STDOUT_FILENO, "Now shutting down system\n", 25);
		reboot(LINUX_REBOOT_CMD_POWER_OFF);
	}
	write(STDERR_FILENO, "Either the reboot/halt/poweroff call failed,\n", 45);
	write(STDERR_FILENO, "or the option wasn't specified correctly.\n", 42);
	write(STDERR_FILENO, "Either way, going into infinite loop. Treat as a halt.\n", 55);
	reboot(LINUX_REBOOT_CMD_HALT);
	for(;;)
		pause();
	return 1;
}
