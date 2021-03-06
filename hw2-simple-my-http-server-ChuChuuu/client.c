#include "client.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <pthread.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>

//#define PORT 7788
char PORT[10];
int sock;
char IP[10];
void message(char* requiremsg,char* one,char* two,char* three);
int Analysis(char* receive,char* con);
void* func(void* file);
void makeall(char *dir);
int mk_dir(char *dir);



int main(int argc, char *argv[])
{

    int sock_fd = 0;
    struct sockaddr_in sock_addr;
    char buffer[4096] = "";
//   int iret;
    char ip[20]= {'\0'};
    int port = atoi(argv[6]);
//    int test;
    //something i need
    strcpy(ip,argv[4]);
    strcpy(PORT,argv[6]);
    message(buffer,argv[2],argv[4],argv[6]);
    //printf("1:%s\n",buffer);
    //create socket
    if( (sock_fd = socket(AF_INET,SOCK_STREAM,0)) < 0) {
        printf("socket error\n");
    }
    sock = sock_fd;
    //send the request to server
    memset(&sock_addr,0,sizeof(sock_addr));
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_port = htons(port);
    sock_addr.sin_addr.s_addr = inet_addr(ip);

    if(connect(sock_fd,(struct sockaddr*)(&sock_addr),sizeof(sock_addr)) == -1) {
        printf("connect fail\n");
    }
//	printf("connect success\n");


    //this is use to write and read msg
//	memset(buffer,'\0',sizeof(buffer));
//	fgets(buffer,sizeof(buffer),stdin);
//    iret = write(sock_fd,buffer,strlen(buffer));//send message
    //printf("write data :%s,len = %d\n",buffer,iret);
//    char a[]="quit";
//	printf("%d\n",strcmp(buffer,a));
//	printf("waiting server msg\n");
//    read(sock_fd,buffer,sizeof(buffer));
//    printf("%s",buffer);

//    Analysis(buffer);

//	makeall("./output/set");


    pthread_t t;
    pthread_create(&t,NULL,func,argv[2]);
//    func(argv[2]);


    //message(buffer,"/secfolder","127.0.0.1","7788");
    //iret = write(sock_fd,buffer,strlen(buffer));
    //read(sock_fd,buffer,sizeof(buffer));
    //printf("%s",buffer);
    pthread_join(t,NULL);
    close(sock_fd);

    return 0;
}
void* func(void* file)
{
    char buffermsg[4096]="";
//	char IP[20];
    char File[128]="";//the file first send
//	char PORT[5];
    char filename[10][256];//receive file name
    char content[3996]="";
    char sendfile[128] = "";//the file second send
    char* temp;
    char cut[]=" \n";
    char C_directory[137] = "./output";
    int iret;
    int iiret;
    int RE;//1 is directory//2 is no need
    int howmanyfile = 0;
    int i;

    FILE *pFile;
    char text[128] = "";
    char *test;//test /
    pthread_t t;

    strcpy(File,file);
    message(buffermsg,File,IP,PORT);
    iret = write(sock,buffermsg,strlen(buffermsg));
    iiret = read(sock,buffermsg,sizeof(buffermsg));
    printf("%s",buffermsg);
    RE = Analysis(buffermsg,content);
//	printf("RE:%d\n\n",RE);

    if(RE == 1) {
        //create the directory
        strcat(C_directory,File);

//	printf("C_direc:%s\n",C_directory);
        makeall(C_directory);
        temp = strtok(content,cut);
        while(temp != NULL) {
            strcpy(filename[howmanyfile],temp);
            temp = strtok(NULL,cut);
            howmanyfile++;
        }
        for( i = 0 ; i < howmanyfile ; i++) {
            strcpy(sendfile,File);
            strcat(sendfile,"/");
            strcat(sendfile,filename[i]);

            pthread_create(&t,NULL,func,sendfile);
            pthread_join(t,NULL);
        }
    } else if(RE == 2) {
//	printf("the content:\n\n%s",content);
        strcat(C_directory,File);
        test = strrchr(C_directory,'/');
        strcpy(text,test);
        *test = '\0';
        makeall(C_directory);
        strcat(C_directory,text);
        pFile = fopen(C_directory,"w");
        fwrite(content,1,sizeof(content),pFile);
        fclose(pFile);
    }

    pthread_exit(NULL);

}



int mk_dir(char *dir)
{
    DIR *mydir =NULL ;
    if( (mydir=opendir(dir))  == NULL) {
        int ret = mkdir(dir,S_IRWXU | S_IRWXG | S_IRWXO);
        if(ret != 0) {
            return -1;
        }
//                printf("OK\n");
    } else {
//                printf("NO\n");
    }

    return 0;
}

void makeall(char *dir)
{
    char Route[200]="";
    char reRoute[200]="";
    char cut[]="/";
    strcpy(Route,dir);
    char* test;
    int ret;
    test = strtok(Route,cut);
    while(test!=NULL) {
        strcat(reRoute,test);
        strcat(reRoute,"/");
        ret = mk_dir(reRoute);
//		printf("%s\n",reRoute);
        test = strtok(NULL,cut);
    }
}

int Analysis(char* receive,char* con)
{
    char Rec_msg[4096] = "";
//    char cut[]="\r\n";
    char Content_type[100]="";
    char File_content[3996]="";
    char OK[10] = "";

    strcpy(Rec_msg,receive);
    sscanf(Rec_msg,"%*s %s %*[^'\r\n'] %*s %[^'\r\n'] %*[^'\r\n'] %3000c",OK,Content_type,File_content);
//    printf("status:%s\n\nType:%s\n\ncontent:%s\n\n",OK,Content_type,File_content);
    if(strcmp(Content_type,"directory") == 0) {
        strcpy(con,File_content);
        return 1;
    } else if(strcmp(OK,"200") == 0) {
        strcpy(con,File_content);
        return 2;
    } else {
        return 3;
    }

}
void message(char* requiremsg,char* one, char* two, char* three)
{

    strcpy(requiremsg,"GET ");
    strcat(requiremsg,one);
    strcat(requiremsg," HTTP/1.x\r\nHOST: ");
    strcat(requiremsg,two);
    strcat(requiremsg,":");
    strcat(requiremsg,three);
    strcat(requiremsg,"\r\n\r\n");
//	printf("%s\n",requiremsg);
    return ;

}
