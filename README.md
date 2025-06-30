# Packager! The nob inspired package manager

In other words, this is a glorified Makefile written in C

Simply add the `packager.h` and `nob.h` files to your project and add this to your c file

```c
#define PACKAGER_IMPLEMENTATION
#include "packager.h"
```

See `example.c` for an example of building raylib and emacs

## TODOS

- [ ] package root should be used as the root directory for where files are installed
- [ ] package deps should be easily define-able
- [ ] utility step functions:
  - [ ] step_clone (git clone + ignore)
  - [ ] pkg_define_clone (git clone + define + ignore)
  - [ ] pkg_step_when (pkg_step + step_ignore)
