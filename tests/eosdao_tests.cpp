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

BOOST_AUTO_TEST_SUITE( eosdao_tests )

BOOST_FIXTURE_TEST_CASE( donate_receive, eosdao_tester ) try {
    printf("Running donate_receive...\n");

   BOOST_REQUIRE_EQUAL( core_sym::from_string("1000.0000"), get_system_balance( "donor1.dao" ) );

   transfer( "donor1.dao", "donation.dao", core_sym::from_string("1000.0000"), "donor1.dao" );

   produce_blocks(2);
    BOOST_REQUIRE_EQUAL( core_sym::from_string("1000.0000"), get_system_balance( "donation.dao" ) );

   BOOST_REQUIRE_EQUAL( dao_sym::from_string("1000.0000"), get_dao_balance( "donor1.dao" ) );
//printf("Running donate_receive complete...\n");

} FC_LOG_AND_RETHROW()


BOOST_FIXTURE_TEST_CASE( vote_weight_decay, eosdao_tester ) try {
    printf("Running vote_weight_decay...\n");

    BOOST_REQUIRE_EQUAL( core_sym::from_string("1000.0000"), get_system_balance( "donor1.dao" ) );
    BOOST_REQUIRE_EQUAL( core_sym::from_string("10000.0000"), get_system_balance( "donor2.dao" ) );
    BOOST_REQUIRE_EQUAL( core_sym::from_string("100000.0000"), get_system_balance( "donor3.dao" ) );

    const uint64_t one_hour_blocks = 60 * 60 * 2;
    //    const uint64_t six_months_blocks = 60 * 60 * 2;

    transfer( "donor2.dao", "donation.dao", core_sym::from_string("1000.0000"), "donor2.dao" );

    produce_blocks(one_hour_blocks);

    transfer( "donor3.dao", "donation.dao", core_sym::from_string("1000.0000"), "donor3.dao" );

    BOOST_REQUIRE_EQUAL( core_sym::from_string("2000.0000"), get_system_balance( "donation.dao" ) );

    uint64_t weight_2 = get_voter_weight(N(donor2.dao));
    uint64_t weight_3 = get_voter_weight(N(donor3.dao));

    BOOST_REQUIRE_EQUAL(weight_2, 10000000);
    // Weight of donor3 should include 1 hour of vote weight increase
    BOOST_REQUIRE_EQUAL(weight_3, 10001604);

} FC_LOG_AND_RETHROW()


BOOST_FIXTURE_TEST_CASE( direct_issue, eosdao_tester ) try {
    printf("Running direct_issue...\n");

    issue_dao( "donor5.dao", dao_sym::from_string("1000.0000"), N(token.dao) );

    BOOST_REQUIRE_EQUAL( dao_sym::from_string("1000.0000"), get_dao_balance( "donor5.dao" ) );

    uint64_t weight_5 = get_voter_weight(N(donor5.dao));

    BOOST_REQUIRE_EQUAL(weight_5, 10000000);

} FC_LOG_AND_RETHROW()


BOOST_FIXTURE_TEST_CASE( voting, eosdao_tester ) try {
    printf("Running voting...\n");

    transfer( "donor1.dao", "donation.dao", core_sym::from_string("1000.0000"), "donor1.dao" );
    transfer( "donor2.dao", "donation.dao", core_sym::from_string("1000.0000"), "donor2.dao" );
    transfer( "donor3.dao", "donation.dao", core_sym::from_string("1000.0000"), "donor3.dao" );
    transfer( "donor4.dao", "donation.dao", core_sym::from_string("1000.0000"), "donor4.dao" );

    BOOST_REQUIRE_EQUAL( dao_sym::from_string("1000.0000"), get_dao_balance( "donor1.dao" ) );
    BOOST_REQUIRE_EQUAL( dao_sym::from_string("1000.0000"), get_dao_balance( "donor2.dao" ) );
    BOOST_REQUIRE_EQUAL( dao_sym::from_string("1000.0000"), get_dao_balance( "donor3.dao" ) );
    BOOST_REQUIRE_EQUAL( dao_sym::from_string("1000.0000"), get_dao_balance( "donor4.dao" ) );

    uint64_t weight_1 = get_voter_weight(N(donor1.dao));

    BOOST_REQUIRE_EQUAL(weight_1, 10000000);

    memberreg(N(donor1.dao), "1df37bdb72c0be963ef2bdfe9b7ef10b", N(eosdao), N(donor1.dao));
    memberreg(N(donor2.dao), "1df37bdb72c0be963ef2bdfe9b7ef10b", N(eosdao), N(donor2.dao));
    memberreg(N(donor3.dao), "1df37bdb72c0be963ef2bdfe9b7ef10b", N(eosdao), N(donor3.dao));
    memberreg(N(donor4.dao), "1df37bdb72c0be963ef2bdfe9b7ef10b", N(eosdao), N(donor4.dao));
    memberreg(N(donor5.dao), "1df37bdb72c0be963ef2bdfe9b7ef10b", N(eosdao), N(donor5.dao));

    vector<name> votes1 = {N(steward1.dao), N(steward2.dao), N(steward3.dao)};
    vector<name> votes2 = {N(steward1.dao), N(steward2.dao), N(steward3.dao)};
    vector<name> votes3 = {N(steward1.dao), N(steward3.dao), N(steward5.dao)};
    vector<name> votes4 = {N(steward1.dao), N(steward3.dao), N(steward4.dao)};
    vector<name> votes5 = {N(steward1.dao), N(steward3.dao), N(steward4.dao)};

    BOOST_REQUIRE_EQUAL(
            error("assertion failure with message: ERR::NEWPERIOD_VOTER_ENGAGEMENT_LOW_ACTIVATE::Voter engagement is insufficient to activate the DAC."),
            newperiod(string("My newperiod message"), N(eosdao), N(donor1.dao))
    );

    vote(N(donor1.dao), votes1, N(eosdao), N(donor1.dao));
    vote(N(donor2.dao), votes2, N(eosdao), N(donor2.dao));
    vote(N(donor3.dao), votes3, N(eosdao), N(donor3.dao));
    vote(N(donor4.dao), votes4, N(eosdao), N(donor4.dao));

    BOOST_REQUIRE_EQUAL(
        error("assertion failure with message: ERR::NEWPERIOD_VOTER_ENGAGEMENT_LOW_ACTIVATE::Voter engagement is insufficient to activate the DAC."),
        newperiod(string("My newperiod message"), N(eosdao), N(donor1.dao))
    );

    // donor5 is required to unlock
    transfer( "donor5.dao", "donation.dao", core_sym::from_string("10000000.0000"), "donor5.dao" );
    BOOST_REQUIRE_EQUAL( dao_sym::from_string("10000000.0000"), get_dao_balance( "donor5.dao" ) );

    uint64_t weight_5 = get_voter_weight(N(donor5.dao));
    BOOST_REQUIRE_EQUAL(weight_5, 10'000'000'0000);

    vote(N(donor5.dao), votes5, N(eosdao), N(donor5.dao));

    BOOST_REQUIRE_EQUAL(
            success(),
            newperiod(string("My newperiod message"), N(eosdao), N(donor1.dao))
    );

} FC_LOG_AND_RETHROW()


BOOST_AUTO_TEST_SUITE_END()
