#pragma once
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/optional/optional.hpp>
#include <chrono>
#include <string>
#include <vector>

#include "easyprospect-data/easyprospect-data.h"

namespace easyprospect
{
namespace data
{
    namespace schema
    {
        namespace salesforce
        {
            enum class ep_sf_tag_type
            {
                NONE = 0,
                ELEMENT_CATALOG  = 0,
                ELEMENT_CATEGORY = 1,
            };

            enum class ep_sf_object_type
            {
                NONE                        = 0,
                IMPORT                      = 1,
                RELATIONSHIP                = 2,
                SF_CATALOG                  = 3,
                SF_CATALOG_HEADER           = 4,
                SF_CATALOG_IMAGE_SETTINGS   = 5,
                SF_CATALOG_CUSTOM_ATTRIBUTE = 6,
                STRING                      = 7,
            };

            class ep_sf_obj_util
            {
              public:
                static ep_sf_tag_type str_to_tag(std::string name)
                {
                    auto res = ep_sf_tag_type::NONE;

                    if(name == "category")
                    {
                        res = ep_sf_tag_type::ELEMENT_CATEGORY;
                    }

                    return res;
                }

                
                template <typename C, typename... T>
                static std::shared_ptr<C> create(T&&... args)
                {
                    auto res = std::make_shared<C>(std::forward<T>(args)...);
                    return res;
                }
            };

            class ep_sf_obj_import_config final
            {
            private:
                const std::string db_host;
              std::shared_ptr<database::ep_sqlite_db> db;

            public:
              ep_sf_obj_import_config(std::string h) : db_host(h)
              {
                 db = std::make_shared<database::ep_sqlite_db>(db_host);
              }
            };

            class ep_sf_obj_import final
            {
              private:
                const uint64_t                              id_;
                const std::string                           label_;
                const std::chrono::system_clock::time_point timestamp_;

              public:
                ep_sf_obj_import() = delete;
                explicit ep_sf_obj_import(
                    const uint64_t                              id,
                    const std::string                           label,
                    const std::chrono::system_clock::time_point timestamp) :
                    id_(id),
                    label_(label), timestamp_(timestamp)
                {
                    
                }

                uint64_t get_id() const
                {
                    return id_;
                }

                std::string get_label() const
                {
                    return label_;
                }

                std::chrono::system_clock::time_point get_timestamp() const
                {
                    return timestamp_;
                }
            };

            class ep_sf_obj_import_builder final
            {
              private:
                uint64_t                              eso_id_;
                std::string                           label_;
                std::chrono::system_clock::time_point timestamp_;

              public:
                explicit ep_sf_obj_import_builder()
                {
                }

                void set_id(const uint64_t id)
                {
                    eso_id_ = id;
                }

                auto to_import()
                {
                    auto res = std::make_shared<ep_sf_obj_import>(eso_id_,label_,timestamp_);
                    return res;
                }
            };

            class ep_sf_object
            {
              private:
                const uint64_t          eso_id_;
                const ep_sf_object_type eso_type_;
                const int64_t           eso_import_id_;
                const std::string       eso_hash_;

              public:
                ep_sf_object() = delete;
                explicit ep_sf_object(const uint64_t id, const ep_sf_object_type type, 
                                       const int64_t import_id)
                : eso_id_(id), eso_type_(type), eso_import_id_(import_id)
                {
                    
                }

                uint64_t get_id() const
                {
                    return eso_id_;
                }

                ep_sf_object_type get_type() const 
                {
                    return eso_type_;
                }

                int64_t get_import_id() const
                {
                    return eso_import_id_;
                }

                std::string get_hash() const
                {
                    return eso_hash_;
                }
            };

            class ep_sf_object_builder
            {
              protected:
                uint64_t          eso_id_;
                ep_sf_object_type eso_type_;
                int64_t           eso_import_id_;
                std::string       eso_hash_;

              public:
                virtual ~ep_sf_object_builder() = default;

                ep_sf_object_builder()
                {
                    eso_id_ = 0;
                    eso_type_ = ep_sf_object_type::NONE;
                    eso_import_id_ = 0;
                    eso_hash_      = "";
                }

                void set_id(const uint64_t id)
                {
                    eso_id_ = id;
                }

                void set_type(const ep_sf_object_type type)
                {
                    eso_type_ = type;
                }

                void set_import_id(const uint64_t id)
                {
                    eso_import_id_ = id;
                }

                void gen_hash()
                {
                    eso_hash_ = "";
                }

                virtual std::shared_ptr<ep_sf_object> to_object() const;

                void save(std::shared_ptr<ep_sf_obj_import_config> conf) const;
            };

            class ep_sf_object_relationship final
            {
              private:
                // Compound key
                const uint64_t id_;
                const int64_t  variable_id_; // for 1-to-many relationships

                const int64_t  import_id_;
                const int64_t  order_;

                const uint64_t          src_id_;
                const uint64_t          dst_id_;
                const ep_sf_object_type dst_type_;
                const ep_sf_object_type src_type_;

                // Sometimes an inline value is best for performance
                const int64_t     dst_int_inline_value_;
                const float       dst_float_inline_value_;
                const std::string dst_str_inline_value_;

              public:
                ep_sf_object_relationship() = delete;
                explicit ep_sf_object_relationship(const uint64_t id, 
                                                    const int64_t import_id,
                    const int64_t  variable_id,
                    const int64_t  order,
                    const int64_t  src_id,
                    const int64_t           dst_id,
                    const ep_sf_object_type dst_type,
                    const ep_sf_object_type src_type,
                    const int64_t           dst_int_inline_value,
                    const float             dst_float_inline_value,
                    const std::string       dst_str_inline_value) :
                    id_(id),variable_id_(variable_id),
                    import_id_(import_id),  order_(order), src_id_(src_id), dst_id_(dst_id),
                    dst_type_(dst_type), src_type_(src_type), dst_int_inline_value_(dst_int_inline_value), 
                    dst_float_inline_value_(dst_float_inline_value), dst_str_inline_value_(dst_str_inline_value)
                {
                    
                }

                uint64_t get_id() const
                {
                    return id_;
                }

                int64_t get_import_id() const
                {
                    return import_id_;
                }

                int64_t get_variable_id() const
                {
                    return variable_id_;
                }

                int64_t get_order() const
                {
                    return order_;
                }

                int64_t get_src_id() const
                {
                    return src_id_;
                }

                int64_t get_dst_id() const
                {
                    return dst_id_;
                }

                ep_sf_object_type get_src_type() const
                {
                    return src_type_;
                }

                ep_sf_object_type get_dst_type() const
                {
                    return dst_type_;
                }
            };

            class ep_sf_object_relationship_builder final
            {
              private:
                // Compound key
                uint64_t id_;
                int64_t  variable_id_; // for 1-to-many relationships

                int64_t import_id_;
                int64_t order_;

                uint64_t          src_id_;
                uint64_t          dst_id_;
                ep_sf_object_type dst_type_;
                ep_sf_object_type src_type_;

                // Sometimes an inline value is best for performance
                int64_t     dst_int_inline_value_;
                float       dst_float_inline_value_;
                std::string dst_str_inline_value_;

            public:
                ep_sf_object_relationship_builder()
                {
                    id_ = 0;
                    variable_id_ = 0;
                }

                void set_id(uint64_t id)
                {
                    id_ = id;
                }

                void set_variable_id(int64_t variable_id)
                {
                    variable_id_ =  variable_id;
                }

                void set_import_id(const int64_t id)
                {
                    import_id_ = id;
                }

                void set_dst_type(const ep_sf_object_type type)
                {
                    dst_type_ = type;
                }

                void set_src_type(const ep_sf_object_type type)
                {
                    src_type_ = type;
                }
            };

            class ep_sf_obj_catalog final : public ep_sf_object
            {
              private:
                const boost::optional<uint64_t> header_;
                const boost::optional<uint64_t> product_attribute_definitions_;
                const boost::optional<uint64_t> category_;
                const boost::optional<uint64_t> product_;
                const boost::optional<uint64_t> product_option_;
                const boost::optional<uint64_t> variation_attribute_;
                const boost::optional<uint64_t> category_assignment_;
                const boost::optional<uint64_t> recommendation_;

                const boost::optional<std::string> catalog_id_;

              public:
                explicit ep_sf_obj_catalog(const uint64_t id, const ep_sf_object_type type, const int64_t import_id,
                    const boost::optional<uint64_t> header_,                        
                    const boost::optional<uint64_t> product_attribute_definitions_,                        
                    const boost::optional<uint64_t> category_,                        
                    const boost::optional<uint64_t> product_,                        
                    const boost::optional<uint64_t> product_option_,                        
                    const boost::optional<uint64_t> variation_attribute_,                        
                    const boost::optional<uint64_t> category_assignment_,                        
                    const boost::optional<uint64_t> recommendation_,                        
                    const boost::optional<std::string> catalog_id) :
                    ep_sf_object(id, type, import_id),
                    catalog_id_(catalog_id)
                {
                    
                }
            };

            class ep_sf_obj_catalog_builder final : public ep_sf_object_builder
            {
              private:
                boost::optional<uint64_t> header_;
                boost::optional<uint64_t> product_attribute_definitions_;
                boost::optional<uint64_t> category_;
                boost::optional<uint64_t> product_;
                boost::optional<uint64_t> product_option_;
                boost::optional<uint64_t> variation_attribute_;
                boost::optional<uint64_t> category_assignment_;
                boost::optional<uint64_t> recommendation_;

                boost::optional<std::string> catalog_id_;

              public:
                explicit ep_sf_obj_catalog_builder() : ep_sf_object_builder()
                {
                    header_                        = boost::none;
                    product_attribute_definitions_ = boost::none;
                    category_                      = boost::none;
                    product_                       = boost::none;
                    product_option_                = boost::none;
                    variation_attribute_           = boost::none;
                    category_assignment_           = boost::none;
                    recommendation_                = boost::none;

                    eso_type_ = ep_sf_object_type::SF_CATALOG;
                }

                void set_header(boost::optional<uint64_t> val)
                {
                    header_ = val;
                }

                void set_product_attribute_definitions(boost::optional<uint64_t> val)
                {
                    product_attribute_definitions_ = val;
                }

                void set_category(boost::optional<uint64_t> val)
                {
                    category_ = val;
                }

                void set_product(boost::optional<uint64_t> val)
                {
                    product_ = val;
                }

                void set_product_option(boost::optional<uint64_t> val)
                {
                    product_option_ = val;
                }

                void set_variation_attribute(boost::optional<uint64_t> val)
                {
                    variation_attribute_ = val;
                }

                void set_category_assignment(boost::optional<uint64_t> val)
                {
                    category_assignment_ = val;
                }

                void set_recommendation(boost::optional<uint64_t> val)
                {
                    recommendation_ = val;
                }

                auto to_catalog() const;
            };

            class ep_sf_obj_catalog_header final : public ep_sf_object
            {
              private:
                boost::optional<uint64_t> image_settings_;
                boost::optional<uint64_t> custom_attributes_;

              public:
                explicit ep_sf_obj_catalog_header(
                    const uint64_t          id,
                    const ep_sf_object_type type,
                    const int64_t           import_id) :
                    ep_sf_object(id, type, import_id)
                {
                }
            };

            class ep_sf_obj_catalog_header_builder final : public ep_sf_object_builder
            {
              private:
                boost::optional<uint64_t> image_settings_;
                boost::optional<uint64_t> custom_attributes_;

              public:
                explicit ep_sf_obj_catalog_header_builder()
                {
                    image_settings_ = boost::none;
                    custom_attributes_ = boost::none;
                }

                void set_image_settings(boost::optional<uint64_t> val)
                {
                    image_settings_ = val;
                }

                void custom_attributes(boost::optional<uint64_t> val)
                {
                    custom_attributes_ = val;
                }

                auto to_catalog_header() const;
            };

            class ep_sf_obj_catalog_header_image_settings final : public ep_sf_object
            {
              private:
                boost::optional<uint64_t> internal_location_d;
                boost::optional<uint64_t> external_location_;
                boost::optional<uint64_t> view_types_;

                boost::optional<std::string> variation_attribute_id_;
                boost::optional<std::string> alt_pattern_;
                boost::optional<std::string> title_pattern_;

              public:
                explicit ep_sf_obj_catalog_header_image_settings(
                    const uint64_t          id,
                    const ep_sf_object_type type,
                    const int64_t           import_id) :
                    ep_sf_object(id, type, import_id)
                {
                }
            };

            class ep_sf_obj_catalog_header_image_settings_builder final : public ep_sf_object_builder
            {
              private:
                boost::optional<uint64_t> internal_location_;
                boost::optional<uint64_t> external_location_;
                boost::optional<uint64_t> view_types_;

                boost::optional<std::string> variation_attribute_id_;
                boost::optional<std::string> alt_pattern_;
                boost::optional<std::string> title_pattern_;

              public:
                explicit ep_sf_obj_catalog_header_image_settings_builder()
                {
                }

                void set_internal_location(boost::optional<uint64_t> val)
                {
                    internal_location_ = val;
                }

                void set_external_location(boost::optional<uint64_t> val)
                {
                   external_location_ = val;
                }

                void set_view_types(boost::optional<uint64_t> val)
                {
                    view_types_ = val;
                }

                void set_variation_attribute_id(boost::optional<std::string> val)
                {
                    variation_attribute_id_= val;
                }

                void set_alt_pattern(boost::optional<std::string> val)
                {
                    alt_pattern_ = val;
                }

                void set_title_pattern(boost::optional<std::string> val)
                {
                    title_pattern_ = val;
                }

                auto to_catalog_header_image_settings() const;

            };

            class ep_sf_obj_catalog_header_image_internal_location final : public ep_sf_object
            {
              private:
                boost::optional<std::string> base_path_;

              public:
                explicit ep_sf_obj_catalog_header_image_internal_location(
                    const uint64_t          id,
                    const ep_sf_object_type type,
                    const int64_t           import_id) :
                    ep_sf_object(id, type, import_id)
                {
                }
            };

            class ep_sf_obj_catalog_header_image_internal_location_builder final : public ep_sf_object_builder
            {
              private:
                boost::optional<std::string> base_path_;
                
              public:
                explicit ep_sf_obj_catalog_header_image_internal_location_builder()
                {
                    base_path_ = boost::none;
                }

                void set_base_path(boost::optional<std::string> val)
                {
                    base_path_ = val;
                }

                auto
                to_catalog_header_image_internal_location()
                const;

            };

            class ep_sf_obj_catalog_header_image_external_location final : public ep_sf_object
            {
              private:
                boost::optional<std::string> http_url_;
                boost::optional<std::string> https_url_;

              public:
                explicit ep_sf_obj_catalog_header_image_external_location(
                    const uint64_t          id,
                    const ep_sf_object_type type,
                    const int64_t           import_id) :
                    ep_sf_object(id, type, import_id)
                {
                }
            };

            class ep_sf_obj_catalog_header_image_external_location_builder final : public ep_sf_object_builder
            {
              private:
                boost::optional<std::string> http_url_;
                boost::optional<std::string> https_url_;

              public:
                explicit ep_sf_obj_catalog_header_image_external_location_builder()
                {
                    http_url_ = boost::none;
                    https_url_ = boost::none;
                }

                void set_http_url(boost::optional<std::string> val)
                {
                    http_url_ = val;
                }

                void set_https_url(boost::optional<std::string> val)
                {
                    https_url_ = val;
                }

                auto
                to_catalog_header_image_external_location() const;
            };

            class ep_sf_obj_catalog_header_image_view_types final : public ep_sf_object
            {
              private:
                boost::optional<std::string> view_type_;

              public:
                explicit ep_sf_obj_catalog_header_image_view_types(
                    const uint64_t          id,
                    const ep_sf_object_type type,
                    const int64_t           import_id) :
                    ep_sf_object(id, type, import_id)
                {
                }
            };

            class ep_sf_obj_catalog_header_image_view_types_builder final : public ep_sf_object_builder
            {
              private:
                boost::optional<std::string> view_type_;

              public:
                explicit ep_sf_obj_catalog_header_image_view_types_builder()
                {
                    
                }

                void set_view_type(boost::optional<std::string> val)
                {
                    view_type_ = val;
                }

                auto
                to_catalog_header_image_view_types() const;
            };

            class ep_sf_obj_catalog_custom_attributes final : public ep_sf_object
            {
              private:
                boost::optional<uint64_t> custom_attribute_;

              public:
                explicit ep_sf_obj_catalog_custom_attributes(
                    const uint64_t          id,
                    const ep_sf_object_type type,
                    const int64_t           import_id) :
                    ep_sf_object(id, type, import_id)
                {
                }
            };

            class ep_sf_obj_catalog_custom_attributes_builder final : public ep_sf_object_builder
            {
              private:
                boost::optional<uint64_t> custom_attribute_;

              public:
                explicit ep_sf_obj_catalog_custom_attributes_builder()
                {
                    custom_attribute_ = boost::none;
                }

                void set_custom_attribute(boost::optional<uint64_t> val)
                {
                    custom_attribute_ = val;
                }

                auto to_catalog_custom_attributes() const;
            };

            class ep_sf_obj_catalog_custom_attribute final : public ep_sf_object
            {
              private:
                boost::optional<std::string>              attribute_id_;
                boost::optional<std::string>              xml_lang_;
                boost::optional<uint64_t>                 value_;

              public:
                explicit ep_sf_obj_catalog_custom_attribute(
                    const uint64_t          id,
                    const ep_sf_object_type type,
                    const int64_t           import_id) :
                    ep_sf_object(id, type, import_id)
                {
                }
            };

            class ep_sf_obj_catalog_custom_attribute_builder final : public ep_sf_object_builder
            {
              private:
                boost::optional<std::string> attribute_id_;
                boost::optional<std::string> xml_lang_;
                boost::optional<uint64_t>    value_;

              public:
                explicit ep_sf_obj_catalog_custom_attribute_builder()
                {
                    attribute_id_ = boost::none;
                    xml_lang_     = boost::none;
                    value_        = boost::none;
                }

                void set_attribute_id(boost::optional<std::string> val)
                {
                    attribute_id_ = val;
                }

                void set_xml_lang(boost::optional<std::string> val)
                {
                    xml_lang_ = val;
                }

                void set_value(boost::optional<uint64_t> val)
                {
                    value_ = val;
                }

                auto to_catalog_custom_attribute() const;

            };

            class ep_sf_catalog_category_link final : public ep_sf_object
            {
              private:
                boost::optional<std::string> category_id_;
                boost::optional<std::string> catalog_id_;

              public:
                explicit ep_sf_catalog_category_link(
                    const uint64_t          id,
                    const ep_sf_object_type type,
                    const int64_t           import_id) :
                    ep_sf_object(id, type, import_id)
                {
                }
            };

            class ep_sf_catalog_category_link_builder final : public ep_sf_object_builder
            {
              private:
                boost::optional<std::string> category_id_;
                boost::optional<std::string> catalog_id_;

              public:
                explicit ep_sf_catalog_category_link_builder()
                {
                    catalog_id_ = boost::none;
                    category_id_ = boost::none;
                }

                void set_category_id(boost::optional<std::string> val)
                {
                    category_id_ = val;
                }

                void set_catalog_id(boost::optional<std::string> val)
                {
                    catalog_id_ = val;
                }

                auto to_catalog_category_link() const;

            };

            class ep_sf_catalog_localized_string final : public ep_sf_object
            {
              private:
                boost::optional<std::string> value_;
                boost::optional<std::string> xml_lang_;

              public:
                explicit ep_sf_catalog_localized_string(
                    const uint64_t          id,
                    const ep_sf_object_type type,
                    const int64_t           import_id) :
                    ep_sf_object(id, type, import_id)
                {
                }
            };

            class ep_sf_catalog_localized_string_builder final : public ep_sf_object_builder
            {
              private:
                boost::optional<std::string> value_;
                boost::optional<std::string> xml_lang_;

              public:
                explicit ep_sf_catalog_localized_string_builder()
                {
                    value_ = boost::none;
                    xml_lang_ = boost::none;
                }

                void set_value(boost::optional<std::string> val)
                {
                    value_ = val;
                }

                void set_xml_lang(boost::optional<std::string> val)
                {
                    xml_lang_ = val;
                }

                auto to_catalog_localized_string() const;
                
            };

            class ep_sf_catalog_page_attributes final : public ep_sf_object
            {
              private:
                boost::optional<uint64_t> page_title_;
                boost::optional<uint64_t> page_description_;
                boost::optional<uint64_t> page_keywords_;
                boost::optional<uint64_t> page_url_;

              public:
                explicit ep_sf_catalog_page_attributes(
                    const uint64_t          id,
                    const ep_sf_object_type type,
                    const int64_t           import_id) :
                    ep_sf_object(id, type, import_id)
                {
                }
            };

            class ep_sf_catalog_page_attributes_builder final : public ep_sf_object_builder
            {
              private:
                boost::optional<uint64_t> page_title_;
                boost::optional<uint64_t> page_description_;
                boost::optional<uint64_t> page_keywords_;
                boost::optional<uint64_t> page_url_;

              public:
                explicit ep_sf_catalog_page_attributes_builder()
                {
                    page_title_ = boost::none;
                    page_description_ = boost::none;
                    page_keywords_ = boost::none;
                    page_url_ = boost::none;
                }

                void set_page_title(boost::optional<uint64_t> val)
                {
                    page_title_ = val;
                }

                void set_description(boost::optional<uint64_t> val)
                {
                    page_description_ = val;
                }

                void set_keywords(boost::optional<uint64_t> val)
                {
                    page_keywords_ = val;
                }

                void set_page_url(boost::optional<uint64_t> val)
                {
                    page_url_ = val;
                }

                auto to_catalog_page_attributes() const;

            };

            class ep_sf_catalog_attribute_reference final : public ep_sf_object
            {
              private:
                boost::optional<std::string> attribute_id_;
                boost::optional<bool>        system_;

              public:
                explicit ep_sf_catalog_attribute_reference(
                    const uint64_t          id,
                    const ep_sf_object_type type,
                    const int64_t           import_id) :
                    ep_sf_object(id, type, import_id)
                {
                }
            };

            class ep_sf_catalog_attribute_reference_builder final : public ep_sf_object_builder
            {
              private:
                boost::optional<std::string> attribute_id_;
                boost::optional<bool>        system_;

              public:
                explicit ep_sf_catalog_attribute_reference_builder()
                {
                    attribute_id_ = boost::none;
                    system_       = boost::none;
                }

                void set_attribute_id(boost::optional<std::string> val)
                {
                    attribute_id_ = val;
                }

                void set_system(boost::optional<bool> val)
                {
                    system_ = val;
                }

                auto to_catalog_attribute_reference() const;

            };

            class ep_sf_catalog_attribute_group final : public ep_sf_object
            {
              private:
                boost::optional<uint64_t> display_name_;
                boost::optional<uint64_t> description_;
                boost::optional<uint64_t> variable_attribute_;

                boost::optional<std::string> group_id_;

              public:
                explicit ep_sf_catalog_attribute_group(
                    const uint64_t          id,
                    const ep_sf_object_type type,
                    const int64_t           import_id) :
                    ep_sf_object(id, type, import_id)
                {
                }
            };

            class ep_sf_catalog_attribute_group_builder final : public ep_sf_object_builder
            {
              private:
                boost::optional<uint64_t> display_name_;
                boost::optional<uint64_t> description_;
                boost::optional<uint64_t> variable_attribute_;

                boost::optional<std::string> group_id_;

              public:
                explicit ep_sf_catalog_attribute_group_builder()
                {
                    display_name_ = boost::none;
                    description_  = boost::none;
                    variable_attribute_ = boost::none;
                }

                void set_display_name(boost::optional<uint64_t> val)
                {
                    display_name_ = val;
                }

                void set_description(boost::optional<uint64_t> val)
                {
                    description_ = val;
                }

                void set_variable_attribute(boost::optional<uint64_t> val)
                {
                    variable_attribute_ = val;
                }

                auto to_catalog_attribute_group() const;

            };

            class ep_sf_catalog_refinement_definitions final : public ep_sf_object
            {
              private:
                boost::optional<uint64_t> refinement_definition_;
                boost::optional<uint64_t> blocked_refinement_definition_;

              public:
                explicit ep_sf_catalog_refinement_definitions(
                    const uint64_t          id,
                    const ep_sf_object_type type,
                    const int64_t           import_id) :
                    ep_sf_object(id, type, import_id)
                {
                }
            };

            class ep_sf_catalog_refinement_definitions_builder final : public ep_sf_object_builder
            {
              private:
                boost::optional<uint64_t> refinement_definition_;
                boost::optional<uint64_t> blocked_refinement_definition_;

              public:
                explicit ep_sf_catalog_refinement_definitions_builder()
                {
                    refinement_definition_ = boost::none;
                    blocked_refinement_definition_ = boost::none;
                }

                void set_refinement_definition(boost::optional<uint64_t> val)
                {
                    refinement_definition_ = val;
                }

                void set_blocked_refinement_definition(boost::optional<uint64_t> val)
                {
                    blocked_refinement_definition_ = val;
                }

                auto to_catalog_refinement_definitions() const;

            };

            class ep_sf_catalog_attribute_refinement_bucket final : public ep_sf_object
            {
              private:
                boost::optional<uint64_t>    display_name_;
                boost::optional<uint64_t>    value_list_;
                boost::optional<std::string> presentation_id_;
                boost::optional<uint64_t>    description_;

                boost::optional<bool> default_;

              public:
                explicit ep_sf_catalog_attribute_refinement_bucket(
                    const uint64_t          id,
                    const ep_sf_object_type type,
                    const int64_t           import_id) :
                    ep_sf_object(id, type, import_id)
                {
                }
            };

            class ep_sf_catalog_attribute_refinement_bucket_builder final : public ep_sf_object_builder
            {
              private:
                boost::optional<uint64_t>    display_name_;
                boost::optional<uint64_t>    value_list_;
                boost::optional<std::string> presentation_id_;
                boost::optional<uint64_t>    description_;

                boost::optional<bool> default_;

              public:
                explicit ep_sf_catalog_attribute_refinement_bucket_builder()
                {
                    display_name_ = boost::none;
                    value_list_   = boost::none;
                    presentation_id_ = boost::none;
                    description_     = boost::none;
                }

                void set_display_name(boost::optional<uint64_t> val)
                {
                    display_name_ = val;
                }

                void set_value_list(boost::optional<uint64_t> val)
                {
                    value_list_ = val;
                }

                void set_presentation_id(boost::optional<std::string> val)
                {
                    presentation_id_ = val;
                }

                void set_description(boost::optional<uint64_t> val)
                {
                    description_ = val;
                }

                auto to_catalog_attribute_refinement() const;

            };

            class ep_sf_catalog_price_refinement_bucket final : public ep_sf_object
            {
              private:
                boost::optional<uint64_t> display_name_;
                boost::optional<double>   threshold_;

                boost::optional<std::string> currency_;

               public:
                explicit ep_sf_catalog_price_refinement_bucket(
                    const uint64_t          id,
                    const ep_sf_object_type type,
                    const int64_t           import_id) :
                    ep_sf_object(id, type, import_id)
                {

                }
            };

            class ep_sf_catalog_price_refinement_bucket_builder final : public ep_sf_object_builder
            {
              private:
                boost::optional<uint64_t> display_name_;
                boost::optional<double>   threshold_;
                boost::optional<std::string> currency_;

              public:
                explicit ep_sf_catalog_price_refinement_bucket_builder()
                {
                    display_name_ = boost::none;
                    threshold_    = boost::none;
                    currency_     = boost::none;
                }

                void set_display_name(boost::optional<uint64_t> val)
                {
                    display_name_ = val;
                }

                void set_threshold(boost::optional<double> val)
                {
                    threshold_ = val;
                }

                void set_currency(boost::optional<std::string> val)
                {
                    currency_ = val;
                }

                auto to_catalog_price_refinement_bucket() const;

            };

            class ep_sf_catalog_threshold_refinement_bucket final : public ep_sf_object
            {
              private:
                boost::optional<uint64_t>    display_name_;
                boost::optional<uint64_t>    threshold_;
                boost::optional<std::string> presentation_id_;
                boost::optional<uint64_t>    description_;

              public:
                explicit ep_sf_catalog_threshold_refinement_bucket(
                    const uint64_t          id,
                    const ep_sf_object_type type,
                    const int64_t           import_id) :
                    ep_sf_object(id, type, import_id)
                {
                }
            };

            class ep_sf_catalog_threshold_refinement_bucket_builder final : public ep_sf_object_builder
            {
              private:
                boost::optional<uint64_t>    display_name_;
                boost::optional<uint64_t>    threshold_;
                boost::optional<std::string> presentation_id_;
                boost::optional<uint64_t>    description_;

              public:
                explicit ep_sf_catalog_threshold_refinement_bucket_builder()
                {
                    display_name_ = boost::none;
                    threshold_    = boost::none;
                    presentation_id_ = boost::none;
                    description_     = boost::none;
                }

                void set_display_name(boost::optional<uint64_t> val)
                {
                    display_name_ = val;
                }

                void set_threshold(boost::optional<uint64_t> val)
                {
                    threshold_ = val;
                }

                void set_presentation_id(boost::optional<std::string> val)
                {
                    presentation_id_ = val;
                }

                void set_description(boost::optional<uint64_t> val)
                {
                    description_ = val;
                }

                auto to_catalog_threshold_refinement_bucket()
                const;
            };

            class ep_sf_catalog_period_refinement_bucket final : public ep_sf_object
            {
              private:
                boost::optional<uint64_t>    display_name_;
                boost::optional<int>         duration_in_days_;
                boost::optional<std::string> presentation_id_;
                boost::optional<uint64_t>    description_;

              public:
                explicit ep_sf_catalog_period_refinement_bucket(
                    const uint64_t          id,
                    const ep_sf_object_type type,
                    const int64_t           import_id) :
                    ep_sf_object(id, type, import_id)
                {
                }
            };

            class ep_sf_catalog_period_refinement_bucket_builder final : public ep_sf_object_builder
            {
              private:
                boost::optional<uint64_t>    display_name_;
                boost::optional<int>         duration_in_days_;
                boost::optional<std::string> presentation_id_;
                boost::optional<uint64_t>    description_;

              public:
                explicit ep_sf_catalog_period_refinement_bucket_builder()
                {
                    display_name_ = boost::none;
                    duration_in_days_ = boost::none;
                    presentation_id_  = boost::none;
                    description_      = boost::none;
                }

                void set_display_name(boost::optional<uint64_t> val)
                {
                    display_name_ = val;
                }

                void set_duration_in_days(boost::optional<int> val)
                {
                    duration_in_days_ = val;
                }

                void set_presentation_id(boost::optional<std::string> val)
                {
                    presentation_id_ = val;
                }

                void set_description(boost::optional<uint64_t> val)
                {
                    description_ = val;
                }

                std::shared_ptr<ep_sf_catalog_period_refinement_bucket> to_catalog_period_refinement_bucket() const;

            };

            class ep_sf_catalog_refinement_buckets final : public ep_sf_object
            {
              private:
                boost::optional<uint64_t> attribute_bucket_;
                boost::optional<uint64_t> price_bucket_;
                boost::optional<uint64_t> threshold_bucket_;
                boost::optional<uint64_t> period_bucket_;

              public:
                explicit ep_sf_catalog_refinement_buckets(
                    const uint64_t          id,
                    const ep_sf_object_type type,
                    const int64_t           import_id) :
                    ep_sf_object(id, type, import_id)
                {
                }
            };

            class ep_sf_catalog_refinement_buckets_builder final : public ep_sf_object_builder
            {
              private:
                boost::optional<uint64_t> attribute_bucket_;
                boost::optional<uint64_t> price_bucket_;
                boost::optional<uint64_t> threshold_bucket_;
                boost::optional<uint64_t> period_bucket_;

              public:
                explicit ep_sf_catalog_refinement_buckets_builder()
                {
                    attribute_bucket_ = boost::none;
                    price_bucket_     = boost::none;
                    threshold_bucket_ = boost::none;
                    period_bucket_    = boost::none;
                }

                void set_attribute_bucket(boost::optional<uint64_t> val)
                {
                    attribute_bucket_ = val;
                }

                void set_price_bucket(boost::optional<uint64_t> val)
                {
                    price_bucket_ = val;
                }

                void set_threshold_bucket(boost::optional<uint64_t> val)
                {
                    threshold_bucket_ = val;
                }

                void set_period_bucket(boost::optional<uint64_t> val)
                {
                    period_bucket_ = val;
                }

                auto to_catalog_refinement_buckets() const;
            };

            class ep_sf_catalog_refinement_definition final : public ep_sf_object
            {
              private:
                boost::optional<uint64_t>    display_name_;
                boost::optional<std::string> value_set_;
                boost::optional<std::string> sort_mode_;
                boost::optional<std::string> sort_direction_;
                boost::optional<std::string> unbucketed_values_mode_;
                boost::optional<int>         cutoff_threshold_;
                boost::optional<uint64_t>    bucket_definitions_;

                boost::optional<std::string> type_;
                boost::optional<std::string> bucket_type_;
                boost::optional<std::string> attribute_id_;
                boost::optional<bool>        system_;

              public:
                explicit ep_sf_catalog_refinement_definition(
                    const uint64_t          id,
                    const ep_sf_object_type type,
                    const int64_t           import_id) :
                    ep_sf_object(id, type, import_id)
                {
                }
            };

            class ep_sf_catalog_refinement_definition_builder final : public ep_sf_object_builder
            {
              private:
                boost::optional<uint64_t>    display_name_;
                boost::optional<std::string> value_set_;
                boost::optional<std::string> sort_mode_;
                boost::optional<std::string> sort_direction_;
                boost::optional<std::string> unbucketed_values_mode_;
                boost::optional<int>         cutoff_threshold_;
                boost::optional<uint64_t>    bucket_definitions_;

                boost::optional<std::string> type_;
                boost::optional<std::string> bucket_type_;
                boost::optional<std::string> attribute_id_;
                boost::optional<bool>        system_;

              public:
                explicit ep_sf_catalog_refinement_definition_builder()
                {
                    display_name_ = boost::none;
                    value_set_    = boost::none;
                    sort_mode_    = boost::none;
                    sort_direction_ = boost::none;
                    unbucketed_values_mode_ = boost::none;
                    cutoff_threshold_       = boost::none;
                    bucket_definitions_     = boost::none;

                    type_ = boost::none;
                    bucket_type_ = boost::none;
                    attribute_id_ = boost::none;
                    system_ = boost::none;
                }

                void set_display_name(boost::optional<uint64_t> val)
                {
                    display_name_ = val;
                }

                void set_value_set(boost::optional<std::string> val)
                {
                    value_set_ = val;
                }

                void set_sort_mode(boost::optional<std::string> val)
                {
                    sort_mode_ = val;
                }

                void set_sort_direction(boost::optional<std::string> val)
                {
                    sort_direction_ = val;
                }

                void set_unbucketed_values_mode(boost::optional<std::string> val)
                {
                    unbucketed_values_mode_ = val;
                }

                void set_cutoff_threshold(boost::optional<int> val)
                {
                    cutoff_threshold_ = val;
                }

                void set_bucket_definitions(boost::optional<uint64_t> val)
                {
                    bucket_definitions_ = val;
                }

                void set_type(boost::optional<std::string> val)
                {
                    type_ = val;
                }

                void set_bucket_type(boost::optional<std::string> val)
                {
                    bucket_type_ = val;
                }

                void set_attribute_id(boost::optional<std::string> val)
                {
                    attribute_id_ = val;
                }

                void set_system(boost::optional<bool> val)
                {
                    system_ = val;
                }

                auto to_catalog_refinement_definition() const;
            };

            class ep_sf_catalog_product_brand_filter final : public ep_sf_object
            {
              private:
                boost::optional<uint64_t>    brand_;
                boost::optional<std::string> operator_;

              public:
                explicit ep_sf_catalog_product_brand_filter(
                    const uint64_t          id,
                    const ep_sf_object_type type,
                    const int64_t           import_id) :
                    ep_sf_object(id, type, import_id)
                {
                }
            };

            class ep_sf_catalog_product_brand_filter_builder final : public ep_sf_object_builder
            {
              private:
                boost::optional<uint64_t>    brand_;
                boost::optional<std::string> operator_;

              public:
                explicit ep_sf_catalog_product_brand_filter_builder()
                {
                    brand_ = boost::none;
                    operator_ = boost::none;
                }

                void set_brand(boost::optional<uint64_t> val)
                {
                    brand_ = val;
                }

                void set_operator(boost::optional<std::string> val)
                {
                    operator_ = val;
                }

                auto to_catalog_product_brand_filter() const;
            };

            class ep_sf_catalog_product_id_filter final : public ep_sf_object
            {
              private:
                boost::optional<uint64_t>    product_id_;
                boost::optional<std::string> operator_;

              public:
                explicit ep_sf_catalog_product_id_filter(
                    const uint64_t          id,
                    const ep_sf_object_type type,
                    const int64_t           import_id) :
                    ep_sf_object(id, type, import_id)
                {
                }
            };

            class ep_sf_catalog_product_id_filter_builder final : public ep_sf_object_builder
            {
              private:
                boost::optional<uint64_t>    product_id_;
                boost::optional<std::string> operator_;

              public:
                explicit ep_sf_catalog_product_id_filter_builder()
                {
                    product_id_ = boost::none;
                    operator_   = boost::none;
                }

                auto to_catalog_product_id_filter() const;
            };

            class ep_sf_catalog_product_category_filter final : public ep_sf_object
            {
              private:
                boost::optional<uint64_t>    category_id_;
                boost::optional<std::string> operator_;
                boost::optional<std::string> catalog_id_;

              public:
                explicit ep_sf_catalog_product_category_filter(
                    const uint64_t          id,
                    const ep_sf_object_type type,
                    const int64_t           import_id) :
                    ep_sf_object(id, type, import_id)
                {
                }
            };

            class ep_sf_catalog_product_category_filter_builder final : public ep_sf_object_builder
            {
              private:
                boost::optional<uint64_t>    category_id_;
                boost::optional<std::string> operator_;
                boost::optional<std::string> catalog_id_;

              public:
                explicit ep_sf_catalog_product_category_filter_builder()
                {
                    category_id_ = boost::none;
                    operator_   = boost::none;
                    catalog_id_ = boost::none;
                }

                void set_category_id(boost::optional<uint64_t> val)
                {
                    category_id_ = val;
                }

                void set_operator(boost::optional<std::string> val)
                {
                    operator_ = val;
                }

                void set_catalog_id(boost::optional<std::string> val)
                {
                    catalog_id_ = val;
                }

                auto to_catalog_product_category_filter() const;

            };

            class ep_sf_catalog_product_attribute_filter final : public ep_sf_object
            {
              private:
                boost::optional<uint64_t>    attribute_value_;
                boost::optional<std::string> operator_;
                boost::optional<std::string> catalog_id_;

              public:
                explicit ep_sf_catalog_product_attribute_filter(
                    const uint64_t          id,
                    const ep_sf_object_type type,
                    const int64_t           import_id) :
                    ep_sf_object(id, type, import_id)
                {
                }
            };

            class ep_sf_catalog_product_attribute_filter_builder final : public ep_sf_object_builder
            {
              private:
                boost::optional<uint64_t>    attribute_value_;
                boost::optional<std::string> operator_;
                boost::optional<std::string> catalog_id_;

              public:
                explicit ep_sf_catalog_product_attribute_filter_builder()
                {
                    attribute_value_ = boost::none;
                    operator_        = boost::none;
                    catalog_id_      = boost::none;
                }

                void set_attribute_value(boost::optional<uint64_t> val)
                {
                    attribute_value_ = val;
                }

                void set_operator_value(boost::optional<std::string> val)
                {
                    operator_ = val;
                }

                void set_catalog_id(boost::optional<std::string> val)
                {
                    catalog_id_ = val;
                }

                auto to_catalog_product_attribute_filter() const;

            };

            class ep_sf_catalog_product_specification_condition_group final : public ep_sf_object
            {
              private:
                boost::optional<uint64_t> brand_condition_;
                boost::optional<uint64_t> product_id_condition_;
                boost::optional<uint64_t> category_condition_;
                boost::optional<uint64_t> attribute_condition_;

              public:
                explicit ep_sf_catalog_product_specification_condition_group(
                    const uint64_t          id,
                    const ep_sf_object_type type,
                    const int64_t           import_id) :
                    ep_sf_object(id, type, import_id)
                {
                }
            };

            class ep_sf_catalog_product_specification_condition_group_builder final : public ep_sf_object_builder
            {
              private:
                boost::optional<uint64_t> brand_condition_;
                boost::optional<uint64_t> product_id_condition_;
                boost::optional<uint64_t> category_condition_;
                boost::optional<uint64_t> attribute_condition_;

              public:
                explicit ep_sf_catalog_product_specification_condition_group_builder()
                {
                    brand_condition_ = boost::none;
                    product_id_condition_ = boost::none;
                    category_condition_   = boost::none;
                    attribute_condition_  = boost::none;
                }

                void set_brand_condition(boost::optional<uint64_t> val)
                {
                    brand_condition_ = val;
                }

                void set_product_id_condition(boost::optional<uint64_t> val)
                {
                    product_id_condition_ = val;
                }

                void set_category_condition(boost::optional<uint64_t> val)
                {
                    category_condition_ = val;
                }

                void set_attribute_condition(boost::optional<uint64_t> val)
                {
                    attribute_condition_ = val;
                }

                auto to_catalog_product_specification_condition_group() const;

            };

            class ep_sf_catalog_product_specification final : public ep_sf_object
            {
              private:
                boost::optional<uint64_t> included_products_;
                boost::optional<uint64_t> excluded_products_;

              public:
                explicit ep_sf_catalog_product_specification(
                    const uint64_t          id,
                    const ep_sf_object_type type,
                    const int64_t           import_id) :
                    ep_sf_object(id, type, import_id)
                {
                }
            };

            class ep_sf_catalog_product_specification_builder final : public ep_sf_object_builder
            {
              private:
                boost::optional<uint64_t> included_products_;
                boost::optional<uint64_t> excluded_products_;

              public:
                explicit ep_sf_catalog_product_specification_builder()
                {
                    included_products_ = boost::none;
                    excluded_products_ = boost::none;
                }

                void set_included_products(boost::optional<uint64_t> val)
                {
                    included_products_ = val;
                }

                void set_excluded_products(boost::optional<uint64_t> val)
                {
                    excluded_products_ = val;
                }

                auto to_catalog_product_specification_group() const;

            };

            class ep_sf_catalog_product_specification_rule final : public ep_sf_object
            {
              private:
                boost::optional<std::string>              name_;
                boost::optional<std::string>              description_;
                boost::optional<bool>                     enabled_flag_;
                boost::optional<boost::posix_time::ptime> last_calculated_;
                boost::optional<uint64_t>                 product_specification_;

              public:
                explicit ep_sf_catalog_product_specification_rule(
                    const uint64_t          id,
                    const ep_sf_object_type type,
                    const int64_t           import_id) :
                    ep_sf_object(id, type, import_id)
                {
                }
            };

            class ep_sf_catalog_product_specification_rule_builder final : public ep_sf_object_builder
            {
              private:
                boost::optional<std::string>              name_;
                boost::optional<std::string>              description_;
                boost::optional<bool>                     enabled_flag_;
                boost::optional<boost::posix_time::ptime> last_calculated_;
                boost::optional<uint64_t>                 product_specification_;

              public:
                explicit ep_sf_catalog_product_specification_rule_builder()
                {
                    name_ = boost::none;
                    description_ = boost::none;
                    enabled_flag_ = boost::none;
                    last_calculated_ = boost::none;
                    product_specification_ = boost::none;
                }

                void set_name(boost::optional<std::string> val)
                {
                    name_ = val;
                }

                void set_description(boost::optional<std::string> val)
                {
                    description_ = val;
                }

                void set_enabled_flag(boost::optional<bool> val)
                {
                    enabled_flag_ = val;
                }

                void set_last_calculated(boost::optional<boost::posix_time::ptime> val)
                {
                    last_calculated_ = val;
                }

                void set_product_specification(boost::optional<uint64_t> val)
                {
                    product_specification_ = val;
                }

                auto to_catalog_product_specification_rule() const;
            };

            class ep_sf_catalog_page_meta_tag_rule final : public ep_sf_object
            {
              private:
                boost::optional<std::string> rule_id_;

              public:
                explicit ep_sf_catalog_page_meta_tag_rule(
                    const uint64_t          id,
                    const ep_sf_object_type type,
                    const int64_t           import_id) :
                    ep_sf_object(id, type, import_id)
                {
                }
            };

            class ep_sf_catalog_page_meta_tag_rule_builder final : public ep_sf_object_builder
            {
              private:
                boost::optional<std::string> rule_id_;

              public:
                explicit ep_sf_catalog_page_meta_tag_rule_builder()
                {
                    rule_id_ = boost::none;
                }

                void set_rule_id(boost::optional<std::string> val)
                {
                    rule_id_ = val;
                }

                auto to_catalog_page_meta_tag_rule() const;
            };

            class ep_sf_obj_catalog_category final : public ep_sf_object
            {
              private:
                const boost::optional<std::string>              xml_lang_;
                const boost::optional<uint64_t>                 display_name_;
                boost::optional<uint64_t>                 description_;
                boost::optional<bool>                     online_flag_;
                boost::optional<boost::posix_time::ptime> online_from_;
                boost::optional<boost::posix_time::ptime> online_to_;
                boost::optional<std::string>              variation_groups_display_mode_;
                boost::optional<std::string>              parent_;
                boost::optional<double>                   position_;
                boost::optional<std::string>              thumbnail_;
                boost::optional<std::string>              image_;
                boost::optional<std::string>              template_;
                boost::optional<int>                      search_placement_;
                boost::optional<int>                      search_rank_;
                boost::optional<bool>                     sitemap_included_flag_;
                boost::optional<std::string>              sitemap_changefrequency_;
                boost::optional<double>                   sitemap_priority_;
                boost::optional<uint64_t>                 page_attributes_;
                boost::optional<uint64_t>                 custom_attributes_;
                boost::optional<uint64_t>                 category_links_;
                boost::optional<uint64_t>                 attribute_groups_;
                boost::optional<uint64_t>                 refinement_definitions_;
                boost::optional<uint64_t>                 product_detail_page_meta_tag_rules_;
                boost::optional<uint64_t>                 product_listing_page_meta_tag_rules_;
                boost::optional<uint64_t>                 product_specification_rule_;

                boost::optional<std::string> mode_;
                const boost::optional<std::string> category_id_;

              public:
                explicit ep_sf_obj_catalog_category(
                    const uint64_t          id,
                    const ep_sf_object_type type,
                    const int64_t           import_id,
                    const boost::optional<uint64_t> display_name,
                    const boost::optional<std::string>    parent,
                    const boost::optional<std::string> category_id) :
                    ep_sf_object(id, type, import_id),
                    display_name_(display_name), parent_(parent), category_id_(category_id)
                {
                }
            };

            class ep_sf_obj_catalog_category_builder final : public ep_sf_object_builder
            {
              private:
                boost::optional<std::string>        xml_lang_;
                boost::optional<uint64_t>           display_name_;
                boost::optional<uint64_t>                 description_;
                boost::optional<bool>                     online_flag_;
                boost::optional<boost::posix_time::ptime> online_from_;
                boost::optional<boost::posix_time::ptime> online_to_;
                boost::optional<std::string>              variation_groups_display_mode_;
                boost::optional<std::string>              parent_;
                boost::optional<double>                   position_;
                boost::optional<std::string>              thumbnail_;
                boost::optional<std::string>              image_;
                boost::optional<std::string>              template_;
                boost::optional<int>                      search_placement_;
                boost::optional<int>                      search_rank_;
                boost::optional<bool>                     sitemap_included_flag_;
                boost::optional<std::string>              sitemap_changefrequency_;
                boost::optional<double>                   sitemap_priority_;
                boost::optional<uint64_t>                 page_attributes_;
                boost::optional<uint64_t>                 custom_attributes_;
                boost::optional<uint64_t>                 category_links_;
                boost::optional<uint64_t>                 attribute_groups_;
                boost::optional<uint64_t>                 refinement_definitions_;
                boost::optional<uint64_t>                 product_detail_page_meta_tag_rules_;
                boost::optional<uint64_t>                 product_listing_page_meta_tag_rules_;
                boost::optional<uint64_t>                 product_specification_rule_;

                boost::optional<std::string>       mode_;
                boost::optional<std::string> category_id_;

              public:
                explicit ep_sf_obj_catalog_category_builder()
                {
                    xml_lang_ = boost::none;
                    display_name_ = boost::none;
                    description_  = boost::none;
                    online_flag_  = boost::none;
                    online_from_  = boost::none;
                    online_to_    = boost::none;
                    variation_groups_display_mode_ = boost::none;
                    parent_                        = boost::none;
                    position_                      = boost::none;
                    thumbnail_                     = boost::none;
                    image_                         = boost::none;
                    template_                      = boost::none;
                    search_placement_              = boost::none;
                    search_rank_                   = boost::none;
                    sitemap_included_flag_         = boost::none;
                    sitemap_changefrequency_       = boost::none;
                    sitemap_priority_              = boost::none;
                    page_attributes_               = boost::none;
                    custom_attributes_             = boost::none;
                    category_links_                = boost::none;
                    attribute_groups_              = boost::none;
                    refinement_definitions_        = boost::none;
                    product_detail_page_meta_tag_rules_ = boost::none;
                    product_listing_page_meta_tag_rules_ = boost::none;
                    product_specification_rule_          = boost::none;
                    mode_                                = boost::none;
                    category_id_                         = boost::none;
                }

                void set_xml_lang(boost::optional<std::string> val)
                {
                    xml_lang_ = val;
                }

                void set_display_name(boost::optional<uint64_t> val)
                {
                    display_name_ = val;
                }

                void set_description(boost::optional<uint64_t> val)
                {
                    description_ = val;
                }

                void set_online_flag(boost::optional<bool> val)
                {
                    online_flag_ = val;
                }

                void set_online_from(boost::optional<boost::posix_time::ptime> val)
                {
                    online_from_ = val;
                }

                void set_online_to(boost::optional<boost::posix_time::ptime> val)
                {
                    online_to_ = val;
                }

                void set_variation_groups_display_mode(boost::optional<std::string> val)
                {
                    variation_groups_display_mode_ = val;
                }

                void set_parent(boost::optional<std::string> val)
                {
                    parent_ = val;
                }

                void set_position(boost::optional<double> val)
                {
                    position_ = val;
                }

                void set_thumbnail(boost::optional<std::string> val)
                {
                    thumbnail_ = val;
                }

                void set_image(boost::optional<std::string> val)
                {
                    image_ = val;
                }

                void set_template(boost::optional<std::string> val)
                {
                    thumbnail_ = val;
                }

                void set_search_placement(boost::optional<int> val)
                {
                    search_placement_ = val;
                }

                void set_search_rank(boost::optional<int> val)
                {
                    search_rank_ = val;
                }

                void set_sitemap_included_flag(boost::optional<bool> val)
                {
                    sitemap_included_flag_ = val;
                }

                void set_sitemap_changefrequency(boost::optional<std::string> val)
                {
                    sitemap_changefrequency_ = val;
                }

                void set_sitemap_priority(boost::optional<double> val)
                {
                    sitemap_priority_ = val;
                }

                void set_page_attributes(boost::optional<uint64_t> val)
                {
                    page_attributes_ = val;
                }

                void set_custom_attributes(boost::optional<uint64_t> val)
                {
                    custom_attributes_ = val;
                }

                void set_attribute_groups(boost::optional<uint64_t> val)
                {
                    attribute_groups_ = val;
                }

                void set_refinement_definitions(boost::optional<uint64_t> val)
                {
                    refinement_definitions_ = val;
                }

                void set_product_detail_page_meta_tag_rules(boost::optional<uint64_t> val)
                {
                    product_detail_page_meta_tag_rules_ = val;
                }

                void set_product_listing_page_meta_tag_rules(boost::optional<uint64_t> val)
                {
                    product_listing_page_meta_tag_rules_ = val;
                }

                void set_product_specification_rule(boost::optional<uint64_t> val)
                {
                    product_specification_rule_ = val;
                }

                void set_mode(boost::optional<std::string> val)
                {
                    mode_ = val;
                }

                void set_category_id(boost::optional<std::string> val)
                {
                    category_id_ = val;
                }

                auto to_catalog_category() const;
            };


        } // namespace salesforce
    }     // namespace schema
} // namespace data
} // namespace easyprospect