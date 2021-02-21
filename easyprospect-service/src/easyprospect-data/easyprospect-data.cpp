#include <easyprospect-data/easyprospect-data.h>
#include <boost/algorithm/string.hpp>
#include <boost/optional/optional.hpp>
#include <boost/variant2/variant.hpp>

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
            boost::trim(sql);
            boost::optional<std::vector<std::vector<boost::variant2::variant<int, std::string>>>> res;

            if(!sql.empty())
            {
                prepare(sql);
            }

            last_rc_ = sqlite3_step(stmt_);
            switch(last_rc_)
            {
                case SQLITE_DONE:
                    break;

                case SQLITE_ROW:
                    {
                        res = std::vector<std::vector<boost::variant2::variant<int, std::string>>>{};
                        int cols = sqlite3_column_count(stmt_);
                        
                        std::vector<int> col_header;

                        for(int i = 0; i < cols; i++)
                        {
                            auto cn = sqlite3_column_name(stmt_, i);
                            col_header[i] = sqlite3_column_type(stmt_, i);
                            auto dt = sqlite3_column_decltype(stmt_, i);
                        }

                        do 
                        {
                            std::vector<boost::variant2::variant<int, std::string>> result_row;

                            for(size_t i=0; i<col_header.size(); i++)
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

                            last_rc_ = sqlite3_step(stmt_);
                        }
                        while(last_rc_ == SQLITE_ROW);
                    }
                    break;

                default:;
            }

            return res;
        }
        
        void ep_sqlite_statement::prepare(std::string sql) 
        {
            boost::trim(sql);

            if(sql.empty())
            {
                throw std::logic_error("Invalid empty SQL");
            }

            auto db = db_.lock();

            auto res = sqlite3_prepare_v2(db->get_db(), sql.c_str(), sql.length(), &stmt_, nullptr);
        }
    } // namespace database
} // namespace data
} // namespace easyprospect