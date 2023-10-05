#ifndef __FILE_H__
#define __FILE_H__

#include <stdbool.h>

#include <sqlite3.h>

/* ========================================================================= */
/* API */

struct de_file_s;
typedef struct de_file_s de_file_t;
typedef de_file_t *de_file;

/* open daec file in read-write mode (if write-protected it might either get opened in read-only mode or fail)*/
int de_open(const char *fname, de_file *de);

/* open daec file in read-only mode */
int de_open_readonly(const char *fname, de_file *de);

/* open a daec database in memory */
int de_open_memory(de_file *pde);

/* close a previously opened daec file */
int de_close(de_file de);

/* delete everything in the given daec file */
int de_truncate(de_file de);

/* ========================================================================= */
/* internal */

/* prepared statements */
typedef enum stmt_name
{
    stmt_new_object = 0,
    stmt_new_object_info,
    stmt_store_scalar,
    stmt_store_tseries,
    stmt_store_mvtseries,
    stmt_new_axis,
    stmt_find_object,
    stmt_find_fullpath,
    stmt_find_axis,
    stmt_load_object,
    stmt_load_scalar,
    stmt_load_tseries,
    stmt_load_mvtseries,
    stmt_load_axis,
    stmt_delete_object,
    stmt_set_attribute,
    stmt_get_attribute,
    stmt_get_all_attributes,
    stmt_get_object_info,
    stmt_count_objects,
    stmt_size,             /* sentinel, gives us the number of statements */
    stmt_last = stmt_size, /* alias, for readability */
} stmt_name_t;

struct de_file_s
{
    sqlite3 *db;
    sqlite3_stmt *stmt[stmt_size];
    bool transaction;
};

/* called when creating a new de_file. creates tables and indexes */
int _init_file(de_file de);

/* return a static buffer containing the SQL text for the given stmt_name */
const char *_get_statement_sql(stmt_name_t stmt_name);

/* return a prepared statement by the given name */
sqlite3_stmt *_get_statement(de_file de, stmt_name_t stmt_name);

/* functions that start and post transactions */
int de_commit(de_file de);
int de_begin_transaction(de_file de);

#endif
