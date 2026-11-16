#ifndef LEXICO_H
#define LEXICO_H

#include <string>
#include <vector>
#include <cctype>
#include <iostream>
#include "lerArquivo.h"
#include "token.h"

using namespace std;

// Analisador lexico da linguagem GYH
//
// Eh a simulacao do automato do JFLAP (automatos.jff / 1.jpeg, 2.jpeg, 3.jpeg):
// o numero de cada estado aqui eh o mesmo numero que esta dentro da bolinha
// no diagrama. O lexema eh lido caractere por caractere, cada caractere faz o
// automato andar de um estado pro outro (funcao transicao), e o tipo do token
// sai do estado final onde ele parou (funcao tipoDoEstadoFinal).
// Nenhuma palavra eh comparada como texto: "DEC" so vira PCDec porque o
// automato andou 1 -D-> 12 -E-> 13 -C-> 14.

class Lexico{
    private:
        LeitorArquivo *arq;
        int id;              // numera os tokens reconhecidos
        vector<int> pilha;   // buffer de caracteres devolvidos
        int linhaAtual;      // msg erro pra saber qual linha
        int erros;           // qtd de erros lexicos encontrados
        ostream *saidaErro = nullptr; // arquivo de output, para tambem registrar os erros la

        static const int ESTADO_INICIAL = 1;
        static const int SEM_TRANSICAO = -1; // o automato nao tem seta pra esse caractere

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
            erros++;
            string msg = "Erro Léxico na linha " + to_string(linha) + ": Desconhecido \"" + lexema + "\"";
            cout << msg << endl;
            if(saidaErro != nullptr){
                (*saidaErro) << msg << endl;
            }
        }

        // classes de caracteres que aparecem nas setas do diagrama
        static bool minuscula(int c){ return c >= 'a' && c <= 'z'; }                 // [a-z]
        static bool maiuscula(int c){ return c >= 'A' && c <= 'Z'; }                 // [A-Z]
        static bool digito(int c){ return c >= '0' && c <= '9'; }                    // [0-9]
        static bool letraOuDigito(int c){ return minuscula(c) || maiuscula(c) || digito(c); } // [a-zA-Z0-9]

        // FUNCAO DE TRANSICAO do automato
        // recebe o estado atual e o caractere lido e devolve o proximo estado
        // (ou SEM_TRANSICAO se nao existe seta saindo do estado com esse caractere)
        // "Outro" do diagrama = qualquer caractere que nao esta nas outras setas do estado
        static int transicao(int estado, int c){
            switch(estado){
                case 1:
                    // operadores relacionais
                    if(c == '<') return 2;
                    if(c == '=') return 5;
                    if(c == '>') return 7;
                    if(c == '!') return 10;
                    // inicio das palavras-chave
                    if(c == 'D') return 12;
                    if(c == 'P') return 15;
                    if(c == 'I') return 19;
                    if(c == 'L') return 30;
                    if(c == 'S') return 33;
                    if(c == 'E') return 73;
                    if(c == 'F') return 45;
                    if(c == 'R') return 48;
                    if(c == 'O') return 62;
                    // variavel, numero e cadeia
                    if(minuscula(c)) return 64;
                    if(digito(c))    return 66;
                    if(c == '"')     return 71;
                    // delimitador / atribuicao
                    if(c == ':') return 52;
                    // simbolos de um caractere so
                    if(c == '+') return 55;
                    if(c == '-') return 56;
                    if(c == '*') return 57;
                    if(c == '/') return 58;
                    if(c == '(') return 59;
                    if(c == ')') return 60;
                    return SEM_TRANSICAO;

                // <  e  <=
                case 2:
                    if(c == '=') return 3;   // <=
                    return 4;                // Outro -> <   (4*)

                // ==
                case 5:
                    if(c == '=') return 6;   // ==
                    return SEM_TRANSICAO;    // '=' sozinho nao existe na linguagem

                // >  e  >=
                case 7:
                    if(c == '=') return 8;   // >=
                    return 9;                // Outro -> >   (9*)

                // !=
                case 10:
                    if(c == '=') return 11;  // !=
                    return SEM_TRANSICAO;    // '!' sozinho nao existe na linguagem

                // DEC
                case 12: if(c == 'E') return 13; return SEM_TRANSICAO;
                case 13: if(c == 'C') return 14; return SEM_TRANSICAO;

                // PROG
                case 15: if(c == 'R') return 16; return SEM_TRANSICAO;
                case 16: if(c == 'O') return 17; return SEM_TRANSICAO;
                case 17: if(c == 'G') return 18; return SEM_TRANSICAO;

                // INT, INI, IMPRIMIR
                case 19:
                    if(c == 'N') return 20;
                    if(c == 'M') return 23;
                    return SEM_TRANSICAO;
                case 20:
                    if(c == 'T') return 22;  // INT
                    if(c == 'I') return 21;  // INI
                    return SEM_TRANSICAO;
                case 23: if(c == 'P') return 24; return SEM_TRANSICAO;
                case 24: if(c == 'R') return 25; return SEM_TRANSICAO;
                case 25: if(c == 'I') return 26; return SEM_TRANSICAO;
                case 26: if(c == 'M') return 27; return SEM_TRANSICAO;
                case 27: if(c == 'I') return 28; return SEM_TRANSICAO;
                case 28: if(c == 'R') return 29; return SEM_TRANSICAO;

                // LER
                case 30: if(c == 'E') return 31; return SEM_TRANSICAO;
                case 31: if(c == 'R') return 32; return SEM_TRANSICAO;

                // SE, SENAO
                case 33: if(c == 'E') return 34; return SEM_TRANSICAO;
                case 34: if(c == 'N') return 35; return SEM_TRANSICAO; // 34 ja eh final (SE)
                case 35: if(c == 'A') return 36; return SEM_TRANSICAO;
                case 36: if(c == 'O') return 37; return SEM_TRANSICAO;

                // E, ENTAO, ENQTO
                case 73:
                    if(c == 'N') return 74;
                    if(letraOuDigito(c)) return SEM_TRANSICAO; // "EX", "E1"... nao sao o operador E
                    return 61;               // Outro -> OpBoolE
                case 74:
                    if(c == 'T') return 40;
                    if(c == 'Q') return 75;
                    return SEM_TRANSICAO;
                case 40: if(c == 'A') return 41; return SEM_TRANSICAO;
                case 41: if(c == 'O') return 42; return SEM_TRANSICAO;
                case 75: if(c == 'T') return 38; return SEM_TRANSICAO;
                case 38: if(c == 'O') return 39; return SEM_TRANSICAO;

                // FIM
                case 45: if(c == 'I') return 46; return SEM_TRANSICAO;
                case 46: if(c == 'M') return 47; return SEM_TRANSICAO;

                // REAL
                case 48: if(c == 'E') return 49; return SEM_TRANSICAO;
                case 49: if(c == 'A') return 50; return SEM_TRANSICAO;
                case 50: if(c == 'L') return 51; return SEM_TRANSICAO;

                // OU
                case 62: if(c == 'U') return 63; return SEM_TRANSICAO;

                // variavel: [a-z] seguido de [a-zA-Z0-9]*
                case 64:
                    if(letraOuDigito(c)) return 64;
                    return 65;               // Outro -> Var

                // numero inteiro / real
                case 66:
                    if(digito(c)) return 66;
                    if(c == '.')  return 68;
                    return 67;               // Outro -> NumInt
                case 68:
                    if(digito(c)) return 69;
                    return 70;               // Outro -> NumReal ("2." tambem eh real)
                case 69:
                    if(digito(c)) return 69;
                    return 70;               // Outro -> NumReal

                // cadeia: tudo que nao for aspas fica no 71, aspas fecha
                case 71:
                    if(c == '"') return 72;
                    if(c == '\n' || c == -1) return SEM_TRANSICAO; // acabou a linha sem fechar
                    return 71;               // [^"]

                // :  e  :=
                case 52:
                    if(c == '=') return 53;  // :=
                    return 54;               // Outro -> Delim
            }
            // os demais estados sao finais sem nenhuma seta de saida
            return SEM_TRANSICAO;
        }

        // estados finais (circulo duplo): devolve true e o tipo do token do rotulo do estado
        static bool tipoDoEstadoFinal(int estado, TipoToken &tipo){
            switch(estado){
                case 3:  tipo = OpRelMenorIgual; return true;
                case 4:  tipo = OpRelMenor;      return true;
                case 6:  tipo = OpRelIgual;      return true;
                case 8:  tipo = OpRelMaiorIgual; return true;
                case 9:  tipo = OpRelMaior;      return true;
                case 11: tipo = OpRelDif;        return true;
                case 14: tipo = PCDec;           return true;
                case 18: tipo = PCProg;          return true;
                case 21: tipo = PCIni;           return true;
                case 22: tipo = PCInt;           return true;
                case 29: tipo = PCImprimir;      return true;
                case 32: tipo = PCLer;           return true;
                case 34: tipo = PCSe;            return true;
                case 37: tipo = PCSenao;         return true;
                case 39: tipo = PCEnqto;         return true;
                case 42: tipo = PCEntao;         return true;
                case 47: tipo = PCFim;           return true;
                case 51: tipo = PCReal;          return true;
                case 53: tipo = Atrib;           return true;
                case 54: tipo = Delim;           return true;
                case 55: tipo = OpAritSoma;      return true;
                case 56: tipo = OpAritSub;       return true;
                case 57: tipo = OpAritMult;      return true;
                case 58: tipo = OpAritDiv;       return true;
                case 59: tipo = AbrePar;         return true;
                case 60: tipo = FechaPar;        return true;
                case 61: tipo = OpBoolE;         return true;
                case 63: tipo = OpBoolOu;        return true;
                case 65: tipo = Var;             return true;
                case 67: tipo = NumInt;          return true;
                case 70: tipo = NumReal;         return true;
                case 72: tipo = Cadeia;          return true;
            }
            return false;
        }

        // estados finais alcancados pela seta "Outro": o caractere que levou ate
        // eles nao faz parte do lexema e tem que ser devolvido (os estados com * )
        static bool estadoComRetrocesso(int estado){
            return estado == 4  || estado == 9  || estado == 54 || estado == 61 ||
                   estado == 65 || estado == 67 || estado == 70;
        }

        // estados do ramo das palavras em maiusculo (palavras-chave, E e OU)
        static bool estadoDePalavra(int estado){
            return (estado >= 12 && estado <= 51) || estado == 62 || estado == 63 ||
                   (estado >= 73 && estado <= 75);
        }

        // termina de ler uma palavra que o automato rejeitou, so pra msg de erro
        // mostrar a palavra inteira (ex: "Parametro" e nao so "P")
        void consumirRestoDaPalavra(string &lexema){
            int c = lerChar();
            while(letraOuDigito(c)){
                lexema += (char)c;
                c = lerChar();
            }
            devolverChar(c);
        }

        Token* novoToken(const string &lexema, TipoToken tipo, int linha){
            id++;
            return new Token(lexema, tipo, id, linha);
        }

    public:
        Lexico(string arquivo){
            id = 0;
            arq = new LeitorArquivo(arquivo); // obj p/ ler caracteres
            linhaAtual = 1;
            erros = 0;
        }

        // libera o que foi alocado no construtor (fecha o arquivo via ~LeitorArquivo)
        ~Lexico(){
            delete arq;
        }

        bool arquivoAberto(){
            return arq->aberto();
        }

        // erros escritos na saida tbm*
        void setSaidaErro(ostream &saida){
            saidaErro = &saida;
        }

        int totalErros(){
            return erros;
        }

        // joga fora o resto da linha atual (o sintatico usa depois de um erro
        // lexico, pra nao acusar varios erros na mesma linha)
        void descartarLinha(){
            int c = lerChar();
            while(c != -1 && c != '\n'){
                c = lerChar();
            }
        }

        // metodo principal: le o proximo token do arquivo
        // devolve um token do tipo FimArquivo quando o arquivo acaba e um token
        // do tipo Erro quando o automato nao reconhece o lexema (a msg ja sai aqui)
        Token* proximoToken(){
            int c;

            // antes de entrar no automato pula o que nao gera token
            while(true){
                c = lerChar();

                // ignora espacos em branco
                if(c == ' ' || c == '\t' || c == '\n' || c == '\r'){
                    continue;
                }

                // '#' eh comentario ate o fim da linha
                if(c == '#'){
                    int cc;
                    while((cc = lerChar()) != -1 && cc != '\n'){
                        // ignora
                    }
                    continue;
                }
                break;
            }

            // guarda pq eh a linha onde o token comeca (nao onde terminou)
            int linhaToken = linhaAtual;

            if(c == -1){
                return new Token("EOF", FimArquivo, id, linhaToken);
            }

            // simulacao do automato
            int estado = ESTADO_INICIAL;
            string lexema;

            while(true){
                int prox = transicao(estado, c);

                // travou: nao tem seta saindo do estado atual com esse caractere
                if(prox == SEM_TRANSICAO){
                    if(estado == ESTADO_INICIAL){
                        // caractere que nao comeca nenhum token
                        lexema += (char)c;
                        if(maiuscula(c)){
                            consumirRestoDaPalavra(lexema);
                        }
                    } else if(estadoDePalavra(estado) && letraOuDigito(c)){
                        // palavra em maiusculo que nao eh reservada (ex: Parametro, ENTO)
                        lexema += (char)c;
                        consumirRestoDaPalavra(lexema);
                    } else {
                        // o caractere nao eh do lexema errado, fica pro proximo token
                        devolverChar(c);
                    }
                    erroLexico(linhaToken, lexema);
                    return novoToken(lexema, Erro, linhaToken);
                }

                estado = prox;
                if(estadoComRetrocesso(estado)){
                    devolverChar(c);      // chegou pelo "Outro": nao consome
                } else {
                    lexema += (char)c;
                }

                TipoToken tipo;
                if(tipoDoEstadoFinal(estado, tipo)){
                    if(!estadoDePalavra(estado)){
                        // final sem seta de saida: aceita
                        if(tipo == Cadeia){
                            // o lexema da cadeia eh so o conteudo, sem as aspas
                            lexema = lexema.substr(1, lexema.size() - 2);
                        }
                        return novoToken(lexema, tipo, linhaToken);
                    }

                    // palavra-chave: espia o proximo caractere pra saber se a
                    // palavra acabou mesmo (SE x SENAO, e DECx nao eh DEC)
                    int espiado = lerChar();
                    if(!letraOuDigito(espiado)){
                        devolverChar(espiado);
                        return novoToken(lexema, tipo, linhaToken);
                    }
                    c = espiado;          // a palavra continua, segue no automato
                    continue;
                }

                c = lerChar();
            }
        }
};

#endif
