#include <easyprospect-data/easyprospect-data.h>
#include <boost/algorithm/string.hpp>
#include <boost/optional/optional.hpp>
#include <boost/variant2/variant.hpp>

#include <easyprospect-data/ep-incbin-01.h>

namespace easyprospect
{
namespace data
{
    namespace database
    {
        ep_sqlite_statement::ep_sqlite_statement(std::weak_ptr<ep_sqlite_db> db) : db_(db)
        {

        }

        auto ep_sqlite_statement::runnow(std::string sql)
        {
            auto db  = db_.lock();
            char *errmsg = NULL;
            auto res = sqlite3_exec(db->get_db(), sql.c_str(), nullptr, nullptr, &errmsg);
            if(res!=SQLITE_OK)
            {
                spdlog::error("Error initializing database: {}", errmsg);

                throw std::logic_error(errmsg);
            }
        }

        auto ep_sqlite_statement::runnow2(std::string sql)
        {
            boost::trim(sql);
            sql.erase(std::remove(sql.begin(), sql.end(), '\r'), sql.end());
            sql.erase(std::remove(sql.begin(), sql.end(), '\n'), sql.end());
            sql.erase(std::remove(sql.begin(), sql.end(), '\t'), sql.end());

            boost::optional<std::vector<std::vector<boost::variant2::variant<int, std::string>>>> res;

            for(auto sql_remainder = sql; !sql_remainder.empty(); sql_remainder = prepare(sql_remainder))
            {
                last_rc_ = sqlite3_step(stmt_);
                switch(last_rc_)
                {
                case SQLITE_DONE:
                    break;

                case SQLITE_ROW:
                {
                    res      = std::vector<std::vector<boost::variant2::variant<int, std::string>>>{};
                    int cols = sqlite3_column_count(stmt_);

                    std::vector<int> col_header;

                    for(int i = 0; i < cols; i++)
                    {
                        auto cn       = sqlite3_column_name(stmt_, i);
                        col_header[i] = sqlite3_column_type(stmt_, i);
                        auto dt       = sqlite3_column_decltype(stmt_, i);
                    }

                    do
                    {
                        std::vector<boost::variant2::variant<int, std::string>> result_row;

                        for(size_t i = 0; i < col_header.size(); i++)
                        {
                            switch(col_header[i])
                            {
                            case SQLITE_INTEGER:
                            {
                                int val_res = sqlite3_column_int(stmt_, i);
                                result_row.emplace_back(val_res);
                            }
                            break;

                            default:;
                            }
                        }

                        res->push_back(result_row);

                        last_rc_ = ::sqlite3_step(stmt_);
                    } while(last_rc_ == SQLITE_ROW);
                }
                break;

                default:;
                }
            }

            return res;
        }
        
        std::string ep_sqlite_statement::prepare(std::string sql) 
        {
            boost::trim(sql);
            std::string res;

            if(sql.empty())
            {
                throw std::logic_error("Invalid empty SQL");
            }

            auto db = db_.lock();

            const char *tail = nullptr;
            auto pres = sqlite3_prepare_v2(db->get_db(), sql.c_str(), -1, &stmt_, &tail);

            if(tail && strlen(tail)>0)
            {
                int count = sql.length() - strlen(tail);
                if(count > 0)
                {
                    sql.erase(0, count);
                    res = sql;
                }
            }

            return res;
        }

        int64_t ep_sqlite_statement::insert_new_object()
        {
            auto db = db_.lock();
            runnow("INSERT INTO ep_sf_object (eso_type, eso_import_id, eso_hash) VALUES (0, 0, '')");
            int res = sqlite3_last_insert_rowid(db->get_db());

            return res;
        }

        int64_t ep_sqlite_statement::insert_new_import(std::string label)
        {
            std::string curr_label = label;

            auto db = db_.lock();
            runnow("INSERT INTO ep_sf_obj_import (label, timestamp) VALUES ('" + curr_label + "', CURRENT_TIMESTAMP)");
            auto res = sqlite3_last_insert_rowid(db->get_db());

            return res;
        }

        std::string ep_sqlite_db::get_dll_sql()
        {
            std::string gddl_str(reinterpret_cast<const char *>(gddl_sqlData));

            return gddl_str;
        }

        void ep_sqlite_db::initialize_schema()
        {
            auto stmt = create_statement();

            stmt->runnow(get_dll_sql());
        }
    } // namespace database
} // namespace data
} // namespace easyprospect