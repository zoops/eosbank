
#include <oracles/oracles.hpp>
#include <eosiolib/asset.hpp>
#include <eosiolib/print.hpp>
#include <eosiolib/time.hpp>
// #include <string>

namespace eosio {


oracles::oracles(name receiver, name code, datastream<const char*> ds): contract(receiver, code, ds){
    // init();
    // TODO: check for configs
}


void oracles::init()
{

}


void oracles::setscore( name account, uint64_t score )
{
    // TODO: add can recruiting and is pause
    require_auth( get_self() );
    oracles oracle( _code, _code.value );

    auto person = oracle.find( account.value );
    bool active_state = ( score > 0 ) ? true : false;

    // TODO: check total scores
    if ( person == oracle.end() ) {
        oracle.emplace(_code, [&]( auto& row ) {
            row.account = account;
            row.score = score;
            row.is_active = active_state;
        });
    }
    else {
        oracle.modify(person, _code, [&]( auto& row ) {
            row.score = score;
            if ( active_state != row.is_active )
                row.is_active = active_state;
        });
    }
    //     for (auto itr = deposit.cbegin(); itr != deposit.cend(); itr++) {
    //       print( itr->amount, "  " );
    // }
}


void oracles::vote( name user, uint8_t type, float value )
{
    require_auth( user );
    // is_pausing
    oracle oracle( _code, _code.value );

    // check for correct type

    const auto& person = oracle.get( user.value, "No Account Founded" );
    eosio_assert ( person.is_active != false, "Not Active" );

    string u = std::to_string(user.value / 200) + std::to_string(type);
    uint64_t unique_id =  std::stoull(u);


    // should check user identifie for solidity
    votes vote( _code, _code.value );
    auto item = vote.find( unique_id );
    if ( item == vote.end() ) {
        vote.emplace(_code, [&]( auto& row ) {
            row.id = unique_id;
            row.type = type;
            row.sum = person.score * value;
            row.account_score = person.score;
        });
    }
    else {
        vote.modify(item, _code, [&]( auto& row ) {
            row.sum = person.score * value;
        });
    }

    uint64_t total_score = 0;
    for (auto itr = oracle.cbegin(); itr != oracle.cend(); itr++) {
        total_score += itr->score;
    }

    uint64_t sum = 0;
    uint64_t vote_scores = 0;
    for (auto itr = vote.cbegin(); itr != vote.cend(); itr++) {
        if ( itr->type != type )
            continue;
        sum += itr->sum;
        vote_scores += itr-> account_score;
    }

    if ( (total_score / vote_scores) < 2 )
        action(
            permission_level{ get_self(), "active"_n },
            "eosbank"_n,
            "setconfig"_n,
            std::make_tuple( type, float(sum / vote_scores) )
        ).send();
}


// void oracles::updateconfig( uint8_t type, value)

} /// namespace eosoracles


extern "C" {
    void apply( uint64_t receiver, uint64_t code, uint64_t action ) {
        if (action == "transfer"_n.value && code == "myteostoken"_n.value) {
            // eosio::execute_action(eosio::name(receiver), eosio::name(code), &eosio::liq::getmyt);
        }
        else if ( code == "oracles"_n.value ) { // code name should set in configs
            if (action == "vote"_n.value) {
                eosio::execute_action( eosio::name(receiver), eosio::name(code), &eosio::oracles::vote );
            }
            else if (action=="setscore"_n.value) {
                eosio::execute_action( eosio::name(receiver), eosio::name(code), &eosio::oracles::setscore );
            }

        }
    }
}
