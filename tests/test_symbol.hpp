#pragma once

#define CORE_SYM_NAME "EOS"
#define CORE_SYM_PRECISION 4
#define DAO_SYM_NAME "DAO"
#define DAO_SYM_PRECISION 4

#define _STRINGIZE1(x) #x
#define _STRINGIZE2(x) _STRINGIZE1(x)

#define CORE_SYM_STR ( _STRINGIZE2(CORE_SYM_PRECISION) "," CORE_SYM_NAME )
#define CORE_SYM  ( ::eosio::chain::string_to_symbol_c( CORE_SYM_PRECISION, CORE_SYM_NAME ) )
#define DAO_SYM_STR ( _STRINGIZE2(DAO_SYM_PRECISION) "," DAO_SYM_NAME )
#define DAO_SYM  ( ::eosio::chain::string_to_symbol( DAO_SYM_PRECISION, DAO_SYM_NAME ) )

const eosio::chain::symbol core_symbol = symbol{CORE_SYM};

struct core_sym {
    static inline eosio::chain::asset from_string(const std::string& s) {
        return eosio::chain::asset::from_string(s + " " CORE_SYM_NAME);
    }
};
struct dao_sym {
    static inline eosio::chain::asset from_string(const std::string& s) {
        return eosio::chain::asset::from_string(s + " " DAO_SYM_NAME);
    }
};
