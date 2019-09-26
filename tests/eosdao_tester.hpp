#pragma once

#include <eosio/testing/tester.hpp>
#include <eosio/chain/abi_serializer.hpp>
#include <eosio/chain/resource_limits.hpp>
#include "contracts.hpp"
#include "test_symbol.hpp"

#include <fc/variant_object.hpp>
#include <fstream>

using namespace eosio::chain;
using namespace eosio::testing;
using namespace fc;

using mvo = fc::mutable_variant_object;

#ifndef TESTER
#ifdef NON_VALIDATING_TEST
#define TESTER tester
#else
#define TESTER validating_tester
#endif
#endif


namespace eosdao {

class eosdao_tester : public TESTER {
public:
#include "setup.hpp"
#include "utils.hpp"
#include "actions.hpp"


    abi_serializer donation_abi_ser;
    abi_serializer token_abi_ser;
    abi_serializer system_token_abi_ser;
    abi_serializer system_abi_ser;
    abi_serializer custodian_abi_ser;
    abi_serializer directory_abi_ser;
    abi_serializer voting_abi_ser;
};


} // end : eosdao namespace
