init used on my ULINUX3 system. use it if you want, but know that it sucks.

scripts directory provides very stripped down implementations (more like inspirations) of the BSD RC framework for service management, and 3 example rc.d scripts are provided to give an idea of how to write them.

### A NOTE
This will fail to build using a pure ISO C compiler, unless \_GNU\_SOURCE is defined (on linux) or \_POSIX\_C\_SOURCE is defined with a value greater than or equal to 200809L (on other systems). I'll deal with the portability problems of using pipe2 and vdprintf later.

## DEPENDENCIES
- A standard Unix C development toolkit
- Meson >= 1.3.0 (build-time)
- Ninja (build-time)
- Perl (only if you're generating the manpages)
