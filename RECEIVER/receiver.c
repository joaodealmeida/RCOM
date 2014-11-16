#include "receiver.h"


int llread (int fd, char *buffer)
{
	unsigned char bit_received = 0x00/*, msg_received*/, *msg_send;
	signed short BytesReceived, Type, aux;
	unsigned short counter = 0, f_found = 0, i, CntrlField = I0, CntrlField2, msg_send_size, receive_flag = 1, msg_end = 0, res, DestuffedSize;
	InfStruct InfPacket;
	unsigned char msg_received[255], *DestuffedMSG;
	unsigned char *IMAGEM;
	IMAGEM = (unsigned char*) malloc (sizeof(unsigned char) * 10968);
	FILE *savefile;
	//savefile = fopen("pinguim2.gif", "wa");

	printf ("Abriu llread!\n");

	//msg_received = (unsigned char*) malloc (sizeof(unsigned char));

	while (receive_flag)
	{	
		BytesReceived = read (fd, &bit_received, 1);
		printf("BytesReceived: %d - %x\n", BytesReceived, bit_received);
		//printf("Found: %d\n", f_found);
		//msg_received = (unsigned char*) malloc (sizeof(unsigned char));

		if (bit_received == F && f_found == 0) // primeiro F
		{
			counter = 0;
			msg_received[counter++] = bit_received;
			//msg_received = (unsigned char*) realloc (msg_received, sizeof(unsigned char) * (counter + 1));
			f_found = 1;
		}
		else if (bit_received == F && f_found == 1) // segundo F
		{
			if(counter == 1)
				continue;
			msg_received[counter++] = bit_received;

			printf ("Mensagem recebida: %x ", msg_received[0]);
			for (i = 1; i < counter; i++)
				printf ("| %x", msg_received[i]);
			printf ("\n");
			f_found = 0;	
			msg_end = 1;
			
			//receive_flag = 0;

			//msg_received = (unsigned char*) malloc (sizeof(unsigned char));

			//msg_send = CreateConfirmMsg (&msg_send_size, CntrlField);
			//write (fd, msg_send, msg_send_size);
		}
		else if (bit_received != F && f_found == 1) // resto
		{
			printf("ENTROU NO RESTO DA MSG\n");
			msg_received[counter++] = bit_received;

			//msg_received = (unsigned char*) realloc (msg_received, sizeof(unsigned char) * (counter + 1));
		}
		if (msg_end){
			printf("A verificar mensagem!\n");
			DestuffedMSG = Destuffing(msg_received, counter, &DestuffedSize);
			Type = ReadInfMsg (&InfPacket, DestuffedMSG, DestuffedSize, CntrlField);
			printf("SeqNumber especial: %d\n", InfPacket.InfData.SeqNumber);
			if(Type == -1)
			{
				//Enviar REJ(CntrlField)

				if(CntrlField == I0)
					CntrlField2 = REJ0;
				else
					CntrlField2 = REJ1;

				printf("A enviar: %x\n", CntrlField2);
				msg_send = CreateConfirmMsg (&msg_send_size, CntrlField2);

				aux = write (fd, msg_send, msg_send_size);
				printf("Bytes Written %d\n", aux);
			}
			if(Type == 1 || Type == 2)
			{
				if (Type == 2)
				{
					savefile = fopen(InfPacket.InfCntrl.Filename, "wa");
				}
				if(Type == 1)
				{
					//printf ("Data colocada:");
					for (i = 0; i < InfPacket.InfData.DataSize; i++)
					{
						//printf (" %x", InfPacket.InfData.Data[i]);
						putc (InfPacket.InfData.Data[i], savefile);
					}
					//printf ("\n");

					printf ("Recebeu mensagem nr %d e inseriu no ficheiro.\n\n\n\n", InfPacket.InfData.SeqNumber);
					printf("ControlField(I0/I1) -> %x |\n", CntrlField);
					//fwrite (InfPacket.InfData.Data, sizeof(unsigned char), sizeof(InfPacket.InfData.Data), savefile);
					//write(savefile, InfPacket.InfData.Data, DestuffedSize-10);
				}

				//Dados enviar proxima trama

				
				if(CntrlField == I0){
					CntrlField2 = RR1;
					CntrlField = I1;
				}
				else{
					CntrlField2 = RR0;
					CntrlField = I0;
				}
				printf("A enviar: %x\n", CntrlField2);

				msg_send = CreateConfirmMsg (&msg_send_size, CntrlField2);
				for(i=0; i<msg_send_size; i++)
					printf("msg_send[%d]: %x\n",i, msg_send[i]);
				printf("Passou CreateConfirmMsg\n");
				res= write (fd, msg_send, msg_send_size);
				printf("Bytes written: %d\n", res);

			}
			if (Type == 3)
			{
				//Controlo end
				//supostamente pede proxima trama normalmente (??) e so depois recebe o disc
				printf("Type 3\n");
				
				if(CntrlField == I0)
				{
					CntrlField2 = RR1;
					CntrlField = I1;
					printf ("A enviar RR1.\n");
				}
				else
				{
					CntrlField2 = RR0;
					CntrlField = I0;
					printf ("A enviar RR0.\n");
				}

				msg_send = CreateConfirmMsg (&msg_send_size, CntrlField2);
				for(i=0; i<msg_send_size; i++)
					printf("msg_send[%d]: %x\n",i, msg_send[i]);
				res = write (fd, msg_send, msg_send_size);
				printf("Bytes written: %d\n", res);
				receive_flag = 0;

				fclose(savefile);
			}

		}
		//free(msg_received);
		msg_end=0;
		//fclose (savefile);

	}

	/* O que falta fazer no llread.

		Em msg_received está a primeira trama com o Pacote de controlo. É preciso criar uma estrutura InfStruct e chamar ReadInfMsg.

		Depois é preciso ler os dados. Comparar sempre se é o campo pretendido (I0/I1) e mandar RR0/RR1.

		Depois ler o último pacote de dados como o primeiro.

	*/

	return 0;
}