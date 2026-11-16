// Nomes: Nathália Rodrigues (RA 2670097), Felipe Kenzo (RA 2669986)
/*
 * Analisador Lexico - Linguagem GYH
 */

#include <iostream>
#include <fstream>
#include <string>
#include "lexico.h"

#ifdef _WIN32
#include <direct.h>
#include <windows.h>
#else
#include <sys/stat.h>
#endif

using namespace std;

// cria a pasta output caso ela ainda nao exista
void criarPastaOutput(){
#ifdef _WIN32
    _mkdir("output");
#else
    mkdir("output", 0777);
#endif
}

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

// cria o analisador lexico
int main(int argc, char* argv[]){
#ifdef _WIN32
    SetConsoleOutputCP(65001); // acentos das mensagens no terminal do windows
#endif

    if(argc < 2){
        cerr << "Uso: " << argv[0] << " <arquivo.gyh>" << endl;
        return 1;
    }

    Lexico lex(argv[1]);
    if(!lex.arquivoAberto()){
        return 1;
    }

    // arq de saida no ouput
    criarPastaOutput();
    string caminhoSaida = "output/" + nomeBase(argv[1]) + ".txt"; // nome base do arq
    ofstream saida(caminhoSaida); // escreve no arq de saida
    lex.setSaidaErro(saida);

    Token * t = lex.proximoToken();
    while(t->tipo != FimArquivo && t->tipo != Erro){
        cout << t -> toString() << endl; // imprime formatado no terminal
        saida << t -> toString() << endl; // escreve no arq de saida
        delete t;
        t = lex.proximoToken();
    }

    // erro lexico: a msg ja saiu no lexico e a analise eh interrompida ali
    if(t->tipo == Erro){
        delete t;
        return 1;
    }

    cout << t -> toString() << endl; // <EOF>
    saida << t -> toString() << endl;
    delete t;

    return 0;
}
