#include "headsock.h"

#define BACKLOG 10

void str_ser(int sockfd, int data_unit_size, float error_prob); // Transmitting and receiving function

int main(int argc, char **argv)
{
    int sockfd, con_fd, ret;
    struct sockaddr_in my_addr;
    struct sockaddr_in their_addr;
    int sin_size;
    pid_t pid;

    // Parse arguments
    if (argc != 3)
    {
        printf("Usage: %s <Data Unit Size> <Error Probability>\n", argv[0]);
        exit(1);
    }
    int data_unit_size = atoi(argv[1]); // Parse data unit size from command line
    float error_prob = atof(argv[2]);   // Parse error probability from command line
    if (data_unit_size <= 0)
    {
        printf("Error: Data unit size must be > 0\n");
        exit(1);
    }
    if (error_prob < 0.0 || error_prob > 1.0)
    {
        printf("Error: Error probability must be between 0.0 and 1.0\n");
        exit(1);
    }

    // Create and connect socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0); // Create socket
    if (sockfd < 0)
    {
        printf("Error: Unable to create socket\n");
        exit(1);
    }
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(MYTCP_PORT);
    my_addr.sin_addr.s_addr = htonl(INADDR_ANY); // inet_addr("172.0.0.1");
    bzero(&(my_addr.sin_zero), 8);
    ret = bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)); // Bind socket
    if (ret < 0)
    {
        printf("Error: Error in binding");
        exit(1);
    }
    ret = listen(sockfd, BACKLOG); // Listen
    if (ret < 0)
    {
        printf("Error: Error in listening");
        exit(1);
    }

    // Receive data
    while (1)
    {
        printf("Waiting for data...\n");
        sin_size = sizeof(struct sockaddr_in);
        con_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size); // Accept the packet
        if (con_fd < 0)
        {
            printf("Error: Error when accepting packet\n");
            exit(1);
        }
        if ((pid = fork()) == 0) // Create acception process
        {
            close(sockfd);
            str_ser(con_fd, data_unit_size, error_prob); // Receive packet and response
            close(con_fd);
            exit(0);
        }
        else
        {
            close(con_fd); // Parent process
        }
    }

    // Clean up
    close(sockfd);
    exit(0);
}

void str_ser(int sockfd, int data_unit_size, float error_prob)
{
    char buf[BUFSIZE];
    FILE *fp;
    struct ack_so ack;
    int end = 0;
    int n = 0;
    int m = 0;
    long lseek = 0;

    printf("Receiving data!\n");

    // Allocate `recvs` dynamically based on data_unit_size
    char *recvs = (char *)malloc(data_unit_size);
    if (recvs == NULL)
    {
        printf("Error: Memory allocation for recvs buffer failed\n");
        exit(2);
    }

    // Receive packets and send acknowledgements
    while (!end)
    {
        // Receive the packet
        n = recv(sockfd, recvs, data_unit_size, 0);
        if (n == -1) 
        {
            printf("Error: Error when receiving packet\n");
            free(recvs);
            exit(1);
        }

        // Check for errors
        if (rand() % 1000 < (error_prob * 1000) || recvs[n - 1] != '\0' && n != data_unit_size)
        {
            printf("Packet corrupted...\n");
            ack.num = 0; // NACK
        }
        else
        {
            memcpy((buf + lseek), recvs, n);
            lseek += n;
            ack.num = 1;              // ACK
            if (recvs[n - 1] == '\0') // If it is the end of the file
            {
                end = 1;
            }
        }

        // Send the ACK/NACK
        ack.len = 0;
        m = send(sockfd, &ack, sizeof(ack), 0); 
        if (m == -1)
        {
            printf("Error: Error when sending acknowledgement!"); 
            free(recvs);
            exit(1);
        }
    }

    // Write data into file
    if ((fp = fopen("myTCPreceive.txt", "wt")) == NULL)
    {
        printf("Error: File doesn't exist\n");
        free(recvs);
        exit(0);
    }
    fwrite(buf, 1, lseek, fp);
    fclose(fp);
    printf("A file has been successfully received!\nTotal data received: %d bytes\n", (int)lseek);
    free(recvs);
}
