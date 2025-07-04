
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

#include <sqlite3.h>

#include "error.h"
#include "file.h"
#include "sql.h"
#include "misc.h"

/* https://www.cprogramming.com/tutorial/unicode.html */

#define RUN_SQL(de, sql)                                              \
    if (SQLITE_OK != sqlite3_exec((de)->db, (sql), NULL, NULL, NULL)) \
        return db_error(de);

int _init_file(de_file de)
{
    /* make tables */
    RUN_SQL(de,
            "CREATE TABLE `objects` ("
            "   `id` INTEGER PRIMARY KEY AUTOINCREMENT,"
            "   `pid` INTEGER NOT NULL,"
            "   `class` INTEGER NOT NULL,"
            "   `type` INTEGER NOT NULL,"
            "   `name` TEXT NOT NULL CHECK(LENGTH(`name`) > 0),"
            "   FOREIGN KEY (`pid`) REFERENCES `objects` (`id`) ON DELETE CASCADE,"
            "   UNIQUE (`pid`, `name` COLLATE BINARY) ON CONFLICT ROLLBACK"
            ") STRICT;"
            // "CREATE INDEX `obj_1` ON `objects`(`pid`, `name`, `type`, `class`);"
            // "CREATE INDEX `obj_2` ON `objects`(`name`, `type`, `pid`, `class`);"
            // "CREATE INDEX `obj_3` ON `objects`(`type`, `pid`, `name`, `class`);"
            // "CREATE INDEX `obj_4` ON `objects`(`class`, `pid`, `name`, `type`);"
    );
    RUN_SQL(de,
            "CREATE TABLE `objects_info` ("
            "   `id` INTEGER PRIMARY KEY,"
            "   `created` INTEGER NOT NULL,"
            "   `depth` INTEGER NOT NULL,"
            "   `fullpath` TEXT NOT NULL,"
            "   FOREIGN KEY (`id`) REFERENCES `objects` (`id`) ON DELETE CASCADE"
            ") STRICT;");
    RUN_SQL(de,
            "CREATE TABLE `attributes` ("
            "   `id` INTEGER NOT NULL,"
            "   `name` TEXT NOT NULL,"
            "   `value` TEXT,"
            "   PRIMARY KEY (`id`, `name`) ON CONFLICT REPLACE,"
            "   FOREIGN KEY (`id`) REFERENCES `objects` (`id`) ON DELETE CASCADE"
            ") STRICT, WITHOUT ROWID;");
    RUN_SQL(de,
            "CREATE TABLE `scalars` ("
            "   `id` INTEGER PRIMARY KEY,"
            "   `frequency` INTEGER NOT NULL,"
            "   `value` BLOB,"
            "   FOREIGN KEY (`id`) REFERENCES `objects` (`id`) ON DELETE CASCADE"
            ") STRICT;");
    RUN_SQL(de,
            "CREATE TABLE `axes`("
            "   `id` INTEGER PRIMARY KEY AUTOINCREMENT,"
            "   `ax_type` INTEGER NOT NULL,"
            "   `length` INTEGER NOT NULL CHECK(`length` >= 0),"
            "   `frequency` INTEGER NOT NULL,"
            "   `data` ANY"
            ") STRICT;");
    RUN_SQL(de,
            "CREATE INDEX `axes_1` ON `axes`(`ax_type`, `length`, `frequency`, `data`);");
    RUN_SQL(de,
            "CREATE TABLE `tseries` ("
            "   `id` INTEGER PRIMARY KEY,"
            "   `eltype` INTEGER NOT NULL,"
            "   `elfreq` INTEGER NOT NULL,"
            "   `axis_id` INTEGER NOT NULL,"
            "   `value` BLOB,"
            "   FOREIGN KEY (`id`) REFERENCES `objects` (`id`) ON DELETE CASCADE,"
            "   FOREIGN KEY (`axis_id`) REFERENCES `axes` (`id`) ON DELETE RESTRICT"
            ") STRICT;");
    RUN_SQL(de,
            "CREATE TABLE `mvtseries` ("
            "   `id` INTEGER PRIMARY KEY,"
            "   `eltype` INTEGER NOT NULL,"
            "   `elfreq` INTEGER NOT NULL,"
            "   `axis1_id` INTEGER NOT NULL,"
            "   `axis2_id` INTEGER NOT NULL,"
            "   `value` BLOB,"
            "   FOREIGN KEY (`id`) REFERENCES `objects` (`id`) ON DELETE CASCADE,"
            "   FOREIGN KEY (`axis1_id`) REFERENCES `axes` (`id`) ON DELETE RESTRICT,"
            "   FOREIGN KEY (`axis2_id`) REFERENCES `axes` (`id`) ON DELETE RESTRICT"
            ") STRICT;");
    RUN_SQL(de,
            "CREATE TABLE `ndaxes` ("
            "   `obj_id` INTEGER NOT NULL,"
            "   `axis_index` INTEGER NOT NULL,"
            "   `axis_id` INTEGER NOT NULL,"
            "   PRIMARY KEY (`obj_id`, `axis_index`) ON CONFLICT REPLACE,"
            "   FOREIGN KEY (`obj_id`) REFERENCES `objects` (`id`) ON DELETE CASCADE,"
            "   FOREIGN KEY (`axis_id`) REFERENCES `axes` (`id`) ON DELETE RESTRICT"
            ") STRICT, WITHOUT ROWID;"
            "");
    RUN_SQL(de,
            "CREATE TABLE `ndtseries` ("
            "   `id` INTEGER PRIMARY KEY,"
            "   `eltype` INTEGER NOT NULL,"
            "   `elfreq` INTEGER NOT NULL,"
            "   `value` BLOB,"
            "   FOREIGN KEY (`id`) REFERENCES `objects` (`id`) ON DELETE CASCADE"
            ") STRICT;");
    RUN_SQL(de,
            "INSERT INTO `objects` (`id`, `pid`, `class`, `type`, `name`)"
            "       VALUES (0, 0, 0, 0, '/');");
    RUN_SQL(de,
            "INSERT INTO `objects_info` (`id`, `created`, `depth`, `fullpath`)"
            "       VALUES (0, unixepoch('now'), 0, '');"
            "");
    RUN_SQL(de,
            "INSERT INTO `attributes` (`id`, `name`, `value`)"
            "       VALUES (0, 'DE_VERSION', '" DE_VERSION "');"
            "");

    return DE_SUCCESS;
}

const char *_get_statement_sql(stmt_name_t stmt_name)
{
    switch (stmt_name)
    {
    case stmt_new_object:
        return "INSERT INTO `objects` (`pid`,`class`,`type`,`name`) VALUES (?,?,?,?);";
    case stmt_new_object_info:
        return "INSERT INTO `objects_info` (`id`,`created`,`depth`,`fullpath`) "
               "SELECT o.`id`, unixepoch('now'), po.`depth` + 1, format('%s/%s', po.`fullpath`, o.`name`) "
               "FROM `objects` as o LEFT JOIN `objects_info` as po on o.`pid` = po.`id` WHERE o.`id` = ?;";
    case stmt_store_scalar:
        return "INSERT INTO `scalars` (`id`, `frequency`, `value`) VALUES (?,?,?);";
    case stmt_store_tseries:
        return "INSERT INTO `tseries` (`id`, `eltype`, `elfreq`, `axis_id`, `value`) VALUES (?,?,?,?,?);";
    case stmt_store_mvtseries:
        return "INSERT INTO `mvtseries` (`id`, `eltype`, `elfreq`, `axis1_id`, `axis2_id`, `value`) VALUES (?,?,?,?,?,?);";
    case stmt_store_ndtseries:
        return "INSERT INTO `ndtseries` (`id`, `eltype`, `elfreq`, `value`) VALUES (?,?,?,?);";
    case stmt_store_ndaxes:
        return "INSERT INTO `ndaxes` (`obj_id`, `axis_index`, `axis_id`) VALUES (?,?,?);";
    case stmt_new_axis:
        return "INSERT INTO `axes` (`ax_type`, `length`, `frequency`, `data`) VALUES (?,?,?,?);";
    case stmt_find_object:
        return "SELECT `id` FROM `objects` WHERE `pid` = ? AND `name` = ?;";
    case stmt_find_fullpath:
        return "SELECT `id` from `objects_info` WHERE `fullpath` = ?;";
    case stmt_find_axis:
        return "SELECT `id`, `data` FROM `axes` WHERE `ax_type` = ? AND `length` = ? AND `frequency` = ?;";
    case stmt_load_object:
        return "SELECT `id`, `pid`, `class`, `type`, `name` FROM `objects` WHERE `id` = ?;";
    case stmt_load_scalar:
        return "SELECT `id`, `frequency`, `value` FROM `scalars` WHERE `id` = ?;";
    case stmt_load_tseries:
        return "SELECT `id`, `eltype`, `elfreq`, `axis_id`, `value` FROM `tseries` WHERE `id` = ?;";
    case stmt_load_mvtseries:
        return "SELECT `id`, `eltype`, `elfreq`, `axis1_id`, `axis2_id`, `value` FROM `mvtseries` WHERE `id` = ?;";
    case stmt_load_ndtseries:
        return "SELECT `id`, `eltype`, `elfreq`, `value` FROM `ndtseries` WHERE `id` = ?;";
    case stmt_load_ndaxes:
        return "SELECT `axes`.*, `ndaxes`.`axis_index` "
               "FROM `ndaxes` LEFT JOIN `axes` ON `ndaxes`.`axis_id` = `axes`.`id` "
               "WHERE `ndaxes`.`obj_id` = ? ORDER BY `ndaxes`.`axis_index`";
    case stmt_load_axis:
        return "SELECT * FROM `axes` WHERE `id` = ?;";
    case stmt_delete_object:
        return "DELETE FROM `objects` WHERE `id` = ?;";
    case stmt_set_attribute:
        return "INSERT INTO `attributes` (`id`, `name`, `value`) VALUES (?, ?, ?);";
    case stmt_get_attribute:
        return "SELECT `value` FROM `attributes` WHERE `id` = ? AND `name` = ?;";
    case stmt_get_all_attributes:
        return "SELECT COUNT(`a`.`id`), GROUP_CONCAT(`a`.`name`, ?1), GROUP_CONCAT(`a`.`value`, ?1) "
               "FROM `objects` AS `o` LEFT JOIN `attributes` AS `a` ON `o`.`id` = `a`.`id` "
               "WHERE `o`.`id` = ?2 GROUP BY `o`.`id`;";
    case stmt_get_object_info:
        return "SELECT `fullpath`, `depth`, `created` FROM `objects_info` WHERE `id` = ?;";
    case stmt_count_objects:
        return "SELECT COUNT(*) from `objects` WHERE `pid` = ?;";
    default:
        error1(DE_INTERNAL, "invalid stmt_name");
        return NULL;
    }
}

sqlite3_stmt *_get_statement(de_file de, stmt_name_t stmt_name)
{
    if ((stmt_name < 0) || (stmt_size <= stmt_name))
    {
        trace_error();
        return NULL;
    }
    sqlite3_stmt *stmt = de->stmt[stmt_name];
    if (stmt != NULL)
        return stmt;
    const char *sql = _get_statement_sql(stmt_name);
    if (sql == NULL)
    {
        trace_error();
        return NULL;
    }
    if (SQLITE_OK != sqlite3_prepare_v2(de->db, sql, -1, &stmt, NULL))
    {
        db_error(de);
        return NULL;
    }
    de->stmt[stmt_name] = stmt;
    return stmt;
}

int _open(const char *fname, de_file *pde, int flags)
{

    if (pde == NULL)
        return error(DE_NULL);

    de_file de = *pde = calloc(1, sizeof(de_file_t));
    if (de == NULL)
        return error(DE_ERR_ALLOC);

    int rc;
    bool file_exists = ((flags & SQLITE_OPEN_MEMORY) == 0) && _isfile(fname);

    if (SQLITE_OK != (rc = sqlite3_open_v2(fname, &de->db, flags, NULL)))
    {
        sqlite3_close(de->db);
        free(de);
        *pde = NULL;
        return rc_error(rc);
    }

    const char *sql_config_db = "PRAGMA foreign_keys = ON;"
                                "PRAGMA temp_store = MEMORY;";
    if (SQLITE_OK != sqlite3_exec(de->db, sql_config_db, NULL, NULL, NULL))
    {
        rc = db_error(de);
        sqlite3_close(de->db);
        free(de);
        *pde = NULL;
        return rc;
    }

    if (file_exists)
        return DE_SUCCESS;

    if (DE_SUCCESS != _init_file(de))
    {
        sqlite3_close(de->db);
        free(de);
        *pde = NULL;
        if (file_exists)
            remove(fname);
        return trace_error();
    }
    return DE_SUCCESS;
}

int de_open(const char *fname, de_file *pde)
{
    TRACE_RUN(_open(fname, pde, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE));
    return DE_SUCCESS;
}

int de_open_readonly(const char *fname, de_file *pde)
{
    TRACE_RUN(_open(fname, pde, SQLITE_OPEN_READONLY));
    return DE_SUCCESS;
}

int de_open_memory(de_file *pde)
{
    TRACE_RUN(_open(":memory:", pde, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_MEMORY));
    return DE_SUCCESS;
}

int de_commit(de_file de)
{
    if (de->transaction)
    {
        if (SQLITE_OK != sqlite3_exec(de->db, "COMMIT;", NULL, NULL, NULL))
            return db_error(de);
        de->transaction = false;
    }
    return DE_SUCCESS;
}

int de_begin_transaction(de_file de)
{
    if (de->transaction)
        return DE_SUCCESS;
    if (SQLITE_OK != sqlite3_exec(de->db, "BEGIN TRANSACTION;", NULL, NULL, NULL))
        return db_error(de);
    de->transaction = true;
    return DE_SUCCESS;
}

int de_close(de_file de)
{
    if (de == NULL)
        return DE_SUCCESS;
    int rc;
    TRACE_RUN(de_commit(de));
    for (stmt_name_t i = 0; i < stmt_last; ++i)
    {
        rc = sqlite3_finalize(de->stmt[i]);
        if (SQLITE_OK == rc)
        {
            de->stmt[i] = NULL;
            continue;
        }
        return rc_error(rc);
    }
    if (SQLITE_OK != sqlite3_close(de->db))
        return db_error(de);
    free(de);
    return rc;
}

int de_truncate(de_file de)
{
    if (de == NULL)
        return error(DE_NULL);
    TRACE_RUN(de_commit(de));
    // https://www.sqlite.org/c3ref/c_dbconfig_defensive.html#sqlitedbconfigresetdatabase
    sqlite3_exec(de->db, "SELECT COUNT(*) FROM `objects` WHERE `pid` = 0;", NULL, NULL, NULL);
    sqlite3_db_config(de->db, SQLITE_DBCONFIG_RESET_DATABASE, 1, 0);
    sqlite3_exec(de->db, "VACUUM", 0, 0, 0);
    sqlite3_db_config(de->db, SQLITE_DBCONFIG_RESET_DATABASE, 0, 0);
    TRACE_RUN(_init_file(de));
    return DE_SUCCESS;
}
