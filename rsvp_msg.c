#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/ip.h>
#include "rsvp_msg.h"
#include "rsvp_db.h"

avl_node *path_tree = NULL;
avl_node *resv_tree = NULL;
resv_msg *r = NULL;
path_msg *p = NULL;

char nhip[16];
extern char src_ip[16], route[16];

void process_path(void *data, int sock, struct in_addr sender_ip, struct in_addr receiver_ip,
                  struct sockaddr_in *dest_addr,
                  char path_packet[PACKET_SIZE],
                  struct rsvp_header *path,
                  struct session_object *session_obj,
                  struct hop_object *hop_obj,
                  struct time_object *time_obj,
                  struct label_req_objecct *label_req_obj,
                  struct session_attr_object *session_attr_obj,
                  struct sender_temp_objeect *sender_temp_obj) {
    p = (path_msg*)data;
    fetch_path_data(p->tunnel_id, path_tree, nhip, &dest_addr, path, session_obj, hop_obj, time_obj, label_req_obj, session_attr_obj, sender_temp_obj);

    printf(" sending message1 = %d\n",sock);
    // Send PATH message
    if (sendto(sock, path_packet, sizeof(path_packet), 0, 
                (struct sockaddr*)&dest_addr, sizeof(dest_addr)) < 0) {
        perror("Send failed");
    } else {
        printf("Sent PATH message to %s\n", inet_ntoa(receiver_ip));
    }
}

void process_resv(void *data, int sock, struct in_addr sender_ip, struct in_addr receiver_ip,
                  struct sockaddr_in *dest_addr,
                  char resv_packet[PACKET_SIZE],
                  struct rsvp_header *resv,
                  struct session_object *session_obj,
                  struct hop_object *hop_obj,
                  struct time_object *time_obj,
                  struct label_object *label_obj) {
    r = (resv_msg*)data;
    fetch_resv_data(r->tunnel_id, resv_tree, nhip, &dest_addr, resv, session_obj, hop_obj, time_obj, label_obj);

    // Send RESV message
    if (sendto(sock, resv_packet, sizeof(resv_packet), 0, 
                (struct sockaddr*)&dest_addr, sizeof(dest_addr)) < 0) {
        perror("Send failed");
    } else {
        printf("Sent RESV message to %s with Label 1001\n", inet_ntoa(sender_ip));
    }
}

// Function to send an RSVP-TE RESV message with label assignment
void send_resv_message(int sock, struct in_addr sender_ip, struct in_addr receiver_ip) {
    struct sockaddr_in dest_addr;
    char resv_packet[PACKET_SIZE];

    struct rsvp_header *resv = (struct rsvp_header*)resv_packet;
    //    struct class_obj *class_obj = (struct class_obj*)(resv_packet + sizeof(struct rsvp_header));
    struct session_object *session_obj = (struct session_object*)(resv_packet + START_SENT_SESSION_OBJ);
    struct hop_object *hop_obj = (struct hop_object*)(resv_packet + START_SENT_HOP_OBJ);
    struct time_object *time_obj = (struct time_object*)(resv_packet + START_SENT_TIME_OBJ);
    struct label_object *label_obj = (struct label_object*)(resv_packet + START_SENT_LABEL);

    fill_resv_tree(resv_tree);
    traverse_avl_tree(resv_tree, process_resv);
}

void get_path_class_obj(int class_obj_arr[]) {
    printf("getting calss obj arr\n");
    class_obj_arr[0] = START_RECV_SESSION_OBJ;
    class_obj_arr[1] = START_RECV_HOP_OBJ;
    class_obj_arr[2] = START_RECV_TIME_OBJ;
    class_obj_arr[3] = START_RECV_LABEL_REQ;
    class_obj_arr[4] = START_RECV_SESSION_ATTR_OBJ;
    class_obj_arr[5] = START_RECV_SENDER_TEMP_OBJ; 	
}

// Function to receive RSVP-TE PATH messages
void receive_path_message(int sock, char buffer[], struct sockaddr_in sender_addr) {
    //char buffer[1024];
    struct class_obj *class_obj;
    int class_obj_arr[10]; 
    int i = 0;
    printf("Listening for RSVP-TE PATH messages...\n");

    struct rsvp_header *rsvp = (struct rsvp_header*)(buffer+20);
    printf("Received PATH message from %s\n", inet_ntoa(sender_addr.sin_addr));

    struct session_object *temp = (struct session_object*)(buffer+START_RECV_SESSION_OBJ);
    printf(" %s   %s\n", inet_ntoa(temp->src_ip), inet_ntoa(temp->dst_ip));   
      
    struct in_addr sender_ip = temp->src_ip; //rsvp->sender_ip;
    struct in_addr receiver_ip = temp->dst_ip; //rsvp->receiver_ip;

    memset(class_obj_arr, 0, sizeof(class_obj_arr));
    get_path_class_obj(class_obj_arr);

	get_nexthop(inet_ntoa(receiver_ip), nhip);
	if(strcmp(nhip, " ") == 0) {
		printf("reached the destiantion end os rsvp tunnel\n");
		send_resv_message(sock, sender_ip, receiver_ip);
	} else {
		printf("send path msg nexthop is %s destination not reached\n", nhip);
		send_path_message(sock, sender_ip, receiver_ip);
	}
/*
    printf("route = %s, nhip = %s, srcip = %s\n", route,inet_ntoa(receiver_ip),src_ip);
    if(strcmp(inet_ntoa(receiver_ip),src_ip) == 0)
    		printf("====== destination reached\n");
    if(dst_reached()) { //chk we have reached the dst or not
                          // update path table, resv table, chk lael req, geneate label and send Resv msg.
                          send_resv_message()
                          } else {
    // update nexthop, path table and send Path msg
    send_path_message();
    }

    while(class_obj_arr[i] != 0) {
        class_obj = (struct class_obj*) (buffer + class_obj_arr[i]);
        switch(class_obj->class_num) {
            case SESSION:
                printf("session obj %d\n",class_obj->class_num);
                break;
            case HOP:
                printf("hoP obj %d\n",class_obj->class_num);
                break;
            case TIME:
                printf("time obj %d\n",class_obj->class_num);
                break;
            case LABEL_REQUEST: 
                // Send a RESV message in response
                send_resv_message(sock, sender_ip, receiver_ip);
                break;
            case SESSION_ATTRIBUTE:
                printf("session attr obj %d\n",class_obj->class_num);
                break;
            case SENDER_TEMPLATE:
                printf("sender temp obj %d\n",class_obj->class_num);
                break;
        }
        i++;
    }*/
}




//Function to send PATH message for label request
void send_path_message(int sock, struct in_addr sender_ip, struct in_addr receiver_ip) {
    struct sockaddr_in dest_addr;
    char path_packet[PACKET_SIZE];

    struct rsvp_header *path = (struct rsvp_header*)path_packet;
    //struct class_obj *class_obj = (struct class_obj*)(path_packet + START_SENT_CLASS_OBJ); 
    struct session_object *session_obj = (struct session_object*)(path_packet + START_SENT_SESSION_OBJ);
    struct hop_object *hop_obj = (struct hop_object*)(path_packet + START_SENT_HOP_OBJ);
    struct time_object *time_obj = (struct time_object*)(path_packet + START_SENT_TIME_OBJ);
    struct label_req_object *label_req_obj = (struct label_req_object*)(path_packet + START_SENT_LABEL_REQ); 
    struct session_attr_object *session_attr_obj = (struct session_attr_object*)(path_packet + START_SENT_SESSION_ATTR_OBJ); 
    struct sender_temp_object *sender_temp_obj = (struct sender_temp_object*)(path_packet + START_SENT_SENDER_TEMP_OBJ);

    fill_path_tree(path_tree);
    traverse_avl_tree(path_tree, process_path);
}

void get_resv_class_obj(int class_obj_arr[]) {
    printf("getting calss obj arr\n");
    class_obj_arr[0] = START_RECV_SESSION_OBJ;
    class_obj_arr[1] = START_RECV_HOP_OBJ;
    class_obj_arr[2] = START_RECV_TIME_OBJ;
    class_obj_arr[3] = START_RECV_FILTER_SPEC_OBJ;
    class_obj_arr[4] = START_RECV_LABEL;
}


// Function to receive an RSVP-TE RESV message
void receive_resv_message(int sock, char buffer[], struct sockaddr_in sender_addr) {

    struct class_obj *class_obj;
    int class_obj_arr[10]; 
    int i = 0;

    printf("Listening for RSVP-TE RESV messages...\n");

    memset(class_obj_arr, 0, sizeof(class_obj_arr));
    get_resv_class_obj(class_obj_arr);
    struct label_object *label_obj;

    /*if(dst_reached()) { //chk we have reached the dst or not
             // storE label in resv table
    } else {
             // get the nexthop, storE label n resv table and generate & send label 
             send_resv_message
    }*/
 
    while(class_obj_arr[i] != 0) {
        class_obj = (struct class_obj*) (buffer + class_obj_arr[i]);
        switch(class_obj->class_num) {

            case SESSION:
                break;
            case HOP:
                break;
            case TIME:
                break;	
            case FILTER_SPEC:
                break;
            case RSVP_LABEL: 
                label_obj = (struct label_object*)(buffer + START_RECV_LABEL);
                printf("Received RESV message from %s with Label %d\n", 
                        inet_ntoa(sender_addr.sin_addr), ntohl(label_obj->label));	
                break;
        }
        i++;
    } 
}

