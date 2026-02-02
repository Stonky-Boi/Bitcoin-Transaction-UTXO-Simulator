#pragma once
#include <cstdlib>
#include <iostream>
#include "defs.cpp"

void waitForEnter() {
    std::cout << "\n" << CYAN << "Press Enter to continue..." << RESET;
    std::cin.ignore();
    std::cin.get();
}

void clearScreen() {
    #ifdef _WIN32
        system("cls");
    #else
        // system("clear");
				std::cout<<"\033[H\033[2J";
    #endif
}

void printHeader(int blockHeight, int mempoolSize, int utxoCount) {
    clearScreen();
    std::cout << BLUE << BOLD << "============================================" << RESET << std::endl;
    std::cout << BLUE << BOLD << "      BITCOIN TRANSACTION SIMULATOR         " << RESET << std::endl;
    std::cout << BLUE << BOLD << "============================================" << RESET << std::endl;
    
    // Status Bar
    std::cout << " " << YELLOW << "Block Height: " << RESET << blockHeight;
    std::cout << " | " << YELLOW << "Mempool: " << RESET << mempoolSize << " txs";
    std::cout << " | " << YELLOW << "UTXO Set: " << RESET << utxoCount << " entries" << std::endl;
    std::cout << BLUE << "============================================" << RESET<< std::endl;
}
