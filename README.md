init used on my ULINUX3 system. use it if you want, but know that it sucks ~~(and also relies on the busybox getty to work right, so just know that)~~ THIS BIT IS NO LONGER TRUE!

scripts directory provides very stripped down implementations (more like inspirations) of the BSD RC framework for service management, and 3 example rc.d scripts are provided to give an idea of how to write them. if you want to replace them with the actual bsd implementations and just use actual bsd rc.d scripts, you are free to do so (there's an exception clause in COPYING explicitly allowing this if you also decide to fork this project).
NOTE: You are absolutely not allowed to strip the gplv3 licensing off of the existing implementations in scripts/ and re-release them as bsd.
