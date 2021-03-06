#pragma once

#include <eosio/asset.hpp>
#include <eosio/multi_index.hpp>
#include <eosio/singleton.hpp>
#include <eosio/action.hpp>
#include <eosio/eosio.hpp>
#include <math.h>

#include "dacdirectory_shared.hpp"

#include <string>

using namespace eosio;
using namespace std;

namespace eosdao {

   using std::string;

   /**
    * @defgroup eosiotoken eosio.token
    * @ingroup eosdaocontracts
    *
    * token.dao contract
    *
    * @details token.dao manages the DAO token.  It is based on the standard eosio.token contract with modifications
    * to allow for membership and vote decay.
    * @{
    */
   class [[eosio::contract]] token : public contract {
      public:
         using contract::contract;

         /**
          * Create action.
          *
          * @details Allows `issuer` account to create a token in supply of `maximum_supply`.
          * @param issuer - the account that creates the token,
          * @param maximum_supply - the maximum supply set for the token created.
          *
          * @pre Token symbol has to be valid,
          * @pre Token symbol must not be already created,
          * @pre maximum_supply has to be smaller than the maximum supply allowed by the system: 1^62 - 1.
          * @pre Maximum supply must be positive;
          *
          * If validation is successful a new entry in statstable for token symbol scope gets created.
          */
         [[eosio::action]]
         void create( const name&   issuer,
                      const asset&  maximum_supply);
         /**
          * Issue action.
          *
          * @details This action issues to `to` account a `quantity` of tokens.
          *
          * @param to - the account to issue tokens to, it must be the same as the issuer,
          * @param quntity - the amount of tokens to be issued,
          * @memo - the memo string that accompanies the token issue transaction.
          */
         [[eosio::action]]
         void issue( const name& to, const asset& quantity, const string& memo );

         /**
          * Transfer action.
          *
          * @details Transfer is only used when issuing tokens, after that it is non-transferrable.
          *
          * @param from - the account to transfer from,
          * @param to - the account to be transferred to,
          * @param quantity - the quantity of tokens to be transferred,
          * @param memo - the memo string to accompany the transaction.
          */
         [[eosio::action]]
         void transfer( const name&    from,
                        const name&    to,
                        const asset&   quantity,
                        const string&  memo );

       /**
        * Member register
        *
        * @details Allows a token holder to agree to the terms and conditions of being a member of the DAO
        *
        * @param sender - the account agreeing to the terms
        * @param agreedterms - md5 hash of the terms document
        * @param dac_id - the dac_id, used for scoping
        */
       [[eosio::action]]
       void memberrege(name sender, string agreedterms, name dac_id);


       /**
        * Register new terms
        *
        * @details Allows a token holder to agree to the terms and conditions of being a member of the DAO
        *
        * @param terms - URL of terms
        * @param hash - md5 hash of the terms document
        * @param dac_id - the dac_id, used for scoping
        */
       [[eosio::action]]
       void newmemtermse(string terms, string hash, name dac_id);
         /**
          * Get supply method.
          *
          * @details Gets the supply for token `sym_code`, created by `token_contract_account` account.
          *
          * @param token_contract_account - the account to get the supply for,
          * @param sym_code - the symbol to get the supply for.
          */
         static asset get_supply( const name& token_contract_account, const symbol_code& sym_code )
         {
            stats statstable( token_contract_account, sym_code.raw() );
            const auto& st = statstable.get( sym_code.raw() );
            return st.supply;
         }

         /**
          * Get balance method.
          *
          * @details Get the balance for a token `sym_code` created by `token_contract_account` account,
          * for account `owner`.
          *
          * @param token_contract_account - the token creator account,
          * @param owner - the account for which the token balance is returned,
          * @param sym_code - the token for which the balance is returned.
          */
         static asset get_balance( const name& token_contract_account, const name& owner, const symbol_code& sym_code )
         {
            accounts accountstable( token_contract_account, owner.value );
            const auto& ac = accountstable.get( sym_code.raw() );
            return ac.balance;
         }

           /* TODO : Use a common eosdac include */
           struct account_balance_delta {
               eosio::name    account;
               eosio::asset   balance_delta;
           };

      private:

        // Standard voting contract tables
       struct [[eosio::table]] account {
           asset    balance;

           uint64_t primary_key()const { return balance.symbol.code().raw(); }
       };

       struct [[eosio::table]] currency_stats {
           asset    supply;
           asset    max_supply;
           name     issuer;

           uint64_t primary_key()const { return supply.symbol.code().raw(); }
       };



       struct [[eosio::table]] termsinfo {
               string terms;
               string hash;
               uint64_t version;

               termsinfo() : terms(""), hash(""), version(0) {}

               termsinfo(string _terms, string _hash, uint64_t _version)
               : terms(_terms), hash(_hash), version(_version) {}

               uint64_t primary_key() const { return version; }
               uint64_t by_latest_version() const { return UINT64_MAX - version; }

               EOSLIB_SERIALIZE(termsinfo, (terms)(hash)(version))
       };


       struct [[eosio::table]] member {
           name sender;
           uint64_t agreedtermsversion;

           uint64_t primary_key() const { return sender.value; }
       };


       typedef multi_index<"members"_n, member> regmembers;
         typedef eosio::multi_index< "accounts"_n, account > accounts;
         typedef eosio::multi_index< "stat"_n, currency_stats > stats;
         typedef eosio::multi_index<"memberterms"_n, termsinfo,
                   indexed_by<"bylatestver"_n, const_mem_fun<termsinfo, uint64_t, &termsinfo::by_latest_version> >
           > memterms;


         // Internal methods
         void sub_balance( const name& owner, const asset& value );
         void add_balance( const name& owner, const asset& value, const name& ram_payer );
   };
} /// namespace eosdao
