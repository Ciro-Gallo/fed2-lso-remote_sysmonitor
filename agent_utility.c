 #include "agent_utility.h"

  
void handleSigInt(int s) {
    if( close(sd) < 0 ) {
        perror("Error closing socket\n");
    }
    exit (-1);
}

void handleSigPipe(int s) {
    if( close(sd) < 0 ) {
        perror("Error closing socket\n");
    }
    printf("Server disconnected\n");
    exit (-1);
}