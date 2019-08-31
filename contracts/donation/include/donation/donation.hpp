#pragma once

#include <eosio/asset.hpp>
#include <eosio/eosio.hpp>

#include <string>

using namespace eosio;
using namespace std;

namespace eosdao {

   /**
    * @defgroup eosdaodonate donation
    * @ingroup eosiocontracts
    *
    * daodonate contract
    *
    * @details daodonate receives donations in the system currency and issues DAO tokens.
    * @{
    */
   class [[eosio::contract]] donation : public contract {
      public:
         using contract::contract;

         /**
          * Handler for incoming transfers.
          *
          * @details Receives notification of a transfer, checks that the token is the system contract and then sends an
          * inline issue action to the token contract.
          *
          * @param from - the account to transfer from,
          * @param to - the account to be transferred to,
          * @param quantity - the quantity of tokens to be transferred,
          * @param memo - the memo string to accompany the transaction (ignored).
          */
         [[eosio::on_notify("eosio.token::transfer")]]
         void transfer( const name&    from,
                        const name&    to,
                        const asset&   quantity,
                        const string&  memo );

       [[eosio::action]]
       void dummy();

   };
} /// namespace eosdao
