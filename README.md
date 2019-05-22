📚 Um mini compilador para uma linguagem simples, SBF, com operadores aritméticos

Data de envio para avaliação: 26 Nov 2018, 19:56
Autores: @lucasdamo e Luiza Del Negro Ciuffo Fernandes

# Executando o programa
Para rodar o programa é necessário criar uma função main. O código fonte contém apenas as funções para compilar e liberar o espaço em memória utilizado.

# Observações
Para simplificar as operações, alocamos os operandos em registradores temporarios,
o primeiro operando ficava no r10d e o segundo no r11d, dessa forma as operações
eram feitas com o seguinte modelo:
r10d = r10d operador r11d

O programa copia imediatamente o valor do parametro para a pilha -4(%rbp), quando
uma nova função é chamada, de modo a preservar o valor, mesmo com outras funções sendo 
chamadas.

# A linguagem SBF
A Linguagem SBF
Funções na linguagem SBF contém apenas atribuições, operações aritméticas, chamadas de outras funções e retorno. Todas as funções SBF são delimitadas por uma marca de início (function) e uma marca de fim (end).

A linguagem tem um único tipo de dado: inteiro de 32 bits, com sinal.

Variáveis locais são denotadas por vi, sendo o índice i utilizado para identificar a variável (ex. v0, v1, etc...). A linguagem permite o uso de no máximo 5 variáveis locais. As variáveis locais serão necessáriamente alocadas na pilha!

As funções recebem apenas um parâmetro, denotado por p0.

Constantes são escritas na forma $i, onde i é um valor inteiro, com um sinal opcional. Por exemplo, $10 representa o valor 10 e $-10 representa o valor -10.

Uma atribuição tem a forma
```
var '=' expr
```
onde var é uma variável local e expr é uma operação aritmética ou uma chamada de função.
Uma operação aritmética tem a forma
```
varpc op varpc
```
onde varpc é uma variável local, o parâmetro da função ou uma constante e op é um dos operadores: + - *
A instrução de chamada de função tem a forma
```
'call' num varpc
```
onde num é um número que indica a função SBF que será chamada, com argumento varpc (uma variável local, o parâmetro da função ou uma constante).
A primeira função do arquivo de entrada será a de número 0, a segunda a de número 1, e assim por diante. Uma função só pode chamar a si mesma ou funções que apareçam antes dela no arquivo de entrada. A última função do arquivo de entrada é a que será chamada pelo programa principal.

Existem dois tipos de retorno: incondicional e condicional. A instrução de retorno incondicional tem a forma
```
'ret' varpc
```
Seu significado é que a função corrente deverá retornar, e o valor de retorno é o segundo operando.
A instrução de retorno condicional tem a forma
```
'zret' varpc varpc
```
Seu significado é que, se o primeiro operando tiver valor igual a zero a função corrente deverá retornar, e o valor de retorno é o segundo operando. Não haverá retorno se o primeiro operando tiver valor diferente de zero.
A sintaxe da linguagem SBF pode ser definida formalmente como abaixo. Note que as cadeias entre ' ' são símbolos terminais da linguagem: os caracteres ' não aparecem nos comandos SBF.
```
pgm	::=	func | func pgm
func	::=	header cmds endf
header	::=	'function\n'
endf	::=	'end\n'
cmds	::=	cmd'\n' | cmd '\n' cmds
cmd	::=	att | ret | zret
att	::=	var '=' expr
expr	::=	oper | call
oper	::=	varpc op varpc
call	::=	'call' num varpc
ret	::=	'ret' varpc
zret	::=	'zret' varpc varpc
var	::=	'v' num
varpc	::=	var | 'p0' | '$' snum
op	::=	'+' | '-' | '*'
num	::=	digito | digito num
snum	::=	[-] num
digito	::=	0' | '1' | '2' | '3' | '4' | '5' | '6' | '7'| '8' | '9'
```

# Testes Realizados
*** Teste 1 *** Função que calcula a soma dos quadrados de 1 até o valor do seu parâmetro
```
function
v0 = p0 * p0
ret v0
end
function
zret p0 $0
v0 = p0 - $1
v1 = call 0 p0
v0 = call 1 v0
v0 = v0 + v1
ret v0
end
```
Valor de retorno = 30 com o parametro 4  
Valor de retorono = 385 com o parametro 10


*** Teste 2 *** Função que calcula o fatorial de seu parâmetro
```
function
zret p0 $1
v0 = p0 + $0
v1 = v0 - $1
v1 = call 0 v1
v0 = v0 * v1
ret v0
end
```
Valor de retorno = 24 com o parametro 4  
Valor de retorno = 720 com o parametro 6


*** Teste 3 *** Função que soma 1 ao parâmetro
```
function
v0 = p0 + $1
ret v0
end
```
Valor de retorno = -1 com o parametro -2  
Valor de retorno = -100 com o parametro -101


*** Teste 4 *** Função que multiplica seu parâmetro por 4, caso seja 0 retorna -1
```
function
v0 = p0 + $1
v1 = v0 - $1
v2 = v1 + $1
v3 = v2 - $1
v4 = v3 * $2
ret v4
end
function
zret p0 $-1
v0 = p0 * $2
v1 = call 0 v0
ret v1
end
```
Valor de retorno = 20 com o parametro 5  
Valor de retorno = -1 com o parametro 0  
Valor de retorno = 108 com o parametro 27
