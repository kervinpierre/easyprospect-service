#pragma once
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/dom/DOMDocument.hpp>
#include <xercesc/sax/ErrorHandler.hpp>
#include <xercesc/sax/EntityResolver.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xalanc/XalanTransformer/XalanTransformer.hpp>
#include <xalanc/DOMSupport\XalanDocumentPrefixResolver.hpp>

#include <memory>
#include <string>
#include <vector>

namespace easyprospect
{
namespace service
{
    namespace plugin
    {
        class ep_srv_plugin_sf_dom_error_handler;
        class easyprospect_service_plugin_salesforce;

        // Context objects keeps the unique_ptr's
        // to various session objects. ::get() to share non-owning
        class ep_srv_plugin_sf_context final
                    : public std::enable_shared_from_this<ep_srv_plugin_sf_context>
        {
          private:
            std::shared_ptr<easyprospect_service_plugin_salesforce> plugin_;
            std::shared_ptr<ep_srv_plugin_sf_dom_error_handler> errors_;

            std::string catalog_xsd;

          public:
            ep_srv_plugin_sf_context() = delete;
            explicit ep_srv_plugin_sf_context(std::shared_ptr<easyprospect_service_plugin_salesforce> plugin);

            std::shared_ptr<ep_srv_plugin_sf_dom_error_handler> get_errors() const;
            void set_errors(std::shared_ptr<ep_srv_plugin_sf_dom_error_handler> val);

            std::string get_catalog_xsd() const
            {
                return catalog_xsd;
            }
            void set_catalog_xsd(std::string val)
            {
                catalog_xsd = val;
            }
        };

        class ep_srv_plugin_sf_doc final : public std::enable_shared_from_this<ep_srv_plugin_sf_doc>
        {
          private:
            std::unique_ptr<xercesc::XercesDOMParser> parser_;
            std::shared_ptr<ep_srv_plugin_sf_context> cxt_;

          public:
            ep_srv_plugin_sf_doc() = delete;
            explicit ep_srv_plugin_sf_doc(std::shared_ptr<ep_srv_plugin_sf_context> cxt, std::unique_ptr<xercesc::XercesDOMParser> d);

            xercesc::DOMDocument *get_doc() const
            {
                if(parser_)
                {
                    return parser_->getDocument();
                }
                else
                {
                    return nullptr;
                }
            }
        };

        class ep_srv_plugin_sf_dom_error
        {
        private:
            std::string msg_;

        public:
            std::string get_msg() const
            {
                return msg_;
            }
            void set_msg(std::string val)
            {
                msg_ = val;
            }
        };

        class ep_srv_plugin_sf_dom_error_handler final : public xercesc::ErrorHandler
        {
          private:
            std::vector<ep_srv_plugin_sf_dom_error> errors;

          public:
            ep_srv_plugin_sf_dom_error_handler() = default;
            ~ep_srv_plugin_sf_dom_error_handler() = default;

            /** @name The error handler interface */
            bool handleError(const xercesc::DOMError& domError);
            void resetErrors(){}

            void add_error(ep_srv_plugin_sf_dom_error e);

            void warning(const xercesc::SAXParseException& exc) override;
            void error(const xercesc::SAXParseException& exc) override;
            void fatalError(const xercesc::SAXParseException& exc) override;
        };

        class ep_srv_plugin_sf_entity_resolver_input
        {
        private:
            // We do not manage this pointer
            xercesc::InputSource *source_input_;
            std::shared_ptr<std::string>          source_string_;

        public:
            ep_srv_plugin_sf_entity_resolver_input()
            {
                source_string_ = std::make_shared<std::string>();
            }

            xercesc::InputSource *get_source_input() const
            {
                return source_input_;
            }
            void set_source_input(xercesc::InputSource *val)
            {
                source_input_ = val;
            }

            const std::shared_ptr<std::string> get_source_string() const
            {
                return source_string_;
            }
            void set_source_string(std::shared_ptr<std::string> val)
            {
                source_string_ = val;
            }
        };

        class ep_srv_plugin_sf_entity_resolver : public xercesc::EntityResolver
        {
        private:
            std::shared_ptr<ep_srv_plugin_sf_context> cxt_;
            std::vector<std::shared_ptr<ep_srv_plugin_sf_entity_resolver_input>> sources_;

        public:
            ep_srv_plugin_sf_entity_resolver() = delete;

            explicit ep_srv_plugin_sf_entity_resolver(std::shared_ptr<ep_srv_plugin_sf_context> cxt)
            {
                cxt_ = cxt;
            }

            xercesc::InputSource* resolveEntity(const XMLCh* const publicId,
                const XMLCh* const systemId) override;
        };

        class ep_xalan_prefix_resolver : public xalanc::PrefixResolver
        {
        private:
            const xalanc::XalanDOMString default_uri_;
            std::unique_ptr<xalanc::XalanDocumentPrefixResolver> prefix_esolver_;

        public:
            explicit ep_xalan_prefix_resolver(const xalanc::XalanDOMString& the_uri,
                                               xalanc::XalanDocument *doc)
                : default_uri_(the_uri)
            {
                prefix_esolver_ = std::make_unique<xalanc::XalanDocumentPrefixResolver>(doc);
            }

            ep_xalan_prefix_resolver() = delete;

            const xalanc::XalanDOMString* getNamespaceForPrefix(const xalanc::XalanDOMString& prefix) const override
            {
                return &default_uri_;
            }

            const xalanc::XalanDOMString& getURI() const override
            {
                return default_uri_;
            }

        };

    } // namespace plugin
} // namespace service
} // namespace easyprospect