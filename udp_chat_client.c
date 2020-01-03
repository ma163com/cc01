#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define LEN 128
#define NAMELEN 32
#define LOGIN 1
#define CHAT 2
#define QUIT 3

//定义消息结构体
typedef struct {
	int MsgType;      //消息的类型 LOGIN  CHAT QUIT
	char Name[NAMELEN];  //用户的名字
	char Text[LEN];     //消息的内容
}MSG; 



int main(int argc,char **argv){
	
	int sfd,clientfd;
	int ret;
	int relen;
	pid_t pid;
	char buf[LEN];
	MSG cmessage;
	
    struct sockaddr_in serveraddr,clientaddr;
	int len = sizeof(serveraddr);
	if(argc!=4){  //判断用户输入参数是否正确
		printf("Usage:%s,ip,port,name\n",argv[0]);
		return 0;
	}
	sfd = socket(AF_INET,SOCK_DGRAM,0);//用SOCK_DGRAM参数创建UDP socket，获得socket文件描述符
	if(sfd <0){
		perror("socket");
		return -1;
	}
    //填充UDP服务器的网络类型，端口号，ip地址
	serveraddr.sin_family = AF_INET;    //填充网络地址类型
	serveraddr.sin_port = htons(atoi(argv[2]));   //注意，端口号是short,atoi函数把字符串转换为整数
	serveraddr.sin_addr.s_addr = inet_addr(argv[1]); //把输入的IP地址填充到serveraddr里面
 
	cmessage.MsgType = LOGIN;    //填充消息类型
	strcpy(cmessage.Name,argv[3]);
	strcpy(cmessage.Text,"");  //填充消息体
	
	
	//给服务器发送登陆消息
	sendto(sfd, &cmessage, sizeof(MSG), 0,(struct sockaddr *)&serveraddr, len);
 
 
	pid = fork();
	
	if(pid<0){
		perror("fork");
		exit(1);
	}else if( 0 == pid){ //子进程处理服务器发来的消息
		
		while(1){
			memset(&cmessage,0,sizeof(MSG));
			relen = recvfrom(sfd, &cmessage, sizeof(MSG), 0,(struct sockaddr *)&clientaddr, &len);
			if(-1 == relen){
				perror("recvfrom");
				return -1;
			}
	
			//根据服务器发来的消息类型，打印不同的字符串
			if(cmessage.MsgType == LOGIN){   //登陆消息
				printf("%s login\n",cmessage.Name);
			}else if (cmessage.MsgType == CHAT){  //聊天消息
				printf("%s said:%s\n",cmessage.Name,cmessage.Text);
				
			}else if (cmessage.MsgType == QUIT){  //退出消息
				printf("%s exit\n",cmessage.Name);
			}
			
		}
		
	}else{ //父进程处理输入的字符串
		
		while(1){ //循环等待客户输入
			fgets(buf,LEN,stdin);
			buf[strlen(buf)-1]='\0';
			if(strcmp(buf,"bye")==0){ //退出消息，发送给服务器，杀死子进程，然后自己退出
				cmessage.MsgType = QUIT;
				strcpy(cmessage.Text,"");
				sendto(sfd, &cmessage, sizeof(MSG), 0,(struct sockaddr *)&serveraddr, len);
				sleep(1);
				kill(pid,SIGKILL);
				wait(NULL);
				exit(0);
			}else{  //如果不是退出消息，则是正常的聊天消息，直接发送聊天消息给服务器
				cmessage.MsgType = CHAT;
				strcpy(cmessage.Text,buf);
				sendto(sfd, &cmessage, sizeof(MSG), 0,(struct sockaddr *)&serveraddr, len);
			}
			
		}
		
	}

	
}
