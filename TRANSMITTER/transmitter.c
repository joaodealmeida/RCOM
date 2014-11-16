#include "transmitter.h"

int time_flag, counter_transmitter = 0;

void call_transmitter()
{
	time_flag = 1;
	counter_transmitter++;
	printf ("Atendeu o alarme.\n");
}

/* --- divideData --- */

unsigned char *divideData(unsigned char *dataBlock, unsigned long dataSize, unsigned short interval, unsigned short *dataDividedSize)
{
	int i, j, start, dataBlockSize;
	unsigned char *dataDivided;
	unsigned short maxInt;
	
	//printf("DataSize %d:", dataSize);
	//printf("PacketSize %d", packetsize);
	if(dataSize % packetsize == 0)
		maxInt = (int) dataSize/packetsize;
	else
		maxInt = ((int) (dataSize/packetsize)) + 1;

	//printf("MAX INT: %d\n", maxInt);
	//printf("------------------------ Sequence: %d\n", interval);

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
	//printf("Start: %d\n", start);
	//printf("DataBlockSize: %d\n", dataBlockSize);

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
	unsigned char *msg_send, *cntrlMsg, *infoMsg, *msgReceived, *DataOne = "123", *DataTwo = "pinguim.gif", *dataBlock, *CntrlPacket, *DataPacket, AuxLength[5], *SendMsg/*,*msg_received*/, bit_received, *newData, msg_received[255], *StuffedMSG,dataFileLength[50];
	unsigned short res, LastSignal, msg_sent = 0, msg_send_size, counter = 0, f_found = 0, msg_read, SendMsgSize, dataDividedSize, BCC2, PacketSize, TypeOne = 0, TypeTwo = 1, LengthOne, LengthTwo = strlen(DataTwo), CntrlField, i, newDataSize,resto =0, StuffedSize;
	signed short BytesReceived;
	int dataInterval = 1, imDone = 0;
	CntrlStruct PacoteCntrl;
	DataStruct PacoteDados;

//divide file

    /*dataBlock = divideData(buffer, length, dataInterval, &dataDividedSize);

    if(dataBlock == NULL)
        printf("Intervalo inserido fora dos limites.\n");*/

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
		printf ("Entrou. Time_flag = %d\n", time_flag);
		if (time_flag)
		{    		
    		printf ("A enviar mensagem: tentativa #%d\n", counter_transmitter + 1);
			
			printf("Trama a Enviar: ");
			for(i = 0; i < SendMsgSize; i++)
				printf("%x |", SendMsg[i]);
			printf("\n");
			printf ("---------------------------------------------------------- A enviar SeqNr: %d\n", SendMsg[5]);

			StuffedMSG = Stuffing(SendMsg, SendMsgSize, &StuffedSize);		
				res = write (fd, StuffedMSG, StuffedSize);
			
			free(StuffedMSG);
			printf("Bytes Written: %d\n", res);

    		alarm (alarm_timeout);
    		time_flag = 0;
    		
    	}
    	else
    	{
			//printf("ignorou timeflag");
    		msg_read = 0;
		    while (msg_read == 0 && time_flag == 0)
			{
				//printf ("Time_flag: %d\n", time_flag);
				//alarm(0);
				BytesReceived = read (fd, &bit_received, 1);
				
				//printf("Resto: %d\n" , resto);
				//printf("BytesReceived: %d - %x\n",BytesReceived, bit_received); 
				//printf("f_found: %d \n", f_found);
				
				
				if (bit_received == F && f_found == 0) // primeiro F
				{	
					printf("Primeiro F");
					counter = 0;
					//msg_received = (unsigned char*) malloc (sizeof(unsigned char));
					msg_received[counter++] = bit_received;
					//msg_received = (unsigned char*) realloc (msg_received, sizeof(unsigned char) * (counter + 1));
					f_found = 1;
				}
				else if (bit_received == F && f_found == 1 && resto == 1) // segundo F
				{
					msg_received[counter++] = bit_received;

					printf ("Mensagem recebida: %x ", msg_received[0]);
					for (i = 1; i < counter; i++)
						printf ("| %x", msg_received[i]);
					printf ("\n");

					f_found = 0;

					msg_read = 1;

					bit_received = 0x00;

					time_flag = 1;
					counter_transmitter = 0;
				}
				else if (bit_received != F && f_found == 1) // resto
				{	
					resto = 1;
					msg_received[counter++] = bit_received;
					//msg_received = (unsigned char*) realloc (msg_received, sizeof(unsigned char) * (counter + 1));
				}
			}
			
			printf ("msg_read: %d\n", msg_read);

			if (msg_read == 0)
			{
				resto = 0;
				counter = 0;
				continue;
			}
			
			
			printf ("Tamanho da mensagem (counter): %d\nMensagem recebida: %x ", counter, msg_received[0]);
			for (i = 1; i < counter; i++)
				printf ("| %x", msg_received[i]);
			printf ("\n");
			
			//time_flag = 0;
			
			/*if (LastSignal == REJ1 || LastSignal == REJ0)
			{
				msg_received[0]= F;
				msg_received[1]= A;
				msg_received[0]= LastSignal;
				msg_received[0]= msg_received[1] ^ msg_received[2];
				msg_received[4]= F;
			}*/

			printf ("Vai analisar esta nova mensagem.\n");
			res = ReadConfirmMsg (msg_received, counter);
			printf ("Analisou esta nova mensagem. Retornou: %x\n", res);

			resto = 0;
			if( (res == RR1 || res == RR0) && imDone == 1)
				msg_sent = 1;

			if (res == RR1) // Recebeu, quer a próxima.
			{
				printf ("Recebeu I0, quer a próxima. ------------------------------- A criar mensagem nr %d.\n", PacoteDados.SeqNumber + 1);
				PacoteDados.SeqNumber++;
				// Creating new Data Packet
				//printf("DEBUG: %d",PacoteCntrl.FileSize[0]);
				newData = divideData(buffer, length, PacoteDados.SeqNumber, &newDataSize);
				if(newData == NULL){
					printf("A DAR NULL ");
					
					CntrlField = 3;
					CntrlPacket = CreateCntrlPacket (PacoteCntrl, &PacketSize, CntrlField, &BCC2);
					CntrlField = I1;
					
					printf("BCC2 Final Pacote: %x\n", BCC2); 
					SendMsg = CreateInfMsg (&SendMsgSize, CntrlField, CntrlPacket, PacketSize, BCC2);
					//CntrlField = DISC;
					imDone = 1;
					
				}
				else{
					PacoteDados.Data = newData;
					/*for(i=0 ; i< newDataSize; i++)
						printf("PacoteDados.data[%d]: %x\n",i, PacoteDados.Data[i]);*/
					PacoteDados.DataSize = newDataSize;
					CntrlField = I1;
					DataPacket = CreateDataPacket(PacoteDados, &PacketSize, 1 , &BCC2);
					SendMsg = CreateInfMsg (&SendMsgSize, CntrlField, DataPacket, PacketSize, BCC2);

				}	
				//Sending Data Packet
				//printf("Size: %d", SendMsgSize);
				//time_flag=1;
				/*for(i = 0; i < SendMsgSize; i++)
					printf("SendMSG[%d]: %x \n", i, SendMsg[i]);*/
				alarm(0);
				LastSignal = RR1;
				printf ("Time_flag.\n");

			}
			else if (res == REJ0) // Não recebeu, quer a mesma.
			{
				printf ("Não recebeu I0, quer a mesma.\n");
				// Creating new Data Packet
				//divideData(buffer, PacoteCntrl.FileSize[0], PacoteDados.SeqNumber, &newDataSize);
				DataPacket = CreateDataPacket(PacoteDados, &PacketSize, 1 , &BCC2);
				//Sending Data Packet
				SendMsg = CreateInfMsg (&SendMsgSize, CntrlField, DataPacket, PacketSize, BCC2);
				
				alarm(0);
				LastSignal = REJ0;
				//time_flag = 1;
				
			}
			else if (res == RR0)
			{
				printf ("Recebeu I1, quer a próxima. ------------------------------- A criar mensagem nr %d.\n", PacoteDados.SeqNumber + 1);
				PacoteDados.SeqNumber++;
				// Creating new Data Packet
				newData = divideData(buffer, length, PacoteDados.SeqNumber, &newDataSize);
				if(newData == NULL){
					printf("A DAR NULL ");
					CntrlField = 3;
					CntrlPacket = CreateCntrlPacket (PacoteCntrl, &PacketSize, CntrlField, &BCC2);
					CntrlField = I0;
					SendMsg = CreateInfMsg (&SendMsgSize, CntrlField, CntrlPacket, PacketSize, BCC2);
					imDone = 1;
				}
				else{
					PacoteDados.Data = newData;
					PacoteDados.DataSize = newDataSize;
					CntrlField = I0;
					DataPacket = CreateDataPacket(PacoteDados, &PacketSize, 1 , &BCC2);
					SendMsg = CreateInfMsg (&SendMsgSize, CntrlField, DataPacket, PacketSize, BCC2);
				}
				//Sending Data Packet
				//printf("Size: %d", SendMsgSize);
				alarm(0);
				LastSignal = RR0;
				//time_flag = 1;
				/*for(i = 0; i < SendMsgSize; i++)
					printf("SendMSG[%d]: %x \n", i, SendMsg[i]);*/
				
			}
			else if (res == REJ1)
			{
				printf ("Não recebeu I1, quer a mesma.\n");
				// Creating new Data Packet
				//divideData(buffer, PacoteCntrl.FileSize[0], PacoteDados.SeqNumber, &newDataSize);
				DataPacket = CreateDataPacket(PacoteDados, &PacketSize, 1 , &BCC2);
				//Sending Data Packet
				SendMsg = CreateInfMsg (&SendMsgSize, CntrlField, DataPacket, PacketSize, BCC2);
				
				alarm(0);
				LastSignal = REJ1;
				//time_flag = 1;
				
			}
			else
				printf ("Não entrou em nenhum sinal.\n");
			
		}
	}

/*	O que falta fazer no llwrite.

	Onde tem o printf("Recebeu, quer a próxima.\n"); - chamar SendMsg = CreateInfMsg(...) com próximo bloco (a seguir ao pacote de controlo vem o de dados)

	Comparar a mensagem recebida com RR0/RR1. Enviar a que ele quer com SendMsg = Crea...

	No fim, voltar a mandar o pacote de controlo.

*/

	return 0;
}
