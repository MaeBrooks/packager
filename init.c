#define PACKAGER_IMPLEMENTATION
#include "packager.h"

#define OPT "/Users/zoft/opt"

int main(int argc, char** argv) {
  // NOB_GO_REBUILD_URSELF(argc, argv);
  SET_PKGS_ROOT(OPT);

  Pkg pkg = {0};
  pkg_define(&pkg, "Raylib", "raylib");
  // pkg_check(&pkg, CHECK_QUIT, "!", "-d", "raylib");

  // TODO: add dep of "git"
  PkgStep* step = pkg_step(&pkg, "Getting Raylib", "git", "clone", "https://github.com/raysan5/raylib.git");

  step_skip_when(step, "test", "-d", "raylib");
  assert(pkg_require(&pkg) == true);
  // pkg_check(&pkg, "test", "-d", OPT "/raylib")

  // Package Raylib
  // Package Font
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
