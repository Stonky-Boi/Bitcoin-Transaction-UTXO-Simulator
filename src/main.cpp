#include "mining.cpp"
int main() {
    UTXOManager manager;
    Mempool mempool;
    manager.generateUTXO("genesis",0,50.0,"Alice");
    manager.generateUTXO("genesis",1,30.0,"Bob");
    manager.generateUTXO("genesis",2,20.0,"Charlie");
    manager.generateUTXO("genesis",3,10.0,"David");
    manager.generateUTXO("genesis",4,5.0,"Eve");
    int choice;
    while (true) {
        std::cout<<"\n=== Bitcoin Transaction Simulator ===\n";
        std::cout<<"1. Create transaction\n2. View UTXO set\n3. View mempool\n4. Mine block\n5. Exit\nEnter choice: ";
        if (!(std::cin>>choice)||choice==5) break;
        if (choice==1) {
            std::string s,r; double a;
            std::cout<<"Sender: ";std::cin>>s;
            std::cout<<"Recipient: ";std::cin>>r;
            std::cout<<"Amount: ";std::cin>>a;
            std::vector<UTXO> owned=manager.getAllUTXOofOwner(s);
            Transaction tx;
            tx.tx_id=genUniqueTransactionID();
            double current=0;
            for (auto& u:owned) {
                tx.inputs.push_back(u);
                current+=u.value;
                if (current>=a+0.001) break;
            }
            if (current>=a+0.001) {
                tx.outputs.push_back({genUniqueUTXOID(),tx.tx_id,r,a});
                tx.outputs.push_back({genUniqueUTXOID(),tx.tx_id,s,current-a-0.001});
                auto res=mempool.add_transaction(tx,manager);
                std::cout<<res.second<<std::endl;
            } else {
                std::cout<<"Insufficient funds"<<std::endl;
            }
        } else if (choice==2) {
            for (auto const& [id,vec]:manager.utxo_set)
                for (auto& u:vec) std::cout<<u.owner<<": "<<u.value<<" ("<<u.id<<")"<<std::endl;
        } else if (choice==3) {
            for (auto& tx:mempool.transactions) std::cout<<tx.tx_id<<" Fee: "<<tx.fee<<std::endl;
        } else if (choice==4) {
            std::string m;std::cout<<"Miner: ";std::cin>>m;
            mine_block(m,mempool,manager);
        }
    }
    return 0;
}
