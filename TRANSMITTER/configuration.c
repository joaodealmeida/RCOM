#include "configuration.h"

// Variáveis globais

int time_flag, counter_config = 0;
struct termios oldtio;

// Funções

void atende()
{
	time_flag = 1;
	counter_config++;
}

int llopen (int porta, int flag)
{
	/* -----	Variáveis	----- */

	unsigned char *send_message, receive_message[5], bit_received;
	unsigned char buffer[1], gate[10] = MODEMDEVICE;
	int vtime, fd, res[2], return_value, i;
	unsigned int msg_sent = 0, state;
	struct termios newtio;

	unsigned short ConfirmMsgSize;

	/* -----	Código		----- */

	sprintf (buffer, "%d", porta);
    strcat (gate, buffer);

    system ("clear");

	printf ("llopen iniciado!\n");

	if (flag == TRANSMITTER)
	{
		send_message = CreateConfirmMsg (&ConfirmMsgSize, SET);

		vtime = 0;

		fd = open(gate, O_RDWR | O_NOCTTY | O_NONBLOCK);
	}
	else if (flag == RECEIVER)
	{
		send_message = CreateConfirmMsg (&ConfirmMsgSize, UA);

		vtime = 0;

		fd = open(gate, O_RDWR | O_NOCTTY);
	}
	else
		return -1;

	if (fd < 0)
			return -1;

	// Teste ao apontador para o Pipe.

	if (tcgetattr (fd, &oldtio) == -1)
	{
		perror ("tcgetattr");
    	return -1;
	}

	// Configuração da ligação.

	bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;
 
    /* set input mode (non-canonical, no echo,...) */
    newtio.c_lflag = 0;

    newtio.c_cc[VTIME]    = vtime;   /* inter-unsigned character timer unused */
    newtio.c_cc[VMIN]     = 1;   /* blocking read until 1 unsigned chars received */

 	/* VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a leitura do(s) próximo(s) caracter(es) */

    tcflush(fd, TCIOFLUSH);
    if (tcsetattr (fd, TCSANOW, &newtio) == -1)
    {
    	perror ("tcsetattr");
      	return -1;
    }

    (void) signal(SIGALRM, atende);

    if (flag == TRANSMITTER)
	{
		for (time_flag = 1, state = 0, counter_config = 0; counter_config < numTransmissions && msg_sent != 1;)
		{
			
    		if (time_flag)
    		{    		
    			printf ("A enviar SETUP: tentativa #%d\n", counter_config + 1);

	    		res[0] = write (fd, send_message, 5);

	    		alarm (alarm_timeout);
	    		time_flag = 0;
	    		
	    	}
	    	else
	    	{
	    		while (state < 5)
	    		{
	    			res[1] = read (fd, &bit_received, 1);

	      			if(res[1] == -1)
	        			break;
	        		else
	        			printf ("Leu %d bytes.\n", res[1]);

	      			switch (state)
	      			{
	        			case 0:
				          if (bit_received == F)
				          {
				          	receive_message[state]=bit_received;
				            state++;
				          }
				          break;

				        case 1:
				          if (bit_received == A)
				          {
				          	receive_message[state]=bit_received;
				          	state++;
				          }
				          else if (bit_received != A && bit_received !=F)
				          	state--;
				          break;
				 
				        case 2:
				          if (bit_received == UA)
				          {
				            receive_message[state]=bit_received;
				            state++;
				          }
				          else if (bit_received == F)
				          	state--;
				          else
				          	state = 0;
				          break;

				        case 3:
				          if (bit_received == (A ^ UA))
				          {
				            receive_message[state]=bit_received;
				            state++;
				          }
				          else
				          	state = 0;
				          break;

				        case 4:
				          if (bit_received == F)
				          {
				            receive_message[state]=bit_received;
				            state++;
				            msg_sent = 1;
				            time_flag=0;
				            printf("ENVIADO");
				          }
				          else
				          	state = 0;
				          break;

						default:
							break;
					}
	    		}
	    	}
    	}

		if (msg_sent == 0)
		{
		    printf ("Mensagem não enviada (não recebeu UA).\n");
		    return_value = -1;
  		}
  		else
  		{
    		printf ("Mensagem UA: ");
		    for(i = 0; i < state; i++)
		    	printf("%x", receive_message[i]);
		    printf ("\n");
    		return_value = fd;
    	}
	}
	else if (flag == RECEIVER)
	{
		state = 0;
		while (state < 5)
		{
			res[1] = read (fd, &bit_received, 1);

  			switch (state)
            {
                case 0:
                    if (bit_received == F)
                    {
                            receive_message[state] = bit_received;
                            state++;
                    }
                    break;
                case 1:
                    if (bit_received == A)
                    {
                            receive_message[state] = bit_received;
                            state++;
                    }
                    else if (bit_received != F && bit_received != A)
                            state--;
                    break;
                case 2:
                    if (bit_received == SET)
                    {
                            receive_message[state] = bit_received;
                            state++;
                    }
                    else if (bit_received == F)
                            state--;
                    else
                            state = 0;
                    break;
                case 3:
                    if (bit_received == (A ^ SET))
                    {
                            receive_message[state] = bit_received;
                            state++;
                    }
                    else if (bit_received == F)
                            state = 1;
                    else
                            state = 0;
                    break;
                case 4:
                    if (bit_received == F)
                    {
                            receive_message[state] = bit_received;
                            state++;
                            msg_sent = 1;
                    }
                    else
                            state = 0;
                    break;
				default:
					break;
            }
		}
    	
		if (msg_sent == 0)
		{
		    printf ("Mensagem não enviada (não recebeu SETUP).\n");
		    return_value = -1;
  		}
  		else
  		{
    		printf ("Mensagem SETUP: ");
		    for(i = 0; i < state; i++)
		    	printf("%x", receive_message[i]);
            printf ("\n");

            res[0] = write(fd, send_message, 5);
	        /*if (!res[0])
				printf("Mensagem (UA) não enviada.\n");
	        else
				printf("%d bytes (UA) enviados.\n", res[0]);*/

    		return_value = fd;
    	}
	}

	printf ("Função llopen a fechar!\n");

	alarm (0);

    return return_value;
}

int llclose (int fd, int mode)
{
	unsigned short i;
	signed short res, return_value = -1, size = 5;
	unsigned char bit_received, *msg_received = malloc (sizeof(unsigned char) * 5), *NewMsg;

	printf ("Função llclose iniciada!\n");

	if (mode == RECEIVER)
	{
		printf ("Modo Receiver!\n");

		msg_received[4] = 0x00;
		while (msg_received[4] != F)
		{
			res = read (fd, &bit_received, 1);
			printf ("0. bit_received: %x\n", bit_received);
			if (bit_received == F)
			{
				msg_received[0] = bit_received;

				res = read (fd, &bit_received, 1);
				printf ("1. bit_received: %x\n", bit_received);
				msg_received[1] = bit_received;

				res = read (fd, &bit_received, 1);
				printf ("2. bit_received: %x\n", bit_received);
				msg_received[2] = bit_received;

				res = read (fd, &bit_received, 1);
				printf ("3. bit_received: %x\n", bit_received);
				msg_received[3] = bit_received;

				res = read (fd, &bit_received, 1);
				printf ("4. bit_received: %x\n", bit_received);
				msg_received[4] = bit_received;
			}
		}

		printf ("msg_received: %x ", msg_received[0]);
		for (i = 1; i < size; i++)
			printf ("| %x ", msg_received[i]);
		printf ("\n");

		printf ("Size: %d\n", size);

		res = ReadConfirmMsg(msg_received, size);

		printf ("RES: %x\n", res);

		if (res == DISC)
		{
			printf ("Recebe DISC!\n");
			NewMsg = CreateConfirmMsg (&size, DISC);
			write (fd, NewMsg, 5);

			msg_received[4] = 0x00;
			while (msg_received[4] != F)
			{
				res = read (fd, &bit_received, 1);
				if (bit_received == F)
				{
					msg_received[0] = bit_received;

					res = read (fd, &bit_received, 1);
					msg_received[1] = bit_received;

					res = read (fd, &bit_received, 1);
					msg_received[2] = bit_received;

					res = read (fd, &bit_received, 1);
					msg_received[3] = bit_received;

					res = read (fd, &bit_received, 1);
					msg_received[4] = bit_received;
				}
			}

			res = ReadConfirmMsg (msg_received, 5);

			if (res == UA)
			{
				printf ("Recebe UA!\n");
				return_value = 0;
			}
			else
				return_value = -1;
		}
		else
			return_value = -1;
	}
	else if (mode == TRANSMITTER)
	{
		NewMsg = CreateConfirmMsg (&size, DISC);
		write (fd, NewMsg, 5);

		msg_received[4] = 0x00;
		while (msg_received[4] != F)
		{
			res = read (fd, &bit_received, 1);
			if (bit_received == F)
			{
				msg_received[0] = bit_received;

				res = read (fd, &bit_received, 1);
				msg_received[1] = bit_received;

				res = read (fd, &bit_received, 1);
				msg_received[2] = bit_received;

				res = read (fd, &bit_received, 1);
				msg_received[3] = bit_received;

				res = read (fd, &bit_received, 1);
				msg_received[4] = bit_received;
			}
		}

		res = ReadConfirmMsg (msg_received, 5);

		if (res == DISC)
		{
			NewMsg = CreateConfirmMsg (&size, UA);
			write (fd, NewMsg, 5);
			return_value = 0;
		}
	}

	close (fd);

	return return_value;
}
