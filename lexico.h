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
        vector<int> pilha;   // buffer de caracteres devolvidos
        int linhaAtual; // msg erro pra saber qual linha
        ostream *saidaErro = nullptr; // arquivo de output, para tambem registrar os erros la

        // le o proximo caractere
        int lerChar(){
            int c;
            // se houver algum caractere devolvido, le ele primeiro
            if(!pilha.empty()){
                c = pilha.back(); // LIFO
                pilha.pop_back();
                // se nao le o arq
            } else {
                c = arq->lerProxCaracter();
            }
            // quebra mantem o contador de linha atualizado
            if(c == '\n'){
                linhaAtual++;
            }
            return c;
        }

        // devolve um caractere para ser relido na proxima chamada de lerChar()
        void devolverChar(int c){
            // se for -1 (EOF) nao devolve
            if(c == -1) return;
            // se for quebra decrementa o contador
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
            // inicializa o id do token
            // new eh pra ler o valor do id qnd cria o token, nao fica ligado a ele
            id = new int(0);
            arq = new LeitorArquivo(arquivo); // obj p/ ler caracteres
            linhaAtual = 1;
        }

        // erros escritos na saida tbm*
        void setSaidaErro(ostream &saida){
            saidaErro = &saida;
        }
        
        // metodo principal: le o proximo token do arquivo
        Token* proximoToken(){
            static const unordered_map<string,string> palavras = { // string-chave pra uma string-valor
                {"DEC","PCDec"}, {"PROG","PCProg"}, {"INT","PCInt"}, {"REAL","PCReal"},
                {"LER","PCLer"}, {"IMPRIMIR","PCImprimir"}, {"SE","PCSe"}, {"ENTAO","PCEntao"},
                {"SENAO","PCSenao"}, {"ENQTO","PCEnqto"}, {"INI","PCIni"}, {"FIM","PCFim"},
                {"E","OpBoolE"}, {"OU","OpBoolOu"}
            };
            // mapa inicializadeo uma vez p/ otimizar

            // loop infinito ate encontrar um token valido ou EOF
            while(true){
                int c = lerChar();
                if(c == -1){
                    return nullptr;
                }

                // converte p/ char p/ comaprar depois
                char ch = (char)c;

                // ignora espacos em branco
                if(isspace((unsigned char)ch)){
                    continue;
                }

                // '#' eh comentario ate o fim da linha
                if(ch == '#'){
                    int cc;
                    while((cc = lerChar()) != -1 && cc != '\n'){
                        // ignora
                    }
                    continue;
                }

                // guarda pq eh a linha pnde o prox token comeca (nao onde terminou)
                int linhaToken = linhaAtual;

                // comecam com letra maiuscula
                if(isupper((unsigned char)ch)){
                    string lexema;
                    // monta lexema comecando c/ a letra maiuscula
                    lexema += ch;

                    // le caracteres ate encontrar que nao seja letra/numero
                    int prox;
                    while((prox = lerChar()) != -1 && isalnum((unsigned char)prox)){
                        lexema += (char)prox;
                    }

                    // caractere p/ ser relido na proxima chamada (pq nao eh parte do lexema)
                    if(prox != -1){
                        devolverChar(prox);
                    }

                    // procura na tabela
                    auto it = palavras.find(lexema);
                    if(it != palavras.end()){
                        (*id)++;
                        return new Token(lexema, it->second, id);
                    }
                    
                    // começa c/ maiusucula mas nao eh reconheccida
                    erroLexico(linhaToken, lexema);
                    continue;
                }

                // variavel: letra minuscula seguida de letras/numeros
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

                    // sempore tratado como variavel 
                    (*id)++;
                    return new Token(lexema, "Var", id);
                }

                // numeros: NumInt ou NumReal (com ponto)
                if(isdigit((unsigned char)ch)){
                    string lexema;
                    lexema += ch;

                    int prox = lerChar();
                    while(prox != -1 && isdigit((unsigned char)prox)){
                        lexema += (char)prox;
                        prox = lerChar();
                    }

                    // verifica se eh real tem ponto e digito dps
                    bool real = false;
                    if(prox == '.'){
                        int prox2 = lerChar();
                        if(prox2 != -1 && isdigit((unsigned char)prox2)){

                            //continua lendo o num real
                            real = true;
                            lexema += '.';
                            lexema += (char)prox2;

                            prox2 = lerChar();
                            while(prox2 != -1 && isdigit((unsigned char)prox2)){
                                lexema += (char)prox2;
                                prox2 = lerChar();
                            }
                            prox = prox2;

                            // se nao tiver digito dps do ponto, devolve o ponto e o prox2 (segundo caractere)
                        } else {
                            if(prox2 != -1){
                                devolverChar(prox2);
                            }
                            devolverChar(prox); // devolve o '.'
                            prox = -1;      
                        }
                    }

                    if(prox != -1){
                        devolverChar(prox);
                    }

                    // cria o token e incrementa o id
                    (*id)++;
                    return new Token(lexema, real ? "NumReal" : "NumInt", id);
                }

                // recconhece adeia de caracteres entre aspas
                if(ch == '"'){
                    string bruto;   // texto incluindo aspa (aberta erro)
                    string lexema;  // conteudo sem aspas
                    // isso eh pra exemplos retornam strings
                    bruto += ch;

                    bool fechado = false;
                    int prox;
                    
                    // tewm quie fecchar antes de acabar a linha
                    while((prox = lerChar()) != -1 && prox != '\n'){
                        if(prox == '"'){
                            fechado = true;
                            break;
                        }
                        bruto += (char)prox;
                        lexema += (char)prox;
                    }
                    
                    //m se nao fechou a string eh erro
                    if(!fechado){
                        if(prox != -1){
                            devolverChar(prox);
                        }
                        erroLexico(linhaToken, bruto);
                        continue;
                    }

                    // foi fechada e cria o token
                    (*id)++;
                    return new Token(lexema, "Cadeia", id);
                }

                // esqueleto pratico feito durante a aula 

                // * / + - ( )
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


                    // < <= > >= == != := :
                    case '<': {
                        int prox = lerChar(); // guarda caractere dps do simbolo lido
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

                    // tem que ser == ou !=
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

                    // atribuicao e delimitador
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

                    // caso default: caractere nao reconhecido, ou seja, erro lexico
                    default:
                        erroLexico(linhaToken, string(1, ch));
                        continue;
                }
            }
        }
};

#endif
