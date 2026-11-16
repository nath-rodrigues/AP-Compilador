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
// Eh a simulacao do automato do JFLAP (automatos.jff):
// o numero de cada estado aqui eh o mesmo numero que esta dentro da bolinha
// no diagrama. O lexema eh lido caractere por caractere, cada caractere faz o
// automato andar de um estado pro outro (funcao transicao), e o tipo do token
// sai do estado final onde ele parou (funcao tipoDoEstadoFinal).
// Nenhuma palavra eh comparada como texto: "dec" so vira PCDec porque o
// automato andou 1 -d-> 32 -e-> 33 -c-> 34.

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
                    // operadores relacionais e atribuicao
                    if(c == '<') return 2;
                    if(c == '>') return 7;
                    if(c == '=') return 10;
                    // simbolos de um caractere so
                    if(c == '+') return 12;
                    if(c == '-') return 13;
                    if(c == '*') return 14;
                    if(c == '/') return 15;
                    if(c == '(') return 16;
                    if(c == ')') return 17;
                    if(c == '[') return 18;
                    if(c == ']') return 19;
                    // variavel, numero e cadeia
                    if(maiuscula(c)) return 20;
                    if(digito(c))    return 22;
                    if(c == '"')     return 27;
                    // inicio das palavras-chave
                    if(c == 'a') return 29;
                    if(c == 'd') return 32;
                    if(c == 'e') return 35;
                    if(c == 'f') return 41;
                    if(c == 'i') return 46;
                    if(c == 'o') return 54;
                    if(c == 'p') return 56;
                    if(c == 'r') return 63;
                    if(c == 's') return 67;
                    if(c == 't') return 72;
                    if(c == 'w') return 76;
                    return SEM_TRANSICAO;

                // <  <=  <>  <<
                case 2:
                    if(c == '=') return 3;   // <=
                    if(c == '>') return 4;   // <>
                    if(c == '<') return 5;   // <<
                    return 6;                // Outro -> <   (6*)

                // >  e  >=
                case 7:
                    if(c == '=') return 8;   // >=
                    return 9;                // Outro -> >   (9*)

                // ==
                case 10:
                    if(c == '=') return 11;  // ==
                    return SEM_TRANSICAO;    // '=' sozinho nao existe na linguagem

                // variavel: [A-Z] seguido de [a-zA-Z0-9]*
                case 20:
                    if(letraOuDigito(c)) return 20;
                    return 21;               // Outro -> Var   (21*)

                // numero inteiro / real
                case 22:
                    if(digito(c)) return 22;
                    if(c == '.')  return 24;
                    return 23;               // Outro -> NumInt   (23*)
                case 24:
                    if(digito(c)) return 25;
                    return 26;               // Outro -> NumReal ("2." tambem eh real)
                case 25:
                    if(digito(c)) return 25;
                    return 26;               // Outro -> NumReal   (26*)

                // cadeia: tudo que nao for aspas fica no 27, aspas fecha
                case 27:
                    if(c == '"') return 28;
                    if(c == '\n' || c == -1) return SEM_TRANSICAO; // acabou a linha sem fechar
                    return 27;               // [^"]

                // and
                case 29: if(c == 'n') return 30; return SEM_TRANSICAO;
                case 30: if(c == 'd') return 31; return SEM_TRANSICAO;

                // dec
                case 32: if(c == 'e') return 33; return SEM_TRANSICAO;
                case 33: if(c == 'c') return 34; return SEM_TRANSICAO;

                // else, end
                case 35:
                    if(c == 'l') return 36;
                    if(c == 'n') return 39;
                    return SEM_TRANSICAO;
                case 36: if(c == 's') return 37; return SEM_TRANSICAO;
                case 37: if(c == 'e') return 38; return SEM_TRANSICAO;
                case 39: if(c == 'd') return 40; return SEM_TRANSICAO;

                // float
                case 41: if(c == 'l') return 42; return SEM_TRANSICAO;
                case 42: if(c == 'o') return 43; return SEM_TRANSICAO;
                case 43: if(c == 'a') return 44; return SEM_TRANSICAO;
                case 44: if(c == 't') return 45; return SEM_TRANSICAO;

                // if, integer
                case 46:
                    if(c == 'f') return 47;
                    if(c == 'n') return 48;
                    return SEM_TRANSICAO;
                case 48: if(c == 't') return 49; return SEM_TRANSICAO;
                case 49: if(c == 'e') return 50; return SEM_TRANSICAO;
                case 50: if(c == 'g') return 51; return SEM_TRANSICAO;
                case 51: if(c == 'e') return 52; return SEM_TRANSICAO;
                case 52: if(c == 'r') return 53; return SEM_TRANSICAO;

                // or
                case 54: if(c == 'r') return 55; return SEM_TRANSICAO;

                // prog, print
                case 56: if(c == 'r') return 57; return SEM_TRANSICAO;
                case 57:
                    if(c == 'o') return 58;
                    if(c == 'i') return 60;
                    return SEM_TRANSICAO;
                case 58: if(c == 'g') return 59; return SEM_TRANSICAO;
                case 60: if(c == 'n') return 61; return SEM_TRANSICAO;
                case 61: if(c == 't') return 62; return SEM_TRANSICAO;

                // read
                case 63: if(c == 'e') return 64; return SEM_TRANSICAO;
                case 64: if(c == 'a') return 65; return SEM_TRANSICAO;
                case 65: if(c == 'd') return 66; return SEM_TRANSICAO;

                // start
                case 67: if(c == 't') return 68; return SEM_TRANSICAO;
                case 68: if(c == 'a') return 69; return SEM_TRANSICAO;
                case 69: if(c == 'r') return 70; return SEM_TRANSICAO;
                case 70: if(c == 't') return 71; return SEM_TRANSICAO;

                // then
                case 72: if(c == 'h') return 73; return SEM_TRANSICAO;
                case 73: if(c == 'e') return 74; return SEM_TRANSICAO;
                case 74: if(c == 'n') return 75; return SEM_TRANSICAO;

                // while
                case 76: if(c == 'h') return 77; return SEM_TRANSICAO;
                case 77: if(c == 'i') return 78; return SEM_TRANSICAO;
                case 78: if(c == 'l') return 79; return SEM_TRANSICAO;
                case 79: if(c == 'e') return 80; return SEM_TRANSICAO;
            }
            // os demais estados sao finais sem nenhuma seta de saida
            return SEM_TRANSICAO;
        }

        // estados finais (circulo duplo): devolve true e o tipo do token do rotulo do estado
        static bool tipoDoEstadoFinal(int estado, TipoToken &tipo){
            switch(estado){
                case 3:  tipo = OpRelMenorIgual; return true;
                case 4:  tipo = OpRelDif;        return true;
                case 5:  tipo = Atrib;           return true;
                case 6:  tipo = OpRelMenor;      return true;
                case 8:  tipo = OpRelMaiorIgual; return true;
                case 9:  tipo = OpRelMaior;      return true;
                case 11: tipo = OpRelIgual;      return true;
                case 12: tipo = OpAritSoma;      return true;
                case 13: tipo = OpAritSub;       return true;
                case 14: tipo = OpAritMult;      return true;
                case 15: tipo = OpAritDiv;       return true;
                case 16: tipo = AbrePar;         return true;
                case 17: tipo = FechaPar;        return true;
                case 18: tipo = IniDelim;        return true;
                case 19: tipo = FimDelim;        return true;
                case 21: tipo = Var;             return true;
                case 23: tipo = NumInt;          return true;
                case 26: tipo = NumReal;         return true;
                case 28: tipo = Cadeia;          return true;
                case 31: tipo = OpBoolE;         return true;
                case 34: tipo = PCDec;           return true;
                case 38: tipo = PCSenao;         return true;
                case 40: tipo = PCFim;           return true;
                case 45: tipo = PCReal;          return true;
                case 47: tipo = PCSe;            return true;
                case 53: tipo = PCInt;           return true;
                case 55: tipo = OpBoolOu;        return true;
                case 59: tipo = PCProg;          return true;
                case 62: tipo = PCImprimir;      return true;
                case 66: tipo = PCLer;           return true;
                case 71: tipo = PCIni;           return true;
                case 75: tipo = PCEntao;         return true;
                case 80: tipo = PCEnqto;         return true;
            }
            return false;
        }

        // estados finais alcancados pela seta "Outro": o caractere que levou ate
        // eles nao faz parte do lexema e tem que ser devolvido (os estados com * )
        static bool estadoComRetrocesso(int estado){
            return estado == 6 || estado == 9 || estado == 21 || estado == 23 || estado == 26;
        }

        // estados do ramo das palavras em minusculo (palavras-chave, and e or)
        static bool estadoDePalavra(int estado){
            return estado >= 29 && estado <= 80;
        }

        // termina de ler uma palavra que o automato rejeitou, so pra msg de erro
        // mostrar a palavra inteira (ex: "parametro" e nao so "p")
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
                        if(minuscula(c)){
                            consumirRestoDaPalavra(lexema);
                        }
                    } else if(estadoDePalavra(estado) && letraOuDigito(c)){
                        // palavra em minusculo que nao eh reservada (ex: parametro, thn)
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
                    // palavra acabou mesmo (decx nao eh dec)
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
