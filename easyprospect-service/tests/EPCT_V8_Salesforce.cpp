#include <boost/filesystem/string_file.hpp>

#include "EPCT_Utils.h"

#include <easyprospect-service-plugin-salesforce/easyprospect-service-plugin-salesforce.h>


#include "easyprospect-data-schema/easyprospect-data-schema.h"
#include "easyprospect-data/easyprospect-data.h"

BOOST_AUTO_TEST_CASE(Ep_SF_Split_Simple_Catalog)
{
    bool res = true;

    auto sfo = easyprospect::service::plugin::easyprospect_service_plugin_salesforce::get_instance();
    auto cxt = sfo->init_context();

    std::string data_dir = R"(..\..\..\data\test\dataset0002\)";
    std::string doc_file = data_dir + "catalog-test-0001.xml";
    std::string                                                schema_path = R"(I:\src\salesforce\DWAPP-20.5.1.30-schema\)";
    std::string                                                xpath_tmpl = "/catalog/products/product[id=$current_id]/[enabled=true]";
    std::map<easyprospect::service::plugin::sf_item_type, int> pages{};
    pages.emplace(easyprospect::service::plugin::sf_item_type::PRODUCT, 5);

    auto doc_path = boost::filesystem::path{doc_file};
    doc_path = boost::filesystem::canonical(doc_path);

    if(!boost::filesystem::exists(doc_path))
    {
        spdlog::debug("'{}' doesn't exist.", doc_path.generic_string());
    }

    std::string doc_str;
    boost::filesystem::load_string_file(doc_path, doc_str);

    cxt->set_catalog_xsd(schema_path + "catalog.xsd");


    easyprospect::service::plugin::easyprospect_service_plugin_salesforce::sf_catalog_split(cxt, doc_str,xpath_tmpl,pages);

    BOOST_TEST(true);
}

BOOST_AUTO_TEST_CASE(Ep_SF_Split_Simple_Catalog2)
{
    spdlog::debug("Ep_SF_Split_Simple_Catalog2");
     
    bool res = true;

    auto sfo = easyprospect::service::plugin::easyprospect_service_plugin_salesforce::get_instance();
    auto cxt = sfo->init_context();

    std::string data_dir    = R"(..\..\..\data\test\dataset0002\)";
    std::string doc_file    = data_dir + "catalog-test-0001.xml";
    std::string schema_path = R"(I:\src\salesforce\DWAPP-20.5.1.30-schema\)";
    std::string db_file = "test01.db";
    std::string xpath_tmpl  = "/catalog/products/product[id=$current_id]/[enabled=true]";

    std::map<easyprospect::service::plugin::sf_item_type, int> pages{};
    pages.emplace(easyprospect::service::plugin::sf_item_type::PRODUCT, 5);

    auto doc_path = boost::filesystem::path{doc_file};
    doc_path      = boost::filesystem::canonical(doc_path);

    if(!boost::filesystem::exists(doc_path))
    {
        spdlog::debug("'{}' doesn't exist.", doc_path.generic_string());
    }

    std::string doc_str;
    boost::filesystem::load_string_file(doc_path, doc_str);

    cxt->set_catalog_xsd(schema_path + "catalog.xsd");

    // Delete the db if it exists
    auto db_path = boost::filesystem::path{db_file};
    db_path      = boost::filesystem::canonical(db_path);

    if(boost::filesystem::exists(db_path))
    {
        boost::system::error_code ec;
        boost::filesystem::remove(db_path, ec);
        if(ec)
        {
            spdlog::warn("'{}' delete failed with: {}.", db_path.generic_string(), ec.message());     
        }

        spdlog::debug("'{}'  exists, delete: {}.", db_path.generic_string(), ec.message());
    }

    auto dbc = std::make_shared<easyprospect::data::database::ep_sqlite>();
    dbc->open(db_path.generic_string());
    auto db = dbc->get_db();
    db->initialize_schema();

    auto bldr = std::make_shared<easyprospect::data::schema::salesforce::ep_sf_obj_import_builder>();

    auto stmt = dbc->get_db()->create_statement();
    auto id   = stmt->insert_new_import();
    bldr->set_id(id);

    auto imp = bldr->to_import();

    easyprospect::service::plugin::easyprospect_service_plugin_salesforce::sf_catalog_split2(
        cxt, dbc, imp, doc_str, xpath_tmpl, pages);

    BOOST_TEST(true);
}