#include "manipulation.h"

#define packetsize 100

/* --- divideData ---

    Param:
        unsigned char *dataBlock:   		Buffer que contém a imagem a ser enviada.
        unsigned long *dataSize:			Tamanho do buffer que contém a imagem.
        unsigned short interval:			Sequencia da mensagem a enviar.
        unsigned short dataDividedSize		Tamanha do buffer returnado pela funcao.
 
    Return:
        unsigned char *dataDivided:     Apontador para o início da sequência *interval em dataBlock.
	
	Notas: Penso ser necessário colocar em argumento uma variável com o size do dataDivided de forma
	a podermos mexer nele no futuro.

--- divideData --- */

unsigned char *divideData(unsigned char *dataBlock, unsigned long dataSize, unsigned short interval, unsigned short *dataDividedSize);

/* --- ReadFile ---

    Param:
        unsigned char *filepath:   	Nome do ficheiro a enviar.
        unsigned long *dataSize:	Tamanho do buffer que contém a imagem.

    Return:
        unsigned char *data:     Apontador para o início da sequência *interval em dataBlock.

--- ReadFile --- */

    unsigned char *readFile(unsigned char *filepath, unsigned long *dataSize);

/* --- llwrite --- */

int llwrite (int fd, char *buffer, int length);