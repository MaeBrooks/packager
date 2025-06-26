#ifndef PACKENGER_H
#define PACKENGER_H

// TODO: This only needs to be here for the Cmd type
// otherwise it should be in the implementation portion?
// Not quite sure?
#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#include "nob.h"

// PACKAGER_IMPLEMENTATION

typedef struct {
  char* name;
  Cmd skip_cmd;
  Cmd cmd;
} PkgStep;

typedef struct {
  PkgStep** items;
  size_t count;
  size_t capacity;
} PkgSteps;

struct Pkg;

typedef struct {
  struct Pkg** items;
  size_t count;
  size_t capacity;
} PkgDeps;

typedef struct {
  char* name;
  char* path;
  bool stopped;
  PkgDeps deps;
  PkgSteps steps;
} Pkg;

//  ___            _                   _        _   _
// |_ _|_ __  _ __| |___ _ __  ___ _ _| |_ __ _| |_(_)___ _ _
//  | || '  \| '_ \ / -_) '  \/ -_) ' \  _/ _` |  _| / _ \ ' \
// |___|_|_|_| .__/_\___|_|_|_\___|_||_\__\__,_|\__|_\___/_||_|
//           |_|
#ifndef PACKAGER_IMPLEMENTATION
  void pkg_define(Pkg* pkg, char* name, char* relpath);
  void SET_PKGS_ROOT(const char* name);
  PkgStep* pkg_step(Pkg* pkg, const char* name, ...);
  void step_skip_when(PkgStep* step, ...);
  bool pkg_require(Pkg* pkg);
  bool pkg_require_deps(PkgDeps* deps);
  bool pkg_run_steps(Pkg* pkg);
  void step_free(PkgStep* step);
  void steps_free(PkgSteps* steps);
#else
#include <stdio.h>
#include "strings.h"
#include <sys/stat.h>

  static char* pkgs_root = NULL;

  bool pkg_require(Pkg* pkg);
  bool pkg_require_deps(PkgDeps* deps) {
    assert(deps);

    // TODO: build dep tree!
    // TODO: check if installed
    // check that deps are required already, if not require them
    for (uint i = 0; i < deps->count; ++i) {
      // checking if its already installed is for silly heads, silly! :3
      if (!pkg_require(&((Pkg*)deps->items)[i])) return false;
    }

    nob_da_free(*deps);

    return true;
  }

  void step_free(PkgStep* step) {
    assert(step);

    nob_da_free(step->cmd);
    nob_da_free(step->skip_cmd);
  }

  void steps_free(PkgSteps* steps) {
    assert(steps);

    nob_da_free(*steps);
  }

  bool pkg_run_steps(Pkg* pkg) {
    assert(pkg);

    for (uint i = 0; i < pkg->steps.count; ++i) {
      PkgStep* step = pkg->steps.items[i];
      bool success = nob_cmd_run_sync(step->skip_cmd);

      if (success) goto free;
      if (step->name) nob_log(INFO, "[STEP] %s\n", step->name);

      success = nob_cmd_run_sync(step->cmd);

    free:
      step_free(pkg->steps.items[i]);

      if (!success) return false;
    }

    return true;
  }

  bool pkg_require(Pkg* pkg) {
    assert(pkg);

    nob_log(INFO, "Building package: [%s]\n", pkg->name);

    if (!pkg_require_deps(&pkg->deps)) return false;
    if (!pkg_run_steps(pkg)) return false;

    nob_da_free(pkg->steps);
    free(pkg->path);
    fprintf(stderr, "%c", '\n');

    return true;
  }

  void pkg_define(Pkg* pkg, const char* name, const char* relpath) {
    assert(pkg);
    assert(relpath);
    assert(pkgs_root);

    unsigned long size = strlen(pkgs_root) + strlen(relpath) + 1;
    char* path = malloc(size);

    snprintf(path, size, "%s%s", pkgs_root, relpath);

    pkg->name = (char*)name;
    pkg->path = (char*)path;
  }

  static bool direxists(const char* path) {
    struct stat st;
    bool err = stat(pkgs_root, &st) == -1;
    if (!err || S_ISDIR(st.st_mode) != 0) {
      return false;
    }

    return true;
  }

  void SET_PKGS_ROOT(const char* root) {
    assert(root);

    if (!direxists(root)) {
      nob_log(ERROR, "Unable to find directory: %s\n", root);
      abort();
    }

    pkgs_root = (char*)root;
  }

  static PkgStep* _pkg_step(Pkg* pkg, const char* name, const char* args[], unsigned int args_size) {
    assert(pkg);

    PkgStep* step = calloc(1, sizeof(PkgStep));

    step->name = (char*)name;
    da_append_many(&step->cmd, args, args_size);

    da_append(&pkg->steps, step);

    return step;
  }

  static void _step_skip_when(PkgStep* step, const char* args[], unsigned int args_size) {
    assert(step);
    assert(step->skip_cmd.count == 0);

    da_append_many(&step->skip_cmd, args, args_size);
  }

#define pkg_step(pkg, name, ...) \
  _pkg_step(pkg, name, \
  ((const char*[]){__VA_ARGS__}), \
  (sizeof((const char*[]){__VA_ARGS__})/sizeof(const char*)))

#define step_skip_when(step, ...) \
  _step_skip_when(step, \
  ((const char*[]){__VA_ARGS__}), \
  (sizeof((const char*[]){__VA_ARGS__})/sizeof(const char*)))

#endif // PACKAGER_IMPLEMENTATION

#endif // PACKENGER_H
