#include <token/token.hpp>

using namespace eosio;

namespace eosdao {

    void token::create( const name&   issuer,
                        const asset&  maximum_supply )
    {
        require_auth( get_self() );

        auto sym = maximum_supply.symbol;
        check( sym.is_valid(), "invalid symbol name" );
        check( maximum_supply.is_valid(), "invalid supply");
        check( maximum_supply.amount > 0, "max-supply must be positive");

        stats statstable( get_self(), sym.code().raw() );
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
        require_auth(get_self());

        auto sym = quantity.symbol;
        check( sym.is_valid(), "invalid symbol name" );
        check( memo.size() <= 256, "memo has more than 256 bytes" );

        stats statstable( get_self(), sym.code().raw() );
        auto existing = statstable.find( sym.code().raw() );
        check( existing != statstable.end(), "token with symbol does not exist, create token before issue" );
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
        stats statstable( get_self(), sym.raw() );
        const auto& st = statstable.get( sym.raw() );

        require_recipient( from );
        require_recipient( to );

        check( quantity.is_valid(), "invalid quantity" );
        check( quantity.amount > 0, "must transfer positive quantity" );
        check( quantity.symbol == st.supply.symbol, "symbol precision mismatch" );
        check( memo.size() <= 256, "memo has more than 256 bytes" );

        sub_balance( from, quantity );
        add_balance( to, quantity, from );

        update_vote_weight(to, quantity);
    }


    void token::memberrege(name sender, string agreedterms, name dac_id) {
        // agreedterms is expected to be the member terms document hash
        require_auth(sender);

        memterms memberterms(_self, dac_id.value);

        check(memberterms.begin() != memberterms.end(), "ERR::MEMBERREG_NO_VALID_TERMS::No valid member terms found.");

        auto latest_member_terms = (--memberterms.end());
        check(latest_member_terms->hash == agreedterms, "ERR::MEMBERREG_NOT_LATEST_TERMS::Agreed terms isn't the latest.");
        regmembers registeredgmembers = regmembers(_self, dac_id.value);

        auto existingMember = registeredgmembers.find(sender.value);
        if (existingMember != registeredgmembers.end()) {
            registeredgmembers.modify(existingMember, sender, [&](member &mem) {
                mem.agreedtermsversion = latest_member_terms->version;
            });
        }
        else {
            registeredgmembers.emplace(sender, [&](member &mem) {
                mem.sender = sender;
                mem.agreedtermsversion = latest_member_terms->version;
            });
        }
    }

    // Private methods

    void token::sub_balance( const name& owner, const asset& value ) {
       accounts from_acnts( get_self(), owner.value );

       const auto& from = from_acnts.get( value.symbol.code().raw(), "no balance object found" );
       check( from.balance.amount >= value.amount, "overdrawn balance" );

       from_acnts.modify( from, owner, [&]( auto& a ) {
             a.balance -= value;
          });
    }

    void token::add_balance( const name& owner, const asset& value, const name& ram_payer )
    {
       accounts to_acnts( get_self(), owner.value );
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

    uint64_t token::get_vote_weight(asset quantity){
        uint32_t six_months = 60 * 60 * 24 * 30 * 6;
        state_item state = state_item::get_state(get_self(), get_self());

        uint32_t genesis = state.genesis.sec_since_epoch();
        uint32_t now = time_point_sec(eosio::current_time_point()).sec_since_epoch();
        if (genesis == 0){
            genesis = now;

            state.genesis = time_point_sec(now);
            state.save(get_self(), get_self());
        }
        uint32_t time_since_epoch = (now - genesis);

        uint64_t multiplier = 1;

        if (time_since_epoch > 0){
            multiplier = 2 ^ (time_since_epoch / six_months);
        }

        // using the asset class overflow protection
        return (multiplier * quantity).amount;
    }

    void token::update_vote_weight(name owner, asset new_tokens){
        uint64_t vote_weight = get_vote_weight(new_tokens);

        // update vote weights table
        weights voter_weights( get_self(), get_self().value );
        auto existing = voter_weights.find( owner.value );
        uint64_t old_weight = 0;
        if( existing == voter_weights.end() ) {
            voter_weights.emplace( get_self(), [&]( auto& w ){
                w.voter = owner;
                w.weight = vote_weight;
            });
        }
        else {
            old_weight = existing->weight;

            voter_weights.modify( existing, same_payer, [&]( auto& w ) {
                w.weight += vote_weight;
            });
        }

        // send inline action to the custodian contract to update any existing votes
        eosio::action(
                eosio::permission_level{ get_self(), "notify"_n },
                get_self(), "transferobs"_n,
                make_tuple(owner, old_weight, vote_weight)
        ).send();
    }

} /// namespace eosdao
