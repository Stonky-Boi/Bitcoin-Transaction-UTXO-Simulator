#include <cstdint>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>
#include <algorithm>
#include <iostream>
#include <ctime>

struct UTXO {
    std::string id;
    std::string parent_tx_id;
    std::string owner;
    double value;
};

bool operator==(const UTXO& u1,const UTXO& u2) {
    return (
        u1.id==u2.id&&
        u1.owner==u2.owner&&
        u1.parent_tx_id==u2.parent_tx_id&&
        u1.value==u2.value
    );
}

struct Transaction {
    std::string tx_id;
    std::vector<UTXO> inputs;
    std::vector<UTXO> outputs;
    double fee;
    bool is_valid=false;
};

std::string genUniqueUTXOID() {
    static int64_t id=0;
    return "UTXO_"+std::to_string(id++);
}

std::string genUniqueTransactionID() {
    static int64_t id=0;
    return "TX_"+std::to_string(id++);
}
