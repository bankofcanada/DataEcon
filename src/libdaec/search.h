#ifndef __SEARCH_H__
#define __SEARCH_H__

#include "file.h"
#include "object.h"

/* ========================================================================= */
/* API */
typedef struct
{
    sqlite3_stmt *stmt; /* we prepare and finalize the statement*/
} search_t;
typedef search_t *de_search;

int de_list_catalog(de_file de, obj_id_t pid, de_search *search);
int de_search_catalog(de_file de, obj_id_t pid, const char *wc,
                      type_t type, class_t cls, de_search *search);

int de_next_object(de_search search, object_t *object);
int de_finalize_search(de_search search);

/* ========================================================================= */
/* internal */

#endif