#pragma once

#include <restinio/all.hpp>
#include <string>

#include "pet_data_types.h"
#include "db_layer.h"

namespace easyprospect
{
    namespace web
    {
        namespace server
        {

            class request_processor_t
            {
            public:
                request_processor_t(db_layer_t& db);

                void
                    on_create_new_pet(
                        const restinio::request_handle_t& req);

                void
                    on_get_all_pets(
                        const restinio::request_handle_t& req);

                void
                    on_get_specific_pet(
                        const restinio::request_handle_t& req,
                        pet_id_t pet_id);

                void
                    on_patch_specific_pet(
                        const restinio::request_handle_t& req,
                        pet_id_t pet_id);

                void
                    on_delete_specific_pet(
                        const restinio::request_handle_t& req,
                        pet_id_t pet_id);

            private:
                db_layer_t& m_db;

                model::pet_identity_t
                    create_new_pet(const restinio::request_handle_t& req);

                model::all_pets_t
                    get_all_pets();

                model::pet_with_id_t
                    get_specific_pet(pet_id_t pet_id);

                model::pet_identity_t
                    patch_specific_pet(
                        const restinio::request_handle_t& req,
                        pet_id_t pet_id);

                model::pet_identity_t
                    delete_specific_pet(pet_id_t pet_id);
            };

        }
    }
}

