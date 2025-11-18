#ifndef PATH_H
#define PATH_H

#include <limits.h>

/* Resolve a command name to an absolute path by searching $PATH.
 * - If cmd contains '/', treat it as explicit and return it as-is (after basic checks).
 * Returns 0 on success (path written to out), -1 if not found or not executable.
 */
int resolve_command_path(const char *cmd, char out[PATH_MAX]);

#endif
