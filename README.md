init used on my ULINUX3 system. use it if you want, but know that it sucks.

scripts directory provides very stripped down implementations (more like inspirations) of the BSD RC framework for service management, and 3 example rc.d scripts are provided to give an idea of how to write them.

### NOTES
This code will not compile if using a pure ISO C compiler. You can try to fiddle with feature test macros, but it's not worth the headache, just use a compiler that can recognize POSIX functions.

While the init binary itself is portable between Linux and the BSDs, the RC framework is very tightly coupled to Linux due to the use of cgroups. However, on the BSDs, it should be expected to only install the actual init binary (there is a meson option to allow this) and let it use the already existing RC framework (of course, that's not really an option on Linux, which is why I wrote this framework in the first place).

## DEPENDENCIES
- A standard Unix C development toolkit, with a C99-compliant compiler
- Meson >= 1.3.0 (build-time)
- Ninja (build-time)
- Perl (only if you're generating the manpages)
