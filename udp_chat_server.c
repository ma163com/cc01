#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>

#define LEN 128
#define NAMELEN 32

#define LOGIN 1
#define CHAT 2
#define QUIT 3

typedef struct {
	int MsgType;
	char Name[NAMELEN];
	char Text[LEN];
}MSG;

typedef struct node{
	
	struct sockaddr_in ClientAddr;
	struct node *next;
	
}ClientAddrS;

ClientAddrS *addrHead = NULL;
int sfd;
struct sockaddr_in serveraddr,clientaddr;

int len = sizeof(serveraddr);

void InserLink(ClientAddrS *node){
	ClientAddrS *p;
	if(addrHead==NULL){
		addrHead = node;
		return;
	}else{
		p = addrHead;
		while(p->next){
			p = p->next;
		}
		p->next = node;
	}
	
}

void DeletLink(struct sockaddr_in caddr){
	ClientAddrS *p,*pre;
	if(addrHead==NULL){
		return;
	}else{
		p = addrHead;
		pre = p;
		while(p){
		    if(memcmp(&caddr,&(p->ClientAddr),sizeof(MSG))==0){
                pre->next = p->next;
                free((ClientAddrS *)p);
                if(p == addrHead){
                   addrHead = p=NULL;
                }else{
                    p=NULL;
                }
                return;
		    }else{
		        
		        pre = p;
                p = p->next;
		    }
		}
	}
	
}

void ClientLogin(struct sockaddr_in addr,MSG msg){
	
	ClientAddrS *addrn,*p;
	addrn = (ClientAddrS *)malloc(sizeof(ClientAddrS));
	memcpy(&addrn->ClientAddr,&addr,sizeof(ClientAddrS));
	addrn->next = NULL;
    printf("%s login\n",msg.Name);
	p = addrHead;// 遍历地址链表，发送消息给其他的客户端
	
	while(p){
		sendto(sfd, &msg, sizeof(MSG), 0,(struct sockaddr *)&(p->ClientAddr), len);
		p=p->next;
	}	
	InserLink(addrn); //插入新生成的地址到地址链表里

}
void  ClientChat (struct sockaddr_in addr,MSG msg){
	ClientAddrS *p;
	p = addrHead;// 遍历地址链表，发送退出消息给其他的客户端
	printf("%s said:%s\n",msg.Name,msg.Text);
	while(p){
	    if(memcmp(&(p->ClientAddr),&addr,sizeof(struct sockaddr_in))!=0){ 	//如果不是自己的地址才发送
			sendto(sfd, &msg, sizeof(MSG), 0,(struct sockaddr *)&(p->ClientAddr), len);
		}
		p=p->next;
	}	
	
}

void ClientQuit(struct sockaddr_in addr,MSG msg){
	ClientAddrS *p;
	DeletLink(addr); //先删除退出的客户端地址
	printf("%s quit\n",msg.Name);
	p = addrHead;// 遍历地址链表，发送消息给其他的客户端
	while(p){
		sendto(sfd, &msg, sizeof(MSG), 0,(struct sockaddr *)&(p->ClientAddr), len);
		p=p->next;
	}	
	
}

int main(int argc,char **argv){
	
	
	int ret;
	int relen;
	pid_t pid;
	char buf[LEN];
	MSG messageS;
	
	
	sfd = socket(AF_INET,SOCK_DGRAM,0);//用SOCK_DGRAM参数创建UDP socket，获得socket文件描述符
	if(sfd <0){
		perror("socket");
		return -1;
	}
	
	
    //填充网络类型，端口号，ip地址
	serveraddr.sin_family = AF_INET;    //填充网络地址类型
	serveraddr.sin_port = htons(55555);   //注意，端口号是short,
	serveraddr.sin_addr.s_addr = 0; //inet_addr("0.0.0.0"); //inet_addr转换字符串ip为网络字节序的32bit整数ip地址
	                                //绑定全0的地址表示绑定主机任意地址
	ret = bind(sfd,(struct sockaddr *)&serveraddr,len);  //绑定ip地址和端口号
	if(ret <0){
		perror("bind");
		return -1;
	}
	
	pid = fork();
	
	if(pid<0){
		
		perror("fork");
		return -1;
	}else if(pid==0){ //子进程处理服务器的字符串输入
		while(1){
			fgets(buf,LEN,stdin);
			buf[strlen(buf)-1]='\0';
			strcpy(messageS.Text,buf);
			messageS.MsgType = CHAT;
			strcpy(messageS.Name,"Server");
			sendto(sfd, &messageS, sizeof(MSG), 0,(struct sockaddr *)&serveraddr, len);
	    }
		
	}else{
		
		while(1){
			
			memset(&messageS,0,sizeof(MSG));
			relen = recvfrom(sfd, &messageS, sizeof(MSG), 0,(struct sockaddr *)&clientaddr, &len);
			
			if(-1 == relen){
				perror("recvfrom");
				return -1;
			}
			//根据客户端器发来的消息类型，进行不同的处理
			if(messageS.MsgType == LOGIN){   //登陆消息
				ClientLogin(clientaddr,messageS);
				
			}else if (messageS.MsgType == CHAT){  //聊天消息
				ClientChat(clientaddr,messageS);
				
			}else if (messageS.MsgType == QUIT){  //退出消息
				ClientQuit(clientaddr,messageS);
			}
	
		}
		
	}

	
}
