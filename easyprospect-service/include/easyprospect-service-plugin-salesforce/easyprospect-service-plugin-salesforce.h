#pragma once

#include <easyprospect-service-plugin-salesforce/easyprospect-service-plugin-salesforce.h>
#include <easyprospect-service-plugin-salesforce/ep-srv-plugin-sf-context.h>

#include <easyprospect-web-worker/easyprospect-service-plugin.h>


#include "easyprospect-data-schema/easyprospect-data-schema.h"
#include "easyprospect-data/easyprospect-data.h"

namespace easyprospect
{
namespace service
{
    namespace plugin
    {
        enum class sf_item_type
        {
            NONE,
            PRODUCT,
            CATEGORY,
            CATEGORY_ASSIGNMENT,
            RECOMMEDATION,
            HEADER,
            CATALOG
        };

        class easyprospect_service_plugin_salesforce final
            : public easyprospect_service_plugin,
              public std::enable_shared_from_this <easyprospect_service_plugin_salesforce>
        {
          protected:
            struct make_shared_enabler
            {
                explicit make_shared_enabler(int){};
            };

          private:
            std::vector<std::shared_ptr<ep_srv_plugin_sf_context>> contexts_;

          public:
            easyprospect_service_plugin_salesforce(const make_shared_enabler&);
            // Global Javascript functions

            static std::shared_ptr<easyprospect_service_plugin_salesforce> get_instance();

            template <typename C, typename... T>
            static ::std::shared_ptr<C> create(T&&... args)
            {
                return ::std::make_shared<C>(make_shared_enabler{0}, ::std::forward<T>(args)...);
            }

            // TODO: register with a module
            static void register_plugin();

            // API.  Uses pointers to make this easy for V8
            ep_srv_plugin_sf_context *init_context_v8();
            std::shared_ptr<ep_srv_plugin_sf_context> init_context();

            static std::string read_str_v8(ep_srv_plugin_sf_context* cxt, std::string path_str);
            static std::string read_str(std::shared_ptr<ep_srv_plugin_sf_context> cxt, std::string path_str);

            static void        write_str_v8(ep_srv_plugin_sf_context* cxt, std::string content, std::string path_str);
            static void        write_str(
                       std::shared_ptr<ep_srv_plugin_sf_context> cxt,
                       std::string                               content,
                       std::string                               path_str);

            static ep_srv_plugin_sf_doc* parse_str_v8(
                ep_srv_plugin_sf_context* cxt,
                std::string               content,
                std::string               schema_path);

            static std::shared_ptr<ep_srv_plugin_sf_doc> parse_str(
                std::shared_ptr<ep_srv_plugin_sf_context> cxt,
                std::string content,
                std::string schema_path);

            static std::string str_doc_v8(ep_srv_plugin_sf_context* cxt, ep_srv_plugin_sf_doc* doc);
            static std::string str_doc(
                std::shared_ptr<ep_srv_plugin_sf_context> cxt,
                std::shared_ptr<ep_srv_plugin_sf_doc>     doc);

            static ep_srv_plugin_sf_dom_error_handler*      get_errors_v8(ep_srv_plugin_sf_context* cxt);
            static std::shared_ptr<ep_srv_plugin_sf_dom_error_handler> get_errors(
                std::shared_ptr<ep_srv_plugin_sf_context> cxt);

            static void transform_str_v8(
                ep_srv_plugin_sf_context* cxt,
                std::string               in_str,
                std::string               out_str,
                std::string               style_str);
            static void transform_str(
                std::shared_ptr<ep_srv_plugin_sf_context> cxt,
                std::string               in_str,
                std::string               out_str,
                std::string               style_str);

            static std::multimap<std::string, std::string> get_attribute(
                xercesc::DOMNode* node);

            template <typename B>
            static std::shared_ptr<typename std::enable_if<
                std::is_base_of<data::schema::salesforce::ep_sf_object_builder, B>::value,
                B>::type>
            new_parser_object(
                std::deque<std::shared_ptr<data::schema::salesforce::ep_sf_object_builder>>& parse_stack,
                bool&                                                                        pop_stack,
                std::shared_ptr<data::database::ep_sqlite>                                   db,
                std::shared_ptr<data::schema::salesforce::ep_sf_obj_import>                  imp);

            template <typename B>
            std::shared_ptr<typename std::enable_if<
                std::is_base_of<data::schema::salesforce::ep_sf_object_builder, B>::value,
                B>::type>
            new_parser_object(
                boost::optional<std::deque<std::shared_ptr<data::schema::salesforce::ep_sf_object_builder>>>&
                                                                            parse_stack,
                bool&                                                       pop_stack,
                std::shared_ptr<data::database::ep_sqlite>                  db,
                std::shared_ptr<data::schema::salesforce::ep_sf_obj_import> imp);

            template <typename B>
            static std::shared_ptr<typename std::enable_if<
                std::is_base_of<data::schema::salesforce::ep_sf_object_builder, B>::value,
                B>::type>
            new_parser_object(
                std::shared_ptr<std::deque<std::shared_ptr<data::schema::salesforce::ep_sf_object_builder>>>
                                                                            parse_stack,
                bool&                                                       pop_stack,
                std::shared_ptr<data::database::ep_sqlite>                  db,
                std::shared_ptr<data::schema::salesforce::ep_sf_obj_import> imp);

            template <class P, class B>
            static auto get_obj_parent(
                std::shared_ptr<data::schema::salesforce::ep_sf_object_builder>
                parent_builder,
                data::schema::salesforce::ep_sf_object_type
                expected_parent_type);

            static std::shared_ptr<data::schema::salesforce::ep_sf_object_builder> from_xml(
                std::shared_ptr<std::deque<std::shared_ptr<data::schema::salesforce::ep_sf_object_builder>>>
                                                                            parse_stack,
                xercesc::DOMNode*                                           node,
                std::shared_ptr<data::database::ep_sqlite>                  db,
                std::shared_ptr<data::schema::salesforce::ep_sf_obj_import> imp);

            static void sf_catalog_split2(
                std::shared_ptr<ep_srv_plugin_sf_context> cxt,
                std::shared_ptr<data::database::ep_sqlite> db,
                std::shared_ptr<data::schema::salesforce::ep_sf_obj_import> imp,
                std::string doc_str,
                std::string validate_xpath_templates,
                std::map<sf_item_type, int> element_page_size);

            // 1. Use DOMCount/DOMPrint example
            // 2. Page elements, creating a DOM tree for each page
            // 3. Every other element goes in FIRST/LAST/EVERY/NONE page as configured
            static void sf_catalog_split(
                std::shared_ptr<ep_srv_plugin_sf_context> cxt,
                std::string doc_str,
                std::string validate_xpath_templates, std::map<sf_item_type, int>
                element_page_size);
        };
    } // namespace plugin
} // namespace service
} // namespace easyprospect