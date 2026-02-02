#pragma once
#include <cstdlib>
#include <iostream>
#include "defs.cpp"

#if __has_include(<windows.h>)
#include <windows.h>
#include <conio.h>
int enableEscapeSequences() {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE) return GetLastError();
    DWORD dwMode = 0;
    if (!GetConsoleMode(hOut, &dwMode)) return GetLastError();
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    if (!SetConsoleMode(hOut, dwMode)) return GetLastError();
    return 0;
}
#else
inline int enableEscapeSequences() {
    return 0;
}
#endif

inline void waitForEnter() {
    std::cout << "\n" << CYAN << "Press Enter to continue..." << RESET;
    std::cin.ignore();
    std::cin.get();
}

inline void clearScreen() {
    #if __has_include(<windows.h>)
        system("cls");
    #else
        // system("clear");
				std::cout<<"\033[H\033[2J";
    #endif
}

inline void printHeader(int blockHeight, int mempoolSize, int utxoCount) {
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
