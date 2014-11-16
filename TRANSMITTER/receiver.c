#include "receiver.h"

int llread (int fd, char *buffer)
{
	unsigned char bit_received = 0x00, *msg_received, *msg_send;
	signed short BytesReceived, Type;
	unsigned short counter = 0, f_found = 0, i, CntrlField = I0, CntrlField2, msg_send_size, receive_flag = 1, msg_end = 0;
	InfStruct *InfPacket;

	printf ("Abriu llread!\n");

	msg_received = (unsigned char*) malloc (sizeof(unsigned char));

	while (receive_flag)
	{
		BytesReceived = read (fd, &bit_received, 1);

		if (bit_received == F && f_found == 0) // primeiro F
		{
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

			msg_end = 1;
			receive_flag = 0;

			//msg_received = (unsigned char*) malloc (sizeof(unsigned char));

			msg_send = CreateConfirmMsg (&msg_send_size, CntrlField);
			write (fd, msg_send, msg_send_size);
		}
		else if (bit_received != F && f_found == 1) // resto
		{
			msg_received[counter++] = bit_received;

			msg_received = (unsigned char*) realloc (msg_received, sizeof(unsigned char) * (counter + 1));
		}
		else if (msg_end){
			Type = ReadInfMsg (InfPacket, msg_received, counter, CntrlField);
			if(Type == -1){
				//Enviar REJ(CntrlField)

				if(CntrlField == I0)
					CntrlField2 = REJ0;
				else
					CntrlField2 = REJ1;

				msg_send = CreateConfirmMsg (&msg_send_size, CntrlField2);
				write (fd, msg_send, msg_send_size);
			}
			if(Type == 1){
				//Dados enviar proxima trama

				if(CntrlField == I0){
					CntrlField2 = RR1;
					CntrlField = I1;
				}
				else{
					CntrlField2 = RR0;
					CntrlField = I0;
				}

				msg_send = CreateConfirmMsg (&msg_send_size, CntrlField2);
				write (fd, msg_send, msg_send_size);

			}
			if(Type == 3){
				//Controlo end
				//supostamente pede proxima trama normalmente (??) e so depois recebe o disc


			}

		}

	}

	/* O que falta fazer no llread.

		Em msg_received está a primeira trama com o Pacote de controlo. É preciso criar uma estrutura InfStruct e chamar ReadInfMsg.

		Depois é preciso ler os dados. Comparar sempre se é o campo pretendido (I0/I1) e mandar RR0/RR1.

		Depois ler o último pacote de dados como o primeiro.

	*/

	return 0;
}