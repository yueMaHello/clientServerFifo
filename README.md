# clientServerFifo
Objectives:
CMPUT379 ASSIGNMENT2 REPORT
This programming project is intended to give me experience in developing client-server programs that utilize FIFOs for communication, and file locking for controlling access to the available FIFOs. Through this project, I learned how to make Fifo and use Fifo. I also understood some blocking features about Fifo.
Design Overview:
The program supports one server and several clients. Each client can send some commands to the server and clients can communicate with each other through a proper input format.
1. The server can be invoked by ‘a2chat -s fifoBasename nclient ’. ‘nclient’ should be no more than 5.
2. The client can be executed by ‘a2chat –c fifoBasename’.
3. After invoking the server, the server will create ‘nclient’ inFifos and ‘nclient’
outFifos. Each client uses an inFifo and an outFifo to communicate with the server. Client send message through the inFifo and receive the server’s response from the outFifo.
4. When the client sends ‘open username’, a new user will open and one inFifo will be locked by this user.
5. ‘who’ command can check who is online.
6. ‘close’ will close the current user without terminating the program.
7. ‘exit’ from the client side will terminate the client process, and ‘exit’ from the
server side will terminate the server.
8. ‘to user1 user2 ....’ Will adds specified users to the list of recipients.
9. ‘<chat line’ will send the message to all specified recipients.
10. Through ‘Makefile’, the user can make, clean and zip the program easily.
Project Status:
The project has been completed with all the functionality and features. There are no known bugs existing in the program currently.There are several difficulties during the implementation:
1. When I open a Fifo and try to read it, if there is no write-in message, the terminal will be blocked.
2. ‘select’ is a good choice to detect Fifo’s input and Stdin’s input. However, the time interval should be set up properly. If the time interval is too short, it may cause some issue.
3. ‘printf’ sometimes causes ‘stderr’, and it need to be followed by a fflush(stdout). ‘write’ function is preferred.
Testing and Results:
I tested the application feature by feature. Firstly, I tested whether the Fifo can be locked properly. Then I tested whether the client’s sending messages can get proper responses. I
opened one server and several clients, and make clients communicate with the server at the same time. I also tested whether clients can talk to each other through the server.
Results: The client can lock and unlock the inFifo properly. The client’s commands can get proper responses from the server, such as ‘who’, ‘open’, ‘to’ and etc.
Acknowledgements:
1. https://www.tutorialspoint.com/cprogramming/c_strings.htm
2. https://stackoverflow.com/questions/14173268/fifo-files-and-read-write
3. http://man7.org/linux/man-pages/man2/select.2.html
4. Textbook: Advanced Programming in UNIX Environment
5. TA: Sepehr and Wanxin
