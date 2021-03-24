Current version
======
* Added Dynarec for ARM64
* Many general CPU opcode added to the Dynarec
* Many SSEx opcodes added to the Dynarec
* Added a few more symbols in libc
* A few changes to the "thread once" handling
* New Logo and Icon from @grayduck

v0.0.4
======
* Added a lot of opcodes
* Added SDL1 and SDL2 wrapping
* OpenGL/GLU wrapping done
* libasound / libpulse wrapping done
* libopenal / libalure / libalut wrapping done
* Many X11 libs wrapped
* Added "install" with binfmt integration
* A few games are actualy starting now (old WorldOfGoo, Into The Breach, VVVVVV and Thimbeweed Park)

v0.0.2
======
* Using box86 as a base, implemented most planned box64 part except dynarec
* Wrapped a few libs
* ELF loader a bit crude, but seems working
* Signal handling implemented
* MMX and x87 are sharing their registers
