#include "dpll_utils.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <cstring>

#include "memutils.h"

namespace fs = std::filesystem;
using Clause = vector<int>;
using ClauseSet = vector<vector<int>>;
[[nodiscard]] ClauseSet read_clauses(const char *file, int& lit_total_count, int& clause_count) {
    std::ifstream f(file);
    f.tie(nullptr);
    clause_count=0, lit_total_count=0;
    ClauseSet clauses;
    while (!f.eof()) {
        std::string line;
        std::getline(f,line);
        if (line.empty()) break;
        if (line =="%" || line=="0") continue;
        while (line.starts_with('c')) {
            std::getline(f,line);
        }
        if (line.starts_with("p cnf ")) {
            line = line.substr(strlen("p cnf "));
            std::istringstream is(line);
            is>>lit_total_count>>clause_count;
            continue;
        }
        std::istringstream is(line);
        Clause c;
        int lit = 0;
        while (is>>lit) {
            c.push_back(lit);
        }
        clauses.emplace_back(c);
    }
    f.close();
    return clauses;
}
int main(int argc, char* argv[]) {
    std::ios::sync_with_stdio(false);
    if(argc != 3){
        std::cerr<<"Wrong input. Usage ./resolution_naive_first_fit <path_to_cnf_file> <path_to_log_file>\n";
        return 1;
    }
    if(!fs::exists(argv[1])){
        std::cerr<<"File not found: "<< argv[1]<<'\n';
        return 1;
    }

    int clause_total_count=0, lit_total_count=0;
    ClauseSet v = read_clauses(argv[1],lit_total_count,clause_total_count);
    ofstream g(argv[2],std::ios::app);
    int poz;
    for (int i=0;i<strlen(argv[1]);i++) {
        if (argv[1][i] == '\\' && i!=strlen(argv[1])-1) {
            poz=i;
        }
    }
    for (int i=poz+1;i<strlen(argv[1]);i++) {
        g<<argv[1][i];
    }
    g<<",";

    Sat s(clause_total_count,lit_total_count,v);
    auto start = std::chrono::high_resolution_clock::now();
    bool result = det_satisfiability(s);
    auto end = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end-start).count();
    size_t peakSize = getPeakRSS();
    if (result) g << "SAT,";
    else g << "UNSAT,";
    g<<elapsed<<"μs,";
    g<<peakSize<<"B,";
    g<< peakSize/1024<<"KB,";
    g<< peakSize/1024/1024<<"MB,";
    g<< peakSize/1024/1024/1024<<"GB\n";
    g.close();
    return 0;
}
