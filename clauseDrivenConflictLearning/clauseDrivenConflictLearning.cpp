#include <algorithm>
#include <cmath>
#include <fstream>
#include <random>
#include <vector>
#include "memutils.h"
#include <filesystem>
#include <cstring>
using namespace std;
#pragma GCC optimize("O3,fast-math,unroll-loops")

enum SatState {
  SAT,
  UNSAT,
  NECUNOSCUT
};

class SAT_cdcl {

  /*
   * 1 - bifat true
   * 0 - bifat false
   * -1 - nebifat
   */
  vector<int> literali;

  vector<vector<int>> literaliPerClauza;

  vector<int> frecventaLiteral;

  vector<int> polaritateLiteral;

  vector<int> frecventaLiteralInitiala;
  int numarLiterali;
  int numarClauze;
  int kappa_antecedent;

  vector<int> nivelDecizieLiteral;

  vector<int> antecedentLiteral;
  int numarLiteraliBifati;
  bool nesatisfiabilGasit;
  int nrAlegeri;
  random_device random_generator;
  mt19937 generator;

  int propagareaUnitatii(int);

  void bifeazaLiteral(int, int, int);
  void debifeazaLiteral(int);
  int literalToValoare(int);
  int analizaConflictAndBacktrack(int);
  vector<int> &rezolventa(vector<int> &,int);
  int alegeUrmatoareVariabila();
  bool totiLiteraliiBifati();
  void showRezultat(int, char *[],long long,size_t);

public:
  SAT_cdcl() : generator(random_generator()) {}
  void initializeaza(char*);
  int CDCL();
  void work( char* []);
};


void SAT_cdcl::initializeaza(char *file) {
  char c;
  string s;
  ifstream f(file);
  f.tie(nullptr);
  while (true) {
    f >> c;
    if (c == 'c') {
      getline(f, s);
    }
    else            {
      f >> s;
      break;
    }
  }
  f >> numarLiterali;
  f >> numarClauze;
  numarLiteraliBifati = 0; // no literals assigned so far
  // set the default values
  kappa_antecedent = -1;
  nrAlegeri = 0;
  nesatisfiabilGasit = false;
  // set the vectors to their appropriate sizes and initial values
  literali.clear();
  literali.resize(numarLiterali, -1);
  frecventaLiteral.clear();
  frecventaLiteral.resize(numarLiterali, 0);
  polaritateLiteral.clear();
  polaritateLiteral.resize(numarLiterali, 0);
  literaliPerClauza.clear();
  literaliPerClauza.resize(numarClauze);
  antecedentLiteral.clear();
  antecedentLiteral.resize(numarLiterali, -1);
  nivelDecizieLiteral.clear();
  nivelDecizieLiteral.resize(numarLiterali, -1);

  int literal;
  int nrLiteraliInClauza = 0;

  for (int i = 0; i < numarClauze; i++) {
    nrLiteraliInClauza = 0;
    while (true)
    {
      f >> literal;
      if (literal > 0){
        literaliPerClauza[i].push_back(literal);
        frecventaLiteral[literal - 1]++;
        polaritateLiteral[literal - 1]++;
      }
      else if (literal < 0) {
        literaliPerClauza[i].push_back(literal);
        frecventaLiteral[-1 - literal]++;
        polaritateLiteral[-1 - literal]--;
      }
      else {
        if (nrLiteraliInClauza == 0) {
          nesatisfiabilGasit = true;
        }
        break;
      }
      nrLiteraliInClauza++;
    }
  }
  frecventaLiteralInitiala = frecventaLiteral;
}


int SAT_cdcl::CDCL() {
  int nivelDecizie = 0;
  if (nesatisfiabilGasit) {
    return SatState::UNSAT;
  }
  int rezultatPropagareaUnitatii = propagareaUnitatii(nivelDecizie);
  if (rezultatPropagareaUnitatii == SatState::UNSAT) {
    return rezultatPropagareaUnitatii;
  }
  while (!totiLiteraliiBifati()) {
    int literalAles = alegeUrmatoareVariabila();
    nivelDecizie++;
    bifeazaLiteral(literalAles, nivelDecizie, -1);
    while (true) {
      rezultatPropagareaUnitatii = propagareaUnitatii(nivelDecizie);
      if (rezultatPropagareaUnitatii == SatState::UNSAT) {
        if (nivelDecizie == 0) {
          return rezultatPropagareaUnitatii;
        }
        nivelDecizie = analizaConflictAndBacktrack(nivelDecizie);
      }
      else {
        break;
      }
    }
  }

  return SatState::SAT;
}


int SAT_cdcl::propagareaUnitatii(int nivelDecizie) {
  bool clauzaUnitateGasita = false;
  int valoriFalse = 0;
  int nrLitNebifati = 0;
  int indexLiteral;
  bool clauzaSatisfiabila = false;
  int ultimulLiteralNebifat = -1;
  do {
    clauzaUnitateGasita = false;
    for (int i = 0; i < literaliPerClauza.size() && !clauzaUnitateGasita;
         i++) {
      valoriFalse = 0;
      nrLitNebifati = 0;
      clauzaSatisfiabila = false;
      for (int j = 0; j < literaliPerClauza[i].size(); j++) {

        indexLiteral =literalToValoare(literaliPerClauza[i][j]);
        if (literali[indexLiteral] == -1)
        {
          nrLitNebifati++;
          ultimulLiteralNebifat = j;
        } else if ((literali[indexLiteral] == 0 &&
                    literaliPerClauza[i][j] > 0) ||
                   (literali[indexLiteral] == 1 &&
                    literaliPerClauza[i][j] <
                        0))
        {
          valoriFalse++;
        } else
        {
          clauzaSatisfiabila = true;
          break;
        }
      }
      if (clauzaSatisfiabila)
      {
        continue;
      }

      if (nrLitNebifati == 1) {

        bifeazaLiteral(literaliPerClauza[i][ultimulLiteralNebifat],
                       nivelDecizie, i);
        clauzaUnitateGasita =true;
        break;
      }
      else if (valoriFalse == literaliPerClauza[i].size()) {

        kappa_antecedent = i;
        return SatState::UNSAT;
      }
    }
  } while (clauzaUnitateGasita);
  kappa_antecedent = -1;
  return SatState::NECUNOSCUT;
}

void SAT_cdcl::bifeazaLiteral(int variabila, int nivelDecizie,int antecedent) {
  int literal = literalToValoare(variabila);
  int valoare;
  if (variabila>0) {
    valoare = 1;
  }
  else {
    valoare = 0;
  }
  literali[literal] = valoare;
  nivelDecizieLiteral[literal] = nivelDecizie;
  antecedentLiteral[literal] = antecedent;
  frecventaLiteral[literal] =-1;
  numarLiteraliBifati++;
}


void SAT_cdcl::debifeazaLiteral(int literal_index) {
  literali[literal_index] = -1;
  nivelDecizieLiteral[literal_index] = -1;
  antecedentLiteral[literal_index] = -1;
  frecventaLiteral[literal_index] =frecventaLiteralInitiala[literal_index];
  numarLiteraliBifati--;
}


int SAT_cdcl::literalToValoare(int variabila) {
  if (variabila > 0)
    return variabila-1;
  return -variabila-1;
}


int SAT_cdcl::analizaConflictAndBacktrack(int nivel_decizie) {
  vector<int> clauza_invatata = literaliPerClauza[kappa_antecedent];
  int nivel_decizie_conflict = nivel_decizie;
  int nr_lit_nivel =0;
  int literal_ales;
  int literal;
  do {
    nr_lit_nivel = 0;
    for (int i = 0; i < clauza_invatata.size(); i++) {
      literal = literalToValoare(clauza_invatata[i]); // get the index
      if (nivelDecizieLiteral[literal] == nivel_decizie_conflict) {
        nr_lit_nivel++;
      }
      if (nivelDecizieLiteral[literal] == nivel_decizie_conflict &&
          antecedentLiteral[literal] != -1) {
        literal_ales = literal;
      }
    }
    if (nr_lit_nivel == 1) {
      break;
    }
    clauza_invatata = rezolventa(clauza_invatata, literal_ales);
  } while (true);
  literaliPerClauza.push_back(clauza_invatata); // add the learnt clause to the list
  for (int i = 0; i < clauza_invatata.size(); i++) {
    int literal_index = literalToValoare(clauza_invatata[i]);
    int update = (clauza_invatata[i] > 0) ? 1 : -1;
    polaritateLiteral[literal_index] += update;
    if (frecventaLiteral[literal_index] != -1) {
      frecventaLiteral[literal_index]++;
    }
    frecventaLiteralInitiala[literal_index]++;
  }
  numarClauze++;
  int backtracked_decision_level = 0;
  for (int i = 0; i < clauza_invatata.size(); i++) {
    int literal_index = literalToValoare(clauza_invatata[i]);
    int decision_level_here = nivelDecizieLiteral[literal_index];
   if (decision_level_here != nivel_decizie_conflict &&
        decision_level_here > backtracked_decision_level) {
      backtracked_decision_level = decision_level_here;
    }
  }
  for (int i = 0; i < literali.size(); i++) {
    if (nivelDecizieLiteral[i] > backtracked_decision_level) {
      debifeazaLiteral(i);
    }
  }
  return backtracked_decision_level;
}


vector<int> &SAT_cdcl::rezolventa(vector<int> &input_clause, int literal) {

  vector<int> second_input =literaliPerClauza[antecedentLiteral[literal]];

  input_clause.insert(input_clause.end(), second_input.begin(),
                      second_input.end());
  for (int i = 0; i < input_clause.size(); i++) {

    if (input_clause[i] == literal + 1 || input_clause[i] == -literal - 1) {
      input_clause.erase(input_clause.begin() + i);
      i--;
    }
  }

  sort(input_clause.begin(), input_clause.end());
  input_clause.erase(unique(input_clause.begin(), input_clause.end()),
                     input_clause.end());
  return input_clause;
}


int SAT_cdcl::alegeUrmatoareVariabila() {
  uniform_int_distribution<int> alegeRamura(1, 10);
  uniform_int_distribution<int> alegeLiteral(0, numarLiterali - 1);
  int valoareRandom = alegeRamura(generator);
  bool thresholdIncercari = false;
  int nrIncercari = 0;
  do {

    if (valoareRandom > 4 || numarLiteraliBifati < numarLiterali / 2 ||
        thresholdIncercari) {
      nrAlegeri++;

      if (nrAlegeri == 20 * numarLiterali) {
        for (int i = 0; i < literali.size(); i++) {
          frecventaLiteralInitiala[i] /= 2;
          if (frecventaLiteral[i] != -1) {
            frecventaLiteral[i] /= 2;
          }
        }
        nrAlegeri = 0;
      }
      int variabila = distance(
          frecventaLiteral.begin(),
          max_element(frecventaLiteral.begin(), frecventaLiteral.end()));
      if (polaritateLiteral[variabila] >= 0) {
        return variabila + 1;
      }
      return -variabila - 1;
    }
    else
    {

      while (nrIncercari < 10 * numarLiterali) {
        int variabila = alegeLiteral(generator);
        if (frecventaLiteral[variabila] != -1){
          if (polaritateLiteral[variabila] >= 0) {
            return variabila + 1;
          }
          return -variabila - 1;
        }
        nrIncercari++;
      }
      thresholdIncercari = true;  
    }
  } while (thresholdIncercari);
}

/*
* funcție pentru a verifica dacă toate variabilele au fost atribuite până acum
* Valoare returnată: adevărat, dacă da; fals, dacă nu
 */
bool SAT_cdcl::totiLiteraliiBifati() {
  return numarLiterali == numarLiteraliBifati;
}

/*
* funcție pentru afișarea rezultatului
* Argumente:
* statusRezultat – starea returnată de CDCL
* argv - vector cu fisierele pt input si output
* elapsed - timpul cat a rulat algoritmul
* peakSize - memoria maxima utilizata
 */
void SAT_cdcl::showRezultat(int statusRezultat, char * argv[],long long elapsed,size_t peakSize) {
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
    if (statusRezultat == SatState::SAT)
    {
      g << "SAT,";
      /*for (int i = 0; i < literali.size(); i++) {
        if (i != 0) {
          g << " ";
        }
        if (literali[i] != -1) {
          g << pow(-1, (literali[i] + 1)) * (i + 1);
        } else // for literals which can take either value, arbitrarily assign
               // them to be true
        {
          g << (i + 1);
        }
      }
      g << " 0";*/
    } else
    {
      g << "UNSAT,";
    }

    g<<elapsed<<"μs,";
    g<<peakSize<<"B,";
    g<< peakSize/1024<<"KB,";
    g<< peakSize/1024/1024<<"MB,";
    g<< peakSize/1024/1024/1024<<"GB\n";
  /*g<<"Timp de execuție: "<<elapsed<<"μs"<<'\n';
  g<<"Memorie consumată: "<< peakSize<<"B."<<'\n';
  g<<"Memorie consumată: "<< peakSize/1024<<"KB."<<'\n';
  g<<"Memorie consumată: "<< peakSize/1024/1024<<"MB."<<'\n';
  g<<"Memorie consumată: "<< peakSize/1024/1024/1024<<"GB."<<'\n';*/
  g.close();
}

/*
 * funcție pentru rezolvarea problemei apelând funcția CDCL() și apoi afișând rezultatul
 */
void SAT_cdcl::work( char *argv[]) {
  auto start = std::chrono::high_resolution_clock::now();
  int statusRezultat = CDCL();
  auto end = std::chrono::high_resolution_clock::now();
  auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end-start).count();
  size_t peakSize = getPeakRSS();

  showRezultat(statusRezultat,argv,elapsed,peakSize );
}

int main(int argc,char *argv[]) {
  std::ios::sync_with_stdio(false);
  SAT_cdcl instanta;
  instanta.initializeaza(argv[1]);

  instanta.work(argv);
  return 0;
}