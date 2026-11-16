# Analisador Léxico — Linguagem GYH

Parte 1 do compilador: reconhece os tokens de um programa em GYH e imprime no terminal e em `output/`.


## Arquivos

- `main.cpp` — lê o arquivo e imprime os tokens
- `lexico.h` — analisador léxico (simulação do autômato)
- `automatos.jff`, `1.jpeg`, `2.jpeg`, `3.jpeg` — diagrama do autômato no JFLAP
- `lerArquivo.h` — leitura de arquivo com lookahead
- `token.h` — classe Token
- `fatorial.txt`, `media.txt` — exemplos de entrada

## Autômato

O `lexico.h` é a simulação do autômato do `automatos.jff`: o número de cada estado no código é o mesmo número do estado no diagrama. O lexema é lido caractere por caractere, cada caractere leva o autômato de um estado para outro (`transicao`) e o tipo do token é o do estado final em que ele parou (`tipoDoEstadoFinal`). Nenhuma palavra é comparada como texto.

Exemplo: `DEC` é reconhecido pelo caminho `1 -D-> 12 -E-> 13 -C-> 14 (PCDec)`.

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

Comentários com `#` até o fim da linha. Palavra maiúscula não reservada ou cadeia sem fechar geram erro léxico. No primeiro erro léxico a mensagem é impressa e a análise é interrompida.
