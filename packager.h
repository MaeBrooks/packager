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
  char* dir;
  Cmd cmd;
  Cmd skip_cmd;
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
  PkgDeps deps;
  PkgSteps steps;
} Pkg;

#define pkg_assert(expr) do { \
  PkgResult e = expr; \
  if (e != Success) { \
  nob_log(ERROR, "%s:%d %s\n", __FILE__, __LINE__, pkg_result_msg(e)); \
  } \
  } while (0)

//  ___       __ _      _ _   _
// |   \ ___ / _(_)_ _ (_) |_(_)___ _ _
// | |) / -_)  _| | ' \| |  _| / _ \ ' \
// |___/\___|_| |_|_||_|_|\__|_\___/_||_|

#ifndef PACKAGER_IMPLEMENTATION
  typedef PkgResult int;
  void SET_PKGS_ROOT(const char* name);
  // Package
  void pkg_define(Pkg* pkg, char* name, char* relpath);
  void pkg_reset(Pkg* pkg);
  PkgResult pkg_require(Pkg* pkg);
  PkgResult pkg_require_reset(Pkg* pkg);
  PkgResult pkg_require_deps(PkgDeps* deps);
  PkgResult pkg_run_steps(Pkg* pkg);
  // Step
  void pkg_step(Pkg* pkg);
  void step_do(Pkg* pkg, ...);
  void step_ignore(Pkg* pkg, ...);
  void step_into(Pkg* pkg, char* dir);
  // Memory Management
  void step_free(PkgStep* step);
  void steps_free(PkgSteps* steps);
  const char* pkg_result_msg(PkgResult e);
#else

//  ___            _                   _        _   _
// |_ _|_ __  _ __| |___ _ __  ___ _ _| |_ __ _| |_(_)___ _ _
//  | || '  \| '_ \ / -_) '  \/ -_) ' \  _/ _` |  _| / _ \ ' \
// |___|_|_|_| .__/_\___|_|_|_\___|_||_\__\__,_|\__|_\___/_||_|
//           |_|

#include <stdio.h>
#include "strings.h"
#include <sys/stat.h>

typedef enum {
  Success,
  Failure,
  Skipped,
} PkgResult;

static char* SUCCESS_RESULT_MSG = "SUCCESS";
static char* SKIPPED_RESULT_MSG = "SKIPPED";
static char* FAILURE_RESULT_MSG = "FAILURE";
static char* UNKNOWN_RESULT_MSG = "UNKNOWN";

#define _is_failure(e) (e != Success)
#define _unwrap_result(action) do { \
  e = (action); \
  if (_is_failure(e)) return e; \
  } while (0)

static char* pkgs_root = NULL;
PkgResult pkg_require(Pkg* pkg);
void pkg_reset(Pkg* pkg);

void step_free(PkgStep* step) {
  assert(step);

  step->cmd.count = 0;
  step->skip_cmd.count = 0;
}

void steps_free(PkgSteps* steps) {
  assert(steps);

  for (uint i = 0; i < steps->count; ++i) {
    step_free(steps->items[i]);
  }

  steps->count = 0;
}

void deps_free(PkgDeps* deps) {
  for (uint i = 0; i < deps->count; ++i) {
    pkg_reset((Pkg*)deps->items[i]);
  }

  deps->count = 0;
}

void pkg_reset(Pkg* pkg) {
  assert(pkg);

  free(pkg->path);
  steps_free(&pkg->steps);
  deps_free(&pkg->deps);
}

PkgResult pkg_require_deps(PkgDeps* deps) {
  assert(deps);
  PkgResult e = Success;

  // TODO: build dep tree!
  // TODO: check if installed ?
  // check that deps are required already, if not require them
  for (uint i = 0; i < deps->count; ++i) {
    // checking if its already installed is for silly heads, silly! :3
    _unwrap_result(pkg_require(&((Pkg*)deps->items)[i]));
  }

  return e;
}

PkgResult pkg_require_reset(Pkg* pkg) {
  PkgResult e = pkg_require(pkg);
  pkg_reset(pkg);
  return e;
}

PkgResult pkg_run_steps(Pkg* pkg) {
  assert(pkg);

  uint i = 0;
  PkgResult e = Success;
  char* current_dir;

  // TODO: pkgs should be installed in a INSTALL_DIR/<pkg-name>
  // TODO: pkgs may need to be added to PATHS and stuff, we should also handle that?
 loop_start:
  if (i >= pkg->steps.count) goto exit;

  PkgStep* step = pkg->steps.items[i];

  if (step->dir) {
    current_dir = (char*)nob_get_current_dir_temp();
    nob_set_current_dir(step->dir);
  }

  if (nob_cmd_run_sync(step->skip_cmd)) {
    e = Skipped;
    goto loop_end;
  }

  if (!nob_cmd_run_sync(step->cmd)) {
    e = Failure;
    goto loop_end;
  }

 loop_end:
  if (step->dir) nob_set_current_dir(current_dir);

  if (e != Failure) {
    e = Success;
    ++i;
    goto loop_start;
  }

 exit:
  return e;
}

PkgResult pkg_require(Pkg* pkg) {
  assert(pkg);
  nob_log(INFO, "Building package: [%s]", pkg->name);

  PkgResult e = Success;
  _unwrap_result(pkg_require_deps(&pkg->deps));
  _unwrap_result(pkg_run_steps(pkg));

  nob_log(INFO, "[%s]: Required!\n", pkg->name);

  return e;
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

void pkg_step(Pkg* pkg) {
  assert(pkg);

  nob_log(INFO, "Package step: %ld", pkg->steps.count);
  da_append(&pkg->steps, (PkgStep*)calloc(1, sizeof(PkgStep)));
}

#define step_do(pkg, ...) \
    _step_do(pkg, \
  ((const char*[]){__VA_ARGS__}), \
  (sizeof((const char*[]){__VA_ARGS__})/sizeof(const char*)))
static void _step_do(Pkg* pkg, const char* args[], unsigned int args_size) {
  assert(pkg);

  da_append_many(&da_last(&pkg->steps)->cmd, args, args_size);
}

#define step_ignore(pkg, ...) \
    _step_ignore(pkg, \
  ((const char*[]){__VA_ARGS__}), \
  (sizeof((const char*[]){__VA_ARGS__})/sizeof(const char*)))
static void _step_ignore(Pkg* pkg, const char* args[], unsigned int args_size) {
  assert(pkg);

  da_append_many(&da_last(&pkg->steps)->skip_cmd, args, args_size);
}

void step_into(Pkg* pkg, char* dir) {
  assert(pkg);
  assert(dir);

  PkgStep* step = da_last(&pkg->steps);
  step->dir = dir;
}

const char* pkg_result_msg(PkgResult e) {
  switch (e) {
  case Success: return SUCCESS_RESULT_MSG;
  case Skipped: return SKIPPED_RESULT_MSG;
  case Failure: return FAILURE_RESULT_MSG;
  default:
    return UNKNOWN_RESULT_MSG;
  }
}
#endif // PACKAGER_IMPLEMENTATION

#undef _unwrap_result
#undef _is_failure

#endif // PACKENGER_H
