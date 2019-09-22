
void issue( const asset& amount, const name& manager = config::system_account_name ) {
    base_tester::push_action( N(eosio.token), N(issue), manager, mutable_variant_object()
            ("to",       manager )
            ("quantity", amount )
            ("memo",     "")
    );
}

void issue_dao( const name& to, const asset& quantity, const name& manager = config::system_account_name ) {
    base_tester::push_action( N(token.dao), N(issue), manager, mutable_variant_object()
            ("to",       to )
            ("quantity", quantity )
            ("memo",     "")
    );
}

void transfer( const name& from, const name& to, const asset& amount, const name& manager = config::system_account_name ) {
    base_tester::push_action( N(eosio.token), N(transfer), manager, mutable_variant_object()
            ("from",    from)
            ("to",      to )
            ("quantity", amount)
            ("memo", "")
    );
}

void transfer_dao( const name& from, const name& to, const asset& amount, const name& manager = config::system_account_name ) {
    base_tester::push_action( N(token.dao), N(transfer), manager, mutable_variant_object()
            ("from",    from)
            ("to",      to )
            ("quantity", amount)
            ("memo", "")
    );
}

void vote( const name& voter, const vector<name>& votes, const name& dac_id, const name& manager = config::system_account_name ) {
    base_tester::push_action( N(steward.dao), N(votecuste), manager, mutable_variant_object()
            ("voter",    voter)
            ("newvotes", votes )
            ("dac_id",   dac_id )
    );
}

void regdac( const name& owner, const name& dac_id, const extended_symbol& dac_symbol, const std::string& title, const name& manager = config::system_account_name ) {

    vector<std::map<uint8_t, name>> refs;
    vector<std::map<uint8_t, name>> accounts;

    base_tester::push_action( N(dacdirectory), N(regdac), manager, mutable_variant_object()
            ("owner",      owner)
            ("dac_id",     dac_id )
            ("dac_symbol", dac_symbol )
            ("title",      title )
            ("refs",       refs )
            ("accounts",   accounts )
    );
}

void regaccount( const name& dac_id, const name& account, const uint8_t type, const name& manager = config::system_account_name ) {
    base_tester::push_action( N(dacdirectory), N(regaccount), manager, mutable_variant_object()
            ("dac_id",  dac_id )
            ("account", account )
            ("type",    type )
    );
}

void configure_custodian( const contr_config newconfig, const name& dac_id, const name& manager = config::system_account_name ) {


    base_tester::push_action( N(steward.dao), N(updateconfige), manager, mutable_variant_object()
            ("newconfig",
             mutable_variant_object()
                     ("lockupasset", newconfig.lockupasset)
                     ("maxvotes", newconfig.maxvotes)
                     ("numelected", newconfig.numelected)
                     ("periodlength", newconfig.periodlength)
                     ("should_pay_via_service_provider", newconfig.should_pay_via_service_provider)
                     ("initial_vote_quorum_percent", newconfig.initial_vote_quorum_percent)
                     ("vote_quorum_percent", newconfig.vote_quorum_percent)
                     ("auth_threshold_high", newconfig.auth_threshold_high)
                     ("auth_threshold_mid", newconfig.auth_threshold_mid)
                     ("auth_threshold_low", newconfig.auth_threshold_low)
                     ("lockup_release_time_delay", newconfig.lockup_release_time_delay)
                     ("requested_pay_max", newconfig.requested_pay_max)

            )
            ("dac_id",  dac_id )
    );
}

void updatememterms(const string& terms, const string& hash, const name& dac_id, const name& manager = config::system_account_name){
    base_tester::push_action( N(token.dao), N(newmemtermse), manager, mutable_variant_object()
            ("terms",  terms )
            ("hash",   hash )
            ("dac_id", dac_id )
    );
}

void memberreg(const name& sender, const string& agreedterms, const name& dac_id, const name& manager = config::system_account_name){
    base_tester::push_action( N(token.dao), N(memberrege), manager, mutable_variant_object()
            ("sender",      sender )
            ("agreedterms", agreedterms )
            ("dac_id",      dac_id )
    );
}

void nominatecand(const name& cand, const asset& requestedpay, const name& dac_id, const name& manager = config::system_account_name){
    base_tester::push_action( N(steward.dao), N(nominatecane), manager, mutable_variant_object()
            ("cand",         cand )
            ("requestedpay", requestedpay )
            ("dac_id",       dac_id )
    );
}

action_result newperiod(const string& message, const name& dac_id, const name& manager = config::system_account_name){
    action act;
    act.account = N(steward.dao);
    act.name = N(newperiode);

    string action_type_name = custodian_abi_ser.get_action_type(act.name);
    variant_object data = mutable_variant_object()("message", message)("dac_id", dac_id);
    act.data = custodian_abi_ser.variant_to_binary(action_type_name , data, abi_serializer_max_time );

    return base_tester::push_action(std::move(act), uint64_t(manager));
}
