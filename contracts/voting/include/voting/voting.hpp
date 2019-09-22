#pragma once

#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/singleton.hpp>
#include <math.h>

#include <libeosdac/directory.hpp>
#include <libeosdac/token.hpp>
#include <libeosdac/notify.hpp>
#include <libeosdac/custodian.hpp>

using namespace eosdac;
using namespace eosdac::token::tables;
using namespace eosio;
using namespace std;

using eosdac::notify::types::account_balance_delta;
using eosdac::notify::types::account_weight_delta;
using eosdac::custodian::types::vote_weight_type;

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
           struct [[eosio::table]] weights : vote_weight_type {};


           void update_vote_weight(name owner, asset new_tokens, name dac_id);
           uint64_t get_vote_weight(asset quantity, name dac_id);
      public:
         using contract::contract;



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

       /**
      * Assert unlock action
      *
      * @details Asserts if the DAC is still locked and has not crossed activation threshold
      *
      * @param dac_id - The dac_id as set in the dacdirectory contract
      *
      * If the DAC has crossed the activation threshold then this action will do nothing.
      */
       ACTION assertunlock(name dac_id);

#ifdef DEBUG
        /**
      * Reset weights action
      *
      * @details Development action to clear the weights
      *
      * @param dac_id - The dac_id as set in the dacdirectory contract
      *
      * If the DAC has crossed the activation threshold then this action will do nothing.
      */
       ACTION resetweights(name dac_id);
#endif

   };
} /// namespace eosdao
