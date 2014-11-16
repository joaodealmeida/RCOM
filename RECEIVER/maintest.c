#include "configuration.h"
#include "transmitter.h"
#include "receiver.h"
#define RECEIVER 0
#define TRANSMITTER 1
void main()
{
    
        /*int fd;
        unsigned char *imgData;
        int dataSize;

        imgData = readFile("pinguim.gif", &dataSize);
        while (!imgData)
        {
            imgData = readFile("pinguim.gif", &dataSize);
        }
        printf ("Antes do llwrite. Fd: %d\n", fd);

    // llopen

        fd = llopen (4, TRANSMITTER);
        printf("%d\n",fd );
    // llwrite
        if(fd!=-1){
        llwrite (fd, imgData, dataSize);

    // llread
        //llread (fd, imgData);
        }
    // llclose
        //printf("%d\n",fd );
        //llclose (fd, TRANSMITTER);*/



        int fd;
        unsigned char *imgData;
        int dataSize;

    // llopen
        fd = llopen (0, RECEIVER);

    // llread 
        llread (fd, imgData);

    //close
        llclose (fd, RECEIVER);

}
