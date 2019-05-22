/*

Desenvolvido como trabalho de curso para a disciplina INF1308,
Software Básico, na PUC-Rio. O objetivo desse programa é
transformar operações aritméticas feita em uma linguagem simples
para código de máquina, funcionando assim, como um compilador.

Copyright (C) 2019 Lucas Rebello Damo & Luiza Del Negro Ciuffo Fernandes

Este programa é um software livre; você pode redistribuí-lo e/ou
modificá-lo sob os termos da Licença Pública Geral GNU como publicada
pela Free Software Foundation; na versão 3 da Licença.

Este programa é distribuído na esperança de que possa ser útil,
mas SEM NENHUMA GARANTIA; sem uma garantia implícita de ADEQUAÇÃO
a qualquer MERCADO ou APLICAÇÃO EM PARTICULAR. Veja a
Licença Pública Geral GNU para mais detalhes.

Você deve ter recebido uma cópia da Licença Pública Geral GNU junto
com este programa. Se não, veja <http://www.gnu.org/licenses/>.

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gera_codigo.h"
#define TAM_MAX_CODIGO 2048   /* Tamanho maximo, em bytes, de um programa SBF */
#define QTD_MAX_FUNCOES 30    /* Numero maximo de funcoes em um programa SBF */

static void error (const char *msg, int line) {
	fprintf(stderr, "erro %s na linha %d\n", msg, line);
	exit(EXIT_FAILURE);
}

unsigned char codigoMaqPilhaVar(char var, int idx){
	int ret;
	if(var == 'p') // p0 = -4(%rbp)
		return 0xfc;
	else if (var == 'v' && idx <=4){
		ret = idx*4*(-1) - 8;
		return (unsigned char)ret;
	}
	printf("Variavel %c%d nao suportada, alocada com o endereco -32(rbp)\n",var,idx);
	ret = -32; /* Caso tente alocar numa variavel nao suportado (ex v6), aloca no espaco -32(%rbp) */
	return (unsigned char) ret;
}

void gera_codigo (FILE *fp, void **code, funcp *entry){
	int line = 1;
	int  c;
	int nAlloc;
	unsigned char *functions[QTD_MAX_FUNCOES];
	int nFunctions;
	unsigned char * codigo = (unsigned char*) malloc(sizeof(char)*TAM_MAX_CODIGO);
	*code = codigo;
	if(fp == NULL){
		printf("Erro ao receber o ponteiro para o arquivo na funcao gera_codigo\n");
		exit(-3);
	}
	nAlloc = 0;
	nFunctions = 0;
	while ((c = fgetc(fp)) != EOF) {
		switch (c) {
			case 'f': { /* function */
				char c0;
				static unsigned char inicializaPilha[] = 
				{0x55,0x48,0x89,0xe5,0x48,0x83,0xec,0x20,	/* Inicializa pilha, pushq %rbp -- movq %rsp %rbp -- subq $12 %rbp */
				 0x89,0x7d,0xfc}; 							/* Empilha o argumento  --   movq %edi, -4(%rbp) */
				if (fscanf(fp, "unction%c", &c0) != 1)
					error("comando invalido", line);
				printf("function\n");
				*entry = (funcp) &codigo[nAlloc];			/* Atualiza o endereco de entrada para a ultima funcao lida */
				functions[nFunctions] = (unsigned char *) &codigo[nAlloc]; /* Poe o endereco da funcao em um vetor de pointeiros para funcoes*/
				nFunctions++;
				memcpy(&codigo[nAlloc], inicializaPilha, sizeof(inicializaPilha));
				nAlloc = nAlloc + sizeof(inicializaPilha);
				break;
			}
			case 'e': { /* end */
				char c0;
				if (fscanf(fp, "nd%c", &c0) != 1)
					error("comando invalido", line);
				printf("end\n");
				break;
			}
			case 'r': {  /* retorno incondicional */
				int idx0;
				char var0;
				if (fscanf(fp, "et %c%d", &var0, &idx0) != 2) 
					error("comando invalido", line);
				printf("ret %c%d\n", var0, idx0);
				if(var0 == '$'){
					int j;
					codigo[nAlloc] = 0xb8;  /* move de reg reg */
					nAlloc++;
					for(j=0; j<4; j++){		/* transforma o numero em bytes \no formato Little Endian */
						codigo[nAlloc] = idx0;
						idx0 = idx0 >> 8;
						nAlloc++;
					}
				}
				else if (var0 == 'v' || var0 == 'p'){
					codigo[nAlloc] = 0x8b; /* move de reg reg */
					nAlloc++;
					codigo[nAlloc] = 0x45; /* do endereco de rbp */ 
					nAlloc++;
					codigo[nAlloc] = codigoMaqPilhaVar(var0,idx0); /* menos algum numero correspondente */
					nAlloc++;
				}
				else
					error("comando invalido",line);
				codigo[nAlloc] = 0xc9; /* leave */
				nAlloc++;
				codigo[nAlloc] = 0xc3; /* ret */
				nAlloc++;
				break;
			}
			case 'z': {  /* retorno condicional */
				int idx0, idx1;
				char var0, var1;
				static unsigned char retCond[] = {0x74,0x02,0xeb}; /*  0x74 0x02 = je 2 -- 0xeb = jmp */
				if (fscanf(fp, "ret %c%d %c%d", &var0, &idx0, &var1, &idx1) != 4) 
					error("comando invalido", line);
				printf("zret %c%d %c%d\n", var0, idx0, var1, idx1);
				codigo[nAlloc] = 0x83; /* cmpl */
				nAlloc++;
				codigo[nAlloc] = 0x7d; /* cmpl */
				nAlloc++;
				codigo[nAlloc] = codigoMaqPilhaVar(var0,idx0); /* compara a variavel com ...*/
				nAlloc++;
				codigo[nAlloc] = 0x00; /* ... 0 */
				nAlloc++;
				memcpy(&codigo[nAlloc], retCond, sizeof(retCond));
				nAlloc= nAlloc + sizeof(retCond);
				if(var1 == '$'){
					int j;
					codigo[nAlloc] = 0x07; /* caso seja uma constante pule 7 linhas */
					nAlloc++;
					codigo[nAlloc] = 0xb8; /* move const para reg */
					nAlloc++;
					for(j=0; j<4; j++){
							codigo[nAlloc] = idx1;
							idx1 = idx1 >> 8;
							nAlloc++;
					}
				}
				else if (var1 == 'v'){
					codigo[nAlloc] = 0x05; /* caso seja uma variavel pule 5 linhas */
					nAlloc++;
					codigo[nAlloc] = 0x8b; /* move de reg reg */
					nAlloc++;
					codigo[nAlloc] = 0x45; /* conteudo de endereço no rbp ... */
					nAlloc++;
					codigo[nAlloc] = codigoMaqPilhaVar(var1,idx1); /* menos n para char pos na pilha */
					nAlloc++;
				}
				else error("comando invalido, segundo operando nao identificado",line);
				codigo[nAlloc] = 0xc9; /* leave */
				nAlloc++;
				codigo[nAlloc] = 0xc3; /* ret */
				nAlloc++;
				break;
			}
			case 'v': {  /* atribuicao */
				int idx0,j;
				char var0 = c, c0;
				unsigned char xidx0;
				if (fscanf(fp, "%d = %c",&idx0, &c0) != 2)
					error("comando invalido",line);
				xidx0 = codigoMaqPilhaVar(var0,idx0);
				if (c0 == 'c') { /* call */
					int f, idx1,diff;
					char var1;
					if (fscanf(fp, "all %d %c%d\n", &f, &var1, &idx1) != 3)
						error("comando invalido",line);
					printf("%c%d = call %d %c%d\n",var0, idx0, f, var1, idx1);
					if(var1 == '$'){
						codigo[nAlloc] = 0xbf;
						nAlloc++;
						for(j=0; j<4; j++){
							codigo[nAlloc] = idx1;
							idx1 = idx1 >> 8;
							nAlloc++;
						}
					}
					else if ((var1 == 'v') || (var1 == 'p')){
						codigo[nAlloc] = 0x8b; /* move de reg reg */
						nAlloc++;
						codigo[nAlloc] = 0x7d;
						nAlloc++;
						codigo[nAlloc] = codigoMaqPilhaVar(var1,idx1);
						nAlloc++;
					}
					codigo[nAlloc] = 0xe8; /* call */
					nAlloc++;
					/* Parametro para a call; Endereco da funcao a ser chamada - endereco do proximo comando */
					diff = (unsigned char*)functions[f] - (unsigned char*)(&codigo[nAlloc-1] + 5);
					for(j=0; j<4; j++){
							codigo[nAlloc] = diff;
							diff = diff >> 8;
							nAlloc++;
					}
					codigo[nAlloc] = 0x89;
					nAlloc++;
					codigo[nAlloc] = 0x45;
					nAlloc++;
					/* movl %eax, var */
					codigo[nAlloc] = codigoMaqPilhaVar(var0, idx0);
					nAlloc++;


				}
				else { /* operacao aritmetica 
				Move o 1o argumento para o r10d
				Move o 2o argumento para o r11d
				r10d = r10d OP r11d
				Move o r10d para a variavel sendo atribuida
				*/
					int idx1, idx2;
					char var1 = c0, var2, op;
					unsigned char xidx1, xidx2; /* Posicoes na memoria */
					if (fscanf(fp, "%d %c %c%d", &idx1, &op, &var2, &idx2) != 4)
						error("comando invalido", line);
					printf("%c%d = %c%d %c %c%d\n",
					var0, idx0, var1, idx1, op, var2, idx2);
					if(!(var1 == 'v' || var1 == '$' || var1 == 'p') ||
						!(var2 == 'v' || var2 == '$' || var2 == 'p')) error("comando invalido", line);


					if(var1 != '$'){ /* 1a variavel nao e const */
						static unsigned char moveParaRegR10d[] = {0x44,0x8b,0x55};
						xidx1 = codigoMaqPilhaVar(var1,idx1);
						memcpy(&codigo[nAlloc], moveParaRegR10d, sizeof(moveParaRegR10d));
						nAlloc = nAlloc + sizeof(moveParaRegR10d);
						codigo[nAlloc] = xidx1;
						nAlloc++;
					}
					else { /* 1a variavel e const */
						int j;
						int aux;
						static unsigned char moveConstParaRegR10d[] = {0x41,0xba};
						memcpy(&codigo[nAlloc], moveConstParaRegR10d, sizeof(moveConstParaRegR10d));
						nAlloc = nAlloc + sizeof(moveConstParaRegR10d);
						aux = idx1;
						for(j=0; j<4; j++){
							codigo[nAlloc] = aux;
							aux = aux >> 8;
							nAlloc++;
						}
					}
					if(var2 != '$'){ /* 2a variavel nao e const */
						static unsigned char moveParaRegR11d[] = {0x44,0x8b,0x5d};
						xidx2 = codigoMaqPilhaVar(var2,idx2);
						memcpy(&codigo[nAlloc], moveParaRegR11d, sizeof(moveParaRegR11d));
						nAlloc = nAlloc + sizeof(moveParaRegR11d);
						codigo[nAlloc] = xidx2;
						nAlloc++;
					}
					else { /* 2a variavel e const */
						int j;
						int aux;
						static unsigned char moveConstParaRegR11d[] = {0x41,0xbb};
						memcpy(&codigo[nAlloc], moveConstParaRegR11d, sizeof(moveConstParaRegR11d));
						nAlloc = nAlloc + sizeof(moveConstParaRegR11d);
						aux = idx2;
						for(j=0; j<4; j++){
							codigo[nAlloc] = aux;
							aux = aux >> 8;
							nAlloc++;
						}
					}

					switch(op){
						case '+':{
							static unsigned char soma[] = {0x45,0x01,0xda};
							memcpy(&codigo[nAlloc], soma, sizeof(soma));
							nAlloc = nAlloc + sizeof(soma);
							break;
						}
						case '-':{
							static unsigned char sub[] = {0x45,0x29,0xda};
							memcpy(&codigo[nAlloc], sub, sizeof(sub));
							nAlloc = nAlloc + sizeof(sub);
							break;
						}
						case '*':{
							static unsigned char mult[] = {0x45,0x0f,0xaf,0xd3};
							memcpy(&codigo[nAlloc], mult, sizeof(mult));
							nAlloc = nAlloc + sizeof(mult);
							break;
						}
					}
					codigo[nAlloc] = 0x44;
					nAlloc++;
					codigo[nAlloc] = 0x89;
					nAlloc++;
					codigo[nAlloc] = 0x55;
					nAlloc++;
					codigo[nAlloc] = xidx0;
					nAlloc++;

				}
				break;
			}
			default: error("comando desconhecido", line);
		}
		line ++;
		fscanf(fp, " ");
	}
	/* PARA VISUALIZAR O CODIGO DE MAQUINA NO CONSOLE

	for(c = 0; c < nAlloc; c++){
		printf("%02x ",codigo[c]);
	}

	*/
}

void libera_codigo(void **code){
	if(code == NULL) printf("Endereco a ser liberado e NULL\n");
	free(code);
	return;
}