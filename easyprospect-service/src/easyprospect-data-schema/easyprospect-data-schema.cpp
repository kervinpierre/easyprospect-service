#include <easyprospect-data-schema/easyprospect-data-schema.h>

namespace easyprospect
{
namespace data
{
    namespace schema
    {
        namespace salesforce
        {
            auto
            ep_sf_object_builder::to_object() const
            {
                auto res = ep_sf_obj_util::create<ep_sf_object>(eso_id_, eso_type_, eso_import_id_);

                return res;
            }

            auto
            ep_sf_obj_catalog_builder::to_catalog() const
            {
                auto res = ep_sf_obj_util::create<ep_sf_obj_catalog>(eso_id_, eso_type_, eso_import_id_,
                                                                      header_,
                                                                      product_attribute_definitions_,
                                                                      category_,
                                                                      product_,
                                                                      product_option_,
                                                                      variation_attribute_,
                                                                      category_assignment_,
                                                                      recommendation_,
                                                                      catalog_id_);

                return res;
            }

            auto
            ep_sf_obj_catalog_header_builder::to_catalog_header() const
            {
                auto res = ep_sf_obj_util::create<ep_sf_obj_catalog_header>(eso_id_, eso_type_, eso_import_id_);

                return res;
            }

            auto ep_sf_obj_catalog_header_image_settings_builder::
            to_catalog_header_image_settings() const
            {
                auto res = ep_sf_obj_util::create<ep_sf_obj_catalog_header_image_settings>(eso_id_, eso_type_, eso_import_id_);

                return res;
            }

            auto
            ep_sf_obj_catalog_header_image_internal_location_builder::
            to_catalog_header_image_internal_location() const
            {
                auto res = ep_sf_obj_util::create<ep_sf_obj_catalog_header_image_internal_location>(eso_id_, eso_type_, eso_import_id_);

                return res;
            }

            auto
            ep_sf_obj_catalog_header_image_external_location_builder::
            to_catalog_header_image_external_location() const
            {
                auto res =
                    ep_sf_obj_util::create<ep_sf_obj_catalog_header_image_external_location>(eso_id_, eso_type_, eso_import_id_);

                return res;
            }

            auto
            ep_sf_obj_catalog_header_image_view_types_builder::
            to_catalog_header_image_view_types() const
            {
                auto res = ep_sf_obj_util::create<ep_sf_obj_catalog_header_image_view_types>(eso_id_, eso_type_, eso_import_id_);

                return res;
            }

            auto
            ep_sf_obj_catalog_custom_attributes_builder::
            to_catalog_custom_attributes() const
            {
                auto res = ep_sf_obj_util::create<ep_sf_obj_catalog_custom_attributes>(eso_id_, eso_type_, eso_import_id_);

                return res;
            }

            auto
            ep_sf_obj_catalog_custom_attribute_builder::
            to_catalog_custom_attribute() const
            {
                auto res = ep_sf_obj_util::create<ep_sf_obj_catalog_custom_attribute>(eso_id_, eso_type_, eso_import_id_);

                return res;
            }

            auto
            ep_sf_catalog_category_link_builder::
            to_catalog_category_link() const
            {
                auto res = ep_sf_obj_util::create<ep_sf_catalog_category_link>(eso_id_, eso_type_, eso_import_id_);

                return res;
            }

            auto
            ep_sf_catalog_localized_string_builder::
            to_catalog_localized_string() const
            {
                auto res = ep_sf_obj_util::create<ep_sf_catalog_localized_string>(eso_id_, eso_type_, eso_import_id_);

                return res;
            }

            auto
            ep_sf_catalog_page_attributes_builder::
            to_catalog_page_attributes() const
            {
                auto res = ep_sf_obj_util::create<ep_sf_catalog_page_attributes>(eso_id_, eso_type_, eso_import_id_);

                return res;
            }

            auto
            ep_sf_catalog_attribute_reference_builder::
            to_catalog_attribute_reference() const
            {
                auto res = ep_sf_obj_util::create<ep_sf_catalog_attribute_reference>(eso_id_, eso_type_, eso_import_id_);

                return res;
            }

            auto
            ep_sf_catalog_attribute_group_builder::
            to_catalog_attribute_group() const
            {
                auto res = ep_sf_obj_util::create<ep_sf_catalog_attribute_group>(eso_id_, eso_type_, eso_import_id_);

                return res;
            }

            auto
            ep_sf_catalog_refinement_definitions_builder::
            to_catalog_refinement_definitions() const
            {
                auto res = ep_sf_obj_util::create<ep_sf_catalog_refinement_definitions>(eso_id_, eso_type_, eso_import_id_);

                return res;
            }

            auto
            ep_sf_catalog_attribute_refinement_bucket_builder::
            to_catalog_attribute_refinement() const
            {
                auto res = ep_sf_obj_util::create<ep_sf_catalog_attribute_refinement_bucket>(eso_id_, eso_type_, eso_import_id_);

                return res;
            }

            auto
            ep_sf_catalog_price_refinement_bucket_builder::
            to_catalog_price_refinement_bucket() const
            {
                auto res = ep_sf_obj_util::create<ep_sf_catalog_price_refinement_bucket>(eso_id_, eso_type_, eso_import_id_);

                return res;
            }

            auto
            ep_sf_catalog_threshold_refinement_bucket_builder::
            to_catalog_threshold_refinement_bucket() const
            {
                auto res = ep_sf_obj_util::create<ep_sf_catalog_threshold_refinement_bucket>(eso_id_, eso_type_, eso_import_id_);

                return res;
            }

            auto
            ep_sf_catalog_refinement_buckets_builder::
            to_catalog_refinement_buckets() const
            {
                auto res = ep_sf_obj_util::create<ep_sf_catalog_refinement_buckets>(eso_id_, eso_type_, eso_import_id_);

                return res;
            }

            auto
            ep_sf_catalog_refinement_definition_builder::
            to_catalog_refinement_definition() const
            {
                auto res = ep_sf_obj_util::create<ep_sf_catalog_refinement_definition>(eso_id_, eso_type_, eso_import_id_);

                return res;
            }

            auto
            ep_sf_catalog_product_brand_filter_builder::
            to_catalog_product_brand_filter() const
            {
                auto res = ep_sf_obj_util::create<ep_sf_catalog_product_brand_filter>(eso_id_, eso_type_, eso_import_id_);

                return res;
            }

            auto
            ep_sf_catalog_product_id_filter_builder::
            to_catalog_product_id_filter() const
            {
                auto res = ep_sf_obj_util::create<ep_sf_catalog_product_id_filter>(eso_id_, eso_type_, eso_import_id_);

                return res;
            }

            auto
            ep_sf_catalog_product_category_filter_builder::
            to_catalog_product_category_filter() const
            {
                auto res = ep_sf_obj_util::create<ep_sf_catalog_product_category_filter>(eso_id_, eso_type_, eso_import_id_);

                return res;
            }

            auto ep_sf_catalog_product_attribute_filter_builder::
            to_catalog_product_attribute_filter() const
            {
                auto res =
                    ep_sf_obj_util::create<ep_sf_catalog_product_attribute_filter>(eso_id_, eso_type_, eso_import_id_);

                return res;
            }

            auto ep_sf_catalog_product_specification_condition_group_builder::
            to_catalog_product_specification_condition_group() const
            {
                auto res =
                    ep_sf_obj_util::create<ep_sf_catalog_product_specification_condition_group>(eso_id_, eso_type_, eso_import_id_);

                return res;
            }

            auto ep_sf_catalog_product_specification_builder::
            to_catalog_product_specification_group() const
            {
                auto res = ep_sf_obj_util::create<ep_sf_catalog_product_specification>(
                    eso_id_, eso_type_, eso_import_id_);

                return res;
            }

            auto ep_sf_catalog_product_specification_rule_builder::
            to_catalog_product_specification_rule() const
            {
                auto res =
                    ep_sf_obj_util::create<ep_sf_catalog_product_specification_rule>(eso_id_, eso_type_, eso_import_id_);

                return res;
            }

            auto ep_sf_catalog_page_meta_tag_rule_builder::
            to_catalog_page_meta_tag_rule() const
            {
                auto res = ep_sf_obj_util::create<ep_sf_catalog_page_meta_tag_rule>(
                    eso_id_, eso_type_, eso_import_id_);

                return res;
            }

            auto ep_sf_obj_catalog_category_builder::to_catalog_category() const
            {
                auto res = ep_sf_obj_util::create<ep_sf_obj_catalog_category>(eso_id_, eso_type_, eso_import_id_, display_name_, parent_, category_id_);

                return res;
            }

            void ep_sf_object_builder::save() const
            {
            }
        } // namespace salesforce
    }     // namespace schema
} // namespace data
} // namespace easyprospect