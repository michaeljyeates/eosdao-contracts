#include <token/token.hpp>

using namespace eosio;
using namespace eosdac;
using namespace eosdac::token;

namespace eosdao {

    void token::create( const name&   issuer,
                        const asset&  maximum_supply )
    {
        require_auth( get_self() );

        auto sym = maximum_supply.symbol;
        check( sym.is_valid(), "invalid symbol name" );
        check( maximum_supply.is_valid(), "invalid supply");
        check( maximum_supply.amount > 0, "max-supply must be positive");

        stat_table statstable( get_self(), sym.code().raw() );
        auto existing = statstable.find( sym.code().raw() );
        check( existing == statstable.end(), "token with symbol already exists" );

        statstable.emplace( get_self(), [&]( auto& s ) {
           s.supply.symbol = maximum_supply.symbol;
           s.max_supply    = maximum_supply;
           s.issuer        = issuer;
        });
    }


    void token::issue( const name& to, const asset& quantity, const string& memo )
    {
//        print("Got issue\n");
//        return;
        require_auth(get_self());

        auto sym = quantity.symbol;
        check( sym.is_valid(), "invalid symbol name" );
        check( memo.size() <= 256, "memo has more than 256 bytes" );

        stat_table statstable( get_self(), sym.code().raw() );
        auto existing = statstable.find( sym.code().raw() );
        check( existing != statstable.end(), "voting with symbol does not exist, create voting before issue" );
        const auto& st = *existing;

        check( quantity.is_valid(), "invalid quantity" );
        check( quantity.amount > 0, "must issue positive quantity" );

        check( quantity.symbol == st.supply.symbol, "symbol precision mismatch" );
        check( quantity.amount <= st.max_supply.amount - st.supply.amount, "quantity exceeds available supply");

        statstable.modify( st, same_payer, [&]( auto& s ) {
           s.supply += quantity;
        });

        // add balance to self and then transfer to the receiver
        add_balance( get_self(), quantity, get_self() );

        eosio::action(
                eosio::permission_level{ get_self(), "xfer"_n },
                get_self(), "transfer"_n,
                make_tuple(get_self(), to, quantity, memo)
        ).send();
    }

    void token::transfer( const name&    from,
                          const name&    to,
                          const asset&   quantity,
                          const string&  memo )
    {

        check(has_auth(get_self()), "ERR:TRANSFERS_FORBIDDEN::Transfers are not allowed");
        check(from == get_self(), "ERR::TRANSFER_NOT_FROM_SELF::Transfer is not from self");

        // Transfer during an issue
        check( from != to, "cannot transfer to self" );
        require_auth( from );
        check( is_account( to ), "to account does not exist");
        auto sym = quantity.symbol.code();
        stat_table statstable( get_self(), sym.raw() );
        const auto& st = statstable.get( sym.raw() );

        require_recipient( to );

        check( quantity.is_valid(), "invalid quantity" );
        check( quantity.amount > 0, "must transfer positive quantity" );
        check( quantity.symbol == st.supply.symbol, "symbol precision mismatch" );
        check( memo.size() <= 256, "memo has more than 256 bytes" );

        sub_balance( from, quantity );
        add_balance( to, quantity, from );

        struct account_balance_delta {
            name    account;
            asset   balance_delta;
        };
        vector<notify::types::account_balance_delta> deltas;
        deltas.push_back(notify::types::account_balance_delta{to, quantity});

        // send inline action to the custodian contract to update any existing votes
        directory::types::dac dac = directory::dac_for_symbol(extended_symbol{quantity.symbol, get_self()});
        eosio::name vote_contract = dac.account_for_type(directory::types::ROUTER);
//        auto obsv_action = notify::balanceobsv_action(vote_contract, { { get_self(), "notify"_n } });
//        obsv_action.send(deltas, "eosdao"_n);
        eosio::action(
                eosio::permission_level{ get_self(), "notify"_n },
                vote_contract, "balanceobsv"_n,
                make_tuple(deltas, dac.dac_id)
        ).send();

    }


    void token::memberrege(name sender, string agreedterms, name dac_id) {
        // agreedterms is expected to be the member terms document hash
        require_auth(sender);

        tables::member_terms_table memberterms(_self, dac_id.value);

        check(memberterms.begin() != memberterms.end(), "ERR::MEMBERREG_NO_VALID_TERMS::No valid member terms found.");

        auto latest_member_terms = (--memberterms.end());
        check(latest_member_terms->hash == agreedterms, "ERR::MEMBERREG_NOT_LATEST_TERMS::Agreed terms isn't the latest.");
        member_table registeredgmembers = member_table(_self, dac_id.value);

        auto existingMember = registeredgmembers.find(sender.value);
        if (existingMember != registeredgmembers.end()) {
            registeredgmembers.modify(existingMember, sender, [&](types::member_type &mem) {
                mem.agreedtermsversion = latest_member_terms->version;
            });
        }
        else {
            registeredgmembers.emplace(sender, [&](types::member_type &mem) {
                mem.sender = sender;
                mem.agreedtermsversion = latest_member_terms->version;
            });
        }
    }

    void token::newmemtermse(string terms, string hash, name dac_id) {

        directory::types::dac dac = directory::dac_for_id(dac_id);
        eosio::name auth_account = dac.account_for_type(directory::types::AUTH);
        require_auth(auth_account);

        // sample IPFS: QmXjkFQjnD8i8ntmwehoAHBfJEApETx8ebScyVzAHqgjpD
        check(!terms.empty(), "ERR::NEWMEMTERMS_EMPTY_TERMS::Member terms cannot be empty.");
        check(terms.length() <= 256, "ERR::NEWMEMTERMS_TERMS_TOO_LONG::Member terms document url should be less than 256 characters long.");

        check(!hash.empty(), "ERR::NEWMEMTERMS_EMPTY_HASH::Member terms document hash cannot be empty.");
        check(hash.length() <= 32, "ERR::NEWMEMTERMS_HASH_TOO_LONG::Member terms document hash should be less than 32 characters long.");

        tables::member_terms_table memberterms(get_self(), dac_id.value);

        // guard against duplicate of latest
        if (memberterms.begin() != memberterms.end()) {
            auto last = --memberterms.end();
            check(!(terms == last->terms && hash == last->hash),
                  "ERR::NEWMEMTERMS_DUPLICATE_TERMS::Next member terms cannot be duplicate of the latest.");
        }

        uint64_t next_version = (memberterms.begin() == memberterms.end() ? 0 : (--memberterms.end())->version) + 1;

        memberterms.emplace(auth_account, [&](types::termsinfo_type &termsinfo) {
            termsinfo.terms = terms;
            termsinfo.hash = hash;
            termsinfo.version = next_version;
        });
    }

    // Private methods

    void token::sub_balance( const name& owner, const asset& value ) {
       account_table from_acnts( get_self(), owner.value );

       const auto& from = from_acnts.get( value.symbol.code().raw(), "no balance object found" );
       check( from.balance.amount >= value.amount, "overdrawn balance" );

       from_acnts.modify( from, owner, [&]( auto& a ) {
             a.balance -= value;
          });
    }

    void token::add_balance( const name& owner, const asset& value, const name& ram_payer )
    {
       account_table to_acnts( get_self(), owner.value );
       auto to = to_acnts.find( value.symbol.code().raw() );
       if( to == to_acnts.end() ) {
          to_acnts.emplace( ram_payer, [&]( auto& a ){
            a.balance = value;
          });
       } else {
          to_acnts.modify( to, same_payer, [&]( auto& a ) {
            a.balance += value;
          });
       }
    }


} /// namespace eosdao
