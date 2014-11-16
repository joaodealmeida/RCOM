#include "configuration.h"
#include "transmitter.h"
#include "receiver.h"

void main()
{

		int fd;
		unsigned char *imgData;
		int dataSize;

		imgData = readFile("pinguim.gif", &dataSize);
	    while (!imgData)
	    {
	        imgData = readFile("pinguim.gif", &dataSize);
	    }
	    printf ("Antes do llwrite. Fd: %d\n", fd);

	// llopen

		fd = llopen (0, TRANSMITTER);
		printf("%d\n",fd );
	// llwrite
	   llwrite (fd, imgData, dataSize);

	// llread
	    //llread (fd, imgData);

	//llclose
	    //printf("%d\n",fd );
		llclose (fd, TRANSMITTER);




}
