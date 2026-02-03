#pragma once
#include "utxo.cpp"
#include <iostream>
class Mempool {
public:
    std::vector<Transaction> transactions;
    int max_size=50;

    std::pair<bool,std::string> add_transaction(Transaction& tx,UTXOManager& manager) {
        if (transactions.size()>=max_size) return {false,"Mempool full"};
        double total_in=0;
        std::set<std::string> local_inputs;
        for (auto& in:tx.inputs) {
            if (!manager.exists(in)) return {false,"Input UTXO does not exist"};
            if (local_inputs.count(in.id)) return {false,"Double-spend in same TX"};
            local_inputs.insert(in.id);
            total_in+=in.value;
        }
        double total_out=0;
        for (auto& out:tx.outputs) {
            if (out.value<0) return {false,"Negative output amount"};
            total_out+=out.value;
        }
        if (total_in<total_out) return {false,"Insufficient funds"};
        tx.fee=total_in-total_out;
        tx.is_valid=true;
        transactions.push_back(tx);
        return {true,"Success"};
    }
};

void mine_block(std::string miner_address, Mempool& mempool, UTXOManager& manager, std::vector<Block>& blockchain) {
    if (mempool.transactions.empty()) {
        std::cout << YELLOW << "Mempool is empty. No transactions to mine." << RESET << std::endl;
        return;
    }

    std::sort(mempool.transactions.begin(),mempool.transactions.end(),[](const Transaction& a,const Transaction& b) {
        return a.fee>b.fee;
    });

    double total_fees=0;
    std::set<std::string> block_spent_ids;
    std::vector<Transaction> valid_txs;

    for (auto& tx:mempool.transactions) {
        bool can_mine=true;
        for (auto& in:tx.inputs) {
            if (!manager.exists(in)||block_spent_ids.count(in.id)) {
                can_mine=false;
                break;
            }
        }
        if (can_mine) {
            for (auto& in:tx.inputs) {
                manager.consumeUTXO(in);
                block_spent_ids.insert(in.id);
            }
            for (int i=0;i<tx.outputs.size();++i) {
                manager.generateUTXO(tx.tx_id,i,tx.outputs[i].value,tx.outputs[i].owner);
            }
            total_fees+=tx.fee;
            valid_txs.push_back(tx);
        } else {
            std::cout << RED << "TX "<<tx.tx_id<<" rejected (UTXO spent)" << RESET << std::endl;
        }
    }

    Block newBlock;
    newBlock.height = blockchain.size() + 1;
    newBlock.miner = miner_address;
    newBlock.transactions = valid_txs;
    newBlock.total_fees = total_fees;
    newBlock.timestamp = std::time(nullptr);
    newBlock.prev_hash = (blockchain.empty()) ? "0000000000" : blockchain.back().hash;
    newBlock.hash = genBlockHash(newBlock.height);

    manager.generateUTXO(genUniqueUTXOID(),0,total_fees,miner_address);
    blockchain.push_back(newBlock);
    mempool.transactions.clear();
    
    std::cout << GREEN << BOLD << "Block mined! Miner "<<miner_address<<" earned "<<total_fees<<" BTC" << RESET << std::endl;
}
