#ifndef TOKEN_H
#define TOKEN_H

#include <string>
#include <iostream>

using namespace std;

// tipos de token da linguagem GYH
// eh um enum pra ninguem precisar comparar texto: o lexico decide o tipo
// pelo estado final do automato e o sintatico compara so o valor do enum
enum TipoToken {
    PCDec, PCProg, PCInt, PCReal, PCLer, PCImprimir,
    PCSe, PCEntao, PCSenao, PCEnqto, PCIni, PCFim,
    OpAritMult, OpAritDiv, OpAritSoma, OpAritSub,
    OpRelMenor, OpRelMenorIgual, OpRelMaior, OpRelMaiorIgual, OpRelIgual, OpRelDif,
    OpBoolE, OpBoolOu,
    IniDelim, FimDelim, Atrib, AbrePar, FechaPar,
    Var, NumInt, NumReal, Cadeia,
    FimArquivo,   // EOF
    Erro          // lexema que o automato nao reconheceu
};

// sigla do tipo, usada so na hora de imprimir
inline string nomeTipo(TipoToken tipo){
    switch(tipo){
        case PCDec:           return "PCDec";
        case PCProg:          return "PCProg";
        case PCInt:           return "PCInt";
        case PCReal:          return "PCReal";
        case PCLer:           return "PCLer";
        case PCImprimir:      return "PCImprimir";
        case PCSe:            return "PCSe";
        case PCEntao:         return "PCEntao";
        case PCSenao:         return "PCSenao";
        case PCEnqto:         return "PCEnqto";
        case PCIni:           return "PCIni";
        case PCFim:           return "PCFim";
        case OpAritMult:      return "OpAritMult";
        case OpAritDiv:       return "OpAritDiv";
        case OpAritSoma:      return "OpAritSoma";
        case OpAritSub:       return "OpAritSub";
        case OpRelMenor:      return "OpRelMenor";
        case OpRelMenorIgual: return "OpRelMenorIgual";
        case OpRelMaior:      return "OpRelMaior";
        case OpRelMaiorIgual: return "OpRelMaiorIgual";
        case OpRelIgual:      return "OpRelIgual";
        case OpRelDif:        return "OpRelDif";
        case OpBoolE:         return "OpBoolE";
        case OpBoolOu:        return "OpBoolOu";
        case IniDelim:        return "IniDelim";
        case FimDelim:        return "FimDelim";
        case Atrib:           return "Atrib";
        case AbrePar:         return "AbrePar";
        case FechaPar:        return "FechaPar";
        case Var:             return "Var";
        case NumInt:          return "NumInt";
        case NumReal:         return "NumReal";
        case Cadeia:          return "Cadeia";
        case FimArquivo:      return "EOF";
        case Erro:            return "Erro";
    }
    return "?";
}

class Token{

    public:
    string lexema;   // x
    TipoToken tipo;  // Var
    int id;          // numeros sequenciais (numera à medida que reconhece)
    int linha;       // linha onde o lexema comeca (pras mensagens de erro)

    Token(string lexema, TipoToken tipo, int id, int linha)
        : lexema(lexema), tipo(tipo), id(id), linha(linha) {}

    // imprime saida formatada
    string toString(){
        if(tipo == FimArquivo){
            return "<EOF>";
        }
        return "<" + nomeTipo(tipo) + ", \"" + lexema + "\">";
    }

};

#endif
