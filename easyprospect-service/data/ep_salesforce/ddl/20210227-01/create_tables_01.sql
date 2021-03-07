create table ep_sf_obj_catalog
(
    header                        INTEGER,
    product_attribute_definitions INTEGER not null,
    category                      INTEGER,
    product                       INTEGER not null,
    product_options               INTEGER not null,
    variations_attribute          INTEGER not null,
    category_assignment           INTEGER not null,
    recommendation                INTEGER not null,
    catalog_id                    INTEGER not null,
    object_id                     INTEGER not null
);

create unique index ep_sf_obj_catalog_object_id_uindex
    on ep_sf_obj_catalog (object_id);

create table ep_sf_obj_catalog_attribute_group
(
    object_id          INTEGER not null,
    display_name       INTEGER,
    description        INTEGER,
    variable_attribute INTEGER,
    group_id           TEXT
);

create unique index ep_sf_obj_catalog_attribute_group_object_id_uindex
    on ep_sf_obj_catalog_attribute_group (object_id);

create table ep_sf_obj_catalog_attribute_reference
(
    object_id    INTEGER not null,
    attribute_id TEXT,
    SYSTEM       INTEGER
);

create unique index ep_sf_obj_catalog_attribute_reference_object_id_uindex
    on ep_sf_obj_catalog_attribute_reference (object_id);

create table ep_sf_obj_catalog_attribute_refinement_bucket
(
    object_id       INTEGER not null,
    display_name    INTEGER,
    value_list      INTEGER,
    presentation_id TEXT,
    description     INTEGER,
    "default"       INTEGER
);

create unique index ep_sf_obj_catalog_attribute_refinement_bucket_object_id_uindex
    on ep_sf_obj_catalog_attribute_refinement_bucket (object_id);

create table ep_sf_obj_catalog_category
(
    object_id                           INTEGER not null,
    xml_lang                            TEXT,
    display_name                        INTEGER,
    description                         INTEGER,
    online_flag                         INTEGER,
    online_from                         INTEGER,
    online_to                           INTEGER,
    variation_groups_display_mode       TEXT,
    parent                              TEXT,
    position                            FLOAT,
    thumbnail                           TEXT,
    image                               TEXT,
    template                            TEXT,
    search_placement                    INTEGER,
    search_rank                         INTEGER,
    sitemap_included_flag               INTEGER,
    sitemap_changefrequency             TEXT,
    sitemap_priority                    FLOAT,
    page_attributes                     INTEGER,
    custom_attributes                   INTEGER,
    category_links                      INTEGER,
    attribute_groups                    INTEGER,
    refinement_definitions              INTEGER,
    product_detail_page_meta_tag_rules  INTEGER,
    product_listing_page_meta_tag_rules INTEGER,
    product_specification_rule          INTEGER,
    mode                                TEXT,
    category_id                         TEXT
);

create unique index ep_sf_obj_catalog_category_object_id_uindex
    on ep_sf_obj_catalog_category (object_id);

create table ep_sf_obj_catalog_category_link
(
    object_id   INTEGER not null,
    category_id TEXT,
    catalog_id  TEXT
);

create unique index ep_sf_obj_catalog_category_link_object_id_uindex
    on ep_sf_obj_catalog_category_link (object_id);

create table ep_sf_obj_catalog_custom_attribute
(
    object_id    INTEGER not null,
    attribute_id TEXT,
    xml_lang     TEXT,
    value        INTEGER
);

create unique index ep_sf_obj_catalog_custom_attribute_object_id_uindex
    on ep_sf_obj_catalog_custom_attribute (object_id);

create table ep_sf_obj_catalog_custom_attributes
(
    object_id        INTEGER not null,
    custom_attribute INTEGER
);

create unique index ep_sf_obj_catalog_custom_attributes_object_id_uindex
    on ep_sf_obj_catalog_custom_attributes (object_id);

create table ep_sf_obj_catalog_header
(
    object_id         INTEGER not null,
    image_settings    INTEGER,
    custom_attributes INTEGER
);

create unique index ep_sf_obj_catalog_header_object_id_uindex
    on ep_sf_obj_catalog_header (object_id);

create table ep_sf_obj_catalog_header_image_external_location
(
    object_id INTEGER not null,
    http_url  TEXT,
    https_url TEXT
);

create unique index ep_sf_obj_catalog_header_image_external_location_object_id_uindex
    on ep_sf_obj_catalog_header_image_external_location (object_id);

create table ep_sf_obj_catalog_header_image_internal_location
(
    object_id INTEGER not null,
    base_path TEXT
);

create unique index ep_sf_obj_catalog_header_image_internal_location_object_id_uindex
    on ep_sf_obj_catalog_header_image_internal_location (object_id);

create table ep_sf_obj_catalog_header_image_settings
(
    object_id              INTEGER not null,
    internal_location      INTEGER,
    external_location      INTEGER,
    view_types             INTEGER,
    variation_attribute_id TEXT,
    alt_pattern            TEXT,
    title_pattern          TEXT
);

create unique index ep_sf_obj_catalog_header_image_settings_object_id_uindex
    on ep_sf_obj_catalog_header_image_settings (object_id);

create table ep_sf_obj_catalog_header_image_view_types
(
    object_id INTEGER not null,
    view_type TEXT
);

create unique index ep_sf_obj_catalog_header_image_view_types_object_id_uindex
    on ep_sf_obj_catalog_header_image_view_types (object_id);

create table ep_sf_obj_catalog_localized_string
(
    object_id INTEGER not null,
    value     TEXT,
    xml_lang  TEXT
);

create unique index ep_sf_obj_catalog_localized_string_object_id_uindex
    on ep_sf_obj_catalog_localized_string (object_id);

create table ep_sf_obj_catalog_page_attributes
(
    object_id        INTEGER not null,
    page_title       INTEGER,
    page_description INTEGER,
    page_keywords    INTEGER,
    page_url         INTEGER
);

create unique index ep_sf_obj_catalog_page_attributes_object_id_uindex
    on ep_sf_obj_catalog_page_attributes (object_id);

create table ep_sf_obj_catalog_page_meta_tag_rule
(
    object_id INTEGER,
    rule_id   TEXT
);

create table ep_sf_obj_catalog_period_refinement_bucket
(
    object_id        INTEGER not null,
    display_name     INTEGER,
    duration_in_days INTEGER,
    presentation_id  TEXT,
    description      INTEGER
);

create unique index ep_sf_obj_catalog_period_refinement_bucket_object_id_uindex
    on ep_sf_obj_catalog_period_refinement_bucket (object_id);

create table ep_sf_obj_catalog_price_refinement_bucket
(
    object_id    INTEGER not null,
    display_name INTEGER,
    threshold    REAL,
    currency     TEXT
);

create unique index ep_sf_obj_catalog_price_refinement_bucket_object_id_uindex
    on ep_sf_obj_catalog_price_refinement_bucket (object_id);

create table ep_sf_obj_catalog_product_attribute_filter
(
    object_id       INTEGER not null,
    attribute_value INTEGER,
    operator        TEXT,
    catalog_id      TEXT
);

create unique index ep_sf_catalog_product_attribute_filter_object_id_uindex
    on ep_sf_obj_catalog_product_attribute_filter (object_id);

create table ep_sf_obj_catalog_product_brand_filter
(
    object_id INTEGER not null,
    brand     INTEGER,
    operator  TEXT
);

create unique index ep_sf_obj_catalog_product_brand_filter_object_id_uindex
    on ep_sf_obj_catalog_product_brand_filter (object_id);

create table ep_sf_obj_catalog_product_category_filter
(
    object_id   INTEGER not null,
    category_id INTEGER,
    operator    TEXT,
    catalog_id  TEXT
);

create unique index ep_sf_catalog_product_category_filter_object_id_uindex
    on ep_sf_obj_catalog_product_category_filter (object_id);

create table ep_sf_obj_catalog_product_id_filter
(
    object_id  INTEGER,
    product_id INTEGER,
    operator   TEXT
);

create table ep_sf_obj_catalog_product_specification
(
    object_id         INTEGER,
    included_products INTEGER,
    excluded_products INTEGER
);

create table ep_sf_obj_catalog_product_specification_condition_group
(
    object_id            INTEGER not null,
    brand_condition      integer,
    product_id_condition INTEGER,
    category_condition   INTEGER,
    attribute_condition  INTEGER
);

create unique index ep_sf_catalog_product_specification_condition_group_object_id_uindex
    on ep_sf_obj_catalog_product_specification_condition_group (object_id);

create table ep_sf_obj_catalog_product_specification_rule
(
    object_id             INTEGER not null,
    name                  TEXT,
    description           TEXT,
    enabled_flag          INTEGER,
    last_calculated       INTEGER,
    product_specification INTEGER
);

create unique index ep_sf_obj_catalog_product_specification_rule_object_id_uindex
    on ep_sf_obj_catalog_product_specification_rule (object_id);

create table ep_sf_obj_catalog_refinement_buckets
(
    object_id        INTEGER not null,
    price_bucket     INTEGER,
    threshold_bucket INTEGER,
    period_bucket    INTEGER
);

create unique index ep_sf_obj_catalog_refinement_buckets_object_id_uindex
    on ep_sf_obj_catalog_refinement_buckets (object_id);

create table ep_sf_obj_catalog_refinement_definition
(
    object_id              INTEGER not null,
    display_name           INTEGER,
    value_set              TEXT,
    sort_mode              TEXT,
    sort_direction         TEXT,
    unbucketed_values_mode TEXT,
    cutoff_threshold       INTEGER,
    bucket_definitions     INTEGER,
    type                   TEXT,
    bucket_type            TEXT,
    attribute_id           TEXT,
    system                 INTEGER
);

create unique index ep_sf_obj_catalog_refinement_definition_object_id_uindex
    on ep_sf_obj_catalog_refinement_definition (object_id);

create table ep_sf_obj_catalog_refinement_definitions
(
    object_id                     INTEGER not null,
    refinement_definition         INTEGER,
    blocked_refinement_definition INTEGER
);

create unique index ep_sf_obj_catalog_refinement_definitions_object_id_uindex
    on ep_sf_obj_catalog_refinement_definitions (object_id);

create table ep_sf_obj_catalog_threshold_refinement_bucket
(
    object_id       INTEGER not null,
    display_name    INTEGER,
    threshold       INTEGER,
    presentation_id TEXT,
    description     INTEGER
);

create unique index ep_sf_obj_catalog_threshold_refinement_bucket_object_id_uindex
    on ep_sf_obj_catalog_threshold_refinement_bucket (object_id);

create table ep_sf_obj_import
(
    id        INTEGER not null
        constraint ep_sf_obj_import_pk
            primary key,
    label     text,
    timestamp int     not null
);

create table ep_sf_object
(
    eso_id        INTEGER not null
        constraint ep_sf_object_pk
            primary key,
    eso_type      int     not null,
    eso_import_id int     not null,
    eso_hash      text
);

create table ep_sf_object_relationship
(
    id          INTEGER not null,
    variable_id INTEGER
        constraint ep_sf_object_relationship_pk
            primary key,
    "order"     INTEGER not null,
    src_id      INTEGER not null,
    dst_id      INTEGER not null,
    dst_type    INTEGER not null,
    src_type    INTEGER not null
);

