#include "transmitter.h"

int time_flag, counter_transmitter = 0;

void call_transmitter()
{
	time_flag = 1;
	counter_transmitter++;
}

/* --- divideData --- */

unsigned char *divideData(unsigned char *dataBlock, unsigned long dataSize, unsigned short interval, unsigned short *dataDividedSize)
{
	int i, j, start, dataBlockSize;
	unsigned char *dataDivided;
	unsigned short maxInt;

	if(dataSize % packetsize == 0)
		maxInt = (int) dataSize/packetsize;
	else
		maxInt = ((int) dataSize/packetsize) + 1;

	if(interval > maxInt)
		return NULL;

	if(interval*packetsize > dataSize){
		dataBlockSize = packetsize - (interval*packetsize - dataSize);
	}
	else
		dataBlockSize = packetsize;
	
	dataDivided = (unsigned char *) malloc (sizeof(unsigned char) * (dataBlockSize));


	start = (interval-1) * packetsize ;

	//Point dataBlock to dataDivided
	printf("Start: %d\n", start);
	printf("DataBlockSize: %d\n", dataBlockSize);

	for(i = start , j = 0; i < dataBlockSize+start; i++, j++)
		dataDivided[j] = dataBlock[i];
	

	*dataDividedSize=dataBlockSize;

	return dataDivided;

}

/* --- ReadFile --- */

unsigned char *readFile(unsigned char *filepath, unsigned long *dataSize)
{
	FILE *pImg;
	unsigned int i, length;
	unsigned char *data;

	system ("clear");

	pImg = fopen (filepath, "r");
    if (pImg == NULL)
    {
        printf ("Ficheiro não encontrado.", 0);
        printf ("\n");
        printf ("(certifique-se que o ficheiro está na pasta do programa)", 0);
        printf ("\n");
        return NULL;
    }
    else
    {
        printf ("Ficheiro encontrado.", 0);
        printf ("\n");
        printf ("Clique <Enter> para continuar.", 0);
        getchar ();
    }

	// Leitura do conteúdo da imagem para variável "buffer"
	fseek (pImg, 0, SEEK_END);
	length = ftell (pImg);
	fseek (pImg, 0, SEEK_SET);
	
	data = (unsigned char *) malloc (sizeof(unsigned char) * length);
	*dataSize= length;

	printf("Length of file: %d\n", length);

	for (i = 0; i < *dataSize; i++)
	    data[i] = getc(pImg);

	fclose (pImg);

	return data;
}

int llwrite (int fd, char *buffer, int length)
{
	unsigned char *msg_send, *cntrlMsg, *infoMsg, *msgReceived, *DataOne = "123", *DataTwo = "pinguin.gif", *dataBlock, *CntrlPacket, *DataPacket, AuxLength[5], *SendMsg, *msg_received, bit_received, *newData;
	unsigned short msg_sent = 0, msg_send_size, counter = 0, f_found = 0, msg_read, res, SendMsgSize, dataDividedSize, BCC2, PacketSize, TypeOne = 0, TypeTwo = 1, LengthOne, LengthTwo = strlen(DataTwo), CntrlField, i, newDataSize;
	signed short BytesReceived;
	int dataInterval = 1;
	CntrlStruct PacoteCntrl;
	DataStruct PacoteDados;

//divide file

    dataBlock = divideData(buffer, length, dataInterval, &dataDividedSize);

    if(dataBlock == NULL)
        printf("Intervalo inserido fora dos limites.\n");

//create control packet

    sprintf (AuxLength, "%d", length);
    DataOne = (unsigned char*) malloc (sizeof(unsigned char) * strlen(AuxLength));
    for (i = 0; i < strlen(AuxLength); i++)
    	DataOne[i] = AuxLength[i];

    LengthOne = strlen(AuxLength);

    CntrlField = 2;

    PacoteCntrl.Type[0] = 0;
    PacoteCntrl.Length[0] = LengthOne;
    PacoteCntrl.FileSize = (unsigned char *) malloc (sizeof (unsigned char) * PacoteCntrl.Length[0]);
    for (i = 0; i < PacoteCntrl.Length[0]; i++)
        PacoteCntrl.FileSize[i] = DataOne[i];

    PacoteCntrl.Type[1] = 1;
    PacoteCntrl.Length[1] = LengthTwo;
    PacoteCntrl.Filename = (unsigned char *) malloc (sizeof (unsigned char) * PacoteCntrl.Length[1]);
    for (i = 0; i < PacoteCntrl.Length[1]; i++)
        PacoteCntrl.Filename[i] = DataTwo[i];

    CntrlPacket = CreateCntrlPacket (PacoteCntrl, &PacketSize, CntrlField, &BCC2);

    CntrlField = I0; // <---- começa com I0

    SendMsg = CreateInfMsg (&SendMsgSize, CntrlField, CntrlPacket, PacketSize, BCC2);

//initialize data packet
    PacoteDados.SeqNumber = 0;


//create control packet

    printf ("Trama I:\n%x ", SendMsg[0]);
    for (i = 1; i < SendMsgSize; i++)
    	printf (" | %x", SendMsg[i]);
    printf ("\n");

    (void) signal(SIGALRM, call_transmitter);

    for (time_flag = 1, counter_transmitter = 0; counter_transmitter < numTransmissions && msg_sent == 0;)
	{
		
		if (time_flag)
		{    		
    		printf ("A enviar mensagem: tentativa #%d\n", counter_transmitter + 1);
			
			for(i = 0; i < SendMsgSize; i++)
					printf("SendMSG[%d]: %x \n", i, SendMsg[i]);
			
    		res = write (fd, SendMsg, SendMsgSize);
			printf("Bytes Written: %d\n", res);

    		alarm (alarm_timeout);
    		time_flag = 0;
    		
    	}
    	else
    	{
			printf("ignorou timeflag");
    		msg_read = 0;
		    while (msg_read == 0)
			{
				BytesReceived = read (fd, &bit_received, 1);
				printf("BytesReceived: %d - %x\n",BytesReceived, bit_received); 
				
				
				if (bit_received == F && f_found == 0) // primeiro F
				{
					msg_received = (unsigned char*) malloc (sizeof(unsigned char));
					msg_received[counter++] = bit_received;
					msg_received = (unsigned char*) realloc (msg_received, sizeof(unsigned char) * (counter + 1));
					f_found = 1;
				}
				else if (bit_received == F && f_found == 1) // segundo F
				{
					msg_received[counter++] = bit_received;

					printf ("Mensagem recebida: %x ", msg_received[0]);
					for (i = 1; i < counter; i++)
						printf ("| %x", msg_received[i]);
					printf ("\n");

					f_found = 0;
					counter = 0;

					msg_read = 1;
				}
				else if (bit_received != F && f_found == 1) // resto
				{	
					
					msg_received[counter++] = bit_received;
					msg_received = (unsigned char*) realloc (msg_received, sizeof(unsigned char) * (counter + 1));
				}
			}

			res = ReadConfirmMsg (msg_received, 5);
			if (res == RR1) // Recebeu, quer a próxima.
			{
				printf ("Recebeu I0, quer a próxima.\n");
				PacoteDados.SeqNumber++;
				// Creating new Data Packet
				newData = divideData(buffer, PacoteCntrl.FileSize, PacoteDados.SeqNumber, &newDataSize);
				if(newData == NULL){
					printf("A DAR NULL ");
					CntrlField = 3;
					CntrlPacket = CreateCntrlPacket (PacoteCntrl, &PacketSize, CntrlField, &BCC2);
					CntrlField = DISC;
				}
				else{
					PacoteDados.Data = newData;
					/*for(i=0 ; i< newDataSize; i++)
						printf("PacoteDados.data[%d]: %x\n",i, PacoteDados.Data[i]);*/
					PacoteDados.DataSize = newDataSize;
					CntrlField = I1;
					DataPacket = CreateDataPacket(PacoteDados, &PacketSize, 1 , &BCC2);

				}	
				//Sending Data Packet
				
				SendMsg = CreateInfMsg (&SendMsgSize, CntrlField, DataPacket, PacketSize, BCC2);
				time_flag=1;
				
				

			}
			else if (res == REJ0) // Não recebeu, quer a mesma.
			{
				printf ("Não recebeu I0, quer a mesma.\n");
				// Creating new Data Packet
				//divideData(buffer, PacoteCntrl.FileSize, PacoteDados.SeqNumber, &newDataSize);
				DataPacket = CreateDataPacket(PacoteDados, &PacketSize, 1 , &BCC2);
				//Sending Data Packet
				SendMsg = CreateInfMsg (SendMsgSize, CntrlField, DataPacket, PacketSize, BCC2);
				
			}
			else if (res == RR0){
				printf ("Recebeu I1, quer a próxima.\n");
				PacoteDados.SeqNumber++;
				// Creating new Data Packet
				newData = divideData(buffer, PacoteCntrl.FileSize, PacoteDados.SeqNumber, &newDataSize);
				if(newData == NULL){
					CntrlField = 3;
					CntrlPacket = CreateCntrlPacket (PacoteCntrl, &PacketSize, CntrlField, &BCC2);
					CntrlField = DISC;
				}
				else{
					PacoteDados.Data = newData;
					PacoteDados.DataSize = &newDataSize;
					CntrlField = I0;
					DataPacket = CreateDataPacket(PacoteDados, &PacketSize, 1 , &BCC2);
				}
				//Sending Data Packet
				SendMsg = CreateInfMsg (SendMsgSize, CntrlField, DataPacket, PacketSize, BCC2);
				
			}
			else if (res == REJ1){
				printf ("Não recebeu I1, quer a mesma.\n");
				// Creating new Data Packet
				//divideData(buffer, PacoteCntrl.FileSize, PacoteDados.SeqNumber, &newDataSize);
				DataPacket = CreateDataPacket(PacoteDados, &PacketSize, 1 , &BCC2);
				//Sending Data Packet
				SendMsg = CreateInfMsg (SendMsgSize, CntrlField, DataPacket, PacketSize, BCC2);
				
			}
			else if (res == DISC){
				printf("Envio do ficheiro terminado.\n");
				msg_sent=1;
				//chamar o llclose e terminar
			}
		}
	}

/*	O que falta fazer no llwrite.

	Onde tem o printf("Recebeu, quer a próxima.\n"); - chamar SendMsg = CreateInfMsg(...) com próximo bloco (a seguir ao pacote de controlo vem o de dados)

	Comparar a mensagem recebida com RR0/RR1. Enviar a que ele quer com SendMsg = Crea...

	No fim, voltar a mandar o pacote de controlo.

*/

	return 0;
}
