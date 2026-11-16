#ifndef SINTATICO_H
#define SINTATICO_H

#include <string>
#include <deque>
#include <iostream>
#include "lexico.h"
#include "token.h"

using namespace std;

// Analisador sintatico da linguagem GYH
//
// Tipo: descendente (TDP) recursivo, sem backtracking. Cada nao-terminal da
// gramatica virou um metodo, e a decisao de qual regra seguir eh sempre tomada
// olhando o proximo token (lookahead), nunca voltando atras.
//
// Gramatica usada (ja sem recursao a esquerda e fatorada, { } = repete 0 ou mais,
// [ ] = opcional):
//
//   Programa            -> '[' dec ']' ListaDeclaracoes '[' prog ']' ListaComandos
//   ListaDeclaracoes    -> Declaracao { Declaracao }
//   Declaracao          -> Var '(' TipoVar ')'
//   TipoVar             -> integer | float
//   ListaComandos       -> Comando { Comando }
//   Comando             -> ComandoAtribuicao | ComandoEntrada | ComandoSaida
//                        | ComandoCondicao | ComandoRepeticao | SubAlgoritmo
//   ComandoAtribuicao   -> Var '<<' ExpressaoAritmetica
//   ComandoEntrada      -> read Var
//   ComandoSaida        -> print ( Var | Cadeia )
//   ComandoCondicao     -> if ExpressaoRelacional then Comando [ else Comando ]
//   ComandoRepeticao    -> while ExpressaoRelacional then Comando
//   SubAlgoritmo        -> start ListaComandos end
//   ExpressaoAritmetica -> TermoAritmetico { ('+' | '-') TermoAritmetico }
//   TermoAritmetico     -> FatorAritmetico { ('*' | '/') FatorAritmetico }
//   FatorAritmetico     -> NumInt | NumReal | Var | '(' ExpressaoAritmetica ')'
//   ExpressaoRelacional -> TermoRelacional { (and | or) TermoRelacional }
//   TermoRelacional     -> ExpressaoAritmetica OpRel ExpressaoAritmetica
//                        | '(' ExpressaoRelacional ')'
//
// Erros: quando um erro eh encontrado a msg eh impressa e o analisador entra em
// modo panico (descarta tokens ate um ponto seguro) pra continuar a analise e
// conseguir apontar os outros erros do programa.

// excecao so pra sair do comando/declaracao que deu erro e ir pra recuperacao
struct ErroSintatico {};

class Sintatico{
    private:
        Lexico &lex;
        deque<Token*> buffer;         // tokens ja lidos do lexico e ainda nao consumidos
        int erros;                    // qtd de erros sintaticos
        int linhaErro;                // linha do ultimo erro (usada na recuperacao)
        int linhaConsumida;           // linha do ultimo token consumido
        int linhaConstrucao;          // linha onde comecou o comando/declaracao atual
        ostream *saidaErro = nullptr; // arquivo de output, pra registrar os erros la tbm

        // ---------- acesso aos tokens ----------

        // olha o token k posicoes a frente sem consumir (k = 0 eh o token atual)
        Token* espiar(size_t k){
            while(buffer.size() <= k){
                // depois do EOF nao tem mais o que ler
                if(!buffer.empty() && buffer.back()->tipo == FimArquivo){
                    return buffer.back();
                }
                buffer.push_back(lex.proximoToken());
            }
            return buffer[k];
        }

        Token* atual(){
            return espiar(0);
        }

        TipoToken tipoAtual(){
            return atual()->tipo;
        }

        // consome o token atual (o EOF nunca eh consumido)
        void avancar(){
            Token *t = atual();
            if(t->tipo == FimArquivo) return;
            linhaConsumida = t->linha;
            buffer.pop_front();
            delete t;
        }

        // ---------- erros ----------

        // registra um erro no token atual, sem interromper a analise
        // se o token atual for um erro lexico a msg ja foi impressa pelo lexico:
        // so descarta o resto da linha pra nao acusar outro erro em cima dela
        void registrarErro(const string &descricao){
            Token *t = atual();
            linhaErro = t->linha;

            if(t->tipo == Erro){
                lex.descartarLinha();
                avancar();
                return;
            }

            // se o comando/declaracao ficou incompleto e o token que sobrou ja esta
            // em outra linha, o erro eh na linha de cima (onde faltou alguma coisa)
            if(linhaConsumida >= linhaConstrucao && linhaConsumida < t->linha){
                linhaErro = linhaConsumida;
            }

            erros++;
            string msg = "Erro sintático, " + descricao; // mesmo formato do enunciado
            cout << msg << endl;
            if(saidaErro != nullptr){
                (*saidaErro) << msg << endl;
            }
        }

        // erro do tipo "esperado X antes de Y" e sai do que estava analisando
        void erroEsperado(const string &esperado){
            registrarErro("esperado " + esperado + " antes de " + nomeTipo(tipoAtual()));
            throw ErroSintatico();
        }

        // se o token atual eh do tipo esperado consome, se nao eh erro
        void consumir(TipoToken tipo, const string &esperado){
            if(tipoAtual() != tipo){
                erroEsperado(esperado);
            }
            avancar();
        }

        // ---------- recuperacao de erro (modo panico) ----------

        static bool palavraQueIniciaComando(TipoToken tipo){
            return tipo == PCLer || tipo == PCImprimir || tipo == PCSe ||
                   tipo == PCEnqto || tipo == PCIni;
        }

        // descarta o token atual durante a recuperacao
        void descartarToken(){
            if(tipoAtual() == Erro){
                lex.descartarLinha(); // erro lexico ja informado pelo lexico
            }
            avancar();
        }

        // descarta o resto da linha onde ocorreu o erro (usado nas declaracoes)
        void pularLinhaDoErro(){
            while(tipoAtual() != FimArquivo && atual()->linha <= linhaErro){
                descartarToken();
            }
        }

        // descarta tokens ate achar o inicio de um proximo comando
        // (uma variavel so conta como inicio de comando se estiver em outra linha,
        // pq na mesma linha ela provavelmente ainda faz parte do comando errado)
        void sincronizarComando(){
            while(true){
                TipoToken tipo = tipoAtual();
                if(tipo == FimArquivo || tipo == PCFim || palavraQueIniciaComando(tipo)){
                    return;
                }
                if(tipo == Var && atual()->linha > linhaErro){
                    return;
                }
                descartarToken();
            }
        }

        // ---------- gramatica ----------

        // Programa -> '[' dec ']' ListaDeclaracoes '[' prog ']' ListaComandos
        void programa(){
            linhaConstrucao = atual()->linha;
            try{
                consumir(IniDelim, "delimitador '['");
                consumir(PCDec, "palavra-chave 'dec'");
                consumir(FimDelim, "delimitador ']'");
            } catch(ErroSintatico&){
                pularLinhaDoErro();
            }

            listaDeclaracoes();

            linhaConstrucao = atual()->linha;
            try{
                consumir(IniDelim, "delimitador '['");
                consumir(PCProg, "palavra-chave 'prog'");
                consumir(FimDelim, "delimitador ']'");
            } catch(ErroSintatico&){
                // se ja parou em cima de um comando nao joga ele fora
                if(!palavraQueIniciaComando(tipoAtual())){
                    pularLinhaDoErro();
                }
            }

            listaComandos(false);
        }

        // ListaDeclaracoes -> Declaracao { Declaracao }
        void listaDeclaracoes(){
            int quantidade = 0;

            // a lista acaba no '[' do "[prog]" (ou no fim do arquivo / inicio de um comando)
            while(tipoAtual() != IniDelim && tipoAtual() != FimArquivo &&
                  !palavraQueIniciaComando(tipoAtual())){
                quantidade++;
                try{
                    declaracao();
                } catch(ErroSintatico&){
                    pularLinhaDoErro();
                }
            }

            if(quantidade == 0){
                registrarErro("esperado declaração de variável antes de " + nomeTipo(tipoAtual()));
            }
        }

        // Declaracao -> Var '(' TipoVar ')'
        void declaracao(){
            linhaConstrucao = atual()->linha;
            consumir(Var, "nome de variável");
            consumir(AbrePar, "abre parêntese '('");
            tipoVar();
            consumir(FechaPar, "fecha parêntese ')'");
        }

        // TipoVar -> integer | float
        void tipoVar(){
            if(tipoAtual() == PCInt || tipoAtual() == PCReal){
                avancar();
            } else {
                erroEsperado("tipo 'integer' ou 'float'");
            }
        }

        // ListaComandos -> Comando { Comando }
        // dentroDeBloco = true quando eh a lista de um start ... end
        void listaComandos(bool dentroDeBloco){
            int quantidade = 0;

            while(tipoAtual() != FimArquivo){
                if(tipoAtual() == PCFim){
                    if(dentroDeBloco){
                        break; // quem chamou (subAlgoritmo) consome o end
                    }
                    registrarErro("'end' sem 'start' correspondente");
                    avancar();
                    continue;
                }

                quantidade++;
                try{
                    comando();
                } catch(ErroSintatico&){
                    sincronizarComando();
                }
            }

            if(quantidade == 0){
                registrarErro("esperado comando antes de " + nomeTipo(tipoAtual()));
            }
        }

        // Comando -> ComandoAtribuicao | ComandoEntrada | ComandoSaida
        //          | ComandoCondicao | ComandoRepeticao | SubAlgoritmo
        // o primeiro token ja diz qual eh o comando
        void comando(){
            linhaConstrucao = atual()->linha;
            switch(tipoAtual()){
                case Var:        comandoAtribuicao(); break;
                case PCLer:      comandoEntrada();    break;
                case PCImprimir: comandoSaida();      break;
                case PCSe:       comandoCondicao();   break;
                case PCEnqto:    comandoRepeticao();  break;
                case PCIni:      subAlgoritmo();      break;
                default:         erroEsperado("comando");
            }
        }

        // ComandoAtribuicao -> Var '<<' ExpressaoAritmetica
        void comandoAtribuicao(){
            consumir(Var, "nome de variável");
            consumir(Atrib, "atribuição '<<'");
            expressaoAritmetica();
        }

        // ComandoEntrada -> read Var
        void comandoEntrada(){
            consumir(PCLer, "palavra-chave 'read'");
            consumir(Var, "nome de variável");
        }

        // ComandoSaida -> print ( Var | Cadeia )
        void comandoSaida(){
            consumir(PCImprimir, "palavra-chave 'print'");
            if(tipoAtual() == Var || tipoAtual() == Cadeia){
                avancar();
            } else {
                erroEsperado("variável ou cadeia");
            }
        }

        // ComandoCondicao -> if ExpressaoRelacional then Comando [ else Comando ]
        void comandoCondicao(){
            consumir(PCSe, "palavra-chave 'if'");
            expressaoRelacional();
            consumir(PCEntao, "palavra-chave 'then'");
            comando();
            if(tipoAtual() == PCSenao){
                avancar();
                comando();
            }
        }

        // ComandoRepeticao -> while ExpressaoRelacional then Comando
        void comandoRepeticao(){
            consumir(PCEnqto, "palavra-chave 'while'");
            expressaoRelacional();
            consumir(PCEntao, "palavra-chave 'then'");
            comando();
        }

        // SubAlgoritmo -> start ListaComandos end
        void subAlgoritmo(){
            consumir(PCIni, "palavra-chave 'start'");
            listaComandos(true);
            consumir(PCFim, "palavra-chave 'end'");
        }

        // ExpressaoAritmetica -> TermoAritmetico { ('+' | '-') TermoAritmetico }
        void expressaoAritmetica(){
            termoAritmetico();
            while(tipoAtual() == OpAritSoma || tipoAtual() == OpAritSub){
                avancar();
                termoAritmetico();
            }
        }

        // TermoAritmetico -> FatorAritmetico { ('*' | '/') FatorAritmetico }
        void termoAritmetico(){
            fatorAritmetico();
            while(tipoAtual() == OpAritMult || tipoAtual() == OpAritDiv){
                avancar();
                fatorAritmetico();
            }
        }

        // FatorAritmetico -> NumInt | NumReal | Var | '(' ExpressaoAritmetica ')'
        void fatorAritmetico(){
            switch(tipoAtual()){
                case NumInt:
                case NumReal:
                case Var:
                    avancar();
                    break;
                case AbrePar:
                    avancar();
                    expressaoAritmetica();
                    consumir(FechaPar, "fecha parêntese ')'");
                    break;
                default:
                    erroEsperado("número, variável ou '('");
            }
        }

        static bool operadorRelacional(TipoToken tipo){
            return tipo == OpRelMenor || tipo == OpRelMenorIgual || tipo == OpRelMaior ||
                   tipo == OpRelMaiorIgual || tipo == OpRelIgual || tipo == OpRelDif;
        }

        static bool operadorBooleano(TipoToken tipo){
            return tipo == OpBoolE || tipo == OpBoolOu;
        }

        // ExpressaoRelacional -> TermoRelacional { (and | or) TermoRelacional }
        void expressaoRelacional(){
            termoRelacional();
            while(operadorBooleano(tipoAtual())){
                avancar();
                termoRelacional();
            }
        }

        // As duas regras do TermoRelacional podem comecar com '(' :
        //   (A + B) > C       -> o parentese eh de uma expressao aritmetica
        //   (A > B) and ...   -> o parentese eh de uma expressao relacional
        // Pra decidir sem backtracking, olha os tokens a frente (so olha, nao
        // consome) ate o parentese fechar: se tiver operador relacional ou
        // booleano la dentro, eh expressao relacional.
        bool parenteseDeExpressaoRelacional(){
            int profundidade = 0;
            for(size_t k = 0; ; k++){
                TipoToken tipo = espiar(k)->tipo;

                if(tipo == AbrePar){
                    profundidade++;
                } else if(tipo == FechaPar){
                    profundidade--;
                    if(profundidade == 0){
                        return false; // fechou e so tinha aritmetica dentro
                    }
                } else if(operadorRelacional(tipo) || operadorBooleano(tipo)){
                    return true;
                } else if(tipo != Var && tipo != NumInt && tipo != NumReal &&
                          tipo != OpAritSoma && tipo != OpAritSub &&
                          tipo != OpAritMult && tipo != OpAritDiv){
                    return false; // token que nao pertence a expressao: para de olhar
                }
            }
        }

        // TermoRelacional -> ExpressaoAritmetica OpRel ExpressaoAritmetica
        //                  | '(' ExpressaoRelacional ')'
        void termoRelacional(){
            if(tipoAtual() == AbrePar && parenteseDeExpressaoRelacional()){
                avancar();
                expressaoRelacional();
                consumir(FechaPar, "fecha parêntese ')'");
            } else {
                expressaoAritmetica();
                if(!operadorRelacional(tipoAtual())){
                    erroEsperado("operador relacional");
                }
                avancar();
                expressaoAritmetica();
            }
        }

    public:
        Sintatico(Lexico &lexico) : lex(lexico), erros(0), linhaErro(0), linhaConsumida(0), linhaConstrucao(0) {}

        ~Sintatico(){
            for(Token *t : buffer){
                delete t;
            }
        }

        // erros escritos na saida tbm
        void setSaidaErro(ostream &saida){
            saidaErro = &saida;
        }

        int totalErros(){
            return erros;
        }

        // analisa o programa inteiro; devolve true se nao teve nenhum erro
        // (nem lexico nem sintatico)
        bool analisar(){
            programa();
            return erros == 0 && lex.totalErros() == 0;
        }
};

#endif
