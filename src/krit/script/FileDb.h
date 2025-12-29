#include "../script/ScriptEngine.h"
#include <memory>
#include <optional>
#include <string>
#include <vector>

struct sqlite3;
struct sqlite3_stmt;

namespace krit {

struct DbRow {
    DbRow(sqlite3_stmt *stmt);
    virtual ~DbRow();

    int getInt(int index);
    std::string getString(int index);
    JSValue getBlob(int index);

private:
    sqlite3_stmt *stmt;
};

struct DbQuery {
    DbQuery(sqlite3 *db, const std::string &query);
    ~DbQuery();
    std::vector<std::string> columnNames();
    int columnCount();
    void
    exec(std::optional<std::function<void(JSValue)>> callback = std::nullopt);
    void bindInt(int index, int value);
    void bindString(int index, const std::string &s);
    void bindBlob(int index, JSValue value);

private:
    sqlite3_stmt *stmt = nullptr;
};

struct FileDb {
    static FileDb *create(const std::string &path) { return new FileDb(path); }
    static int returnRows(void *, int argc, char **argv, char **azColName);

    bool valid = false;

    FileDb(const std::string &path);
    virtual ~FileDb();

    void
    exec(const std::string &query,
         std::optional<std::function<void(JSValue)>> callback = std::nullopt);
    std::unique_ptr<DbQuery> prepare(const std::string &query);

private:
    JSValue thisObj;
    JSValue resultCallback;
    sqlite3 *db = nullptr;
};

}
