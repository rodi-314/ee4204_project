#include "headsock.h"

float str_cli(FILE *fp, int sockfd, long *len, int data_unit_size); // Transmission function
void tv_sub(struct timeval *out, struct timeval *in);               // Calculate the time interval between out and in

int main(int argc, char **argv)
{
    int sockfd, ret;
    float ti, rt;
    long len;
    struct sockaddr_in ser_addr;
    char **pptr;
    struct hostent *sh;
    struct in_addr **addrs;
    FILE *fp;

    // Parse arguments
    if (argc != 3)
    {
        printf("Usage: %s <Server IP> <Data Unit Size>\n", argv[0]);
        exit(1);
    }
    int data_unit_size = atoi(argv[2]); // Parse data unit size from command line
    if (data_unit_size <= 0)
    {
        printf("Error: Data unit size must be > 0\n");
        exit(1);
    }
    sh = gethostbyname(argv[1]); // Get host's information
    if (sh == NULL)
    {
        printf("Error: Unable to resolve host %s\n", argv[1]);
        exit(0);
    }

    // Print the remote host's information
    printf("Canonical name: %s\n", sh->h_name);
    for (pptr = sh->h_aliases; *pptr != NULL; pptr++)
        printf("The aliases' names are: %s\n", *pptr);
    switch (sh->h_addrtype)
    {
    case AF_INET:
        printf("Address Family: AF_INET\n");
        break;
    default:
        printf("Unknown Address Family\n");
        break;
    }

    // Create and connect socket
    addrs = (struct in_addr **)sh->h_addr_list;
    sockfd = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
    if (sockfd < 0)
    {
        printf("Error: Unable to create socket\n");
        exit(1);
    }
    ser_addr.sin_family = AF_INET;
    ser_addr.sin_port = htons(MYTCP_PORT);
    memcpy(&(ser_addr.sin_addr.s_addr), *addrs, sizeof(struct in_addr));
    bzero(&(ser_addr.sin_zero), 8);
    ret = connect(sockfd, (struct sockaddr *)&ser_addr, sizeof(struct sockaddr)); // Connect the socket with the host
    if (ret != 0)
    {
        printf("Error: Connection failed\n");
        close(sockfd);
        exit(1);
    }

    // Open file
    if ((fp = fopen("myfile.txt", "r+t")) == NULL)
    {
        printf("Error: File doesn't exist\n");
        exit(0);
    }

    // Transmit data
    ti = str_cli(fp, sockfd, &len, data_unit_size); // Perform the transmission and receiving
    rt = (len / (float)ti);                         // Calculate the average transmission rate
    printf("Time: %.3f ms\nData sent: %d bytes\nData rate: %f Kbytes/s\n", ti, (int)len, rt);

    // Clean up
    close(sockfd);
    fclose(fp);
    exit(0);
}

float str_cli(FILE *fp, int sockfd, long *len, int data_unit_size)
{
    char *buf;
    long lsize;
    long ci = 0;
    struct ack_so ack;
    int n, slen;
    int m = 0;
    float time_inv = 0.0;
    struct timeval sendt, recvt;

    // Get file size
    fseek(fp, 0, SEEK_END);
    lsize = ftell(fp);
    rewind(fp);
    printf("File length: %d bytes\n", (int)lsize);
    printf("Packet length: %d bytes\n", data_unit_size);

    // Allocate memory to contain the whole file
    buf = (char *)malloc(lsize);
    if (buf == NULL)
    {
        printf("Error: Memory allocation failed\n");
        exit(2);
    }

    // Allocate `sends` dynamically based on data_unit_size
    char *sends = (char *)malloc(data_unit_size);
    if (sends == NULL)
    {
        printf("Error: Memory allocation for sends buffer failed\n");
        free(buf);
        exit(2);
    }

    // Copy the file into the buffer.
    fread(buf, 1, lsize, fp);

    /*** the whole file is loaded in the buffer. ***/
    buf[lsize] = '\0';          // Append the end byte
    gettimeofday(&sendt, NULL); // Get the current time
    while (ci <= lsize)
    {
        // Copy part of file to `sends` buffer
        if ((lsize + 1 - ci) <= data_unit_size)
            slen = lsize + 1 - ci;
        else
            slen = data_unit_size;
        memcpy(sends, (buf + ci), slen);

        // Send packet
        n = send(sockfd, sends, slen, 0);
        // printf("%ld\n", ci);
        // printf("%d\n", n);
        if (n == -1)
        {
            printf("Error: Send failed\n"); // send the data
            free(buf);
            free(sends);
            exit(1);
        }

        // Wait until acknowledgement is received
        m = recv(sockfd, &ack, sizeof(ack), 0);
        if (m == -1)
        {
            printf("Error: Receiving acknowledgment failed\n");
            free(buf);
            free(sends);
            exit(1);
        }

        // Move to next packet if current packet is received correctly (ACK)
        if (ack.num == 1 && ack.len == 0)
        {
            ci += slen;
        }
        // Else resend the same packet (NACK)
        else
        {
            printf("Packet error detected, resending...\n");
        }
    }

    // Get transmission time
    gettimeofday(&recvt, NULL); // Get current time
    *len = ci;
    tv_sub(&recvt, &sendt); // Get the whole transmission time
    time_inv += (recvt.tv_sec) * 1000.0 + (recvt.tv_usec) / 1000.0;

    // Clean up
    free(buf);
    free(sends);

    return time_inv;
}

void tv_sub(struct timeval *out, struct timeval *in)
{
    if ((out->tv_usec -= in->tv_usec) < 0)
    {
        --out->tv_sec;
        out->tv_usec += 1000000;
    }
    out->tv_sec -= in->tv_sec;
}
