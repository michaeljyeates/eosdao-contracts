
uint32_t last_block_time() const {
    return time_point_sec( control->head_block_time() ).sec_since_epoch();
}

uint64_t get_voter_weight( const name& voter ){
    vector<char> data = get_row_by_account( N(voting.dao), N(eosdao), N(weights), voter );
    if (data.empty()){
        printf("DATA empty in get_voter_weight\n");
    }
    return data.empty() ? 0 : voting_abi_ser.binary_to_variant( "vote_weight_type", data, abi_serializer_max_time )["weight"].as<uint64_t>();
}

asset get_system_balance( const account_name& act, symbol balance_symbol = symbol{CORE_SYM} ) {
    vector<char> data = get_row_by_account( N(eosio.token), act, N(accounts), balance_symbol.to_symbol_code().value );
    return data.empty() ? asset(0, balance_symbol) : system_token_abi_ser.binary_to_variant("account", data, abi_serializer_max_time)["balance"].as<asset>();
}

asset get_dao_balance( const account_name& act, symbol balance_symbol = symbol{DAO_SYM} ) {
    vector<char> data = get_row_by_account( N(token.dao), act, N(accounts), balance_symbol.to_symbol_code().value );
    return data.empty() ? asset(0, balance_symbol) : token_abi_ser.binary_to_variant("account_type", data, abi_serializer_max_time)["balance"].as<asset>();
}


transaction_trace_ptr create_account_with_resources( account_name a, account_name creator, asset ramfunds, bool multisig,
                                                     asset net = core_sym::from_string("10.0000"), asset cpu = core_sym::from_string("10.0000") ) {
    signed_transaction trx;
    set_transaction_headers(trx);

    authority owner_auth;
    if (multisig) {
        // multisig between account's owner key and creators active permission
        owner_auth = authority(2, {key_weight{get_public_key( a, "owner" ), 1}}, {permission_level_weight{{creator, config::active_name}, 1}});
    } else {
        owner_auth =  authority( get_public_key( a, "owner" ) );
    }

    trx.actions.emplace_back( vector<permission_level>{{creator,config::active_name}},
                              newaccount{
                                      .creator  = creator,
                                      .name     = a,
                                      .owner    = owner_auth,
                                      .active   = authority( get_public_key( a, "active" ) )
                              });

    trx.actions.emplace_back( get_action( config::system_account_name, N(buyram), vector<permission_level>{{creator,config::active_name}},
                                          mvo()
                                                  ("payer", creator)
                                                  ("receiver", a)
                                                  ("quant", ramfunds) )
    );

    trx.actions.emplace_back( get_action( config::system_account_name, N(delegatebw), vector<permission_level>{{creator,config::active_name}},
                                          mvo()
                                                  ("from", creator)
                                                  ("receiver", a)
                                                  ("stake_net_quantity", net )
                                                  ("stake_cpu_quantity", cpu )
                                                  ("transfer", 0 )
                              )
    );

    set_transaction_headers(trx);
    trx.sign( get_private_key( creator, "active" ), control->get_chain_id()  );
    return push_transaction( trx );
}


void set_code_perms( name account, name code, name permission, string parent = "active", const vector<permission_level> perms = {} ) {
    vector<name> codes{code};
    return set_code_perms(account, codes, permission, parent, perms);
}

void set_code_perms( name account, vector<name> codes, name permission, string parent = "active", const vector<permission_level> perms = {} ) {

    vector<key_weight> key_weights{};
    vector<permission_level_weight> permission_weights;
    vector<wait_weight> wait_weights{};

    for (name code: codes){
        permission_level code_perm{code, N(eosio.code)};
        permission_level_weight code_perm_weight{code_perm, 1};
        permission_weights.push_back(code_perm_weight);
    }

    authority code_authority(
            1,
            key_weights,
            permission_weights,
            wait_weights);

    variant_object auth_act;
    auth_act =  mutable_variant_object()
            ("account",    account )
            ("permission", permission )
            ("parent",     parent.c_str() )
            ("auth",       code_authority );


    if (perms.size()){
        base_tester::push_action(N(eosio), N(updateauth), perms, auth_act, 6, 0 );
    }
    else {
        base_tester::push_action(N(eosio), N(updateauth), account, auth_act );
    }

    produce_block();
}

void link_perms( name account, name code, name type, name requirement, name manager = config::system_account_name ){

    auto link_act =  mutable_variant_object()
            ("account",     account )
            ("code",        code )
            ("type",        type )
            ("requirement", requirement );

    base_tester::push_action(N(eosio), N(linkauth), account, link_act );
}

void create_currency( name contract, asset maxsupply, name manager = config::system_account_name ) {
    auto act =  mutable_variant_object()
            ("issuer",       manager )
            ("maximum_supply", maxsupply );

    base_tester::push_action(contract, N(create), contract, act );
}


fc::variant get_system_stats( const string& symbolname ) {
    auto symb = eosio::chain::symbol::from_string(symbolname);
    auto symbol_code = symb.to_symbol_code().value;
    vector<char> data = get_row_by_account( N(eosio.token), symbol_code, N(stat), symbol_code );
    return data.empty() ? fc::variant() : system_token_abi_ser.binary_to_variant( "currency_stats", data, abi_serializer_max_time );
}

fc::variant get_dao_stats( const string& symbolname ) {
    auto symb = eosio::chain::symbol::from_string(symbolname);
    auto symbol_code = symb.to_symbol_code().value;
    vector<char> data = get_row_by_account( N(token.dao), symbol_code, N(stat), symbol_code );
    return data.empty() ? fc::variant() : token_abi_ser.binary_to_variant( "currency_stats", data, abi_serializer_max_time );
}

uint64_t microseconds_since_epoch_of_iso_string( const fc::variant& v ) {
    return static_cast<uint64_t>( time_point::from_iso_string( v.as_string() ).time_since_epoch().count() );
}
