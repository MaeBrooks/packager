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
  pkg_assert(pkg_require(&pkg));

  // Package Emacs
  // make -j8 configure="--prefix=/opt/emacs --with-tree-sitter --with-ns CFLAGS='-O0 -g3'"

  /*
    #define OPT "/home/foo/opt"
    Pkg pkg = {0};
    pkg_define(&pkg, "Raylib");

    pkg_check(&pkg, "test", "-d", OPT "/raylib");

    pkg_check(&pkg, "!", "test", "-d", "/raylib");
    pkg_step(&pkg, "git", "clone", "https://github.com/foobar/raylib.git");

    pkg_step()

    pkg_require_and_reset(&pkg);
    ...
  */


  // TODO: Group them into kinds
  // TODO: Break each down into steps
  // TODO: figure out how to define this!
  // TODO: Can I group the packages dependencies into a dependency tree? and if so, how can i group those together?
  // TODO: also, how can i handle paths? should i put everything into a ./opt/ folder and let the user deal with paths?
  // TODO: what if the packages require the path for an item to be available? can i alter the path env in the program
  //       execute it that way?
  // TODO: How do i check if something is already installed?

  return 0;
}
