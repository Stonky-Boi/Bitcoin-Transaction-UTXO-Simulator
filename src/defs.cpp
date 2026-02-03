#pragma once
#include <cstdint>
#include <map>
#include <set>
#include <string>
#include <string_view>
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
    std::string result = "0000";
    for(int i = 0; i < 16; ++i) {
        result += chars[rand() % chars.length()];
    }
    return result;
}
struct ToPay {
    std::string payer;
    std::string payee;
    double amount;
};
struct Transaction {
    std::string tx_id;
    std::vector<UTXO> inputs;
    std::vector<UTXO> outputs;
    double fee;
    bool is_valid = false;
    Transaction() : fee(0), is_valid(false) {}
    Transaction(const std::string& sender, const std::vector<ToPay>& payments, 
                const std::vector<UTXO>& available_utxos) {
        
        tx_id = genUniqueTransactionID();
        double total_to_pay = 0;
        for (const auto& p : payments) total_to_pay += p.amount;

        double current_input_sum = 0;
        const double fee_per_input = 0.001;
        
        for (const auto& u : available_utxos) {
            inputs.push_back(u);
            current_input_sum += u.value;
            
            fee = inputs.size() * fee_per_input;
            
            if (current_input_sum >= total_to_pay + fee) break;
        }

        if (current_input_sum < total_to_pay + fee) {
            is_valid = false;
            return;
        }

        for (const auto& p : payments) {
            outputs.push_back({genUniqueUTXOID(), tx_id, p.payee, p.amount});
        }

        double change = current_input_sum - total_to_pay - fee;
        if (change > 0) {
            outputs.push_back({genUniqueUTXOID(), tx_id, sender, change});
        }

        is_valid = true;
    }
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
