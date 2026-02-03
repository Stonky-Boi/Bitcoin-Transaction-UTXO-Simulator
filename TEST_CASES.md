# UTXO Simulator Test Cases

This document details the 10 mandatory test cases used to validate the integrity, security, and logic of the Bitcoin Transaction Simulator. The tests cover standard operations, edge cases, and security exploits like double-spending and race attacks.

## Test Summary

| ID | Test Name | Result | Description |
| :--- | :--- | :--- | :--- |
| **1** | Basic Valid Transaction | PASS | Verifies simple transfer, change calculation, and fees. |
| **2** | Multiple Inputs | PASS | Aggregates multiple UTXOs to cover a large transaction amount. |
| **3** | Double-Spend (Same TX) | PASS | Prevents using the same UTXO twice within a single transaction. |
| **4** | Mempool Double-Spend | PASS | **First-Seen Rule:** Rejects a second transaction trying to spend a pending UTXO. |
| **5** | Insufficient Funds | PASS | Rejects transactions where input sum < output sum. |
| **6** | Negative Amount | PASS | Rejects outputs with negative values. |
| **7** | Zero Fee Transaction | PASS | Accepts valid transactions with 0 fee (protocol compliant). |
| **8** | Race Attack (First-Seen) | PASS | Ensures the first transaction seen is locked, rejecting subsequent higher-fee replacements (No RBF). |
| **9** | Complete Mining Flow | PASS | Validates block mining, UTXO set updates, and miner rewards. |
| **10** | Unconfirmed Chain | PASS | Prevents spending of unconfirmed (mempool) outputs before they are mined. |

---

## Detailed Test Scenarios

### 1. Basic Valid Transaction
* **Input:** Alice (Has 50 BTC) sends **10 BTC** to Bob.
* **What's Going On:**
    * The system selects Alice's 50 BTC UTXO as input.
    * **Fee:** calculated as `1 Input * 0.001 = 0.001 BTC`.
    * **Change:** `50 - 10 - 0.001 = 39.999 BTC` is calculated back to Alice.
* **Output:**
    * Transaction accepted.
    * Mempool size: 1.
    * Fee verified: **0.001 BTC**.

### 2. Multiple Inputs
* **Input:** Alice (Has 50 BTC + 20 BTC) sends **60 BTC** to Bob.
* **What's Going On:**
    * The simulator identifies that 50 BTC is insufficient.
    * It aggregates both UTXOs (Total Input: 70 BTC).
    * **Fee:** `2 Inputs * 0.001 = 0.002 BTC`.
    * **Change:** `70 - 60 - 0.002 = 9.998 BTC`.
* **Output:**
    * Transaction accepted with 2 Inputs.
    * Fee verified: **0.002 BTC**.

### 3. Double-Spend in Same Transaction
* **Input:** Maliciously constructed transaction where Alice tries to use the **same 50 BTC UTXO twice** in the input list to pay 80 BTC.
* **What's Going On:**
    * `Mempool::add_transaction` iterates through inputs.
    * It maintains a local `std::set` of input IDs used in the current transaction.
    * It detects the duplicate ID immediately.
* **Output:**
    * `Error: Double-spend in same TX`
    * Transaction Rejected.

### 4. Mempool Double-Spend (The "First-Seen" Rule)
* **Input:**
    1. **TX1:** Alice sends UTXO_A to Bob. (Added to Mempool).
    2. **TX2:** Alice sends **the same UTXO_A** to Charlie.
* **What's Going On:**
    * TX1 is valid and sits in the mempool.
    * When TX2 arrives, the Mempool checks against all *pending* transactions.
    * It sees UTXO_A is already locked by TX1.
* **Output:**
    * `Error: Double-spend: Input already pending in mempool`
    * TX2 Rejected.

### 5. Insufficient Funds
* **Input:** Bob (Has 30 BTC) tries to send **35 BTC** to Alice.
* **What's Going On:**
    * The transaction constructor sums Bob's available UTXOs (30).
    * Calculates `Input (30) < Output (35) + Fee`.
    * Sets internal validity flag `is_valid = false`.
* **Output:**
    * `Error: Insufficient funds`
    * Transaction Rejected.

### 6. Negative Amount
* **Input:** Alice tries to send **-5.0 BTC** to Bob.
* **What's Going On:**
    * The Mempool validator checks `out.value < 0` for all outputs.
* **Output:**
    * `Error: Negative output amount`
    * Transaction Rejected.

### 7. Zero Fee Transaction
* **Input:** Alice sends 10 BTC. The transaction is tampered with to have `Input == Output` (Fee = 0).
* **What's Going On:**
    * The protocol allows 0-fee transactions (though miners may prioritize them last).
    * `Total Input (50) - Total Output (50) = 0`.
    * The simulator accepts this as valid logic.
* **Output:**
    * Transaction Accepted.
    * Fee verified: **0.0 BTC**.

### 8. Race Attack Simulation
* **Input:**
    1. **TX1 (Low Fee):** Alice -> Bob (Fee 0.001). Arrives First.
    2. **TX2 (High Fee):** Alice -> Charlie (Fee 0.1). Arrives Second. Same UTXO.
* **What's Going On:**
    * TX1 is accepted into Mempool.
    * TX2 is submitted.
    * **Logic:** The system enforces strict "First-Seen Safe" policy. It does **not** support Replace-By-Fee (RBF).
    * TX2 is rejected immediately because the input is locked by TX1.
* **Output:**
    * TX2 Rejected (`Double-spend pending`).
    * **Mining Result:** Block contains TX1. Miner earns 0.001 BTC (not 0.1).

### 9. Complete Mining Flow
* **Input:** Mempool contains 1 valid transaction (Fee 0.001). "Hasher" mines the block.
* **What's Going On:**
    1. Block created with height `N+1`.
    2. Transaction inputs removed from UTXO set (consumed).
    3. Transaction outputs added to UTXO set (generated).
    4. **Coinbase TX:** Hasher receives the block reward + fees (0.001).
    5. Mempool cleared.
* **Output:**
    * `Block mined! Miner Hasher earned 0.001 BTC`
    * Blockchain height increases.
    * Miner balance increases.

### 10. Unconfirmed Chain
* **Input:**
    1. Alice sends 50 BTC to Bob (TX1 is in Mempool, **not mined**).
    2. Bob immediately tries to spend that 50 BTC.
* **What's Going On:**
    * Bob queries `UTXOManager`.
    * The Manager only tracks *mined/confirmed* UTXOs.
    * TX1 outputs are not yet in the global UTXO set.
* **Output:**
    * Bob's available balance: **0 BTC** (or initial 30).
    * Spending attempt fails (Sender has no UTXOs).
