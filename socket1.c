#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/ip.h>
#include "socket.h"
#include <linux/if.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <netdb.h>
#include <net/route.h>
#include <netinet/in.h>

char srcip[16];
char dstip[16];
struct src_dst_ip *ip = NULL;

struct session* path_head;
struct session* resv_head;

int sock = 0;

#define IP_ADDRLEN 16

void get_ip(char buffer[], char *sender_ip, char *receiver_ip) {

	struct session_object *temp = (struct session_object*)(buffer+START_RECV_SESSION_OBJ);
	//inet_pton(AF_INET, "192.168.13.2", &sender_ip);
	inet_ntop(AF_INET, &temp->src_ip, sender_ip, IP_ADDRLEN);
	inet_ntop(AF_INET, &temp->dst_ip, receiver_ip, IP_ADDRLEN);

	printf(" ip is %s %s\n", sender_ip,receiver_ip);
} 	
		
int main() {

    char buffer[512];
    struct sockaddr_in sender_addr;
    socklen_t addr_len = sizeof(sender_addr);

    struct sockaddr_in dest_addr;
    struct sockaddr_in addr;

    char sender_ip[16];
    char receiver_ip[16];

    sock = socket(AF_INET, SOCK_RAW, RSVP_PROTOCOL);
    if (sock < 0) {
        perror("Socket creation failed");
        return 1;
    }

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
	perror("binding failed");
	close(sock);
	exit(EXIT_FAILURE);
    }

    // only in PE1 or PE2 where we configure the tunnel for RSVP.
    // ------------------------------------------------------
	
    //ip = (struct src_dst_ip *)malloc(sizeof(struct src_dst_ip ));
    printf("Enter src ip : \n");
    fgets(srcip, 16, stdin);
	
    printf("Enter dst ip: \n");
    fgets(dstip, 16, stdin);

    int len = strlen(srcip);
    if(srcip[len-1] == '\n') 
	srcip[len-1] = '\0';

    strlen(dstip);
    if(dstip[len-1] == '\n')
        dstip[len-1] = '\0';
    strcpy(sender_ip, srcip);
    strcpy(receiver_ip, dstip);
    
    if(resv_head == NULL) {
	resv_head = insert_session(resv_head, sender_ip, receiver_ip);
    } else {
        insert_session(resv_head, sender_ip, receiver_ip);
    }
    //---------------------------------------------------------

    //get_nexthop(dstip, nhip);
    //printf("next hop -> %s\n", nhip);

    path_event_handler(); //send path msg
	 
    while(1) {
   	memset(buffer, 0, sizeof(buffer));
	int bytes_received = recvfrom(sock, buffer, sizeof(buffer), 0,
       				(struct sockaddr*)&sender_addr, &addr_len);
       	if (bytes_received < 0) {
	        perror("Receive failed");
       		continue;
	}

       	struct rsvp_header *rsvp = (struct rsvp_header*)(buffer+20);

	switch(rsvp->msg_type) {

		case PATH_MSG_TYPE:

                        // get ip from the received path packet
                        get_ip(buffer, sender_ip, receiver_ip);
                        //Receive PATH Message
                        printf("insert_path_session\n");
                        if(path_head == NULL) {
                                path_head = insert_session(path_head, sender_ip, receiver_ip);
                        } else {
                                insert_session(path_head, sender_ip, receiver_ip);
                        }

                        receive_path_message(sock,buffer,sender_addr);

                        break;

                case RESV_MSG_TYPE:
                        //get ip from the received resv msg
                        get_ip(buffer, sender_ip, receiver_ip);
                        // Receive RSVP-TE RESV Message
                        printf("insert_resv_session\n");
                        if(resv_head == NULL) {
                                resv_head = insert_session(resv_head, sender_ip, receiver_ip);
                        } else {
                                insert_session(resv_head, sender_ip, receiver_ip);
                        }

                        receive_resv_message(sock,buffer,sender_addr);
                        break;
	}
    }
    
    close(sock);
    return 0;
}

