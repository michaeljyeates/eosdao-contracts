#include <voting/voting.hpp>

using namespace eosio;
using namespace eosdac;

namespace eosdao {

    void voting::balanceobsv(vector<account_balance_delta> account_balance_deltas, name dac_id) {
        auto dac = directory::dac_for_id(dac_id);
        auto token_contract = dac.symbol.get_contract();
        require_auth(token_contract);
        auto dac_symbol = dac.symbol.get_symbol();

        for (auto abd: account_balance_deltas){
            check(dac_symbol == abd.balance_delta.symbol, "ERR::SYM_NOT_DAC::The symbol of the balance delta does not match this DAC");
            check(abd.balance_delta.amount > 0, "ERR::INVALID_BALANCE_DELTA::Balance delta must be > 0");

            update_vote_weight(abd.account, abd.balance_delta, dac_id);
        }
    }

    void voting::resetweights(name dac_id) {
        weights voter_weights( get_self(), dac_id.value );
        auto w = voter_weights.begin();

        while (w != voter_weights.end()){
            w = voter_weights.erase(w);
        }
    }

    void voting::update_vote_weight(name owner, asset new_tokens, name dac_id){
        uint64_t vote_weight = get_vote_weight(new_tokens, dac_id);

        // update vote weights table
        weights voter_weights( get_self(), dac_id.value );
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

        vector<account_weight_delta> account_weights;
        account_weights.push_back(account_weight_delta{owner, vote_weight});

        auto dac = directory::dac_for_id(dac_id);
        eosio::name custodian_contract = dac.account_for_type(directory::types::CUSTODIAN);

//        eosio::action(
//                eosio::permission_level{ get_self(), "notify"_n },
//                custodian_contract, "weightobsv"_n,
//                make_tuple(account_weights, dac.dac_id)
//        ).send();

        print("notifying weight change to ", custodian_contract, "::weightobsv");
    }


    uint64_t voting::get_vote_weight(asset quantity, name dac_id){
        uint32_t six_months = 60 * 60 * 24 * 30 * 6;
        state_item state = state_item::get_state(get_self(), dac_id);

        uint32_t genesis = state.genesis.sec_since_epoch();
        uint32_t now = eosio::current_time_point().sec_since_epoch();
        if (genesis == 0){
            genesis = now;

            state.genesis = time_point_sec(now);
            state.save(get_self(), dac_id, get_self());
        }
        uint32_t time_since_epoch = (now - genesis);


        double multiplier = 1.0;

        if (time_since_epoch > 0){
            multiplier = pow(2, (double(time_since_epoch) / double(six_months)));
        }

        double d_amount = (double)quantity.amount;
        check( multiplier < (std::numeric_limits<double>::max() / d_amount), "Weight overflow");
        uint64_t weight = (uint64_t)(multiplier * d_amount);

        return weight;
    }


} /// namespace eosdao
