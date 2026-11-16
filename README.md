# Analisador Sintático — Linguagem GYH

Parte 2 do compilador: verifica se um programa em GYH segue a gramática da linguagem e aponta os erros léxicos e sintáticos encontrados.

Tipo do analisador: **descendente recursivo (TDP sem backtracking)**, feito à mão, sem ferramentas geradoras.

Esta parte usa a versão nova da linguagem (palavras-chave em minúsculo, `[dec]`, `[prog]`, `<<`, `<>`, variável começando com letra maiúscula).

## Arquivos

- `main.cpp` — lê o arquivo, chama o analisador e imprime o resultado
- `sintatico.h` — analisador sintático
- `lexico.h` — analisador léxico (simulação do autômato)
- `automatos.jff` — autômato do léxico no JFLAP
- `lerArquivo.h` — leitura do arquivo caractere por caractere
- `token.h` — classe Token e os tipos de token
- `testes/` — programas de exemplo

## Como rodar

```bash
g++ -o compilador main.cpp
./compilador testes/fatorial.txt
./compilador testes/fatorial_erros.txt
./compilador -t testes/fatorial.txt
```

A opção `-t` roda só o léxico e imprime a lista de tokens. Sem ela, a saída aparece no terminal e também em `output/<nome do arquivo>.txt`.

Programa correto:

```text
Análise sintática concluída: nenhum erro encontrado.
```

Programa com erros (`testes/fatorial_erros.txt`):

```text
Erro sintático, esperado abre parêntese '(' antes de PCInt
Erro Léxico na linha 9: Desconhecido "="
Erro Léxico na linha 13: Desconhecido "="
Análise concluída com erros: 2 léxico(s) e 1 sintático(s).
```

## Tokens

| Lexema | Sigla |
|---|---|
| `dec`, `prog`, `integer`, `float`, `read`, `print`, `if`, `else`, `then`, `while`, `start`, `end` | `PCDec`, `PCProg`, `PCInt`, `PCReal`, `PCLer`, `PCImprimir`, `PCSe`, `PCSenao`, `PCEntao`, `PCEnqto`, `PCIni`, `PCFim` |
| `*`, `/`, `+`, `-` | `OpAritMult`, `OpAritDiv`, `OpAritSoma`, `OpAritSub` |
| `<`, `<=`, `>`, `>=`, `==`, `<>` | `OpRelMenor`, `OpRelMenorIgual`, `OpRelMaior`, `OpRelMaiorIgual`, `OpRelIgual`, `OpRelDif` |
| `and`, `or` | `OpBoolE`, `OpBoolOu` |
| `[`, `]` | `IniDelim`, `FimDelim` |
| `<<` | `Atrib` |
| `(`, `)` | `AbrePar`, `FechaPar` |
| letras ou números começando com letra maiúscula | `Var` |
| dígitos sem ponto / com ponto | `NumInt` / `NumReal` |
| caracteres entre aspas | `Cadeia` |

Comentários com `#` até o fim da linha.

## Autômato do léxico

O `lexico.h` é a simulação do autômato do `automatos.jff`: o número de cada estado no código é o mesmo número do estado no diagrama. O lexema é lido caractere por caractere, cada caractere leva o autômato de um estado para outro (`transicao`) e o tipo do token é o do estado final em que ele parou (`tipoDoEstadoFinal`). Nenhuma palavra é comparada como texto.

Exemplo: `dec` é reconhecido pelo caminho `1 -d-> 32 -e-> 33 -c-> 34 (PCDec)`.

Os estados marcados com `*` são alcançados pela seta "Outro": o caractere lido não faz parte do lexema e é devolvido.

## Gramática

Gramática da GYH já sem recursão à esquerda e fatorada (`{ }` = repete zero ou mais vezes, `[ ]` = opcional):

```text
Programa            -> '[' dec ']' ListaDeclaracoes '[' prog ']' ListaComandos
ListaDeclaracoes    -> Declaracao { Declaracao }
Declaracao          -> Var '(' TipoVar ')'
TipoVar             -> integer | float
ListaComandos       -> Comando { Comando }
Comando             -> ComandoAtribuicao | ComandoEntrada | ComandoSaida
                     | ComandoCondicao | ComandoRepeticao | SubAlgoritmo
ComandoAtribuicao   -> Var '<<' ExpressaoAritmetica
ComandoEntrada      -> read Var
ComandoSaida        -> print ( Var | Cadeia )
ComandoCondicao     -> if ExpressaoRelacional then Comando [ else Comando ]
ComandoRepeticao    -> while ExpressaoRelacional then Comando
SubAlgoritmo        -> start ListaComandos end
ExpressaoAritmetica -> TermoAritmetico { ('+' | '-') TermoAritmetico }
TermoAritmetico     -> FatorAritmetico { ('*' | '/') FatorAritmetico }
FatorAritmetico     -> NumInt | NumReal | Var | '(' ExpressaoAritmetica ')'
ExpressaoRelacional -> TermoRelacional { (and | or) TermoRelacional }
TermoRelacional     -> ExpressaoAritmetica OpRel ExpressaoAritmetica
                     | '(' ExpressaoRelacional ')'
```

Cada não-terminal é um método da classe `Sintatico`. A regra a seguir é sempre escolhida olhando o próximo token. O único caso em que um token só não basta é o `(` no `TermoRelacional` (pode abrir uma expressão aritmética ou uma relacional): ali o analisador olha os tokens à frente, sem consumir, até o parêntese fechar.

## Tratamento de erros

- **Erro léxico**: o léxico imprime a mensagem e o resto da linha é descartado, para não gerar erros sintáticos em cima de uma linha que já está errada.
- **Erro sintático**: a mensagem diz o que era esperado e antes de qual token (`esperado abre parêntese '(' antes de PCInt`). Depois o analisador descarta tokens até o início do próximo comando (ou da próxima linha, nas declarações) e continua, para apontar os outros erros do programa.
