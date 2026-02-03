# Bitcoin Transaction UTXO Simulator

A comprehensive educational simulator for understanding Bitcoin's Unspent Transaction Output (UTXO) model and blockchain mechanics. This project provides an interactive, command-line interface to demonstrate how transactions are created, validated, and confirmed through mining.

## Overview

The Bitcoin Transaction UTXO Simulator is a C++ application designed to help developers and cryptocurrency enthusiasts understand the fundamental concepts of Bitcoin's transaction model. Rather than using traditional account-based systems (like Ethereum), Bitcoin employs a UTXO-based approach where transactions consume previous outputs as inputs and produce new outputs.

This simulator allows users to:
- Create transactions between participants
- Manage the UTXO set (unspent transaction outputs)
- Observe mempool behavior (pending transactions)
- Mine blocks and extend the blockchain
- Explore transaction validation and fee mechanisms

## Features

### Core Functionality

- **Transaction Management**: Create transactions by specifying senders, recipients, and amounts with automatic change management
- **UTXO Set Tracking**: Maintain and display the current set of unspent outputs with flexible sorting options:
  - Sort by owner (alphabetical)
  - Sort by amount (descending)
  - Default view by transaction ID
- **Mempool Simulation**: Queue transactions before they're included in mined blocks
- **Mining System**: Simulate mining blocks with transaction inclusion and fee collection
- **Blockchain History**: View all mined blocks and their transactions

### User Interface

- Color-coded terminal output for easy navigation
- Real-time status display showing block height, mempool size, and UTXO count
- Interactive menu-driven interface
- Cross-platform support (Windows and Unix-based systems)

## Architecture

### Component Structure

- **defs.cpp**: Core data structures and definitions
  - `UTXO`: Represents an unspent transaction output
  - `Transaction`: Contains inputs (consumed UTXOs) and outputs (new UTXOs)
  - `Block`: Represents a mined block in the blockchain
  - Utility functions for ID generation and hashing

- **utxo.cpp**: UTXO management
  - `UTXOManager`: Manages the UTXO set, validates existence, tracks balances
  - Operations: generate, consume, query, and aggregate UTXOs

- **mining.cpp**: Mining and mempool logic
  - Block mining mechanics
  - Transaction validation
  - Fee calculation

- **utils.hpp**: Utility functions and display helpers
  - Terminal rendering and formatting
  - Cross-platform compatibility utilities
  - Color-coded output management

- **main.cpp**: Application entry point and interactive CLI

## Building and Running

### Prerequisites

- C++14 or later
- GCC compiler (g++)
- Make (optional, but recommended)

### Build Instructions

Using Make:
```bash
make build
```

Manual compilation:
```bash
g++ src/main.cpp -o build/main
```

### Running the Simulator

```bash
./build/main
```

### Cleanup

```bash
make clean
```

## Usage Example

1. **Launch the simulator** and observe the initial UTXO set (genesis UTXOs for Alice, Bob, Charlie, David, and Eve)

2. **Create a transaction**:
   - Select option 1 from the menu
   - Enter sender name (e.g., "Alice")
   - Enter recipient name (e.g., "Bob")
   - Enter amount to transfer (e.g., 10)
   - The transaction appears in the mempool

3. **View the UTXO Set**: Select option 2 to see current UTXOs with different sort options

4. **Check the Mempool**: Select option 3 to view pending transactions awaiting inclusion in a block

5. **Mine a Block**: Select option 4 to mine and include pending transactions in a new block

6. **Review Blockchain History**: Select option 5 to view all confirmed blocks and their contents

## Technical Details

### Transaction Model

- **Inputs**: Consumed UTXOs that fund the transaction
- **Outputs**: New UTXOs created as a result of the transaction
- **Fees**: Transaction cost calculated as `(total_input_value - total_output_value)`
- **Change**: Automatically managed output returning excess funds to the sender

### UTXO Set State

The UTXO set is stored as a map where:
- **Key**: Parent transaction ID
- **Value**: Vector of UTXOs created by that transaction

This structure enables efficient lookup and consumption of UTXOs.

### Mining Process

- Transactions are held in the mempool until mining occurs
- Miners select transactions from the mempool and include them in a new block
- Transaction validation occurs during mining (checking input/output validity)
- Miner collects fees from all transactions in the block

## Educational Value

This simulator is designed for:
- **Learning**: Understanding UTXO mechanics without dealing with actual Bitcoin complexity
- **Development**: Prototyping transaction logic for Bitcoin-based applications
- **Teaching**: Demonstrating blockchain concepts in classroom settings
- **Research**: Experimenting with transaction patterns and fee mechanisms

This is an educational project created for learning purposes. It is a simplified simulation of Bitcoin's transaction model and should not be used for any production or financial purposes.