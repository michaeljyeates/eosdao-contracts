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

BOOST_AUTO_TEST_SUITE(eosdao_tests)

BOOST_FIXTURE_TEST_CASE( donate_receive, eosdao_tester ) try {

   BOOST_REQUIRE_EQUAL( core_sym::from_string("100000.0000"), get_system_balance( "donor1.dao" ) );
   BOOST_REQUIRE_EQUAL( core_sym::from_string("1000000.0000"), get_system_balance( "donor2.dao" ) );
   BOOST_REQUIRE_EQUAL( core_sym::from_string("10000000.0000"), get_system_balance( "donor3.dao" ) );

   transfer( "donor1.dao", "donation.dao", core_sym::from_string("1000.0000"), "eosio" );
   transfer( "donor2.dao", "donation.dao", core_sym::from_string("1000.0000"), "eosio" );
   transfer( "donor3.dao", "donation.dao", core_sym::from_string("1000.0000"), "eosio" );

   produce_blocks(2);

   BOOST_REQUIRE_EQUAL( dao_sym::from_string("1000.0000"), get_dao_balance( "donor1.dao" ) );
   BOOST_REQUIRE_EQUAL( dao_sym::from_string("1000.0000"), get_dao_balance( "donor2.dao" ) );
   BOOST_REQUIRE_EQUAL( dao_sym::from_string("1000.0000"), get_dao_balance( "donor3.dao" ) );

} FC_LOG_AND_RETHROW()


BOOST_FIXTURE_TEST_CASE( vote_weight_decay, eosdao_tester ) try {

   BOOST_REQUIRE_EQUAL( core_sym::from_string("100000.0000"), get_system_balance( "donor1.dao" ) );
   BOOST_REQUIRE_EQUAL( core_sym::from_string("1000000.0000"), get_system_balance( "donor2.dao" ) );
   BOOST_REQUIRE_EQUAL( core_sym::from_string("10000000.0000"), get_system_balance( "donor3.dao" ) );

   transfer( "donor1.dao", "donation.dao", core_sym::from_string("1000.0000"), "eosio" );
   transfer( "donor2.dao", "donation.dao", core_sym::from_string("1000.0000"), "eosio" );
   transfer( "donor3.dao", "donation.dao", core_sym::from_string("1000.0000"), "eosio" );

   produce_blocks(2);

   BOOST_REQUIRE_EQUAL( dao_sym::from_string("1000.0000"), get_dao_balance( "donor1.dao" ) );
   BOOST_REQUIRE_EQUAL( dao_sym::from_string("1000.0000"), get_dao_balance( "donor2.dao" ) );
   BOOST_REQUIRE_EQUAL( dao_sym::from_string("1000.0000"), get_dao_balance( "donor3.dao" ) );

   const uint64_t six_months_blocks = 60 * 60 * 24 * 30 * 6 * 2;

   produce_blocks(six_months_blocks);

   transfer( "donor1.dao", "donation.dao", core_sym::from_string("1000.0000"), "eosio" );

   // Weight of donor1 should now be 1.5x the weight of donor2
   BOOST_REQUIRE_EQUAL(get_voter_weight(N(donor1.dao)), get_voter_weight(N(donor2.dao)) * 1.5);

} FC_LOG_AND_RETHROW()


BOOST_AUTO_TEST_SUITE_END()
