#pragma once
#include <map>
#include <string>
#include <vector>
#include <boost/beast/http/buffer_body.hpp>
#include <boost/optional/optional.hpp>
#include <boost/beast/http/write.hpp>
#include <boost/beast/http/verb.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/nil_generator.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>

namespace easyprospect
{
namespace service
{
    namespace shared
    {
        class easyprospect_http_request_continuation;
        class easyprospect_http_request;
        class easyprospect_http_request_result;
        using send_worker_req_impl_type = std::function<void(
            std::shared_ptr<const easyprospect_http_request>              first_req,
            int                                                           position,
            std::shared_ptr<const easyprospect_http_request_continuation> req_continuation,
                                            boost::beast::error_code&,
                                            std::function<void(boost::beast::http::response<boost::beast::http::string_body>)>)>;

        class easyprospect_http_request final
        {
            //has_more_data()
            //async_get_more_data()
            //forward_rest(Stream)
        private:
            boost::optional<unsigned long long> content_length_;
            boost::optional<std::string> body_;
            boost::optional<std::string> raw_;
            boost::optional<std::string> content_type_;
            std::string                  url_;
            std::string                  host_;
            std::string                  method_;
            std::vector<std::pair<std::string,std::string>> headers_;
            bool                                             done_;
            boost::uuids::uuid                              session_id_;

        public:

            easyprospect_http_request(const boost::optional<unsigned long long>              cl,
                                    const boost::optional<std::string>                     b,
                                    const boost::optional<std::string>                     r,
                                    const boost::optional<std::string>                     ct,
                                    const std::string                                      u,
                                    const std::string                                      m,
                                    const bool                                             d,
                                    const boost::uuids::uuid                               s,
                const std::vector<std::pair<std::string, std::string>> h)
            {
                content_length_ = cl;
                body_           = b;
                raw_            = r;
                content_type_   = ct;
                url_            = u;
                method_         = m;
                done_           = d;
                headers_        = h;
                session_id_     = s;
            }

            bool has_raw() const
            {
                return raw_.has_value();
            }

            bool has_content_length() const
            {
                return content_length_.has_value();
            }

            bool has_body() const
            {
                return body_.has_value();
            }

            bool has_content_type() const
            {
                return content_type_.has_value();
            }

            unsigned long long get_content_length() const
            {
                if ( has_content_length() )
                {
                    return content_length_.get();
                }
                
                throw std::logic_error("Content Length not found");
            }

            std::string get_raw() const
            {
                if (has_raw())
                {
                    return raw_.get();
                }

                throw std::logic_error("Raw request not found");
            }

            std::string get_body() const
            {
                if (has_body())
                {
                    return body_.get();
                }

                throw std::logic_error("Request body not found");
            }

            std::string get_method() const
            {
                return method_;
            }

            std::string get_content_type() const
            {
                if (has_content_type())
                {
                    return content_type_.get();
                }

                throw std::logic_error("Content type not found");
            }

            std::vector<std::pair<std::string, std::string>> get_headers() const
            {
                return headers_;
            }

            std::string get_url() const
            {
                return url_;
            }

            std::string get_host() const
            {
                return host_;
            }

            auto get_session_id() const
            {
                return session_id_;
            }

            bool is_done() const
            {
                return done_;
            }

            std::string to_string() const
            {
                std::stringstream str;

                str << "url     : " << get_url() << std::endl;
                str << "method  : " << get_method() << std::endl;
                str << "session : " << get_session_id() << std::endl;

                if ( content_length_ )
                {
                    str << "content length: " << *content_length_ << std::endl;
                }
                else
                {
                    str << "content length: (uninitialized)" << std::endl;     
                }

                if (content_type_)
                {
                    str << "content type: " << *content_type_ << std::endl;
                }
                else
                {
                    str << "content type: (uninitialized)" << std::endl;
                }

                str << "is done: " << done_ << std::endl;

                str << "headers: " << headers_.size() << std::endl;
                for (auto& i : headers_)
                {
                    str << "    " << i.first << ": " << i.second << std::endl;
                }

                str << std::endl;

                return str.str();
            }

            boost::beast::http::request<boost::beast::http::string_body> to_beast_request() const
            {
                boost::beast::http::request<boost::beast::http::string_body> res;
                res.method(boost::beast::http::string_to_verb(get_method()));
                res.target(url_);
                res.version(11);

                if (has_body())
                {
                    res.body() = get_body();
                }

                if (get_session_id() != boost::uuids::nil_uuid())
                {
                    res.set("X-EP-SESSION-ID", boost::uuids::to_string(get_session_id()));
                }

                for ( auto h : headers_ )
                {
                    res.set(h.first, h.second);
                }
               //boost::beast::http::request<boost::beast::http::string_body> be_req{
               //boost::beast::http::verb::get, target, version};
               // be_req.set(boost::beast::http::field::host, host);
               // be_req.set(boost::beast::http::field::user_agent, BOOST_BEAST_VERSION_STRING);

                return res;
            }
        };

        class easyprospect_http_request_continuation final
        {
          private:
            const std::shared_ptr<const easyprospect_http_request> first_request_;
            const std::vector<unsigned char>                 buffer_{};
            const bool                                       done_;

          public:
            easyprospect_http_request_continuation(
                const std::shared_ptr<const easyprospect_http_request> f,
                const std::vector<unsigned char>& b,
                const bool                                       d) :
                first_request_(f),
                buffer_(b), done_(d)
            {
                ;
            }

            auto get_request() const
            {
                return first_request_;
            }

            auto& get_buffer() const
            {
                return buffer_;
            }

            auto is_done() const
            {
                return done_;
            }
        };

        class easyprospect_http_request_builder final
        {
          private:
            boost::optional<unsigned long long>              content_length_;
            boost::optional<std::string>                     body_;
            boost::optional<std::string>                     raw_;
            boost::optional<std::string>                     content_type_;
            std::string                                      url_;
            std::string                                      host_;
            std::string                                      method_;
            std::vector<std::pair<std::string, std::string>> headers_;
            std::vector<char>                                input_buffer_;
            bool                                             header_done = false;
            bool                                             done        = false;
            int                                              total_bytes_read_ = 0;
            boost::uuids::uuid                               session_id_;

          public:

            void set_session_id(boost::uuids::uuid id)
            {
                session_id_ = id;
            }

            void set_content_length(boost::optional<unsigned long long> cl)
            {
                content_length_ = cl;
            }

            void set_content_length(unsigned long long cl)
            {
                content_length_ = cl;
            }

            void set_body(std::string b)
            {
                body_ = b;
            }

            void set_host(std::string h)
            {
                host_ = h;
            }

            void set_method(std::string m)
            {
                method_ = m;
            }

            void set_headers(std::vector<std::pair<std::string, std::string>> h)
            {
                headers_ = h;
            }

            void set_content_type(std::string c)
            {
                content_type_ = c;
            }

            void set_url(std::string u)
            {
                url_ = u;
            }

            bool is_header_done() const
            {
                return header_done;
            }
            void set_header_done(bool val)
            {
                header_done = val;
            }

            void set_done(bool val)
            {
                done = val;
            }

            void set_input_buffer_capacity(int cap)
            {
                input_buffer_.resize(cap);
            }

            void set_input_buffer_contents(char *val, int size)
            {
                input_buffer_.insert(input_buffer_.end(), val, val+size);
            }

            void set_total_bytes_read(int t)
            {
                total_bytes_read_ = t;
            }

            explicit easyprospect_http_request_builder(const easyprospect_http_request &req)
            {
                set_url(req.get_url());
                set_headers(req.get_headers());
               // set_body(req.get_body());

                if ( req.has_content_length())
                    set_content_length(req.get_content_length());

                if (req.has_content_type())
                    set_content_type(req.get_content_type());

                set_host(req.get_host());
                set_method(req.get_method());
                set_done(req.is_done());
                set_session_id(req.get_session_id());
            }

            easyprospect_http_request_builder(
                boost::beast::http::parser<true, boost::beast::http::buffer_body>& req_p,
                boost::uuids::uuid                                                 sess_id = boost::uuids::nil_uuid())
            {
                auto& req = req_p.get();

                if ( sess_id == boost::uuids::nil_uuid())
                {
                    boost::uuids::random_generator gen;
                    set_session_id( gen());
                }
                else
                {
                    set_session_id(sess_id);     
                }

                std::stringstream str;
                //str << req;

                spdlog::trace(str.str());

               // set_body(req.body());
                set_content_length(req_p.content_length());
                set_content_type( req[boost::beast::http::field::content_type].to_string());
                set_host( req[boost::beast::http::field::host].to_string());
                set_method( boost::beast::http::to_string(req.method()).to_string());
                set_url(req.target().to_string());
                set_done(req_p.is_done());
                set_header_done(req_p.is_header_done());

                std::vector<std::pair<std::string, std::string>> h;
                for ( auto& i : req)
                {
                    h.emplace_back(i.name_string(), i.value());
                }
                set_headers(h);
            }

            auto to_request() const
            {
                auto res = std::make_shared<easyprospect_http_request>(content_length_, body_, raw_, content_type_, url_, method_, done, session_id_, headers_);

                return res;
            }

        };

        class easyprospect_http_request_continuation_builder final
        {
          private:
            std::shared_ptr<const easyprospect_http_request> first_request_;
            std::vector<unsigned char> buffer_;
            bool              done_ = false;

          public:

            easyprospect_http_request_continuation_builder(
                boost::beast::http::parser<true, boost::beast::http::buffer_body>& req_p,
                const std::shared_ptr<const easyprospect_http_request> first_req)
            {
                set_done(req_p.is_done());
                set_request(first_req);

                auto b = req_p.get().body();

                //buffer_.assign(b.size, (unsigned char)(b.data));
                set_buffer_contents(static_cast<unsigned char*>(b.data), b.size);
            }

            void set_done(bool val)
            {
                done_ = val;
            }

            void set_buffer_contents(unsigned char* val, u_int64 size)
            {
                buffer_.clear();
                buffer_.resize(size);
                buffer_.insert(buffer_.end(), val, val + size);
            }

            void set_request(std::shared_ptr<const easyprospect_http_request> r)
            {
                first_request_ = r;
            }

            auto to_continuation() const
            {
                auto res = std::make_shared<easyprospect_http_request_continuation>(first_request_, buffer_, done_);

                return res;
            }
        };

        class easyprospect_http_request_result final
        {
        };
    }
} // namespace service
} // namespace easyprospect