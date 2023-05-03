#include "FileDb.h"
#include "krit/Engine.h"
#include "krit/script/ScriptValue.h"
#include "krit/utils/Log.h"
#include <quickjs.h>
#include <sqlite3.h>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

namespace krit {

DbRow::DbRow(sqlite3_stmt *stmt) : stmt(stmt) {}

DbRow::~DbRow() {}

int DbRow::getInt(int index) { return sqlite3_column_int(stmt, index); }

std::string DbRow::getString(int index) {
    return (char *)sqlite3_column_text(stmt, index);
}

JSValue DbRow::getBlob(int index) {
    size_t len = sqlite3_column_bytes(stmt, index);
    const void *buf = sqlite3_column_blob(stmt, index);
    auto ctx = engine->script.ctx;
    JSValue result = JS_NewArrayBufferCopy(ctx, (const uint8_t *)buf, len);
    return result;
}

DbQuery::DbQuery(sqlite3 *db, const std::string &query) {
    int res = sqlite3_prepare_v3(db, query.c_str(), query.size(),
                                 SQLITE_PREPARE_PERSISTENT, &stmt, nullptr);
    if (res != SQLITE_OK) {
        LOG_ERROR("failure compiling query (%i): %s", res, query.c_str());
        stmt = nullptr;
    }
}

DbQuery::~DbQuery() {
    if (stmt) {
        sqlite3_finalize(stmt);
    }
}

std::vector<std::string> DbQuery::columnNames() {
    if (!stmt) {
        return {};
    }
    std::vector<std::string> result;
    int count = sqlite3_column_count(stmt);
    result.reserve(count);
    for (int i = 0; i < count; ++i) {
        result.emplace_back(sqlite3_column_name(stmt, i));
    }
    return result;
}

int DbQuery::columnCount() {
    if (!stmt) {
        return 0;
    }
    return sqlite3_column_count(stmt);
}

void DbQuery::bindInt(int index, int value) {
    sqlite3_bind_int(stmt, index, value);
}

void DbQuery::bindString(int index, const std::string &s) {
    sqlite3_bind_text(stmt, index, s.c_str(), s.size(), SQLITE_TRANSIENT);
}

void DbQuery::bindBlob(int index, JSValue value) {
    auto ctx = engine->script.ctx;
    size_t size;
    uint8_t *buf = JS_GetArrayBuffer(ctx, &size, value);
    sqlite3_bind_blob(stmt, index, buf, size, SQLITE_TRANSIENT);
}

void DbQuery::exec(JSValue callback) {
    int status = 0;
    DbRow row(stmt);
    auto ctx = engine->script.ctx;
    JSValue thisObj = JS_UNDEFINED, rowObj = JS_UNDEFINED;
    if (!JS_IsUndefined(callback)) {
        thisObj = ScriptValueToJs<DbQuery *>::valueToJs(ctx, this);
        rowObj = ScriptValueToJs<DbRow *>::valueToJs(ctx, &row);
    }
    while (true) {
        status = sqlite3_step(stmt);
        if (status == SQLITE_ROW) {
            if (!JS_IsUndefined(callback)) {
                JS_FreeValue(ctx, JS_Call(ctx, callback, thisObj, 1, &rowObj));
            }
        } else if (status == SQLITE_DONE) {
            break;
        } else {
            LOG_ERROR("query error: %i", status);
            break;
        }
    }
    JS_FreeValue(ctx, rowObj);
    JS_FreeValue(ctx, thisObj);
    sqlite3_reset(stmt);
}

FileDb::FileDb(const std::string &path) {
    int res = sqlite3_open(path.c_str(), &db);
    if (res) {
        LOG_ERROR("couldn't open database: %s", path.c_str());
        db = nullptr;
        valid = false;
        return;
    }
    valid = true;
    auto ctx = engine->script.ctx;
    thisObj = ScriptValueToJs<FileDb *>::valueToJs(ctx, this);
}

FileDb::~FileDb() {
    if (db) {
        sqlite3_close(db);
    }
    auto ctx = engine->script.ctx;
    JS_FreeValue(ctx, thisObj);
}

void FileDb::exec(const std::string &query, JSValue callback) {
    if (!db) {
        return;
    }
    resultCallback = callback;
    char *errorMessage = nullptr;
    sqlite3_exec(db, query.c_str(),
                 JS_IsUndefined(callback) ? nullptr : FileDb::returnRows, this,
                 &errorMessage);
    if (errorMessage) {
        LOG_ERROR("db query returned error: %s", errorMessage);
        auto ctx = engine->script.ctx;
        JSValue err = JS_NewError(ctx);
        JS_SetPropertyStr(ctx, err, "message", JS_NewString(ctx, errorMessage));
        JS_Throw(ctx, err);
        JS_FreeValue(ctx, err);
        sqlite3_free(errorMessage);
    }

#ifdef __EMSCRIPTEN__
    EM_ASM(
        FS.syncfs(function() {});
    );
#endif
}

int FileDb::returnRows(void *_db, int argc, char **argv, char **azColName) {
    FileDb *db = (FileDb *)_db;
    auto ctx = engine->script.ctx;
    JSValue obj = JS_NewObject(ctx);
    for (int i = 0; i < argc; ++i) {
        JS_SetPropertyStr(ctx, obj, azColName[i],
                          argv[i] ? JS_NewString(ctx, argv[i]) : JS_NULL);
    }
    JS_FreeValue(ctx, JS_Call(ctx, db->resultCallback, db->thisObj, 1, &obj));
    return 0;
}

std::unique_ptr<DbQuery> FileDb::prepare(const std::string &query) {
    return std::unique_ptr<DbQuery>(new DbQuery(db, query));
}

}
