#ifndef __MISC_H__
#define __MISC_H__

#include <stdbool.h>
#include <stdint.h>

/* ========================================================================= */
/* API */

/* ========================================================================= */
/* internal */

/* check if the given path is a file that can be opened for reading */
bool _isfile(const char *path);

/* make a string "id=N" given integer N */
const char *_id2str(int64_t id);

/* make a string "pid=N,name='abc'" given integer N */
const char *_pidnm2str(int64_t pid, const char *name);

#endif
