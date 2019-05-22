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
