# EE4204 — Computer Networks Project

This individual project was completed as part of the EE4204 Computer Networks course taught at the National University of Singapore (NUS). It focuses on implementing a client server socket program with TCP transport protocol for transferring messages using a flow control protocol. The instructions for the project are given below.

## Project Description

Develop a TCP-based client-server socket program for transferring a large message. Here, the message transmitted from the client to server is read from a large file. The message is split into short data-units which are sent by using stop and-wait flow control. Also, a data-unit sent could be damaged with some error probability. Verify if the file has been sent completely and correctly by comparing the received file with the original file. Measure the message transfer time and throughput for various sizes of data-units. Also, measure the performance for various error probabilities and for the error-free scenario.  

Choose appropriate values for parameters such as data unit size and error probability. You can simulate errors according to the frame error probability. You are free to implement the ARQ in your own way, but with stop-and-wait. For example, you may want to avoid TIMEOUTs and handle retransmissions in some other way (you may want to choose negative acknowledgement). You may also want to simulate errors with a certain probability by generating a random number. For example, to simulate error probability 0.1, generate a random number in the range between 0 and 999;  if this  number falls within the range 0 to 99, then assume there is an error in the data unit received, otherwise there is no error. Repeat the experiment several times and plot the average values in a report with a brief description of results, assumptions made, etc. Choose at  least six values for data unit size in the range between 200 and 1400 bytes. Choose at  least six values for error probability in the range between 0.0 and 0.40. Include the following performance figures in your report:

1. Transfer time vs error probability  (2 graphs; size = 500 and size = 1000 bytes)
1. Throughput vs error probability (2 graphs; size = 500 and size = 1000 bytes)
1. Transfer time vs data unit size (2 graphs; error probability = 0, error probability = 0.1)
1. Throughput vs data unit size (2 graphs; error probability = 0, error probability = 0.1)

## Usage

There is a client script and a server script that is meant to be run on 2 separate Ubuntu systems in the same network. A usage message will be printed out when running the script describing the order of the command-line arguments to be inputted.

### Run client `./client <Server IP> <Data Unit Size>`

Example:

```
./client 127.0.0.1 500
```

### Run server: `./server <Data Unit Size (in bytes)> <Error Probability>`

Example:

```
./server 500 0.1
```

## Results

![image](https://github.com/user-attachments/assets/397a95c6-3423-47a7-bfb3-50d25ae819ff)

![image](https://github.com/user-attachments/assets/b34fdbe4-4785-4d86-8647-d8a67a28dd77)
