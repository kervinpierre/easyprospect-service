#pragma once
#include <map>
#include <string>
#include <vector>
#include <boost/optional/optional.hpp>
#include <boost/beast/http/write.hpp>
namespace easyprospect
{
namespace service
{
    namespace shared
    {
        class easyprospect_http_request;
        class easyprospect_http_request_result;
        using send_worker_req_impl_type = std::function<easyprospect_http_request_result(easyprospect_http_request, boost::beast::error_code &ec)>;

        class easyprospect_http_request final
        {
        private:
            boost::optional<unsigned long long> content_length_;
            boost::optional<std::string> body_;
            boost::optional<std::string> raw_;
            boost::optional<std::string> content_type_;
            std::string                  url_;
            std::vector<std::pair<std::string,std::string>> headers_;

        public:

            easyprospect_http_request(
                const boost::optional<unsigned long long>         cl,
                const boost::optional<std::string> b,
                const boost::optional<std::string> r,
                const boost::optional<std::string> ct,
                const std::string                                      u,
                const std::vector<std::pair<std::string, std::string>> h)
            {
                content_length_ = cl;
                body_           = b;
                raw_            = r;
                content_type_   = ct;
                url_            = u;
                headers_        = h;
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

            std::string to_string()
            {
                std::stringstream str;

                str << "url: " << get_url() << std::endl;

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

                str << "headers: " << headers_.size() << std::endl;
                for (auto& i : headers_)
                {
                    str << "    " << i.first << ": " << i.second << std::endl;
                }

                str << std::endl;

                return str.str();
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
            std::vector<std::pair<std::string, std::string>> headers_;

          public:

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

            easyprospect_http_request_builder(boost::beast::http::parser<true, boost::beast::http::string_body>& req_p)
            {
                auto& req = req_p.get();

                std::stringstream str;
                str << req;

                spdlog::trace(str.str());

                set_body(req.body());
                set_content_length(req_p.content_length());
                set_content_type( req[boost::beast::http::field::content_type].to_string());
                set_url(req.target().to_string());

                std::vector<std::pair<std::string, std::string>> h;
                for ( auto& i : req)
                {
                    h.emplace_back(i.name_string(), i.value());
                }
                set_headers(h);
            }

            easyprospect_http_request to_request() const
            {
                easyprospect_http_request r(content_length_, body_, raw_, content_type_, url_, headers_);

                return r;
            }

        };

        class easyprospect_http_request_result final
        {
        };
    }
} // namespace service
} // namespace easyprospect