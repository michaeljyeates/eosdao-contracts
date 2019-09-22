
struct contr_config {
    extended_asset lockupasset;
    uint8_t maxvotes = 5;
    uint8_t numelected = 3;
    uint32_t periodlength = 7 * 24 * 60 * 60;
    bool should_pay_via_service_provider;
    uint32_t initial_vote_quorum_percent;
    uint32_t vote_quorum_percent;
    uint8_t auth_threshold_high;
    uint8_t auth_threshold_mid;
    uint8_t auth_threshold_low;
    uint32_t lockup_release_time_delay;
    extended_asset requested_pay_max;
};

void basic_setup() {
//    printf("Running basic_setup...\n");
    produce_blocks();

    create_accounts({ N(eosio.token), N(eosio.ram), N(eosio.ramfee), N(eosio.stake),
                      N(eosio.bpay), N(eosio.vpay), N(eosio.saving), N(eosio.names), N(eosio.rex) });

    create_accounts({ N(auth.dao), N(token.dao), N(donation.dao), N(voting.dao), N(steward.dao), N(dacdirectory) });

    produce_blocks();
    set_code( N(eosio.token), testing::contracts::util::system_token_wasm());
    set_abi( N(eosio.token), testing::contracts::util::system_token_abi().data() );
    {
        const auto& accnt = control->db().get<account_object,by_name>( N(eosio.token) );
        abi_def abi;
        BOOST_REQUIRE_EQUAL(abi_serializer::to_abi(accnt.abi, abi), true);
        system_token_abi_ser.set_abi(abi, abi_serializer_max_time);
    }

    set_code( N(eosio), testing::contracts::util::system_wasm());
    set_abi( N(eosio), testing::contracts::util::system_abi().data() );


    {
        const auto& accnt = control->db().get<account_object,by_name>( N(eosio) );
        abi_def abi;
        BOOST_REQUIRE_EQUAL(abi_serializer::to_abi(accnt.abi, abi), true);
        system_abi_ser.set_abi(abi, abi_serializer_max_time);
    }

    // create and issue core voting
    FC_ASSERT( core_symbol.decimals() == 4, "create_core_token assumes core voting has 4 digits of precision" );
    create_currency( N(eosio.token), asset(100000000000000, core_symbol) );
    issue( asset(10000000000000, core_symbol) );
    BOOST_REQUIRE_EQUAL( asset(10000000000000, core_symbol), get_system_balance( "eosio", core_symbol ) );


    base_tester::push_action(config::system_account_name, N(init),
                             config::system_account_name,  mutable_variant_object()
                                     ("version", 0)
                                     ("core", CORE_SYM_STR)
    );
}

void deploy_contracts() {
//    printf("Running deploy_contracts...\n");

    set_code( N(token.dao), testing::contracts::token_wasm() );
    set_abi( N(token.dao), testing::contracts::token_abi().data() );
    {
        const auto& accnt = control->db().get<account_object,by_name>( N(token.dao) );
        abi_def abi;
        BOOST_REQUIRE_EQUAL(abi_serializer::to_abi(accnt.abi, abi), true);
        token_abi_ser.set_abi(abi, abi_serializer_max_time);
    }


    set_code( N(donation.dao), testing::contracts::donation_wasm() );
    set_abi( N(donation.dao), testing::contracts::donation_abi().data() );
    {
        const auto& accnt = control->db().get<account_object,by_name>( N(donation.dao) );
        abi_def abi;
        BOOST_REQUIRE_EQUAL(abi_serializer::to_abi(accnt.abi, abi), true);
        donation_abi_ser.set_abi(abi, abi_serializer_max_time);
    }


    set_code( N(voting.dao), testing::contracts::voting_wasm() );
    set_abi( N(voting.dao), testing::contracts::voting_abi().data() );
    {
        const auto& accnt = control->db().get<account_object,by_name>( N(voting.dao) );
        abi_def abi;
        BOOST_REQUIRE_EQUAL(abi_serializer::to_abi(accnt.abi, abi), true);
        voting_abi_ser.set_abi(abi, abi_serializer_max_time);
    }


    set_code( N(steward.dao), testing::contracts::eosdac::custodian_wasm() );
    set_abi( N(steward.dao), testing::contracts::eosdac::custodian_abi().data() );
    {
        const auto& accnt = control->db().get<account_object,by_name>( N(steward.dao) );
        abi_def abi;
        BOOST_REQUIRE_EQUAL(abi_serializer::to_abi(accnt.abi, abi), true);
        custodian_abi_ser.set_abi(abi, abi_serializer_max_time);
    }


    set_code( N(dacdirectory), testing::contracts::eosdac::directory_wasm() );
    set_abi( N(dacdirectory), testing::contracts::eosdac::directory_abi().data() );

    {
        const auto& accnt = control->db().get<account_object,by_name>( N(dacdirectory) );
        abi_def abi;
        BOOST_REQUIRE_EQUAL(abi_serializer::to_abi(accnt.abi, abi), true);
        directory_abi_ser.set_abi(abi, abi_serializer_max_time);
    }
}

void configure_contracts() {
//    printf("Running configure_contracts...\n");

    extended_asset lockup = extended_asset(asset(0, symbol(4, "DAO")), N(token.dao));
    extended_asset reqpay = extended_asset(asset(1, symbol(4, "EOS")), N(eosio.token));

    contr_config config = {
            lockup, // extended_asset lockupasset
            3, //uint8_t maxvotes = 5;
            5, //uint8_t numelected = 3;
            24 * 60 * 60, //uint32_t periodlength = 7 * 24 * 60 * 60;
            false, //bool should_pay_via_service_provider;
            1, //uint32_t initial_vote_quorum_percent;
            1, //uint32_t vote_quorum_percent;
            4, //uint8_t auth_threshold_high;
            3, //uint8_t auth_threshold_mid;
            3, //uint8_t auth_threshold_low;
            600, //uint32_t lockup_release_time_delay;
            reqpay //extended_asset requested_pay_max;
    };

    configure_custodian(config, N(eosdao), N(auth.dao));
    produce_blocks();

    string terms_url = "https://raw.githubusercontent.com/eosdac/eosdac-constitution/master/boilerplate_constitution.md";
    string terms_hash = "1df37bdb72c0be963ef2bdfe9b7ef10b";
    updatememterms(terms_url, terms_hash, N(eosdao), N(auth.dao));
}

void set_permissions(){
    // Add code rights for the donation contract so it can call issue
    set_code_perms(N(token.dao), N(donation.dao), N(issue));
    link_perms(N(token.dao), N(token.dao), N(issue), N(issue));
    // Add code rights to the token contract so that it can transfer from itself
    set_code_perms(N(token.dao), N(token.dao), N(xfer));
    link_perms(N(token.dao), N(token.dao), N(transfer), N(xfer));
    // Notify from token contract to voting
    set_code_perms(N(token.dao), N(token.dao), N(notify));
    link_perms(N(token.dao), N(voting.dao), N(balanceobsv), N(notify));

    // so the voting contract can notify the custodian contract (and vice versa)
    vector<name> codes = { N(steward.dao), N(voting.dao) };
    set_code_perms(N(voting.dao), codes, N(notify));
    link_perms(N(voting.dao), N(steward.dao), N(weightobsv), N(notify));
    link_perms(N(voting.dao), N(voting.dao), N(assertunlock), N(notify));

    // create auth permissions
    vector<permission_level> perms = { {N(auth.dao), N(owner)} };
    set_code_perms(N(auth.dao), N(steward.dao), N(owner), "", perms);
    set_code_perms(N(auth.dao), N(steward.dao), N(high), "active");
    set_code_perms(N(auth.dao), N(steward.dao), N(med), "high");
    set_code_perms(N(auth.dao), N(steward.dao), N(low), "med");
    set_code_perms(N(auth.dao), N(steward.dao), N(one), "low");

    // permissions on custodian account

}

void create_dao_token(){
//    printf("Running create_dao_token...\n");
    FC_ASSERT( DAO_SYM_PRECISION == 4, "create_dao_token assumes DAO token has 4 digits of precision" );
    create_currency( N(token.dao), dao_sym::from_string("1000000000000.0000"), N(token.dao) );
}

void setup_directory(){
//    printf("Running setup_directory...\n");
    extended_symbol es = extended_symbol{symbol(4, "DAO"), N(token.dao)};
    regdac( N(auth.dao), N(eosdao), es, "EOS DAO", N(auth.dao) );
    regaccount( N(eosdao), N(auth.dao), 0, N(auth.dao) );
    regaccount( N(eosdao), N(steward.dao), 2, N(auth.dao) );
    regaccount( N(eosdao), N(voting.dao), 8, N(auth.dao) );
    regaccount( N(eosdao), N(voting.dao), 9, N(auth.dao) );
}

void register_candidates(){
    create_account_with_resources( N(steward1.dao), config::system_account_name, core_sym::from_string("1000.0000"), false );
    create_account_with_resources( N(steward2.dao), config::system_account_name, core_sym::from_string("1000.0000"), false );
    create_account_with_resources( N(steward3.dao), config::system_account_name, core_sym::from_string("1000.0000"), false );
    create_account_with_resources( N(steward4.dao), config::system_account_name, core_sym::from_string("1000.0000"), false );
    create_account_with_resources( N(steward5.dao), config::system_account_name, core_sym::from_string("1000.0000"), false );

    memberreg(N(steward1.dao), "1df37bdb72c0be963ef2bdfe9b7ef10b", N(eosdao), N(steward1.dao));
    memberreg(N(steward2.dao), "1df37bdb72c0be963ef2bdfe9b7ef10b", N(eosdao), N(steward2.dao));
    memberreg(N(steward3.dao), "1df37bdb72c0be963ef2bdfe9b7ef10b", N(eosdao), N(steward3.dao));
    memberreg(N(steward4.dao), "1df37bdb72c0be963ef2bdfe9b7ef10b", N(eosdao), N(steward4.dao));
    memberreg(N(steward5.dao), "1df37bdb72c0be963ef2bdfe9b7ef10b", N(eosdao), N(steward5.dao));

    nominatecand( N(steward1.dao), core_sym::from_string("0.0000"), N(eosdao), N(steward1.dao) );
    nominatecand( N(steward2.dao), core_sym::from_string("0.0000"), N(eosdao), N(steward2.dao) );
    nominatecand( N(steward3.dao), core_sym::from_string("0.0000"), N(eosdao), N(steward3.dao) );
    nominatecand( N(steward4.dao), core_sym::from_string("0.0000"), N(eosdao), N(steward4.dao) );
    nominatecand( N(steward5.dao), core_sym::from_string("0.0000"), N(eosdao), N(steward5.dao) );

}

void remaining_setup() {
//    printf("Running remaining_setup...\n");
    produce_blocks();

    create_account_with_resources( N(donor1.dao), config::system_account_name, core_sym::from_string("1000.0000"), false );
    create_account_with_resources( N(donor2.dao), config::system_account_name, core_sym::from_string("1000.0000"), false );
    create_account_with_resources( N(donor3.dao), config::system_account_name, core_sym::from_string("1000.0000"), false );
    create_account_with_resources( N(donor4.dao), config::system_account_name, core_sym::from_string("1000.0000"), false );
    create_account_with_resources( N(donor5.dao), config::system_account_name, core_sym::from_string("1000.0000"), false );

    transfer(config::system_account_name, N(donor1.dao), core_sym::from_string("1000.0000"));
    transfer(config::system_account_name, N(donor2.dao), core_sym::from_string("10000.0000"));
    transfer(config::system_account_name, N(donor3.dao), core_sym::from_string("100000.0000"));
    transfer(config::system_account_name, N(donor4.dao), core_sym::from_string("1000000.0000"));
    transfer(config::system_account_name, N(donor5.dao), core_sym::from_string("10000000.0000"));

    BOOST_REQUIRE_EQUAL( asset(10000000, core_symbol), get_system_balance( "donor1.dao", core_symbol ) );
    BOOST_REQUIRE_EQUAL( asset(100000000, core_symbol), get_system_balance( "donor2.dao", core_symbol ) );
    BOOST_REQUIRE_EQUAL( asset(1000000000, core_symbol), get_system_balance( "donor3.dao", core_symbol ) );
    BOOST_REQUIRE_EQUAL( asset(10000000000, core_symbol), get_system_balance( "donor4.dao", core_symbol ) );
    BOOST_REQUIRE_EQUAL( asset(100000000000, core_symbol), get_system_balance( "donor5.dao", core_symbol ) );
}



enum class setup_level {
    none,
    minimal,
    deploy_contracts,
    set_permissions,
    dao_token,
    directory,
    configure_contracts,
    register_candidates,
    full
};

eosdao_tester( setup_level l = setup_level::full ) {
    if( l == setup_level::none ) return;

    basic_setup();
    if( l == setup_level::minimal ) return;
    produce_blocks();

    deploy_contracts();
    if( l == setup_level::deploy_contracts ) return;
    produce_blocks();

    set_permissions();
    if( l == setup_level::set_permissions ) return;
    produce_blocks();

    create_dao_token();
    if( l == setup_level::dao_token ) return;
    produce_blocks();

    setup_directory();
    if( l == setup_level::directory ) return;
    produce_blocks();

    configure_contracts();
    if( l == setup_level::configure_contracts ) return;
    produce_blocks();

    register_candidates();
    if( l == setup_level::register_candidates ) return;
    produce_blocks();

    remaining_setup();
}

template<typename Lambda>
eosdao_tester(Lambda setup) {
    setup(*this);

    basic_setup();
    deploy_contracts();
    set_permissions();
    create_dao_token();
    setup_directory();
    configure_contracts();
    register_candidates();
    remaining_setup();
}
