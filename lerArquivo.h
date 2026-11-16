#ifndef LEITOR_ARQUIVO_H
#define LEITOR_ARQUIVO_H

#include <fstream>
#include <string>
#include <iostream>

using namespace std;

// esqueleto usado durante a prática em aula

class LeitorArquivo {
private:
    ifstream file;

// abre, e se falhar exibe a msg
public:
    LeitorArquivo(string arquivo) {
        file.open(arquivo);
        if (!file.is_open()) {
            cerr << "Erro: arquivo nao encontrado -> " << arquivo << endl;
        }
    }
// destruotr fecha o arquivo
    ~LeitorArquivo() {
        if (file.is_open()) {
            file.close();
        }
    }

    bool aberto() {
        return file.is_open();
    }

    int lerProxCaracter() {
        int caractere = -1; // iniciliza
        //le cada carctyere e devolve como numero
        if (file.is_open()) {
            char c;
            if (file.get(c)) {
                // unsigned pra letra acentuada nao virar numero negativo (-1 eh so o EOF)
                caractere = (unsigned char)c;
            } else {
                caractere = -1;
            }
        }

        return caractere;
    }
};
#endif
