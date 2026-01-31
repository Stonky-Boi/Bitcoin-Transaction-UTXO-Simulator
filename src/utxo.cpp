#include "defs.cpp"
class UTXOManager {
public:
    std::map<std::string,std::vector<UTXO>> utxo_set;
    void generateUTXO(std::string tx_id,int index,double amount,std::string owner) {
        utxo_set[tx_id].push_back(UTXO{genUniqueUTXOID(),tx_id,owner,amount});
    }
    double consumeUTXO(const UTXO& utxo) {
        if (!utxo_set.count(utxo.parent_tx_id)) return 0.0;
        auto& v=utxo_set[utxo.parent_tx_id];
        for (auto it=v.begin();it!=v.end();++it) {
            if (*it==utxo) {
                double val=it->value;
                v.erase(it);
                if (v.empty()) utxo_set.erase(utxo.parent_tx_id);
                return val;
            }
        }
        return 0.0;
    }
    double getBalance(const std::string& owner) {
        double balance=0;
        for (auto&& i:utxo_set) for (auto j:i.second) {
            if (j.owner==owner) balance+=j.value;
        }
        return balance;
    }
    bool exists(const UTXO& utxo) {
        if (!utxo_set.count(utxo.parent_tx_id)) return false;
        for (int i=0;i<utxo_set[utxo.parent_tx_id].size();i++) {
            if (utxo_set[utxo.parent_tx_id][i]==utxo) {
                return true;
            }
        }
        return false;
    }
    std::pair<std::string,int64_t> getIndex(const UTXO& utxo) {
        if (!utxo_set.count(utxo.parent_tx_id)) return {};
        for (int i=0;i<utxo_set[utxo.parent_tx_id].size();i++) {
            if (utxo_set[utxo.parent_tx_id][i]==utxo) {
                return {utxo.parent_tx_id,i};
            }
        }
        return {};
    }
    std::vector<UTXO> getAllUTXOofOwner(const std::string& owner) {
        std::vector<UTXO> res;
        for (auto const& [tx_id,v]:utxo_set) {
            for (auto const& u:v) {
                if (u.owner==owner) res.push_back(u);
            }
        }
        std::sort(res.begin(),res.end(),[](const UTXO& a,const UTXO& b) {
            return a.value<b.value;
        });
        return res;
    }
};
