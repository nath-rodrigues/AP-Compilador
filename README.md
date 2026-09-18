# Analisador Léxico — Linguagem GYH

Parte 1 do compilador: reconhece os tokens de um programa em GYH e imprime no terminal e em `output/`.


## Arquivos

- `main.cpp` — lê o arquivo e imprime os tokens
- `lexico.h` — analisador léxico
- `lerArquivo.h` — leitura de arquivo com lookahead
- `token.h` — classe Token
- `fatorial.txt`, `media.txt` — exemplos de entrada

## Como rodar

```bash
g++ -o compilador main.cpp
./compilador testes/fatorial.txt
./compilador testes/programa0.gyh
```

Saída: tokens no formato `<TIPO, "lexema">`, terminando em `<EOF>`.

## Tokens reconhecidos

| Categoria | Exemplos | Tipo |
|---|---|---|
| Reservadas | DEC, PROG, INT, REAL, LER, IMPRIMIR, SE, ENTAO, SENAO, ENQTO, INI, FIM | PCDec, PCProg, ... |
| Booleanos | E, OU | OpBoolE, OpBoolOu |
| Variável | media, x | Var |
| Número inteiro | 10 | NumInt |
| Número real | 7.0 | NumReal |
| Cadeia | "texto" | Cadeia |
| Aritméticos | + - * / | OpAritSoma, ... |
| Relacionais | < <= > >= == != | OpRelMenor, ... |
| Atribuição | := | Atrib |
| Delimitador | : | Delim |
| Parênteses | ( ) | AbrePar, FechaPar |

Comentários com `#` até o fim da linha. Palavra maiúscula não reservada ou cadeia sem fechar geram erro léxico.
