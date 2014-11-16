#ifndef MANIPULATION_H
#define MANIPULATION_H

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <termios.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>

// Variáveis de configuração

#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS"
#define RECEIVER 0
#define TRANSMITTER 1
#define alarm_timeout 3
#define numTransmissions 3
#define filename "pinguim.gif"
#define packetsize 100

// Variáveis de mensagem

#define F 0x7e
#define A 0x03
#define UA 0x07
#define SET 0x03
 
#define I0 0x00
#define I1 0x40
 
#define RR0 0x05
#define RR1 0x85
 
#define REJ0 0x01
#define REJ1 0x81
 
#define DISC 0x0B

typedef struct
{
    unsigned long SeqNumber, DataSize;
    unsigned char *Data;
} DataStruct;

typedef struct 
{
    unsigned short Type[2], Length[2];
    unsigned char *Filename, *FileSize;
} CntrlStruct;

typedef struct 
{
    unsigned short InfType;
    DataStruct InfData;
    CntrlStruct InfCntrl;
} InfStruct;

/* --- Stuffing ---

    Param:
        unsigned char *Destuffed:   Mensagem original, que ainda não levou stuffing.
        int DestuffedSize:          Tamanho da mensagem original, que ainda não levou stuffing.
        int *StuffedSize:           Tamanho da nova mensagem, que levará stuffing.

    Return:
        unsigned char *Stuffed:     Nova mensagem, que levou stuffing.

    P.S.: A função já ignora o 1º e último elementos, pelo que Destuffed já deverá conter as flags F.

--- Stuffing --- */

unsigned char *Stuffing(unsigned char *Destuffed, unsigned short DestuffedSize, unsigned short *StuffedSize);

/* --- Destuffing ---

    Param:
        unsigned char *Stuffed:     Mensagem original, que levou stuffing.
        int StuffedSize:            Tamanho da mensagem original, que levou stuffing.
        int *DestuffedSize:         Tamanho da nova mensagem, que levará destuffing.

    Return:
        unsigned char *Destuffed:   Nova mensagem, que levou destuffing.

    P.S.: A função já ignora o 1º e último elementos, pelo que Stuffed já deverá conter as flags F.

--- Destuffing --- */

unsigned char *Destuffing(unsigned char *Stuffed, unsigned short StuffedSize, unsigned short *DestuffedSize);

/* --- CreateDataPacket ---

    Param:
        DataStruct DataPacket:      Estrutura que contém tudo o que um DataPacket precisa (SeqNumber, K e Data).
        unsigned short *PacketSize: Tamanho da string onde será colocado o pacote de dados com o cabeçalho inteiro.
        unsigned char CntrlField:   Valor do campo de controlo.
        unsigned short *BCC2:       Valor que contêm o "ou exclusivo" para verificação do pacote de dados.

    Return:
        unsigned char *DataString:  String onde será colocado o pacote de dados com o cabeçalho inteiro.

--- CreateDataPacket --- */

unsigned char *CreateDataPacket (DataStruct DataPacket, unsigned short *PacketSize, unsigned short CntrlField, unsigned short *BCC2);

/* --- ReadDataPacket ---

    Param:
        DataStruct DataPacket:      Estrutura para colocar tudo o que um DataPacket precisa (SeqNumber, K e Data).
        unsigned char *DataString:  String onde está colocado o pacote de dados com o cabeçalho inteiro.
        unsigned short PacketSize:  Tamanho da string onde está colocado o pacote de dados com o cabeçalho inteiro.

    Return:
        signed short *BCC2:         Valor que contêm o "ou exclusivo" para verificação do pacote de dados.

--- ReadDataPacket --- */

signed short ReadDataPacket (DataStruct *DataPacket, unsigned char *DataString, unsigned short PacketSize);

/* --- CreateCntrlPacket ---

    Param:
        CntrlStruct CntrlPacket:    Estrutura que contém tudo o que um CntrlPacket precisa (Types, Lengths, Filename e FileSize).
        unsigned short *PacketSize: Tamanho da string onde será colocado o pacote de dados com o cabeçalho inteiro.
        unsigned short CntrlField:  Valor do campo de controlo.
        unsigned short *BCC2:       Valor que contêm o "ou exclusivo" para verificação do pacote de dados.

    Return:
        unsigned char *CntrlString:  String onde será colocado o pacote de controlo com o cabeçalho inteiro.

--- CreateCntrlPacket --- */

unsigned char *CreateCntrlPacket (CntrlStruct CntrlPacket, unsigned short *PacketSize, unsigned short CntrlField, unsigned short *BCC2);

/* --- ReadCntrlPacket ---

    Param:
        DataStruct CntrlPacket:      Estrutura para colocar tudo o que um DataPacket precisa (SeqNumber, K e Data).
        unsigned char *CntrlString:  String onde está colocado o pacote de dados com o cabeçalho inteiro.
        unsigned short PacketSize: Tamanho da string onde está colocado o pacote de dados com o cabeçalho inteiro.

    Return:
        signed short BCC2:       Valor que contêm o "ou exclusivo" para verificação do pacote de dados.

--- ReadCntrlPacket --- */

signed short ReadCntrlPacket (CntrlStruct *CntrlPacket, unsigned char *CntrlString, unsigned short PacketSize);

/* --- CreateConfirmMsg ---

    Param:
        unsigned short *ConfirmMsgSize: Tamanho da mensagem de confirmação (será sempre 5).
        unsigned short CntrlField:  Valor do campo de controlo.

    Return:
        unsigned char *ConfirmMsg:  String onde será colocado mensagem de confirmação com o cabeçalho inteiro.

--- CreateConfirmMsg --- */

unsigned char *CreateConfirmMsg (unsigned short *ConfirmMsgSize, unsigned short CntrlField);

/* --- ReadConfirmMsg ---

    Param:
        unsigned char *ConfirmMsg:      String onde está colocada a mensagem de confirmação.
        unsigned short *ConfirmMsgSize: Tamanho da mensagem de informação.

    Return:
        signed short CntrlField:  Campo de controlo. Retorna -1 caso a mensagem não seja válida. Case seja válida retorna a Flag da ConfirmMsg.

--- ReadConfirmMsg --- */

signed short ReadConfirmMsg (unsigned char *ConfirmMsg, unsigned short ConfirmMsgSize); // <-- Confirmar State final com ConfirmMsgSize

/* --- CreateInfMsg ---

    Param:
        unsigned short *InfMsgSize: Tamanho da mensagem de informação.
        unsigned short CntrlField:  Valor do campo de controlo.
        unsigned char *DataField:   Conteúdo de dados a colocar.
        unsigend short DataSize:    Tamanho do conteúdo de dados.
        unsigned short BCC2:        BCC2 do conteúdo de dados.

    Return:
        unsigned char *InfMsg:      String onde será colocado mensagem de informação com o cabeçalho inteiro.

--- CreateInfMsg --- */

unsigned char *CreateInfMsg (unsigned short *InfMsgSize, unsigned short CntrlField, unsigned char *DataField, unsigned short DataSize, unsigned short BCC2);

/* --- ReadInfMsg ---

    Param:
        InfStruct *InfPacket:       Estrutura para colocar tudo o que uma mensagem de Informação precisa.
        unsigned char *InfString:   String onde está colocado mensagem de informação com o cabeçalho inteiro.
        unsigned short InfMsgSize:  Tamanho da mensagem de informação.

    Return:
        signed short Type:          Devolve -1 se a InfMsg for inválida. Caso válida, devolve o tipo de dados que contêm.

--- ReadInfMsg --- */

signed short ReadInfMsg (InfStruct *InfPacket, unsigned char *InfString, unsigned short InfMsgSize, signed short CntrlField);

#endif
