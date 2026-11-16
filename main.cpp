// Nomes: Nathália Rodrigues (RA 2670097), Felipe Kenzo (RA 2669986)
/*
 * Analisador Sintatico - Linguagem GYH
 * Tipo: descendente recursivo (TDP sem backtracking)
 */

#include <iostream>
#include <fstream>
#include <string>
#include "lexico.h"
#include "sintatico.h"

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

// so a analise lexica: imprime os tokens no formato <Tipo, "lexema">
void imprimirTokens(Lexico &lex){
    Token *t = lex.proximoToken();
    while(t->tipo != FimArquivo){
        // token de erro nao eh impresso: a msg de erro ja saiu no lexico
        if(t->tipo != Erro){
            cout << t->toString() << endl;
        }
        delete t;
        t = lex.proximoToken();
    }
    cout << t->toString() << endl; // <EOF>
    delete t;
}

int main(int argc, char* argv[]){
#ifdef _WIN32
    SetConsoleOutputCP(65001); // acentos das mensagens no terminal do windows
#endif

    // com a opcao -t so roda o lexico e imprime a lista de tokens
    bool soTokens = (argc >= 3 && argv[1][0] == '-' && argv[1][1] == 't');
    if(argc < 2 || (argv[1][0] == '-' && !soTokens)){
        cerr << "Uso: " << argv[0] << " [-t] <arquivo.gyh>" << endl;
        return 1;
    }
    const char *arquivo = soTokens ? argv[2] : argv[1];

    // o lexico entrega os tokens e o sintatico vai pedindo um por um
    Lexico lex(arquivo);
    if(!lex.arquivoAberto()){
        return 1;
    }

    if(soTokens){
        imprimirTokens(lex);
        return lex.totalErros() == 0 ? 0 : 1;
    }
    Sintatico sint(lex);

    // arq de saida no output (mesmas mensagens do terminal)
    criarPastaOutput();
    string caminhoSaida = "output/" + nomeBase(arquivo) + ".txt"; // nome base do arq
    ofstream saida(caminhoSaida);
    lex.setSaidaErro(saida);
    sint.setSaidaErro(saida);

    bool ok = sint.analisar();

    string resultado;
    if(ok){
        resultado = "Análise sintática concluída: nenhum erro encontrado.";
    } else {
        resultado = "Análise concluída com erros: " +
                    to_string(lex.totalErros()) + " léxico(s) e " +
                    to_string(sint.totalErros()) + " sintático(s).";
    }
    cout << resultado << endl;
    saida << resultado << endl;

    return ok ? 0 : 1;
}
