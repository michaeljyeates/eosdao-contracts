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
       printf("Running basic_setup...\n");
      produce_blocks( 2 );

       create_accounts({ N(eosio.token), N(eosio.ram), N(eosio.ramfee), N(eosio.stake),
                         N(eosio.bpay), N(eosio.vpay), N(eosio.saving), N(eosio.names), N(eosio.rex) });

      create_accounts({ N(token.dao), N(donation.dao), N(steward.dao) });

      produce_blocks( 100 );
       set_code( N(eosio.token), testing::contracts::util::system_token_wasm());
       set_abi( N(eosio.token), testing::contracts::util::system_token_abi().data() );
       {
           const auto& accnt = control->db().get<account_object,by_name>( N(eosio.token) );
           abi_def abi;
           BOOST_REQUIRE_EQUAL(abi_serializer::to_abi(accnt.abi, abi), true);
           system_token_abi_ser.set_abi(abi, abi_serializer_max_time);
       }

       set_code( N(eosio), testing::contracts::util::system_wasm());
       set_abi( N(eosio), testing::contracts::util::system_abi().data() );


       {
           const auto& accnt = control->db().get<account_object,by_name>( N(eosio) );
           abi_def abi;
           BOOST_REQUIRE_EQUAL(abi_serializer::to_abi(accnt.abi, abi), true);
           system_abi_ser.set_abi(abi, abi_serializer_max_time);
       }

        // create and issue core voting
      FC_ASSERT( core_symbol.decimals() == 4, "create_core_token assumes core voting has 4 digits of precision" );
      create_currency( N(eosio.token), config::system_account_name, asset(100000000000000, core_symbol) );
      issue( asset(10000000000000, core_symbol) );
      BOOST_REQUIRE_EQUAL( asset(10000000000000, core_symbol), get_system_balance( "eosio", core_symbol ) );


       base_tester::push_action(config::system_account_name, N(init),
                                config::system_account_name,  mutable_variant_object()
                                        ("version", 0)
                                        ("core", CORE_SYM_STR)
       );
   }

   void deploy_contracts() {
       printf("Running deploy_contracts...\n");
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


       set_code( N(steward.dao), testing::contracts::custodian_wasm() );
       set_abi( N(steward.dao), testing::contracts::custodian_abi().data() );


       {
           const auto& accnt = control->db().get<account_object,by_name>( N(steward.dao) );
           abi_def abi;
           BOOST_REQUIRE_EQUAL(abi_serializer::to_abi(accnt.abi, abi), true);
           custodian_abi_ser.set_abi(abi, abi_serializer_max_time);
       }
   }

   void set_permissions(){
       printf("Running set_permissions...\n");
       // Add code rights to the voting contract so that it can transfer from itself
       set_code_perms(N(token.dao), N(token.dao), N(xfer), N(token.dao), N(transfer));
       // Add code rights for the donation contract so it can call issue
       set_code_perms(N(token.dao), N(donation.dao), N(issue), N(token.dao), N(issue));
       // Notify from voting contract to custodian
       set_code_perms(N(token.dao), N(token.dao), N(notify), N(steward.dao), N(weightobsv));
   }

   void create_dao_token(){
       printf("Running create_dao_token...\n");
       FC_ASSERT( DAO_SYM_PRECISION == 4, "create_dao_token assumes DAO voting has 4 digits of precision" );
       create_currency( N(token.dao), N(token.dao), dao_sym::from_string("1000000000000.0000") );
   }

   void remaining_setup() {
       printf("Running remaining_setup...\n");
       produce_blocks();


       create_account_with_resources( N(donor1.dao), config::system_account_name, core_sym::from_string("1000.0000"), false );
       create_account_with_resources( N(donor2.dao), config::system_account_name, core_sym::from_string("1000.0000"), false );
       create_account_with_resources( N(donor3.dao), config::system_account_name, core_sym::from_string("1000.0000"), false );
       create_account_with_resources( N(donor4.dao), config::system_account_name, core_sym::from_string("1000.0000"), false );
       create_account_with_resources( N(donor5.dao), config::system_account_name, core_sym::from_string("1000.0000"), false );

//       create_accounts({ N(donor1.dao), N(donor2.dao), N(donor3.dao), N(donor4.dao), N(donor5.dao) });

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
        if (data.empty()){
            printf("DATA empty in get_voter_weight\n");
        }
        return data.empty() ? 0 : token_abi_ser.binary_to_variant( "vote_weight", data, abi_serializer_max_time )["weight"].as<uint64_t>();
    }

    asset get_system_balance( const account_name& act, symbol balance_symbol = symbol{CORE_SYM} ) {
       vector<char> data = get_row_by_account( N(eosio.token), act, N(accounts), balance_symbol.to_symbol_code().value );
       return data.empty() ? asset(0, balance_symbol) : system_token_abi_ser.binary_to_variant("account", data, abi_serializer_max_time)["balance"].as<asset>();
    }

    asset get_dao_balance( const account_name& act, symbol balance_symbol = symbol{DAO_SYM} ) {
       vector<char> data = get_row_by_account( N(token.dao), act, N(accounts), balance_symbol.to_symbol_code().value );
       return data.empty() ? asset(0, balance_symbol) : token_abi_ser.binary_to_variant("account", data, abi_serializer_max_time)["balance"].as<asset>();
    }


    transaction_trace_ptr create_account_with_resources( account_name a, account_name creator, asset ramfunds, bool multisig,
                                                         asset net = core_sym::from_string("10.0000"), asset cpu = core_sym::from_string("10.0000") ) {
        signed_transaction trx;
        set_transaction_headers(trx);

        authority owner_auth;
        if (multisig) {
            // multisig between account's owner key and creators active permission
            owner_auth = authority(2, {key_weight{get_public_key( a, "owner" ), 1}}, {permission_level_weight{{creator, config::active_name}, 1}});
        } else {
            owner_auth =  authority( get_public_key( a, "owner" ) );
        }

        trx.actions.emplace_back( vector<permission_level>{{creator,config::active_name}},
                                  newaccount{
                                          .creator  = creator,
                                          .name     = a,
                                          .owner    = owner_auth,
                                          .active   = authority( get_public_key( a, "active" ) )
                                  });

        trx.actions.emplace_back( get_action( config::system_account_name, N(buyram), vector<permission_level>{{creator,config::active_name}},
                                              mvo()
                                                      ("payer", creator)
                                                      ("receiver", a)
                                                      ("quant", ramfunds) )
        );

        trx.actions.emplace_back( get_action( config::system_account_name, N(delegatebw), vector<permission_level>{{creator,config::active_name}},
                                              mvo()
                                                      ("from", creator)
                                                      ("receiver", a)
                                                      ("stake_net_quantity", net )
                                                      ("stake_cpu_quantity", cpu )
                                                      ("transfer", 0 )
                                  )
        );

        set_transaction_headers(trx);
        trx.sign( get_private_key( creator, "active" ), control->get_chain_id()  );
        return push_transaction( trx );
    }


    void set_code_perms( name account, name code, name permission, name link_code, name link_type, name manager = config::system_account_name ) {

       eosio::chain::permission_level code_perm{code, N(eosio.code)};
       eosio::chain::authority code_authority(code_perm);

        auto auth_act =  mutable_variant_object()
                ("account",    account )
                ("permission", permission )
                ("parent",     "active" )
                ("auth",       code_authority );
        auto link_act =  mutable_variant_object()
                ("account",     account )
                ("code",        link_code )
                ("type",        link_type )
                ("requirement", permission );

        base_tester::push_action(N(eosio), N(updateauth), account, auth_act );
        base_tester::push_action(N(eosio), N(linkauth), account, link_act );
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

    void transfer_dao( const name& from, const name& to, const asset& amount, const name& manager = config::system_account_name ) {
        base_tester::push_action( N(token.dao), N(transfer), manager, mutable_variant_object()
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
   abi_serializer system_abi_ser;
   abi_serializer custodian_abi_ser;
};


} // end : eosdao namespace
