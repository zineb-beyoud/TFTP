#include <stdio.h>
#include <stdlib.h>
#include "tftp_server.h"
int main(void)
{

   char Filename[]="test.txt";

        tftp_server_init();
   // while(1){
        tftp_server_run();
    //}

    //TFTP_Packet(Filename);

    //printf("Hello world!\n");
    return 0;
}
