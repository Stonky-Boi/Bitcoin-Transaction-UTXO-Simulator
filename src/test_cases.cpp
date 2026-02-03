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
    
    // Alice sends 10 to Bob
    std::vector<UTXO> aliceUTXOs = state.manager.getAllUTXOofOwner("Alice");
    // Transaction(sender, payments_vector, available_utxos)
    std::vector<ToPay> payments = {{"Alice", "Bob", 10.0}};
    Transaction tx("Alice", payments, aliceUTXOs);
    
    if (!tx.is_valid) {
        std::cout << RED << " [FAIL] Transaction constructed as invalid" << RESET << std::endl;
        return false;
    }

    auto res = state.mempool.add_transaction(tx, state.manager);
    
    ASSERT_TRUE(res.first, "Transaction should be valid");
    ASSERT_EQ((double)state.mempool.transactions.size(), 1.0, "Mempool should have 1 TX");
    
    // Verify Fee: 1 Input * 0.001
    ASSERT_EQ(state.mempool.transactions[0].fee, 0.001, "Fee should be exactly 0.001");
    
    // Verify Change exists (50 - 10 - 0.001 = 39.999)
    bool hasChange = false;
    for(auto& out : tx.outputs) {
        if(out.owner == "Alice" && std::abs(out.value - 39.999) < 1e-9) hasChange = true;
    }
    ASSERT_TRUE(hasChange, "Change output of 39.999 BTC not found");

    std::cout << GREEN << " [PASS]" << RESET << std::endl;
    return true;
}

bool test_multiple_inputs() {
    std::cout << "Test 2: Multiple Inputs... ";
    TestState state;
    
    // Alice needs 60 BTC. She has 50. Give her another 20.
    state.manager.generateUTXO("extra_funding", 0, 20.0, "Alice");
    std::vector<UTXO> aliceUTXOs = state.manager.getAllUTXOofOwner("Alice"); // 50 + 20
    
    // Send 60 BTC. Logic should pick both UTXOs (50+20=70).
    std::vector<ToPay> payments = {{"Alice", "Bob", 60.0}};
    Transaction tx("Alice", payments, aliceUTXOs);
    
    ASSERT_TRUE(tx.is_valid, "Transaction should be valid");
    ASSERT_EQ((double)tx.inputs.size(), 2.0, "Should consume 2 inputs");
    
    auto res = state.mempool.add_transaction(tx, state.manager);
    ASSERT_TRUE(res.first, "Mempool should accept multi-input TX");
    
    // Fee = 2 inputs * 0.001 = 0.002
    ASSERT_EQ(tx.fee, 0.002, "Fee calculation for 2 inputs incorrect");
    
    std::cout << GREEN << " [PASS]" << RESET << std::endl;
    return true;
}

bool test_double_spend_same_tx() {
    std::cout << "Test 3: Double-Spend in Same Transaction... ";
    TestState state;
    
    std::vector<UTXO> aliceUTXOs = state.manager.getAllUTXOofOwner("Alice");
    
    // Force the constructor to use the same UTXO twice by providing a list with duplicates
    std::vector<UTXO> corruptedUTXOs;
    corruptedUTXOs.push_back(aliceUTXOs[0]); // 50 BTC
    corruptedUTXOs.push_back(aliceUTXOs[0]); // 50 BTC (Duplicate)
    
    // Request 80 BTC. Greedy algo will pick first 50, then second 50 (which is the same ID).
    std::vector<ToPay> payments = {{"Alice", "Bob", 80.0}};
    Transaction tx("Alice", payments, corruptedUTXOs);
    
    // the Mempool MUST reject it.
    auto res = state.mempool.add_transaction(tx, state.manager);
    
    ASSERT_FALSE(res.first, "Should reject double usage of UTXO in one TX");
    std::cout << GREEN << " [PASS]" << RESET << std::endl;
    return true;
}

bool test_mempool_double_spend() {
    std::cout << "Test 4: Mempool Double-Spend... ";
    TestState state;
    
    std::vector<UTXO> aliceUTXOs = state.manager.getAllUTXOofOwner("Alice");
    
    // TX1: Alice -> Bob (10 BTC)
    Transaction tx1("Alice", {{"Alice", "Bob", 10.0}}, aliceUTXOs);
    state.mempool.add_transaction(tx1, state.manager);
    
    // TX2: Alice -> Charlie (10 BTC) using SAME inputs
    // The constructor is deterministic, it will pick the same 50 BTC UTXO again.
    Transaction tx2("Alice", {{"Alice", "Charlie", 10.0}}, aliceUTXOs);
    
    auto res = state.mempool.add_transaction(tx2, state.manager);
    
    ASSERT_FALSE(res.first, "Second TX spending same UTXO must be rejected by Mempool");
    
    std::cout << GREEN << " [PASS]" << RESET << std::endl;
    return true;
}

bool test_insufficient_funds() {
    std::cout << "Test 5: Insufficient Funds... ";
    TestState state;
    
    std::vector<UTXO> bobUTXOs = state.manager.getAllUTXOofOwner("Bob"); // Has 30
    
    // Try to send 35
    Transaction tx("Bob", {{"Bob", "Alice", 35.0}}, bobUTXOs);
    
    ASSERT_FALSE(tx.is_valid, "Transaction should be marked invalid by constructor");
    
    std::cout << GREEN << " [PASS]" << RESET << std::endl;
    return true;
}

bool test_negative_amount() {
    std::cout << "Test 6: Negative Amount... ";
    TestState state;
    
    std::vector<UTXO> aliceUTXOs = state.manager.getAllUTXOofOwner("Alice");
    
    // Try to send -5.0
    // The constructor creates the output object with -5.0. 
    // The Mempool validation checks "out.value < 0".

    Transaction tx("Alice", {{"Alice", "Bob", -5.0}}, aliceUTXOs);
    
    auto res = state.mempool.add_transaction(tx, state.manager);
    
    ASSERT_FALSE(res.first, "Should reject negative output amount");
    std::cout << GREEN << " [PASS]" << RESET << std::endl;
    return true;
}

bool test_zero_fee() {
    std::cout << "Test 7: Zero Fee Transaction... ";
    TestState state;
    
    std::vector<UTXO> aliceUTXOs = state.manager.getAllUTXOofOwner("Alice");
    
    // The constructor ENFORCES a fee of 0.001 per input. 
    // To test "Zero Fee", we must manually tamper with the transaction 
    Transaction tx("Alice", {{"Alice", "Bob", 10.0}}, aliceUTXOs);
    
    // Tamper: Refund the fee to the change output and set fee variable to 0
    for(auto& out : tx.outputs) {
        if(out.owner == "Alice") {
            out.value += tx.fee; // Give fee back to Alice
        }
    }
    tx.fee = 0.0; 
    
    auto res = state.mempool.add_transaction(tx, state.manager);
    
    ASSERT_TRUE(res.first, "Zero fee transactions are technically valid in Bitcoin (though usually non-standard)");
    ASSERT_EQ(state.mempool.transactions[0].fee, 0.0, "Fee should be 0");
    
    std::cout << GREEN << " [PASS]" << RESET << std::endl;
    return true;
}

bool test_race_attack() {
    std::cout << "Test 8: Race Attack (First Seen vs RBF)... ";
    TestState state;
    std::vector<UTXO> inputs = state.manager.getAllUTXOofOwner("Alice");
    
    // TX1: Low Fee (Standard Constructor = 0.001)
    Transaction tx1("Alice", {{"Alice", "Bob", 10.0}}, inputs);
    state.mempool.add_transaction(tx1, state.manager);

    // TX2: High Fee (Using same inputs)
    // We construct it normally, then tamper with it to increase fee.
    Transaction tx2("Alice", {{"Alice", "Charlie", 10.0}}, inputs);
    
    // Tamper: Decrease change output by 0.1 BTC (implicitly increasing fee)
    for(auto& out : tx2.outputs) {
        if(out.owner == "Alice") {
            out.value -= 0.1; 
        }
    }

    // Attempt to add TX2. With First-Seen rule, this should FAIL.
    auto res = state.mempool.add_transaction(tx2, state.manager);
    
    if(res.first) {
         std::cout << RED << " [FAIL] High fee TX was accepted into mempool (Should be rejected by First-Seen)" << RESET << std::endl;
         return false;
    }

    // Mine and verify TX1 is in block
    mine_block("Miner", state.mempool, state.manager, state.blockchain);
    
    bool lowMined = false;
    for(auto& tx : state.blockchain[0].transactions) {
        if(tx.tx_id == tx1.tx_id) lowMined = true;
    }
    
    ASSERT_TRUE(lowMined, "First-Seen transaction (TX1) should have been mined");
    std::cout << GREEN << " [PASS]" << RESET << std::endl;
    return true;
}

bool test_complete_mining_flow() {
    std::cout << "Test 9: Complete Mining Flow... ";
    TestState state;
    std::vector<UTXO> aliceUTXOs = state.manager.getAllUTXOofOwner("Alice");
    
    // Normal Transaction
    Transaction tx("Alice", {{"Alice", "Bob", 10.0}}, aliceUTXOs);
    state.mempool.add_transaction(tx, state.manager); 
    
    mine_block("Miner1", state.mempool, state.manager, state.blockchain);
    
    ASSERT_EQ((double)state.blockchain.size(), 1.0, "Blockchain height should be 1");
    ASSERT_EQ((double)state.mempool.transactions.size(), 0.0, "Mempool should be empty");
    
    // Miner earns fee (0.001)
    ASSERT_EQ(state.manager.getBalance("Miner1"), 0.001, "Miner didn't receive correct fees");
    
    std::cout << GREEN << " [PASS]" << RESET << std::endl;
    return true;
}

bool test_unconfirmed_chain() {
    std::cout << "Test 10: Unconfirmed Chain... ";
    TestState state;
    
    std::vector<UTXO> aliceUTXOs = state.manager.getAllUTXOofOwner("Alice");
    Transaction tx1("Alice", {{"Alice", "Bob", 50.0}}, aliceUTXOs); // Sends everything to Bob
    state.mempool.add_transaction(tx1, state.manager);
    
    // Bob checks balance. Since TX1 is in Mempool (not mined), 
    // UTXOManager (which tracks mined UTXOs) should not show it yet.
    double bobBal = state.manager.getBalance("Bob");
    ASSERT_EQ(bobBal, 30.0, "Unconfirmed funds should not be spendable yet (Balance should remain initial 30)");
    std::cout << GREEN << " [PASS]" << RESET << std::endl;
    return true;
}

int main() {
    enableEscapeSequences();
    std::cout << BOLD << "\nRUNNING TESTS..." << RESET << "\n--------------------------------------------\n";
    
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