#pragma once

#include <eosio/testing/tester.hpp>
#include <eosio/chain/abi_serializer.hpp>
#include <eosio/chain/resource_limits.hpp>
#include "contracts.hpp"
#include "test_symbol.hpp"

#include <fc/variant_object.hpp>
#include <fstream>

using namespace eosio::chain;
using namespace eosio::testing;
using namespace fc;

using mvo = fc::mutable_variant_object;

#ifndef TESTER
#ifdef NON_VALIDATING_TEST
#define TESTER tester
#else
#define TESTER validating_tester
#endif
#endif


namespace eosdao {

class eosdao_tester : public TESTER {
public:

   void basic_setup() {
      produce_blocks( 2 );

      create_accounts({ N(eosio.token), N(token.dao), N(donation.dao) });

      produce_blocks( 100 );
      set_code( N(eosio.token), testing::contracts::util::system_token_wasm());
      set_abi( N(eosio.token), testing::contracts::util::system_token_abi().data() );
      {
         const auto& accnt = control->db().get<account_object,by_name>( N(eosio.token) );
         abi_def abi;
         BOOST_REQUIRE_EQUAL(abi_serializer::to_abi(accnt.abi, abi), true);
         system_token_abi_ser.set_abi(abi, abi_serializer_max_time);
      }

        // create and issue core token
      FC_ASSERT( core_symbol.decimals() == 4, "create_core_token assumes core token has 4 digits of precision" );
      create_currency( N(eosio.token), config::system_account_name, asset(100000000000000, core_symbol) );
      issue( asset(10000000000000, core_symbol) );
      BOOST_REQUIRE_EQUAL( asset(10000000000000, core_symbol), get_system_balance( "eosio", core_symbol ) );
   }

   void deploy_contracts() {
      set_code( N(token.dao), testing::contracts::token_wasm() );
      set_abi( N(token.dao), testing::contracts::token_abi().data() );

      {
         const auto& accnt = control->db().get<account_object,by_name>( N(token.dao) );
         abi_def abi;
         BOOST_REQUIRE_EQUAL(abi_serializer::to_abi(accnt.abi, abi), true);
         token_abi_ser.set_abi(abi, abi_serializer_max_time);
      }


      set_code( N(donation.dao), testing::contracts::donation_wasm() );
      set_abi( N(donation.dao), testing::contracts::donation_abi().data() );

      {
         const auto& accnt = control->db().get<account_object,by_name>( N(donation.dao) );
         abi_def abi;
         BOOST_REQUIRE_EQUAL(abi_serializer::to_abi(accnt.abi, abi), true);
         donation_abi_ser.set_abi(abi, abi_serializer_max_time);
      }
   }

   void set_permissions(){
       // Add code rights to the token contract so that it can transfer from itself
       set_code_perms(N(token.dao), N(token.dao), N(xfer), N(token.dao), N(transfer));
       // Add code rights for the donation contract so it can call issue
       set_code_perms(N(token.dao), N(donation.dao), N(issue), N(token.dao), N(issue));
   }

   void create_dao_token(){
       FC_ASSERT( DAO_SYM_PRECISION == 4, "create_dao_token assumes DAO token has 4 digits of precision" );
       create_currency( N(dao.token), N(dao.token), dao_sym::from_string("1000000000000.0000") );
   }

   void remaining_setup() {
       produce_blocks();

       create_accounts({ N(donor1.dao), N(donor2.dao), N(donor3.dao), N(donor4.dao), N(donor5.dao) });

       transfer(config::system_account_name, N(donor1.dao), core_sym::from_string("1000.0000"));
       transfer(config::system_account_name, N(donor2.dao), core_sym::from_string("10000.0000"));
       transfer(config::system_account_name, N(donor3.dao), core_sym::from_string("100000.0000"));
       transfer(config::system_account_name, N(donor4.dao), core_sym::from_string("1000000.0000"));
       transfer(config::system_account_name, N(donor5.dao), core_sym::from_string("10000000.0000"));

       BOOST_REQUIRE_EQUAL( asset(10000000, core_symbol), get_system_balance( "donor1.dao", core_symbol ) );
       BOOST_REQUIRE_EQUAL( asset(100000000, core_symbol), get_system_balance( "donor2.dao", core_symbol ) );
       BOOST_REQUIRE_EQUAL( asset(1000000000, core_symbol), get_system_balance( "donor3.dao", core_symbol ) );
       BOOST_REQUIRE_EQUAL( asset(10000000000, core_symbol), get_system_balance( "donor4.dao", core_symbol ) );
       BOOST_REQUIRE_EQUAL( asset(100000000000, core_symbol), get_system_balance( "donor5.dao", core_symbol ) );
   }

   enum class setup_level {
      none,
      minimal,
       deploy_contracts,
       set_permissions,
      dao_token,
      full
   };

   eosdao_tester( setup_level l = setup_level::full ) {
      if( l == setup_level::none ) return;

      basic_setup();
      if( l == setup_level::minimal ) return;

       deploy_contracts();
       if( l == setup_level::deploy_contracts ) return;

       set_permissions();
       if( l == setup_level::set_permissions ) return;

      create_dao_token();
      if( l == setup_level::dao_token ) return;

      remaining_setup();
   }

   template<typename Lambda>
   eosdao_tester(Lambda setup) {
      setup(*this);

      basic_setup();
      deploy_contracts();
       set_permissions();
      create_dao_token();
      remaining_setup();
   }


   uint32_t last_block_time() const {
      return time_point_sec( control->head_block_time() ).sec_since_epoch();
   }

    uint64_t get_voter_weight( const name& voter ){
        vector<char> data = get_row_by_account( N(token.dac), N(token.dac), N(weights), voter.value );
        return data.empty() ? 0 : token_abi_ser.binary_to_variant( "vote_weight", data, abi_serializer_max_time )["weight"].as<uint64_t>();
    }

    asset get_system_balance( const account_name& act, symbol balance_symbol = symbol{CORE_SYM} ) {
       vector<char> data = get_row_by_account( N(eosio.token), act, N(accounts), balance_symbol.to_symbol_code().value );
       return data.empty() ? asset(0, balance_symbol) : system_token_abi_ser.binary_to_variant("account", data, abi_serializer_max_time)["balance"].as<asset>();
    }

    asset get_dao_balance( const account_name& act, symbol balance_symbol = symbol{CORE_SYM} ) {
       vector<char> data = get_row_by_account( N(token.dao), act, N(accounts), balance_symbol.to_symbol_code().value );
       return data.empty() ? asset(0, balance_symbol) : token_abi_ser.binary_to_variant("account", data, abi_serializer_max_time)["balance"].as<asset>();
    }

    void set_code_perms( name account, name code, name permission, name link_code, name link_type, name manager = config::system_account_name ) {

       eosio::chain::permission_level code_perm{code, N(eosio.code)};
       eosio::chain::authority code_authority(code_perm);

        auto auth_act =  mutable_variant_object()
                ("account",    account )
                ("permission", permission )
                ("parent",     N("active") )
                ("auth",       code_authority );
        auto link_act =  mutable_variant_object()
                ("account",     account )
                ("code",        link_code )
                ("type",        link_type )
                ("requirement", permission );

        base_tester::push_action(manager, N(updateauth), manager, auth_act );
        base_tester::push_action(manager, N(linkauth), manager, link_act );
    }

    void create_currency( name contract, name manager, asset maxsupply ) {
        auto act =  mutable_variant_object()
                ("issuer",       manager )
                ("maximum_supply", maxsupply );

        base_tester::push_action(contract, N(create), contract, act );
    }

   void issue( const asset& amount, const name& manager = config::system_account_name ) {
      base_tester::push_action( N(eosio.token), N(issue), manager, mutable_variant_object()
                                ("to",       manager )
                                ("quantity", amount )
                                ("memo",     "")
                                );
   }

    void transfer( const name& from, const name& to, const asset& amount, const name& manager = config::system_account_name ) {
        base_tester::push_action( N(eosio.token), N(transfer), manager, mutable_variant_object()
                ("from",    from)
                ("to",      to )
                ("quantity", amount)
                ("memo", "")
        );
    }

    void vote( const name& voter, const vector<name>& votes, const name& manager = config::system_account_name ) {
        base_tester::push_action( N(steward.dao), N(votecust), manager, mutable_variant_object()
                ("voter",    voter)
                ("votes",      votes )
        );
    }

    fc::variant get_system_stats( const string& symbolname ) {
       auto symb = eosio::chain::symbol::from_string(symbolname);
       auto symbol_code = symb.to_symbol_code().value;
       vector<char> data = get_row_by_account( N(eosio.token), symbol_code, N(stat), symbol_code );
       return data.empty() ? fc::variant() : system_token_abi_ser.binary_to_variant( "currency_stats", data, abi_serializer_max_time );
    }

    fc::variant get_dao_stats( const string& symbolname ) {
       auto symb = eosio::chain::symbol::from_string(symbolname);
       auto symbol_code = symb.to_symbol_code().value;
       vector<char> data = get_row_by_account( N(token.dao), symbol_code, N(stat), symbol_code );
       return data.empty() ? fc::variant() : token_abi_ser.binary_to_variant( "currency_stats", data, abi_serializer_max_time );
    }

   uint64_t microseconds_since_epoch_of_iso_string( const fc::variant& v ) {
      return static_cast<uint64_t>( time_point::from_iso_string( v.as_string() ).time_since_epoch().count() );
   }

   abi_serializer donation_abi_ser;
   abi_serializer token_abi_ser;
   abi_serializer system_token_abi_ser;
};


} // end : eosdao namespace
