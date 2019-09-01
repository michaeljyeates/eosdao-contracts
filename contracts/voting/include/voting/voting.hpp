#pragma once

#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/singleton.hpp>
#include <math.h>
#include "./dacdirectory_shared.hpp"

using namespace eosdac;
using namespace eosio;
using namespace std;

namespace eosdao {

   /**
    * @defgroup eosdaovoting voting
    * @ingroup eosdaocontracts
    *
    * voting.dao contract
    *
    * @details voting.dao manages the vote weight changes when signaled by the token contract during issue
    *
    * It provides a standard table called "weights" which is read by the custodian contract when votes are changed.
    * @{
    */
   class [[eosio::contract]] voting : public contract {
       private:


           // DAC tables
           struct state_item;
           typedef eosio::singleton<"state"_n, state_item> state_container;
           struct [[eosio::table("state")]] state_item {
               eosio::time_point_sec genesis = time_point_sec(0);

               static state_item get_state(eosio::name account, eosio::name scope) {
                   return state_container(account, scope.value).get_or_default(state_item());
               }

               void save(eosio::name account, eosio::name scope, eosio::name payer = same_payer) {
                   state_container(account, scope.value).set(*this, payer);
               }
           };

           // Standard voting contract tables
           struct [[eosio::table]] vote_weight {
               name     voter;
               uint64_t weight;

               uint64_t primary_key()const { return voter.value; }
           };

           typedef eosio::multi_index< "weights"_n, vote_weight > weights;

           void update_vote_weight(name owner, asset new_tokens, name dac_id);
           uint64_t get_vote_weight(asset quantity, name dac_id);
      public:
         using contract::contract;

       /* TODO : Use a common eosdac include */
       struct account_balance_delta {
           eosio::name    account;
           eosio::asset   balance_delta;
       };
       struct account_weight_delta {
           eosio::name    account;
           uint64_t       weight_delta;
       };


       /**
          * Balance observation action
          *
          * @details Observes a change to the balance of a user, in this case the balance delta sould always be positive.
          * If it is not positive then this action will fail and cause the upstream action to fail.
          *
          * @param account_balance_deltas - the account that creates the token,
          * @param dac_id - The dac_id as set in the dacdirectory contract
          *
          * @pre Each balance delta must be positive
          *
          * If validation is successful the weight of each account will be updated to reflect the weight at the time
          * of the balance delta.
          */

           ACTION balanceobsv(vector<account_balance_delta> account_balance_deltas, name dac_id);

           ACTION resetweights(name dac_id);


   };
} /// namespace eosdao
