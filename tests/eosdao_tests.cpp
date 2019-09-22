#include <boost/test/unit_test.hpp>
#include <eosio/chain/contract_table_objects.hpp>
#include <eosio/chain/global_property_object.hpp>
#include <eosio/chain/resource_limits.hpp>
#include <eosio/chain/wast_to_wasm.hpp>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <fc/log/logger.hpp>
#include <eosio/chain/exceptions.hpp>
#include <Runtime/Runtime.h>

#include "eosdao_tester.hpp"

using namespace eosdao;

BOOST_AUTO_TEST_SUITE( eosdao_token_tests )

BOOST_FIXTURE_TEST_CASE( donate_receive, eosdao_tester ) try {
    printf("Running donate_receive...\n");

   BOOST_REQUIRE_EQUAL( core_sym::from_string("1000.0000"), get_system_balance( "donor1.dao" ) );

   transfer( "donor1.dao", "donation.dao", core_sym::from_string("1000.0000"), "donor1.dao" );

   produce_blocks(2);
    BOOST_REQUIRE_EQUAL( core_sym::from_string("1000.0000"), get_system_balance( "donation.dao" ) );

   BOOST_REQUIRE_EQUAL( dao_sym::from_string("1000.0000"), get_dao_balance( "donor1.dao" ) );
printf("Running donate_receive complete...\n");

} FC_LOG_AND_RETHROW()


BOOST_FIXTURE_TEST_CASE( vote_weight_decay, eosdao_tester ) try {

    BOOST_REQUIRE_EQUAL( core_sym::from_string("1000.0000"), get_system_balance( "donor1.dao" ) );
    BOOST_REQUIRE_EQUAL( core_sym::from_string("10000.0000"), get_system_balance( "donor2.dao" ) );
    BOOST_REQUIRE_EQUAL( core_sym::from_string("100000.0000"), get_system_balance( "donor3.dao" ) );

   const uint64_t one_hour_blocks = 60 * 60 * 2;
//    const uint64_t six_months_blocks = 60 * 60 * 2;

    transfer( "donor2.dao", "donation.dao", core_sym::from_string("1000.0000"), "donor2.dao" );

    printf("Waiting 1 hour...\n");
    produce_blocks(one_hour_blocks);
    printf("That was fast...\n");

    transfer( "donor3.dao", "donation.dao", core_sym::from_string("1000.0000"), "donor3.dao" );

    BOOST_REQUIRE_EQUAL( core_sym::from_string("2000.0000"), get_system_balance( "donation.dao" ) );

    uint64_t weight_2 = get_voter_weight(N(donor2.dao));
    uint64_t weight_3 = get_voter_weight(N(donor3.dao));

    BOOST_REQUIRE_EQUAL(weight_2, 10000000);
    // Weight of donor3 should include 1 hour of vote weight increase
    BOOST_REQUIRE_EQUAL(weight_3, 10001604);

} FC_LOG_AND_RETHROW()


BOOST_AUTO_TEST_SUITE_END()
