#pragma once

#include <stdexcept>
#include <string>
#include <sqlite/sqlite3.h>
#include <spdlog/spdlog.h>

namespace easyprospect
{
namespace data
{
    namespace database
    {
        class ep_sqlite_db final
        {
        private:
            sqlite3* db_ = nullptr;

        public:
            ep_sqlite_db() = delete;

            explicit ep_sqlite_db(std::string host)
            {
                auto rc = sqlite3_open(host.c_str(), &db_);
                if(rc != SQLITE_OK)
                {
                    auto msg = fmt::format("Can't open database: {}\n", sqlite3_errmsg(db_));
                    spdlog::error(msg);

                    sqlite3_close(db_);

                    throw std::logic_error(msg);
                }
            }

            ~ep_sqlite_db()
            {
                sqlite3_close(db_);
            }
        };

        // http://sqlite.1065341.n5.nabble.com/SQLite-Extensions-td85715.html
//        These functions can be most useful to maintain an in-table hash of the row contents, useful for comparing data in the table without comparing all the columns by using triggers.
//
//create table data ( x text, y text, hash blob);
//
//        create trigger data_hash_insert after insert on data begin update data set hash = binmd5(x, y) where rowid =
//            new.rowid;
//        end;
//        create trigger data_hash_update after update of x,
//            y on data begin update data set hash = binmd5(x, y) where rowid = new.rowid;
//        end;
//
//        insert into data values('Keith', 'Medcalf', null);
//        insert into data values('Carl', 'Medcalf', null);
//
//        select x, y, hex(hash) from data;

        class ep_sqlite
        {
        private:
            std::shared_ptr<ep_sqlite_db> db_;
        public:
            explicit ep_sqlite()
            {
               // auto res = sqlite3_initialize();
            }

            ~ep_sqlite()
            {
               // auto res = sqlite3_shutdown();
            }

            void open(std::string h)
            {
                db_ = std::make_shared<ep_sqlite_db>(h);
            }
        };
    }
} // namespace data
} // namespace easyprospect