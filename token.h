#ifndef TOKEN_H
#define TOKEN_H

#include <string>
#include <iostream>

using namespace std;

class Token{

    // tres atributos
    public:
    string lexema; // x
    string tipo; // Var
    int id; // numeros sequenciais

    // construtor
    // id guarda valor token (numera à medida que reconhece)
    Token(string lexema, string tipo, int *id):lexema(lexema), tipo (tipo), id(*id){}
    
    // imprime saida formatada
    string toString(){
        return "<" + tipo + ", \"" + lexema + "\">";
    }

};

#endif