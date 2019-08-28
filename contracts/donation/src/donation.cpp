#include <donation/donation.hpp>

namespace eosdao {

    void donation::receive(
            const name&    from,
            const name&    to,
            const asset&   quantity,
            const string&  memo )
    {
        if ( to == get_self() ){
            check(quantity.amount >= 10000, "ERR::QUANTITY_TOO_LOW::Minimum donation is 1.0000 EOS");
            // Receiving donation, send inline action to issue tokens back to the sender
            const asset dao_quantity = asset(quantity.amount, symbol{symbol_code("DAO"), 4});

            eosio::action(
                    eosio::permission_level{ get_self(), "issue"_n },
                    "token.dao"_n, "issue"_n,
                    make_tuple(to, dao_quantity, "Thank you for your donation to EOS DAO")
            ).send();
        }
    }


} /// namespace eosdao
