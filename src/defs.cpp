#pragma once
#include <cstdint>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>
#include <algorithm>
#include <iostream>
#include <ctime>

const std::string RESET   = "\033[0m";
const std::string RED     = "\033[38;5;196m";
const std::string GREEN   = "\033[38;5;46m";
const std::string YELLOW  = "\033[38;5;226m";
const std::string BLUE    = "\033[38;5;39m";
const std::string MAGENTA = "\033[38;5;201m"; 
const std::string CYAN    = "\033[38;5;51m";
const std::string BOLD    = "\033[1m";

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

struct Block {
    int height;
    std::string hash; // Simplified hash (ID)
    std::string prev_hash;
    std::string miner;
    std::vector<Transaction> transactions;
    double total_fees;
    std::time_t timestamp;
};

std::string genUniqueUTXOID() {
    static int64_t id=0;
    return "UTXO_"+std::to_string(id++);
}

std::string genUniqueTransactionID() {
    static int64_t id=0;
    return "TX_"+std::to_string(id++);
}

std::string genBlockHash(int a) {
    std::string chars = "0123456789abcdef";
    std::string result = "0000"; // Typical mock-bitcoin prefix
    for(int i = 0; i < 16; ++i) {
        result += chars[rand() % chars.length()];
    }
    return result;
}
