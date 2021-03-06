#include <donation/donation.hpp>

namespace eosdao {

    void donation::transfer(
            const name&    from,
            const name&    to,
            const asset&   quantity,
            const string&  memo )
    {
        if ( to == get_self() ){
            check(quantity.symbol == symbol(symbol_code("EOS"), 4), "Only EOS payments are accepted");
            check(quantity.amount >= 10000, "ERR::QUANTITY_TOO_LOW::Minimum donation is 1.0000 EOS");

            // Receiving donation, send inline action to issue tokens back to the sender
            const asset dao_quantity = asset(quantity.amount, symbol{symbol_code("DAO"), 4});

            string memo = "Thank you for your donation to EOS DAO";

            eosio::action(
                    eosio::permission_level{ "oadtestingtk"_n, "issue"_n },
                    "oadtestingtk"_n, "issue"_n,
                    make_tuple(from, dao_quantity, memo)
            ).send();
        }
    }

    void donation::dummy(){}


} /// namespace eosdao
