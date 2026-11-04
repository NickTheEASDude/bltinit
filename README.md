init used on my ULINUX3 system. use it if you want, but know that it sucks.

scripts directory provides very stripped down implementations (more like inspirations) of the BSD RC framework for service management, and 3 example rc.d scripts are provided to give an idea of how to write them.

### A NOTE
This code will not compile if using a pure ISO C compiler. You can try to fiddle with feature test macros, but it's not worth the headache, just use a compiler that can recognize POSIX functions.

## DEPENDENCIES
- A standard Unix C development toolkit, with a C99-compliant compiler
- Meson >= 1.3.0 (build-time)
- Ninja (build-time)
- Perl (only if you're generating the manpages)
