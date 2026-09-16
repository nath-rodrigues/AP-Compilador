/*
 * Analisador Lexico - Linguagem GYH
 * Nomes: Nathália (RA 2670097), Felipe Higashi (RA 2669986)
 */

#include <iostream>
#include <fstream>
#include <string>
#include "lexico.h"

using namespace std;

// extrai o nome do arquivo (sem pasta e sem extensao) a partir do caminho de entrada
string nomeBase(const string &caminho){
    size_t inicio = caminho.find_last_of("/\\");
    inicio = (inicio == string::npos) ? 0 : inicio + 1;

    size_t fim = caminho.find_last_of('.');
    if(fim == string::npos || fim < inicio){
        fim = caminho.size();
    }

    return caminho.substr(inicio, fim - inicio);
}

int main(int argc, char* argv[]){
    Lexico lex(argv[1]);

    string caminhoSaida = "output/" + nomeBase(argv[1]) + ".txt";
    ofstream saida(caminhoSaida);
    lex.setSaidaErro(saida);

    Token * t = lex.proximoToken();
    while(t != nullptr){
        cout << t -> toString() << endl;
        saida << t -> toString() << endl;
        delete t;
        t = lex.proximoToken();
    }
    cout << "<EOF>" << endl;
    saida << "<EOF>" << endl;

    return 0;
}