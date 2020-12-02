#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/string_file.hpp>
#include <easyprospect-service-plugin-salesforce/ep-srv-plugin-sf-context.h>
#include <xercesc/util/XMLString.hpp>
#include <spdlog/spdlog.h>
#include <xercesc/sax/SAXParseException.hpp>
#include <xercesc/framework/MemBufInputSource.hpp>

namespace easyprospect
{
namespace service
{
    namespace plugin
    {
        std::shared_ptr<ep_srv_plugin_sf_dom_error_handler> ep_srv_plugin_sf_context::
        get_errors() const
        {
            return errors_;
        }

        void ep_srv_plugin_sf_context::set_errors(
            std::shared_ptr<ep_srv_plugin_sf_dom_error_handler> val)
        {
            errors_ = val;
        }

        bool ep_srv_plugin_sf_dom_error_handler::handleError(const xercesc::DOMError& domError)
        {
            // Display whatever error message passed from the serializer
            if (domError.getSeverity() == xercesc::DOMError::DOM_SEVERITY_WARNING)
                spdlog::warn("\nWarning Message: ");
            else if (domError.getSeverity() == xercesc::DOMError::DOM_SEVERITY_ERROR)
                spdlog::error("\nError Message: ");
            else
                spdlog::error( "\nFatal Message: ");

            char*                msg = xercesc::XMLString::transcode(domError.getMessage());
            spdlog::error("%", msg);

            ep_srv_plugin_sf_dom_error err;
            err.set_msg(msg);
            add_error(err);

            xercesc::XMLString::release(&msg);

            // Instructs the serializer to continue serialization if possible.
            return true;
        }

        void ep_srv_plugin_sf_dom_error_handler::add_error(
            ep_srv_plugin_sf_dom_error e)
        {
            errors.push_back(e);
        }

        std::string reportParseException(const xercesc::SAXParseException& ex)
        {
            char* msg = xercesc::XMLString::transcode(ex.getMessage());
            auto msgstr = fmt::format("at line {} column {}, {}\n", ex.getColumnNumber(), ex.getLineNumber(), msg);
            xercesc::XMLString::release(&msg);

            return msgstr;
        }

        void ep_srv_plugin_sf_dom_error_handler::warning(
            const xercesc::SAXParseException& exc)
        {
            spdlog::warn("Warn: {}", reportParseException(exc));
        }

        void ep_srv_plugin_sf_dom_error_handler::error(
            const xercesc::SAXParseException& exc)
        {
            spdlog::error("Error: {}", reportParseException(exc));
        }

        void ep_srv_plugin_sf_dom_error_handler::fatalError(
            const xercesc::SAXParseException& exc)
        {
            spdlog::error("Fatal Error: {}", reportParseException(exc));
        }

        xercesc::InputSource* ep_srv_plugin_sf_entity_resolver::resolveEntity(
            const XMLCh* const publicId, const XMLCh* const systemId)
        {
            auto *pub_str = xercesc::XMLString::transcode(publicId);
            auto *sys_str = xercesc::XMLString::transcode(systemId);

            std::string pub_string;
            std::string sys_string;

            if(sys_str)
            {
                sys_string = sys_str;
            }

            if(pub_str)
            {
                pub_string = pub_str;
            }

            auto msgstr = fmt::format("publicId '{}'. systemId '{}'\n", pub_str ? pub_str : "(null)", 
                                        sys_str ? sys_str : "(null)");

            if( !sys_str )
            {
                spdlog::error("systemId is missing");
            }

            xercesc::XMLString::release(&pub_str);
            xercesc::XMLString::release(&sys_str);

            auto f = cxt_->get_catalog_xsd();
            auto p = boost::filesystem::path{f}.parent_path();

            std::string schema_path = fmt::format("{}/{}", p.generic_string(), sys_string);

            auto curr_source = std::make_shared<ep_srv_plugin_sf_entity_resolver_input>();
            auto schema_str = curr_source->get_source_string();

            auto        schema          = boost::filesystem::path{schema_path};
            auto        schema_path_obj = boost::filesystem::canonical(schema);
            boost::filesystem::load_string_file(schema_path_obj, *schema_str);

            schema_str->erase(std::remove(schema_str->begin(), schema_str->end(), '\r'), schema_str->end());

            // Parser owns and manages this.
            auto *curr_mem = new(xercesc::XMLPlatformUtils::fgMemoryManager)
                xercesc::MemBufInputSource((XMLByte*)schema_str->c_str(), 
                                            schema_str->size(), 
                                            sys_string.append(".xsd").c_str());

            curr_source->set_source_input(curr_mem);
           // xercesc::MemBufInputSource schema_buff((XMLByte*)schema_str.c_str(), schema_str.size(), "xsd");

            sources_.push_back(curr_source);

            return curr_mem;
        }

        
        ep_srv_plugin_sf_doc::ep_srv_plugin_sf_doc(
            std::shared_ptr<ep_srv_plugin_sf_context> cxt,
            std::unique_ptr<xercesc::XercesDOMParser> p)
        {
            parser_ = std::move(p);
        }
    }
} // namespace service
} // namespace easyprospect