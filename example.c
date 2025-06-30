#define PACKAGER_IMPLEMENTATION
#include "packager.h"

#define OPT "/Users/zoft/opt"

int main(int argc, char** argv) {
  Pkg pkg = {0};

  SET_PKGS_ROOT(OPT);

  pkg_define(&pkg, "Raylib", "raylib");
  pkg_step(&pkg);
    step_ignore(&pkg, "test", "-d", "raylib");
    step_do(&pkg, "git", "clone", "https://github.com/raysan5/raylib.git");
  pkg_step(&pkg);
    step_into(&pkg, "raylib/src");
    step_ignore(&pkg, "test", "-f", "libraylib.a");
    step_do(&pkg, "make", "PLATFORM=PLATFORM_DESKTOP");
  pkg_assert(pkg_require_reset(&pkg));

  pkg_define(&pkg, "Emacs", "emacs");
  pkg_step(&pkg);
    step_ignore(&pkg, "test", "-d", "emacs");
    step_do(&pkg, "git", "clone", "https://github.com/emacs-mirror/emacs.git");
  pkg_step(&pkg);
    step_into(&pkg, "emacs");
    step_ignore(&pkg, "test", "-f", "src/emacs");
    step_do(&pkg, "make", "-j8",
          "configure=\"--prefix=/opt/emacs --with-tree-sitter --with-ns CFLAGS='-O0 -g3'\"");

  pkg_assert(pkg_require_reset(&pkg));

  return 0;
}
