//
//  archat.c
//  ass2
//
//  Created by My on 2017-10-16.
//
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/file.h>
#include <time.h>
#define MAX_BUF 1024
#define max_user 5

int serverSide(char *argv[],char *fifoOutList[], char *fifoInList[]);
int clientSide(char *argv[],char *fifoOutList[], char *fifoInList[]);
char* concat(const char *s1, const char *s2);
const char* firstCharAfterSpace(const char* input);
const char* firstCharAfterSymbol(const char* input);
void removeChar(char *str, char garbage);
void insertArray(int a[10],int length,int x);
void deleteArray(int a[10],int x);

int main(int argc, char *argv[]){
    char * fifobaseName = argv[2];
    //only generate fifo's name, but not create fifo!
    char * fifo_1_out = concat(fifobaseName,"-1.out");
    char * fifo_2_out = concat(fifobaseName,"-2.out");
    char * fifo_3_out = concat(fifobaseName,"-3.out");
    char * fifo_4_out = concat(fifobaseName,"-4.out");
    char * fifo_5_out = concat(fifobaseName,"-5.out");
    char * fifo_1_in = concat(fifobaseName,"-1.in");
    char * fifo_2_in = concat(fifobaseName,"-2.in");
    char * fifo_3_in =concat(fifobaseName,"-3.in");
    char * fifo_4_in = concat(fifobaseName,"-4.in");
    char * fifo_5_in = concat(fifobaseName,"-5.in");
    //put all fifosin to fifoList
    char *fifoOutList[] = {fifo_1_out,fifo_2_out,fifo_3_out,fifo_4_out,fifo_5_out};
    char *fifoInList[] = {fifo_1_in,fifo_2_in,fifo_3_in,fifo_4_in,fifo_5_in};
    
    char * symbol = argv[1];
    //go to server side
    if (strcmp(symbol, "-s") == 0){
        int a = serverSide(argv,fifoOutList,fifoInList);
        printf("server stopped\n");
    }
    //go to client side
    else if(strcmp(symbol, "-c") == 0){
        int b =clientSide(argv,fifoOutList,fifoInList);
        printf("client stopped\n");
    }
    //something else
    else{
        perror("Wrong format input!");
        return 0;
    }
    
    return 0;
}
int serverSide(char *argv[],char *fifoOutList[], char *fifoInList[]){
    
    int nclient = atoi(argv[3]);
    char * baseName = argv[2];
    int numberOfClients = 0;
    char user1[100];
    char user2[100];
    char user3[100];
    char user4[100];
    char user5[100];
    char *userNameList[] = {user1,user2,user3,user4,user5};
    int user1_fifo;
    int user2_fifo;
    int user3_fifo;
    int user4_fifo;
    int user5_fifo;
    int user1ChatFifoList[10];
    int user2ChatFifoList[10];
    int user3ChatFifoList[10];
    int user4ChatFifoList[10];
    int user5ChatFifoList[10];
    int *userChatFifoList[] = {user1ChatFifoList,user2ChatFifoList,user3ChatFifoList,user4ChatFifoList,user5ChatFifoList};
    
    
    //create all #nclient fifo firstly
    for(int i = 0;i<nclient;i++){
        mkfifo(fifoInList[i],0644);
        mkfifo(fifoOutList[i],0644);
    }
    //check number of clients
    if(nclient>max_user){
        printf( "The chat program only surport up to 5 user!");
        return 1;
    }
    
    printf("Chat server begins [nclient = %d]%s\n",nclient,baseName);
    
    //begin detecting
    while(1){
        int selectResult;
        int fdIn,fdOut;
        int index2;
        char inBuf[MAX_BUF];
        char stdinBuf[MAX_BUF];
        int nread;
        fd_set readFds,readSTDIN;
        struct timeval timeInterval;
        int found = 0;
        
        for(int i= 0;i<max_user;i++){
            memset(inBuf, 0, sizeof(inBuf));
            fdIn = open(fifoInList[i],O_RDWR|O_NONBLOCK);
            FD_ZERO(&readFds);
            FD_ZERO(&readSTDIN);
            FD_SET(fdIn,&readFds);
            FD_SET(STDIN_FILENO,&readSTDIN);
            timeInterval.tv_sec = 0;
            timeInterval.tv_usec = 100;
            //select error
            selectResult = select(2, &readSTDIN, NULL, NULL, &timeInterval);
            if(selectResult== -1){
                continue;
            }
            else if(selectResult != 0){
                int readStdin = read(STDIN_FILENO,stdinBuf,sizeof(stdinBuf));
                if(readStdin>0){
                    if(strncmp(stdinBuf,"exit",strlen("exit"))==0){
                        for(int i = 0;i<nclient;i++){
                            //clean all the fifos
                            unlink(fifoInList[i]);
                            unlink(fifoOutList[i]);
                        }
                        //exit the program
                        return 1;
                    }
                }
            }
            selectResult = select(fdIn+1, &readFds, NULL, NULL, &timeInterval);
            //select error
            if(selectResult== -1) {
                close(fdIn);
                perror("select error");
                continue;
            }
            //no data
            else if(selectResult == 0) {
                close(fdIn);
                continue;
            }
            //get data!
            else{
                nread = read(fdIn, inBuf, sizeof(inBuf));
                const char msg[] = "Received:";
                write(STDOUT_FILENO,msg,sizeof(msg)-1);
                write(STDOUT_FILENO,inBuf,sizeof(inBuf));
                index2 = i;
                found =1;
                close(fdIn);
                break;
            }
            
        }
        //there is no active inFIFO
        if (found ==0){
            continue;
        }
        //generate response to the client
        fdOut= open(fifoOutList[index2],O_RDWR|O_NONBLOCK);
        if(fdOut == -1)
        {
            perror("open FIFO");
            exit(1);
        }
        char outBuf[MAX_BUF];
        memset(outBuf, '\0', sizeof(outBuf));
        const char msg5[] = "\nResponse: ";
        write(STDOUT_FILENO,msg5,sizeof(msg5)-1);
        //open username
        if(strncmp(inBuf,"open ",strlen("open "))==0){
            int duplicated = 0;
            if(numberOfClients>nclient){
                const char msg6[] = "There is no room for you to chat, and try again later";
                write(STDOUT_FILENO,msg6,sizeof(msg6)-1);
                sprintf(outBuf,"There is no room for you to chat (up to %d clients), and try again later!",nclient);
                write(fdOut,outBuf,sizeof(outBuf));
                close(fdOut);
                continue;
                
            }
            const char* userName =  firstCharAfterSpace(inBuf);
            removeChar((char*)userName,'\n');
            //check the same name user
            int n2;
            for (n2=0;n2<max_user;n2++){
                if(strcmp(userName,userNameList[n2])==0){
                    
                    sprintf(outBuf,"[server] This name has been occupied by another user, please try another one! Also, your successfull locked inFifo has been unlocked.\n");
                    write(STDOUT_FILENO,outBuf,sizeof(outBuf)-1);
                    fdOut= open(fifoOutList[index2],O_RDWR|O_NONBLOCK);
                    write(fdOut,outBuf,sizeof(outBuf));
                    close(fdOut);
                    duplicated = 1;
                    
                    break;
                }
            }
            
            //no duplicated name
            if (duplicated==0){
                
                sprintf(outBuf,"[server] User %s connected on FIFO %d\n",userName,index2+1);
                fdOut= open(fifoOutList[index2],O_RDWR|O_NONBLOCK);
                write(STDOUT_FILENO,outBuf,sizeof(outBuf));
                write(fdOut,outBuf,sizeof(outBuf));
                close(fdOut);
                numberOfClients = numberOfClients+1;
                //store username
                if(index2== 0){
                    sprintf(user1,"%s",userName);
                    user1_fifo = index2;
                }
                else if(index2 ==1 ){
                    sprintf(user2,"%s",userName);
                    user2_fifo = index2;
                }
                else if(index2 ==2){
                    sprintf(user3,"%s",userName);
                    user3_fifo = index2;
                }
                else if(index2 ==3){
                    sprintf(user4,"%s",userName);
                    user4_fifo = index2;
                }
                else if(index2 ==4){
                    sprintf(user5,"%s",userName);
                    user5_fifo = index2;
                }
                else{
                    perror("assign user");
                    exit(1);
                }
                
            }
        }
        //close this user
        else if(strncmp(inBuf,"close",strlen("close"))==0){
            sprintf(outBuf,"[server] done\n");
            write(STDOUT_FILENO,outBuf,sizeof(outBuf));
            write(fdOut,outBuf,sizeof(outBuf));
            close(fdOut);
            numberOfClients = numberOfClients-1;
            //clean the record of that user
            if(index2 == 0){
                memset(user1, '\0', sizeof(user1));
                user1_fifo = -1;
                
                for (int i = 0; i < sizeof(userChatFifoList[0] )/sizeof(userChatFifoList[0][0]); i++) {
                    userChatFifoList[0][i] = 1;
                }
                //delete userList
                int n;
                for(n=0;n<max_user;n++){
                    deleteArray(userChatFifoList[n],1);
                }
            }
            else if(index2 == 1){
                memset(user2, '\0', sizeof(user2));
                user2_fifo = -1;
                for (int i = 0; i < sizeof(userChatFifoList[1] )/sizeof(userChatFifoList[1][0]); i++) {
                    userChatFifoList[1][i] = 1;
                }
                int n;
                for(n=0;n<max_user;n++){
                    deleteArray(userChatFifoList[n],2);
                }
            }
            else if(index2 == 2){
                memset(user3, '\0', sizeof(user3));
                user3_fifo = -1;
                for (int i = 0; i < sizeof(userChatFifoList[2] )/sizeof(userChatFifoList[2][0]); i++) {
                    userChatFifoList[2][i] = 1;
                }
                int n;
                for(n=0;n<max_user;n++){
                    deleteArray(userChatFifoList[n],3);
                }
            }
            else if(index2 == 3){
                memset(user4, '\0', sizeof(user4));
                user4_fifo = -1;
                for (int i = 0; i < sizeof(userChatFifoList[3] )/sizeof(userChatFifoList[3][0]); i++) {
                    userChatFifoList[3][i] = 1;
                }
                int n;
                for(n=0;n<max_user;n++){
                    deleteArray(userChatFifoList[n],4);
                }
            }
            else if(index2 == 4){
                memset(user5, '\0', sizeof(user5));
                user5_fifo = -1;
                for (int i = 0; i < sizeof(userChatFifoList[4] )/sizeof(userChatFifoList[4][0]); i++) {
                    userChatFifoList[4][i] = 1;
                }
                int n;
                for(n=0;n<max_user;n++){
                    deleteArray(userChatFifoList[n],5);
                }
            }
            else {perror("delete user");exit(1);}
            
        }
        //exit
        else if(strncmp(inBuf,"exit",strlen("exit"))==0){
            if(index2 == 0){
                memset(user1, '\0', sizeof(user1));
                user1_fifo = -1;
            }
            else if(index2 == 1){
                memset(user2, '\0', sizeof(user2));
                user2_fifo = -1;
            }
            else if(index2 == 2){
                memset(user3, '\0', sizeof(user3));
                user3_fifo = -1;
            }
            else if(index2 == 3){
                memset(user4, '\0', sizeof(user4));
                user4_fifo = -1;
            }
            else if(index2 == 4){
                memset(user5, '\0', sizeof(user5));
                user5_fifo = -1;
            }
            else {
                perror("delete user");
                exit(1);
            }
            
        }
        else if(strncmp(inBuf,"who",strlen("who"))==0){
            memset(outBuf, '\0', sizeof(outBuf));
            sprintf(outBuf,"[server]: Current user:");
            if(strlen(user1)!=0){
                sprintf(outBuf+ strlen(outBuf),"[%d] %s",user1_fifo+1,user1);
            }
            if(strlen(user2)!=0){
                sprintf(outBuf + strlen(outBuf),", [%d] %s",user2_fifo+1,user2);
            }
            if(strlen(user3)!=0){
                sprintf(outBuf + strlen(outBuf),", [%d] %s",user3_fifo+1,user3);
            }
            if(strlen(user4)!=0){
                sprintf(outBuf + strlen(outBuf),", [%d] %s",user4_fifo+1,user4);
            }
            if(strlen(user5)!=0){
                sprintf(outBuf + strlen(outBuf),", [%d] %s",user5_fifo+1,user5);
            }
            sprintf(outBuf + strlen(outBuf),"%s","\n");
            write(STDOUT_FILENO,outBuf,sizeof(outBuf));
            write(fdOut,outBuf,sizeof(outBuf));
            close(fdOut);
        }
        else if(strncmp(inBuf,"to ",strlen("to "))==0){
            //split by space
            char splitStrings[10][10]; //can store 10 words of 10 characters
            char buffer[1024];
            int rv = sprintf(buffer, "%s",inBuf);
            char *token = strtok(buffer, " \t");
            int i = 0;
            sprintf(outBuf,"[server] recipients added: ");
            while (token) {
                char s[100];
                memset(s,'\0',sizeof(s));
                sprintf(s,"%s", token);
                int size = strlen(s)-1;
                if(strncmp(s,user1,size)==0){
                    insertArray(userChatFifoList[index2],9,1);
                    sprintf(outBuf + strlen(outBuf),"%s ",s);
                    
                }
                else if( strncmp(s,user2,size)==0){
                    insertArray(userChatFifoList[index2],9,2);
                    sprintf(outBuf + strlen(outBuf),"%s ",s);
                    
                }
                else if( strncmp(s,user3,size)==0){
                    insertArray(userChatFifoList[index2],9,3);
                    sprintf(outBuf + strlen(outBuf),"%s ",s);
                }
                else if( strncmp(s,user4,size)==0){
                    insertArray(userChatFifoList[index2],9,4);
                    sprintf(outBuf + strlen(outBuf),"%s ",s);
                }
                else if( strncmp(s,user5,size)==0){
                    insertArray(userChatFifoList[index2],9,5);
                    sprintf(outBuf + strlen(outBuf),"%s ",s);
                }
                //printf("cm%d\n",C);
                token = strtok(NULL, " \t");
                
            }
            sprintf(outBuf+strlen(outBuf),"%s","\n");
            
            write(STDOUT_FILENO,outBuf,sizeof(outBuf));
            write(fdOut,outBuf,sizeof(outBuf));
            close(fdOut);
            
        }
        else if(strncmp(inBuf,"<",strlen("<"))==0){
            char someBuf[MAX_BUF];
            const char* userWords =  firstCharAfterSymbol(inBuf);
            memset(someBuf, '\0', sizeof(outBuf));
            sprintf(someBuf,"[Dell]%s",userWords);
            for (int n=0;n<10;n++){
                printf("%d",n);
                if(userChatFifoList[index2][n] == 0){
                    continue;
                }
                int fdOutn;
                fdOutn= open(fifoOutList[userChatFifoList[index2][n]-1],O_RDWR|O_NONBLOCK);
                write(fdOutn,someBuf,sizeof(someBuf));
                write(STDOUT_FILENO,someBuf,sizeof(someBuf));
                close(fdOutn);
            }
            
        }
        //something else, just echo
        else{
            write(STDOUT_FILENO,inBuf,sizeof(inBuf));
            write(fdOut,inBuf,sizeof(inBuf));
            close(fdOut);
        }
    }
    
    return 1;
}

int clientSide(char *argv[],char *fifoOutList[], char *fifoInList[]){
    char * baseName = argv[2];
    int index;
    int found = 0;
    const char beginMsg[] = "Chat client begins \n";
    write(STDOUT_FILENO,beginMsg,sizeof(beginMsg)-1);
    const char msg[] = "a2chat_client: ";
    write(STDOUT_FILENO,msg,sizeof(msg)-1);
    
    while(1){
        char inBuf[MAX_BUF];//send to sever
        char outBuf[MAX_BUF];//receive from the server
        memset(inBuf, '\0', sizeof(inBuf));
        memset(outBuf, '\0', sizeof(outBuf));
        
        int selectResult;
        int fdIn,fdOut;
        char stdinBuf[MAX_BUF];
        //set for 'select'
        fd_set readSTDIN;
        struct timeval timeInterval;
        FD_ZERO(&readSTDIN);
        FD_SET(STDIN_FILENO,&readSTDIN);
        timeInterval.tv_sec = 0;
        timeInterval.tv_usec = 100;
        
        selectResult = select(1, &readSTDIN, NULL, NULL, &timeInterval);
        
        if(selectResult){
            read(STDIN_FILENO, inBuf, sizeof(inBuf));
            //open
            
            if(found == 0&&strncmp(inBuf,"open ",strlen("open "))==0){
                const char* userName =  firstCharAfterSpace(inBuf);
                removeChar((char*)userName,'\n');
                for(int i =0;i<=max_user;i++){
                    fdIn = open(fifoInList[i],O_RDWR);
                    //find an unavailable fifo
                    if (lockf(fdIn,F_TEST,0)==0){
                        index = i;
                        if(lockf(fdIn,F_TLOCK,0)==0){
                            pid_t lockPID = getpid();
                            printf("FIFO [%s] has been successfully locked by PID [%d]\n",fifoInList[i],lockPID);
                            found = 1;
                            write(fdIn, inBuf,sizeof(inBuf));
                            break;//found, break the for loop
                        };
                    }
                    
                }
                if(found==0){
                    const char msgs[] ="No avaliable Fifo or room for you to chat,please try again later!\n";
                    write(STDOUT_FILENO,msgs,sizeof(msgs)-1);
                    const char msg[] = "a2chat_client: ";
                    write(STDOUT_FILENO,msg,sizeof(msg)-1);
                    continue;
                }
                
            }
            else if(found == 1 &&strncmp(inBuf,"open ",strlen("open "))==0){
                char temBuf[MAX_BUF];
                memset(temBuf, '\0', sizeof(temBuf));
                sprintf(temBuf,"%s","Opening a new user failed.You have already opened a user!\n");
                
                write(STDOUT_FILENO,temBuf,sizeof(temBuf));
                const char msg[] = "a2chat_client: ";
                write(STDOUT_FILENO,msg,sizeof(msg)-1);
                continue;
                
            }
            else if(found==1 && strncmp(inBuf,"close",strlen("close"))==0){
                fdIn = open(fifoInList[index],O_RDWR);
                write(fdIn, inBuf,sizeof(inBuf));
                if(lockf(fdIn,F_ULOCK,fdIn)!=0){
                    perror("unlock FIFO");
                    exit(1);
                    
                }
                found =0;
                close(fdIn);
                
                
            }
            else if(found == 1 && strncmp(inBuf,"exit",strlen("exit"))==0){
                fdIn = open(fifoInList[index],O_RDWR);
                write(fdIn, inBuf,sizeof(inBuf));
                break;
            }
            else if(found == 1 && strncmp(inBuf,"who",strlen("who"))==0){
                fdIn = open(fifoInList[index],O_RDWR);
                write(fdIn, inBuf,sizeof(inBuf));
                
                
            }
            else if(found == 1 && strncmp(inBuf,"to ",strlen("to "))==0){
                fdIn = open(fifoInList[index],O_RDWR);
                write(fdIn, inBuf,sizeof(inBuf));
                
            }
            else if(found == 1 && strncmp(inBuf,"<",strlen("<"))==0){
                fdIn = open(fifoInList[index],O_RDWR);
                write(fdIn, inBuf,sizeof(inBuf));
                const char msg[] = "a2chat_client: ";
                write(STDOUT_FILENO,msg,sizeof(msg)-1);
                
            }
            else if(found==0){
                
                const char msg[] = "You haven't open a user\na2chat_client: ";
                write(STDOUT_FILENO,msg,sizeof(msg)-1);
                continue;
            }
            else if(found == 1){
                const char msg[] = "Wrong format!\na2chat_client:";
                write(STDOUT_FILENO,msg,sizeof(msg)-1);
            }
            
        }
        if(found ==1||strncmp(inBuf,"close",strlen("close"))==0){
            fd_set readFds;
            FD_ZERO(&readFds);
            fdOut = open(fifoOutList[index],O_RDWR|O_NONBLOCK);
            FD_SET(fdOut,&readFds);
            timeInterval.tv_sec = 1;
            timeInterval.tv_usec = 100;
            int selectFd;
            selectFd = select(fdOut+1, &readFds, NULL, NULL, &timeInterval);
            if(selectFd == -1){
                perror("select error");
                continue;
            }
            else if(selectFd){
                read(fdOut,outBuf,sizeof(outBuf));
                write(STDOUT_FILENO,outBuf,sizeof(outBuf));
                if(strstr(outBuf,"[server] This name")){
                    //since the duplicated name, you have to unlock your inFifo
                    
                    if(lockf(fdIn,F_ULOCK,fdIn)!=0){
                        perror("unlock FIFO");
                        exit(1);
                        
                    }
                    found =0;
                    close(fdIn);
                    close(fdOut);
                    
                }
                const char msg[] = "a2chat_client: ";
                write(STDOUT_FILENO,msg,sizeof(msg)-1);
            }
            close(fdOut);
        }
        
    }
    return 1;
}
char* concat(const char *s1, const char *s2){
    const size_t len1 = strlen(s1);
    const size_t len2 = strlen(s2);
    char *result = malloc(len1+len2+1);
    memcpy(result, s1, len1);
    memcpy(result+len1, s2, len2+1);
    return result;
}
const char* firstCharAfterSpace(const char* input) {
    const char* starting = input;
    while (*starting != ' ') {
        starting++;
    }
    // first one _after_
    return starting+1;
}
const char* firstCharAfterSymbol(const char* input) {
    const char* starting = input;
    while (*starting != '<') {
        starting++;
    }
    // first one _after_
    return starting+1;
}
void removeChar(char *str, char garbage) {
    char *src, *dst;
    for (src = dst = str; *src != '\0'; src++) {
        *dst = *src;
        if (*dst != garbage) dst++;
    }
    *dst = '\0';
}
void insertArray(int a[10],int length,int x){
    int i,n;
    n=0;
    for (i=length;i>n;i--){ a[i]=a[i-1];}
    a[n]=x;
    
}
void deleteArray(int a[10],int x){
    int i,n;
    n=0;
    for (i=9;i>n;i--){
        if(a[i] == x){
            a[i]=0;
        }
        
    }
}


