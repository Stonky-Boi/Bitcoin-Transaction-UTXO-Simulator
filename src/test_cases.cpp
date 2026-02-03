#include <iostream>
#include <vector>
#include <cmath>
#include <functional>
#include "mining.cpp"
#include "utils.hpp"

#define ASSERT_TRUE(condition, msg) \
    if (!(condition)) { \
        std::cout << RED << " [FAIL] " << msg << RESET << std::endl; \
        return false; \
    }

#define ASSERT_FALSE(condition, msg) \
    if (condition) { \
        std::cout << RED << " [FAIL] " << msg << RESET << std::endl; \
        return false; \
    }

#define ASSERT_EQ(val1, val2, msg) \
    if (std::abs((val1) - (val2)) > 1e-9) { \
        std::cout << RED << " [FAIL] " << msg << " (Expected " << val2 << ", got " << val1 << ")" << RESET << std::endl; \
        return false; \
    }

struct TestState {
    UTXOManager manager;
    Mempool mempool;
    std::vector<Block> blockchain;

    TestState() {
        manager.generateUTXO("genesis", 0, 50.0, "Alice");
        manager.generateUTXO("genesis", 1, 30.0, "Bob");
        manager.generateUTXO("genesis", 2, 20.0, "Charlie");
    }
};

// ==========================================
// Mandatory Test Cases
// ==========================================

bool test_basic_valid_transaction() {
    std::cout << "Test 1: Basic Valid Transaction... ";
    TestState state;
    
    std::vector<UTXO> aliceUTXOs = state.manager.getAllUTXOofOwner("Alice");
    Transaction tx;
    tx.tx_id = genUniqueTransactionID();
    tx.inputs.push_back(aliceUTXOs[0]); // 50
    tx.outputs.push_back({genUniqueUTXOID(), tx.tx_id, "Bob", 10.0});
    tx.outputs.push_back({genUniqueUTXOID(), tx.tx_id, "Alice", 39.999});
    
    auto res = state.mempool.add_transaction(tx, state.manager);
    
    ASSERT_TRUE(res.first, "Transaction should be valid");
    ASSERT_EQ(state.mempool.transactions.size(), 1.0, "Mempool size mismatch");
    ASSERT_EQ(state.mempool.transactions[0].fee, 0.001, "Fee calculation incorrect");
    
    std::cout << GREEN << " [PASS]" << RESET << std::endl;
    return true;
}

bool test_multiple_inputs() {
    std::cout << "Test 2: Multiple Inputs... ";
    TestState state;
    
    state.manager.generateUTXO("extra_funding", 0, 20.0, "Alice");
    
    std::vector<UTXO> inputs = state.manager.getAllUTXOofOwner("Alice"); // 50 + 20
    Transaction tx;
    tx.tx_id = genUniqueTransactionID();
    
    double totalIn = 0;
    for(auto& u : inputs) {
        tx.inputs.push_back(u);
        totalIn += u.value;
    }
    
    tx.outputs.push_back({genUniqueUTXOID(), tx.tx_id, "Bob", 60.0});
    tx.outputs.push_back({genUniqueUTXOID(), tx.tx_id, "Alice", totalIn - 60.0 - 0.001});
    
    auto res = state.mempool.add_transaction(tx, state.manager);
    
    ASSERT_TRUE(res.first, "Transaction with multiple inputs should be accepted");
    ASSERT_EQ(state.mempool.transactions[0].fee, 0.001, "Fee calculation incorrect");
    
    std::cout << GREEN << " [PASS]" << RESET << std::endl;
    return true;
}

bool test_double_spend_same_tx() {
    std::cout << "Test 3: Double-Spend in Same Transaction... ";
    TestState state;
    
    std::vector<UTXO> aliceUTXOs = state.manager.getAllUTXOofOwner("Alice");
    Transaction tx;
    tx.inputs.push_back(aliceUTXOs[0]);
    tx.inputs.push_back(aliceUTXOs[0]); 
    tx.outputs.push_back({genUniqueUTXOID(), "tx", "Bob", 10.0});
    
    auto res = state.mempool.add_transaction(tx, state.manager);
    
    ASSERT_FALSE(res.first, "Should reject double usage of UTXO in one TX");
    std::cout << GREEN << " [PASS]" << RESET << std::endl;
    return true;
}

bool test_mempool_double_spend() {
    std::cout << "Test 4: Mempool Double-Spend... ";
    TestState state;
    
    std::vector<UTXO> aliceUTXOs = state.manager.getAllUTXOofOwner("Alice");
    
    // TX1: Alice -> Bob
    Transaction tx1;
    tx1.tx_id = "TX1";
    tx1.inputs.push_back(aliceUTXOs[0]);
    tx1.outputs.push_back({genUniqueUTXOID(), tx1.tx_id, "Bob", 10.0});
    state.mempool.add_transaction(tx1, state.manager);
    
    // TX2: Alice -> Charlie (Using SAME input)
    Transaction tx2;
    tx2.tx_id = "TX2";
    tx2.inputs.push_back(aliceUTXOs[0]);
    tx2.outputs.push_back({genUniqueUTXOID(), tx2.tx_id, "Charlie", 10.0});
    
    auto res = state.mempool.add_transaction(tx2, state.manager);
    
    // With First-Seen Rule, this must be FALSE
    ASSERT_FALSE(res.first, "Second TX spending same UTXO must be rejected immediately");
    
    std::cout << GREEN << " [PASS]" << RESET << std::endl;
    return true;
}

bool test_insufficient_funds() {
    std::cout << "Test 5: Insufficient Funds... ";
    TestState state;
    std::vector<UTXO> bobUTXOs = state.manager.getAllUTXOofOwner("Bob");
    Transaction tx;
    tx.inputs.push_back(bobUTXOs[0]);
    tx.outputs.push_back({genUniqueUTXOID(), "tx", "Alice", 35.0});
    
    auto res = state.mempool.add_transaction(tx, state.manager);
    ASSERT_FALSE(res.first, "Should reject insufficient funds");
    std::cout << GREEN << " [PASS]" << RESET << std::endl;
    return true;
}

bool test_negative_amount() {
    std::cout << "Test 6: Negative Amount... ";
    TestState state;
    std::vector<UTXO> aliceUTXOs = state.manager.getAllUTXOofOwner("Alice");
    Transaction tx;
    tx.inputs.push_back(aliceUTXOs[0]);
    tx.outputs.push_back({genUniqueUTXOID(), "tx", "Bob", -5.0});
    auto res = state.mempool.add_transaction(tx, state.manager);
    ASSERT_FALSE(res.first, "Should reject negative output");
    std::cout << GREEN << " [PASS]" << RESET << std::endl;
    return true;
}

bool test_zero_fee() {
    std::cout << "Test 7: Zero Fee Transaction... ";
    TestState state;
    std::vector<UTXO> aliceUTXOs = state.manager.getAllUTXOofOwner("Alice");
    Transaction tx;
    tx.inputs.push_back(aliceUTXOs[0]);
    tx.outputs.push_back({genUniqueUTXOID(), "tx", "Bob", 50.0});
    auto res = state.mempool.add_transaction(tx, state.manager);
    ASSERT_TRUE(res.first, "Zero fee transactions are valid");
    ASSERT_EQ(state.mempool.transactions[0].fee, 0.0, "Fee should be 0");
    std::cout << GREEN << " [PASS]" << RESET << std::endl;
    return true;
}

bool test_race_attack() {
    std::cout << "Test 8: Race Attack (First Seen vs RBF)... ";
    TestState state;
    std::vector<UTXO> inputs = state.manager.getAllUTXOofOwner("Alice");
    
    // TX1: Low Fee (Arrives First)
    Transaction tx1;
    tx1.tx_id = "TX_LOW";
    tx1.inputs.push_back(inputs[0]);
    tx1.outputs.push_back({genUniqueUTXOID(), "TX_LOW", "Bob", 49.9});
    state.mempool.add_transaction(tx1, state.manager);

    // TX2: High Fee (Arrives Second)
    Transaction tx2;
    tx2.tx_id = "TX_HIGH";
    tx2.inputs.push_back(inputs[0]);
    tx2.outputs.push_back({genUniqueUTXOID(), "TX_HIGH", "Charlie", 49.0});
    
    // Add transaction checks if it returns FALSE now
    auto res = state.mempool.add_transaction(tx2, state.manager);
    if(res.first) {
         std::cout << RED << " [FAIL] High fee TX was accepted into mempool (Should be rejected by First-Seen)" << RESET << std::endl;
         return false;
    }

    mine_block("Miner", state.mempool, state.manager, state.blockchain);
    
    bool lowMined = false;
    for(auto& tx : state.blockchain[0].transactions) {
        if(tx.tx_id == "TX_LOW") lowMined = true;
    }
    
    ASSERT_TRUE(lowMined, "First-Seen transaction (TX_LOW) should have been mined");
    std::cout << GREEN << " [PASS]" << RESET << std::endl;
    return true;
}

bool test_complete_mining_flow() {
    std::cout << "Test 9: Complete Mining Flow... ";
    TestState state;
    std::vector<UTXO> aliceUTXOs = state.manager.getAllUTXOofOwner("Alice");
    Transaction tx;
    tx.tx_id = "TX_MINE";
    tx.inputs.push_back(aliceUTXOs[0]);
    tx.outputs.push_back({genUniqueUTXOID(), "TX_MINE", "Bob", 10.0});
    state.mempool.add_transaction(tx, state.manager); 
    
    mine_block("Miner1", state.mempool, state.manager, state.blockchain);
    
    ASSERT_EQ((double)state.blockchain.size(), 1.0, "Blockchain height should be 1");
    ASSERT_EQ((double)state.mempool.transactions.size(), 0.0, "Mempool should be empty");
    ASSERT_EQ(state.manager.getBalance("Miner1"), 40.0, "Miner didn't receive correct fees");
    
    std::cout << GREEN << " [PASS]" << RESET << std::endl;
    return true;
}

bool test_unconfirmed_chain() {
    std::cout << "Test 10: Unconfirmed Chain... ";
    TestState state;
    std::vector<UTXO> aliceUTXOs = state.manager.getAllUTXOofOwner("Alice");
    Transaction tx1;
    tx1.tx_id = "TX1";
    tx1.inputs.push_back(aliceUTXOs[0]);
    tx1.outputs.push_back({genUniqueUTXOID(), "TX1", "Bob", 50.0});
    state.mempool.add_transaction(tx1, state.manager);
    
    double bobBal = state.manager.getBalance("Bob");
    ASSERT_EQ(bobBal, 30.0, "Unconfirmed funds should not be spendable yet");
    
    std::cout << GREEN << " [PASS]" << RESET << std::endl;
    return true;
}

int main() {
    enableEscapeSequences();
    std::cout << BOLD << "\nRUNNING MANDATORY TESTS..." << RESET << "\n--------------------------------------------\n";
    
    int passed = 0;
    int total = 10;
    
    if(test_basic_valid_transaction()) passed++;
    if(test_multiple_inputs()) passed++;
    if(test_double_spend_same_tx()) passed++;
    if(test_mempool_double_spend()) passed++;
    if(test_insufficient_funds()) passed++;
    if(test_negative_amount()) passed++;
    if(test_zero_fee()) passed++;
    if(test_race_attack()) passed++;
    if(test_complete_mining_flow()) passed++;
    if(test_unconfirmed_chain()) passed++;

    std::cout << "\n--------------------------------------------\n";
    if (passed == total) {
        std::cout << GREEN << BOLD << "ALL TESTS PASSED (" << passed << "/" << total << ")" << RESET << std::endl;
    } else {
        std::cout << RED << BOLD << "SOME TESTS FAILED (" << passed << "/" << total << ")" << RESET << std::endl;
    }
    
    return 0;
}