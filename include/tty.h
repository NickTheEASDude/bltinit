#ifndef INIT_TTY_H
#define INIT_TTY_H
#include <sys/types.h>
#include <stdbool.h>
#define MAX_CONSOLES 32

typedef enum { SPAWN_ONCE, SPAWN_REPEAT } smode_t;
typedef struct {
        char      device[64];
        char      baud[16];
        smode_t   mode;
        char      prog[256];
        pid_t     pid;
} console_t;
extern console_t consoles[MAX_CONSOLES];
extern int nConsoles;
int loadConsoles(const char *path);
bool execGetty(console_t *c);
void spawnConsoles(void);
void consoleExit(pid_t reapPID);
void broadcast(const char *restrict format, ...);
#endif
