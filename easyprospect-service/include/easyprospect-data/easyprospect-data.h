#pragma once

#include <spdlog/spdlog.h>
#include <sqlite/sqlite3.h>
#include <stdexcept>
#include <string>

namespace easyprospect
{
namespace data
{
    namespace database
    {
        class ep_sqlite_db;

        class ep_sqlite_statement final
        {
          private:
            sqlite3_stmt* stmt_ = nullptr;
            std::weak_ptr<ep_sqlite_db> db_;
            int                         last_rc_ = 0;

          public:
            ep_sqlite_statement() = delete;

            explicit ep_sqlite_statement(std::weak_ptr<ep_sqlite_db> db);

            auto runnow(std::string sql);
            auto runnow2(std::string sql);

            std::string prepare(std::string sql);

            void bind_int(int idx, int val) const
            {
                auto res = sqlite3_bind_int(stmt_, idx, val);
                if(res!= SQLITE_OK)
                {
                    throw std::logic_error("Bind Integer failed");
                }
            }

            void bind_text(int idx, std::string val) const
            {
                auto res = sqlite3_bind_text(stmt_, idx, val.c_str(), val.length(), nullptr );
                if(res != SQLITE_OK)
                {
                    throw std::logic_error("Bind failed");
                }
            }
                
            int get_int(int idx) const
            {
                auto curr_res = sqlite3_step(stmt_);
                int  val_res;

                if( curr_res == SQLITE_OK )
                {
                    val_res = sqlite3_column_int(stmt_, idx);
                }

                return val_res;
            }

            std::string get_text(int idx) const
            {
                auto curr_res = sqlite3_step(stmt_);
                std::string  val_res;

                if(curr_res == SQLITE_OK)
                {
                    auto str = sqlite3_column_text(stmt_, idx);
                    val_res = std::string(reinterpret_cast<const char*>(str));
                }

                return val_res;
            }

            int insert_new_object();

            void reset() const
            {
                sqlite3_reset(stmt_);
                sqlite3_clear_bindings(stmt_);
            }

            auto get_statement() const
            {
                return stmt_;
            }

            ~ep_sqlite_statement()
            {
                sqlite3_finalize(stmt_);
                stmt_ = nullptr;
            }
        };

        class ep_sqlite_db final : public std::enable_shared_from_this<ep_sqlite_db>
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

            auto get_db() const
            {
                return db_;
            }

            static std::string get_dll_sql();

            auto create_statement()
            {
                auto res = std::make_shared<ep_sqlite_statement>(shared_from_this());

                return res;
            }

            void initialize_schema();

            ~ep_sqlite_db()
            {
                sqlite3_close(db_);
                db_ = nullptr;
            }
        };

        // http://sqlite.1065341.n5.nabble.com/SQLite-Extensions-td85715.html
        //        These functions can be most useful to maintain an in-table hash of the row contents, useful for
        //        comparing data in the table without comparing all the columns by using triggers.
        //
        // create table data ( x text, y text, hash blob);
        //
        //        create trigger data_hash_insert after insert on data begin update data set hash = binmd5(x, y) where
        //        rowid =
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

        class ep_sqlite final
        {
          private:
            std::shared_ptr<ep_sqlite_db> db_;

          public:
            explicit ep_sqlite()
            {
                auto res = sqlite3_initialize();
            }

            ~ep_sqlite()
            {
                auto res = sqlite3_shutdown();
            }

            auto get_db() const
            {
                return db_;
            }

            void open(std::string h)
            {
                db_ = std::make_shared<ep_sqlite_db>(h);
            }
        };
    } // namespace database
} // namespace data
} // namespace easyprospect