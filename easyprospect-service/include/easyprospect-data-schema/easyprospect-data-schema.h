#pragma once
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/optional/optional.hpp>
#include <chrono>
#include <string>
#include <vector>

namespace easyprospect
{
namespace data
{
    namespace schema
    {
        namespace salesforce
        {
            enum class ep_sf_object_type
            {
                NONE                        = 0,
                IMPORT                      = 1,
                RELATIONSHIP                = 2,
                SF_CATALOG                  = 3,
                SF_CATALOG_HEADER           = 4,
                SF_CATALOG_IMAGE_SETTINGS   = 5,
                SF_CATALOG_CUSTOM_ATTRIBUTE = 6,
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

            class ep_sf_object
            {
              private:
                const uint64_t          id_;
                const ep_sf_object_type type_;
                const int64_t         import_id_;

                std::string       hash_;

              public:
                ep_sf_object() = delete;
                explicit ep_sf_object(const uint64_t id, const ep_sf_object_type type, 
                                       const int64_t import_id)
                : id_(id), type_(type), import_id_(import_id)
                {
                    
                }

                uint64_t get_id() const
                {
                    return id_;
                }

                ep_sf_object_type get_type() const 
                {
                    return type_;
                }

                int64_t get_import_id() const
                {
                    return import_id_;
                }

                std::string get_hash() const
                {
                    return hash_;
                }

                void set_hash(const std::string h)
                {
                    hash_ = h;
                }
            };

            class ep_sf_object_relationship final
            {
              private:
                const uint64_t id_;
                const int64_t  import_id_;
                const int64_t  variable_id_;
                const int64_t  order_;

                const int64_t           src_id_;
                const int64_t           dst_id_;
                const ep_sf_object_type dst_type_;
                const ep_sf_object_type src_type_;

              public:
                ep_sf_object_relationship() = delete;
                explicit ep_sf_object_relationship(const uint64_t id, 
                                                    const int64_t import_id,
                    const int64_t  variable_id,
                    const int64_t  order,
                    const int64_t  src_id,
                    const int64_t           dst_id,
                    const ep_sf_object_type dst_type,
                    const ep_sf_object_type src_type) :
                    id_(id),
                    import_id_(import_id), variable_id_(variable_id), order_(order), src_id_(src_id), dst_id_(dst_id),
                    dst_type_(dst_type), src_type_(src_type)
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

            class ep_sf_obj_catalog final : public ep_sf_object
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
                explicit ep_sf_obj_catalog(const uint64_t id, const ep_sf_object_type type, const int64_t import_id) :
                    ep_sf_object(id,type,import_id)
                {
                    
                }
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

            class ep_sf_obj_catalog_header_image_settings final : public ep_sf_object
            {
              private:
                boost::optional<uint64_t> internal_location_;
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

            class ep_sf_catalog_category_link final : public ep_sf_object
            {
              private:
                boost::optional<std::string> category_id_;
                boost::optional<std::string> catalog_id_;
            };

            class ep_sf_catalog_localized_string final : public ep_sf_object
            {
              private:
                boost::optional<std::string> value_;
                boost::optional<std::string> xml_lang_;
            };

            class ep_sf_catalog_page_attributes final : public ep_sf_object
            {
              private:
                boost::optional<uint64_t> page_title_;
                boost::optional<uint64_t> page_description_;
                boost::optional<uint64_t> page_keywords_;
                boost::optional<uint64_t> page_url_;
            };

            class ep_sf_catalog_attribute_reference final : public ep_sf_object
            {
              private:
                boost::optional<std::string> attribute_id_;
                boost::optional<bool>        system_;
            };

            class ep_sf_catalog_attribute_group final : public ep_sf_object
            {
              private:
                boost::optional<uint64_t> display_name_;
                boost::optional<uint64_t> description_;
                boost::optional<uint64_t> variable_attribute_;

                boost::optional<std::string> group_id_;
            };

            class ep_sf_catalog_refinement_definitions final : public ep_sf_object
            {
              private:
                boost::optional<uint64_t> refinement_definition_;
                boost::optional<uint64_t> blocked_refinement_definition_;
            };

            class ep_sf_catalog_attribute_refinement_bucket final : public ep_sf_object
            {
              private:
                boost::optional<uint64_t>    display_name_;
                boost::optional<uint64_t>    value_list_;
                boost::optional<std::string> presentation_id_;
                boost::optional<uint64_t>    description_;

                boost::optional<bool> default_;
            };

            class ep_sf_catalog_price_refinement_bucket final : public ep_sf_object
            {
              private:
                boost::optional<uint64_t> display_name_;
                boost::optional<double>   threshold_;

                boost::optional<std::string> currency_;
            };

            class ep_sf_catalog_threshold_refinement_bucket final : public ep_sf_object
            {
              private:
                boost::optional<uint64_t>    display_name_;
                boost::optional<uint64_t>    threshold_;
                boost::optional<std::string> presentation_id_;
                boost::optional<uint64_t>    description_;
            };

            class ep_sf_catalog_period_refinement_bucket final : public ep_sf_object
            {
              private:
                boost::optional<uint64_t>    display_name_;
                boost::optional<int>         duration_in_days_;
                boost::optional<std::string> presentation_id_;
                boost::optional<uint64_t>    description_;
            };

            class ep_sf_catalog_refinement_buckets final : public ep_sf_object
            {
              private:
                boost::optional<uint64_t> attribute_bucket_;
                boost::optional<uint64_t> price_bucket_;
                boost::optional<uint64_t> threshold_bucket_;
                boost::optional<uint64_t> period_bucket_;
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
            };

            class ep_sf_catalog_product_brand_filter final : public ep_sf_object
            {
              private:
                boost::optional<uint64_t>    brand_;
                boost::optional<std::string> operator_;
            };

            class ep_sf_catalog_product_id_filter final : public ep_sf_object
            {
              private:
                boost::optional<uint64_t>    product_id_;
                boost::optional<std::string> operator_;
            };

            class ep_sf_catalog_product_category_filter final : public ep_sf_object
            {
              private:
                boost::optional<uint64_t>    category_id_;
                boost::optional<std::string> operator_;
                boost::optional<std::string> catalog_id_;
            };

            class ep_sf_catalog_product_attribute_filter final : public ep_sf_object
            {
              private:
                boost::optional<uint64_t>    attribute_value_;
                boost::optional<std::string> operator_;
                boost::optional<std::string> catalog_id_;
            };

            class ep_sf_catalog_product_specification_condition_group final : public ep_sf_object
            {
              private:
                boost::optional<uint64_t> brand_condition_;
                boost::optional<uint64_t> product_id_condition_;
                boost::optional<uint64_t> category_condition_;
                boost::optional<uint64_t> attribute_condition_;
            };

            class ep_sf_catalog_product_specification final : public ep_sf_object
            {
              private:
                boost::optional<uint64_t> included_products_;
                boost::optional<uint64_t> excluded_products_;
            };

            class ep_sf_catalog_product_specification_rule final : public ep_sf_object
            {
              private:
                boost::optional<std::string>              name_;
                boost::optional<std::string>              description_;
                boost::optional<bool>                     enabled_flag_;
                boost::optional<boost::posix_time::ptime> last_calculated_;
                boost::optional<uint64_t>                 product_specification_;
            };

            class ep_sf_catalog_page_meta_tag_rule final : public ep_sf_object
            {
              private:
                boost::optional<std::string> rule_id_;
            };

            class ep_sf_obj_catalog_category final : public ep_sf_object
            {
              private:
                uint64_t                                  id_;
                boost::optional<std::string>              xml_lang_;
                boost::optional<uint64_t>                 display_name_;
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
                boost::optional<std::string> category_id_;

              public:
            };
        } // namespace salesforce
    }     // namespace schema
} // namespace data
} // namespace easyprospect