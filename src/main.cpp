#include "mining.cpp"
#include "utils.hpp"

int main() {
    enableEscapeSequences();
    UTXOManager manager;
    Mempool mempool;
    std::vector<Block> blockchain;

    // Genesis state
    manager.generateUTXO("genesis",0,50.0,"Alice");
    manager.generateUTXO("genesis",1,30.0,"Bob");
    manager.generateUTXO("genesis",2,20.0,"Charlie");
    manager.generateUTXO("genesis",3,10.0,"David");
    manager.generateUTXO("genesis",4,5.0,"Eve");

    int choice;
    while (true) {
        int utxoCount = 0;
        for(auto const& [id, vec] : manager.utxo_set) utxoCount += vec.size();
        
        printHeader(blockchain.size(), mempool.transactions.size(), utxoCount);

        std::cout << BOLD << "\nActions:" << RESET << std::endl;
        std::cout << " " << GREEN << "1." << RESET << " Create transaction\n";
        std::cout << " " << GREEN << "2." << RESET << " View UTXO set\n";
        std::cout << " " << GREEN << "3." << RESET << " View mempool\n";
        std::cout << " " << GREEN << "4." << RESET << " Mine block\n";
        std::cout << " " << GREEN << "5." << RESET << " View Blockchain (History)\n";
        std::cout << " " << RED   << "6." << RESET << " Exit\n";
        std::cout << "\n" << CYAN << "Enter choice: " << RESET;
        
        if (!(std::cin>>choice)) {
            std::cin.clear();
            std::cin.ignore(1000, '\n');
            continue;
        }

        if (choice==6) break;

        std::cout << "\n--------------------------------------------\n";
        if (choice == 1) {
            std::string s, r; double a;
            std::cout << "Sender: "; std::cin >> s;
            std::cout << "Recipient: "; std::cin >> r;
            std::cout << "Amount: "; std::cin >> a;
            
            std::vector<UTXO> owned = manager.getAllUTXOofOwner(s);
            
            if (owned.empty()) {
                std::cout << RED << "Sender has no UTXOs!" << RESET << std::endl;
            } else {
                std::vector<ToPay> payments = {{s, r, a}};
                Transaction tx(s, payments, owned);

                if (tx.is_valid) {
                    auto res = mempool.add_transaction(tx, manager);
                    if(res.first) std::cout << GREEN << res.second << RESET << std::endl;
                    else std::cout << RED << "Mempool Error: " << res.second << RESET << std::endl;
                } else {
                    std::cout << RED << "Error: Insufficient funds" << RESET << std::endl;
                }
            }
        } else if (choice==2) {
            std::cout << BOLD << "Current UTXO Set:" << RESET << std::endl;
            bool empty = true;
            for (auto const& [id,vec]:manager.utxo_set) {
                for (auto& u:vec) {
                    std::cout << " - " << CYAN << u.owner << RESET << ": " << YELLOW << u.value << " BTC" << RESET << " (" << u.id << ")" << std::endl;
                    empty = false;
                }
            }
            if(empty) std::cout << " (Empty)" << std::endl;

        } else if (choice==3) {
            std::cout << BOLD << "Mempool Transactions:" << RESET << std::endl;
            if (mempool.transactions.empty()) std::cout << " (Empty)" << std::endl;
            for (auto& tx:mempool.transactions) {
                std::cout << " - " << tx.tx_id << " | Fee: " << GREEN << tx.fee << RESET << std::endl;
            }

        } else if (choice==4) {
            std::string m;
            std::cout << "Miner Name/Address: "; std::cin >> m;
            mine_block(m, mempool, manager, blockchain);

        } else if (choice==5) {
            std::cout << BOLD << "Blockchain History:" << RESET << std::endl;
            if (blockchain.empty()) std::cout << " (No blocks mined yet)" << std::endl;
            
            for(const auto& block : blockchain) {
                std::cout << MAGENTA << "Block #" << block.height << RESET << " [" << block.hash << "]\n";
                std::cout << "  Miner: " << block.miner << "\n";
                std::cout << "  Prev Hash: " << block.prev_hash << "\n";
                std::cout << "  Tx Count: " << block.transactions.size() << "\n";
                std::cout << "  Total Fees: " << block.total_fees << "\n";
                std::cout << "--------------------------------------------\n";
            }
        }

        waitForEnter();
    }
    return 0;
}
