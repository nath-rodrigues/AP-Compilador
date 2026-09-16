#ifndef LEXICO_H
#define LEXICO_H

#include <string>
#include <vector>
#include <unordered_map>
#include <cctype>
#include <iostream>
#include "lerArquivo.h"
#include "token.h"

using namespace std;

class Lexico{
    private:
        LeitorArquivo *arq;
        int *id;
        vector<int> pilha;   // buffer de caracteres "devolvidos"
        int linhaAtual;
        ostream *saidaErro = nullptr; // arquivo de output, para tambem registrar os erros la

        // le o proximo caractere, respeitando o buffer de lookahead,
        // e mantem o contador de linha atualizado
        int lerChar(){
            int c;
            if(!pilha.empty()){
                c = pilha.back();
                pilha.pop_back();
            } else {
                c = arq->lerProxCaracter();
            }
            if(c == '\n'){
                linhaAtual++;
            }
            return c;
        }

        // devolve um caractere para ser relido na proxima chamada de lerChar()
        void devolverChar(int c){
            if(c == -1) return;
            if(c == '\n'){
                linhaAtual--;
            }
            pilha.push_back(c);
        }

        void erroLexico(int linha, const string &lexema){
            string msg = "Erro Léxico na linha " + to_string(linha) + ": Desconhecido \"" + lexema + "\"";
            cout << msg << endl;
            if(saidaErro != nullptr){
                (*saidaErro) << msg << endl;
            }
        }

    public:
        Lexico(string arquivo){
            id = new int(0);
            arq = new LeitorArquivo(arquivo);
            linhaAtual = 1;
        }

        // define o arquivo de saida onde os erros tambem devem ser gravados
        void setSaidaErro(ostream &saida){
            saidaErro = &saida;
        }

        Token* proximoToken(){
            static const unordered_map<string,string> palavras = {
                {"DEC","PCDec"}, {"PROG","PCProg"}, {"INT","PCInt"}, {"REAL","PCReal"},
                {"LER","PCLer"}, {"IMPRIMIR","PCImprimir"}, {"SE","PCSe"}, {"ENTAO","PCEntao"},
                {"SENAO","PCSenao"}, {"ENQTO","PCEnqto"}, {"INI","PCIni"}, {"FIM","PCFim"},
                {"E","OpBoolE"}, {"OU","OpBoolOu"}
            };

            while(true){
                int c = lerChar();
                if(c == -1){
                    return nullptr;
                }
                char ch = (char)c;

                if(isspace((unsigned char)ch)){
                    continue;
                }

                // Comentario: # ate o fim da linha
                if(ch == '#'){
                    int cc;
                    while((cc = lerChar()) != -1 && cc != '\n'){
                        // ignora
                    }
                    continue;
                }

                int linhaToken = linhaAtual;

                // Palavras reservadas e operadores booleanos (E, OU):
                // comecam com letra maiuscula
                if(isupper((unsigned char)ch)){
                    string lexema;
                    lexema += ch;

                    int prox;
                    while((prox = lerChar()) != -1 && isalnum((unsigned char)prox)){
                        lexema += (char)prox;
                    }
                    if(prox != -1){
                        devolverChar(prox);
                    }

                    auto it = palavras.find(lexema);
                    if(it != palavras.end()){
                        (*id)++;
                        return new Token(lexema, it->second, id);
                    }

                    erroLexico(linhaToken, lexema);
                    continue;
                }

                // Variavel: letra minuscula seguida de letras/numeros
                if(islower((unsigned char)ch)){
                    string lexema;
                    lexema += ch;

                    int prox;
                    while((prox = lerChar()) != -1 && isalnum((unsigned char)prox)){
                        lexema += (char)prox;
                    }
                    if(prox != -1){
                        devolverChar(prox);
                    }

                    (*id)++;
                    return new Token(lexema, "Var", id);
                }

                // Numeros: NumInt ou NumReal (com ponto)
                if(isdigit((unsigned char)ch)){
                    string lexema;
                    lexema += ch;

                    int prox = lerChar();
                    while(prox != -1 && isdigit((unsigned char)prox)){
                        lexema += (char)prox;
                        prox = lerChar();
                    }

                    bool real = false;
                    if(prox == '.'){
                        int prox2 = lerChar();
                        if(prox2 != -1 && isdigit((unsigned char)prox2)){
                            real = true;
                            lexema += '.';
                            lexema += (char)prox2;

                            prox2 = lerChar();
                            while(prox2 != -1 && isdigit((unsigned char)prox2)){
                                lexema += (char)prox2;
                                prox2 = lerChar();
                            }
                            prox = prox2;
                        } else {
                            if(prox2 != -1){
                                devolverChar(prox2);
                            }
                            devolverChar(prox); // devolve o '.'
                            prox = -1;          // ja tratado, nada mais a devolver
                        }
                    }

                    if(prox != -1){
                        devolverChar(prox);
                    }

                    (*id)++;
                    return new Token(lexema, real ? "NumReal" : "NumInt", id);
                }

                // Cadeia de caracteres entre aspas
                if(ch == '"'){
                    string bruto;   // texto lido, incluindo a aspa inicial (usado so na mensagem de erro)
                    string lexema;  // conteudo da string, sem as aspas
                    // isso eh pra exemplos retornam strings
                    bruto += ch;

                    bool fechado = false;
                    int prox;
                    while((prox = lerChar()) != -1 && prox != '\n'){
                        if(prox == '"'){
                            fechado = true;
                            break;
                        }
                        bruto += (char)prox;
                        lexema += (char)prox;
                    }

                    if(!fechado){
                        if(prox != -1){
                            devolverChar(prox);
                        }
                        erroLexico(linhaToken, bruto);
                        continue;
                    }

                    (*id)++;
                    return new Token(lexema, "Cadeia", id);
                }

                switch(ch){
                    case '*':
                        (*id)++;
                        return new Token("*", "OpAritMult", id);
                    case '/':
                        (*id)++;
                        return new Token("/", "OpAritDiv", id);
                    case '+':
                        (*id)++;
                        return new Token("+", "OpAritSoma", id);
                    case '-':
                        (*id)++;
                        return new Token("-", "OpAritSub", id);
                    case '(':
                        (*id)++;
                        return new Token("(", "AbrePar", id);
                    case ')':
                        (*id)++;
                        return new Token(")", "FechaPar", id);

                    case '<': {
                        int prox = lerChar();
                        if(prox == '='){
                            (*id)++;
                            return new Token("<=", "OpRelMenorIgual", id);
                        }
                        if(prox != -1){
                            devolverChar(prox);
                        }
                        (*id)++;
                        return new Token("<", "OpRelMenor", id);
                    }
                    case '>': {
                        int prox = lerChar();
                        if(prox == '='){
                            (*id)++;
                            return new Token(">=", "OpRelMaiorIgual", id);
                        }
                        if(prox != -1){
                            devolverChar(prox);
                        }
                        (*id)++;
                        return new Token(">", "OpRelMaior", id);
                    }
                    case '=': {
                        int prox = lerChar();
                        if(prox == '='){
                            (*id)++;
                            return new Token("==", "OpRelIgual", id);
                        }
                        if(prox != -1){
                            devolverChar(prox);
                        }
                        erroLexico(linhaToken, "=");
                        continue;
                    }
                    case '!': {
                        int prox = lerChar();
                        if(prox == '='){
                            (*id)++;
                            return new Token("!=", "OpRelDif", id);
                        }
                        if(prox != -1){
                            devolverChar(prox);
                        }
                        erroLexico(linhaToken, "!");
                        continue;
                    }
                    case ':': {
                        int prox = lerChar();
                        if(prox == '='){
                            (*id)++;
                            return new Token(":=", "Atrib", id);
                        }
                        if(prox != -1){
                            devolverChar(prox);
                        }
                        (*id)++;
                        return new Token(":", "Delim", id);
                    }

                    default:
                        erroLexico(linhaToken, string(1, ch));
                        continue;
                }
            }
        }
};

#endif
