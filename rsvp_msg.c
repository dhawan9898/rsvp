#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/ip.h>
#include "rsvp_msg.h"
#include "avl_tree.h"

#define PACKET_SIZE 256
avl_node *path_tree = NULL;
avl_node *resv_tree = NULL;

char nhip[16];
extern char src_ip[16], route[16];

/* Define path_msg structure */
typedef struct path_msg {
    int tunnel_id;
    struct in_addr src_ip;
    struct in_addr dest_ip;
    struct in_addr next_hop_ip;
    uint8_t IFH;
    uint8_t time_interval;
    uint8_t setup_priority;
    uint8_t hold_priority;
    uint8_t flags;
    char name[32];
} path_msg;

/* Define resv_msg structure */
typedef struct resv_msg {
    int tunnel_id;
    struct in_addr src_ip;
    struct in_addr dest_ip;
    struct in_addr next_hop_ip;
    uint8_t IFH;
    uint8_t time_interval;
} resv_msg;

int compare_path(void* a, void* b) {
    return ((path_msg*)a)->tunnel_id - ((path_msg*)b)->tunnel_id;
}

int compare_resv(void* a, void* b) {
    return ((resv_msg*)a)->tunnel_id - ((resv_msg*)b)->tunnel_id;
}

/* Function to create a sample path_msg */
path_msg* create_path(int tunnel_id, char *src, char *dest, char *next_hop, uint8_t IFH, uint8_t time_interval, uint8_t setup_priority, uint8_t hold_priority, uint8_t flags, char *name) {
    path_msg *p = (path_msg*)malloc(sizeof(path_msg));
    if (!p) {
        printf("Memory allocation failed!\n");
        return NULL;
    }
    p->tunnel_id = tunnel_id;
    p->src_ip.s_addr = inet_addr(src);
    p->dest_ip.s_addr = inet_addr(dest);
    p->next_hop_ip.s_addr = inet_addr(next_hop);
    p->IFH = IFH;
    p->time_interval = time_interval;
    p->setup_priority = setup_priority;
    p->hold_priority = hold_priority;
    p->flags = flags;
    strncpy(p->name, name, sizeof(p->name) - 1);
    p->name[sizeof(p->name) - 1] = '\0';
    return p;
}

/* Function to create a sample path_msg */
resv_msg* create_resv(int tunnel_id, char *src, char *dest, char *next_hop, uint8_t IFH, uint8_t time_interval) {
    resv_msg *p = (resv_msg*)malloc(sizeof(resv_msg));
    if (!p) {
        printf("Memory allocation failed!\n");
        return NULL;
    }
    p->tunnel_id = tunnel_id;
    p->src_ip.s_addr = inet_addr(src);
    p->dest_ip.s_addr = inet_addr(dest);
    p->next_hop_ip.s_addr = inet_addr(next_hop);
    p->IFH = IFH;
    p->time_interval = time_interval;
    return p;
}

void fill_resv_tree() {
    /* Insert test data */
    resv_tree = insert_node(resv_tree, create_resv(100, "192.168.1.1", "192.168.1.2", "192.168.1.3", 5, 10), compare_resv);
    resv_tree = insert_node(resv_tree, create_resv(50, "192.168.2.1", "192.168.2.2", "192.168.2.3", 3, 20), compare_resv);
    resv_tree = insert_node(resv_tree, create_resv(150, "192.168.3.1", "192.168.3.2", "192.168.3.3", 7, 15), compare_resv);
    resv_tree = insert_node(resv_tree, create_resv(75, "192.168.4.1", "192.168.4.2", "192.168.4.3", 2, 25), compare_resv);
    resv_tree = insert_node(resv_tree, create_resv(125, "192.168.5.1", "192.168.5.2", "192.168.5.3", 8, 12), compare_resv);
}

void fill_path_tree() {
    /* Insert test data */
    path_tree = insert_node(path_tree, create_path(100, "192.168.1.1", "192.168.1.2", "192.168.1.3", 5, 10, 2, 3, 0, "Path A"), compare_path);
    path_tree = insert_node(path_tree, create_path(50, "192.168.2.1", "192.168.2.2", "192.168.2.3", 3, 20, 1, 4, 1, "Path B"), compare_path);
    path_tree = insert_node(path_tree, create_path(150, "192.168.3.1", "192.168.3.2", "192.168.3.3", 7, 15, 2, 2, 0, "Path C"), compare_path);
    path_tree = insert_node(path_tree, create_path(75, "192.168.4.1", "192.168.4.2", "192.168.4.3", 2, 25, 3, 1, 1, "Path D"), compare_path);
    path_tree = insert_node(path_tree, create_path(125, "192.168.5.1", "192.168.5.2", "192.168.5.3", 8, 12, 2, 3, 0, "Path E"), compare_path);
}

void fetch_resv_data(int tunnel_id, 
        avl_node *resv_tree, 
        struct sockaddr_in *dest_addr,
        struct rsvp_header *resv,
        struct session_object *session_obj, 
        struct hop_object *hop_obj,
        struct time_object *time_obj,
        struct label_object *label_obj) {

    resv_msg search_key = { .tunnel_id = tunnel_id };
    avl_node *node = search_node(resv_tree, &search_key, compare_resv);
    resv_msg *data = (resv_msg*)node->data;

    // Populate RSVP RESV header
    resv->version_flags = 0x10;  // RSVP v1
    resv->msg_type = RESV_MSG_TYPE;
    resv->length = htons(PACKET_SIZE);
    resv->checksum = 0;
    resv->ttl = 255;
    resv->reserved = 0;
    //resv->sender_ip = data->src_ip;
    //resv->receiver_ip = data->dest_ip;

    //session object for RESV msg
    session_obj->class_obj.class_num = 1;
    session_obj->class_obj.c_type = 7;
    session_obj->class_obj.length = htons(sizeof(struct session_object));
    session_obj->dst_ip = data->dest_ip;
    //inet_pton(AF_INET, receiver_ip, &session_obj->dst_ip); 
    session_obj->tunnel_id = tunnel_id;
    session_obj->src_ip = data->src_ip;

    //hop object for PATH?RESV msg
    hop_obj->class_obj.class_num = 3;
    hop_obj->class_obj.c_type = 1;
    hop_obj->class_obj.length = htons(sizeof(struct hop_object));
    get_nexthop(inet_ntoa(data->src_ip), nhip); //ip->dst_ip, ip->nhip);            
    inet_pton(AF_INET, nhip, &hop_obj->next_hop);
    //hop_obj->next_hop = data->next_hop_ip;
    hop_obj->IFH = data->IFH;

    time_obj->class_obj.class_num = 5;
    time_obj->class_obj.c_type = 1;
    time_obj->class_obj.length = htons(sizeof(struct time_object));
    time_obj->interval = data->time_interval;

    // Populate Label Object
    label_obj->class_obj.class_num = 16;  // Label class
    label_obj->class_obj.c_type = 1;  // Generic Label
    label_obj->class_obj.length = htons(sizeof(struct label_object));
    label_obj->label = htonl(1001);  // Assigned Label (1001)

    // Set destination (ingress router)
    dest_addr->sin_family = AF_INET;
    dest_addr->sin_addr = hop_obj->next_hop;
    dest_addr->sin_port = 0;
}

void fetch_path_data(int tunnel_id, 
        avl_node *path_tree, 
        struct sockaddr_in *dest_addr,
        struct rsvp_header *path,
        struct session_object *session_obj, 
        struct hop_object *hop_obj,
        struct time_object *time_obj,
        struct label_req_object *label_req_obj,
        struct session_attr_object *session_attr_obj,
        struct sender_temp_object *sender_temp_obj) {

    path_msg search_key = { .tunnel_id = tunnel_id };
    avl_node *node = search_node(path_tree, &search_key, compare_resv);
    path_msg *data = (path_msg*)node->data;

    // Populate RSVP PATH header
    path->version_flags = 0x10;  // RSVP v1
    path->msg_type = PATH_MSG_TYPE;
    path->length = htons(PACKET_SIZE);
    path->checksum = 0;
    path->ttl = 255;
    path->reserved = 0;
    //path->sender_ip = data->src_ip;
    //path->receiver_ip = data->dest_ip;

    //session object for PATH msg
    session_obj->class_obj.class_num = 1;
    session_obj->class_obj.c_type = 7;
    session_obj->class_obj.length = htons(sizeof(struct session_object));
    session_obj->dst_ip = data->dest_ip;
    //inet_pton(AF_INET, receiver_ip, &session_obj->dst_ip); 
    session_obj->tunnel_id = tunnel_id;
    session_obj->src_ip = data->src_ip;
    //inet_pton(AF_INET, sender_ip, &session_obj->src_ip);

    //hop object for PATH and RESV msg
    hop_obj->class_obj.class_num = 3;
    hop_obj->class_obj.c_type = 1;
    hop_obj->class_obj.length = htons(sizeof(struct hop_object));
    get_nexthop(inet_ntoa(data->dest_ip), nhip); //ip->dst_ip, ip->nhip);            
    inet_pton(AF_INET, nhip, &hop_obj->next_hop);
    //hop_obj->next_hop = data->next_hop_ip;
    hop_obj->IFH = data->IFH;

    time_obj->class_obj.class_num = 5;
    time_obj->class_obj.c_type = 1;
    time_obj->class_obj.length = htons(sizeof(struct time_object));
    time_obj->interval = data->time_interval;

    // Populate Label Object                                        
    label_req_obj->class_obj.class_num = 19;  // Label Request class
    label_req_obj->class_obj.c_type = 1;  // Generic Label                   
    label_req_obj->class_obj.length = htons(sizeof(struct label_req_object));
    label_req_obj->L3PID = htonl(0x0800);  // Assigned Label (1001)

    //session attribute object for PATH msg
    session_attr_obj->class_obj.class_num = 207;
    session_attr_obj->class_obj.c_type = 1;
    session_attr_obj->class_obj.length = htons(sizeof(struct session_attr_object));
    session_attr_obj->setup_prio = data->setup_priority;
    session_attr_obj->hold_prio = data->hold_priority;
    session_attr_obj->flags = data->flags;
    session_attr_obj->name_len = sizeof(data->name);
    //strcpy("PE1", session_attr_obj->Name);

    //Sender template object for PATH msg
    sender_temp_obj->class_obj.class_num = 11;
    sender_temp_obj->class_obj.c_type = 7;
    sender_temp_obj->class_obj.length = htons(sizeof(struct sender_temp_object));    
    //inet_pton(AF_INET, sender_ip, &sender_temp_obj->src_ip);
    sender_temp_obj->src_ip = data->src_ip;
    sender_temp_obj->Reserved = 0;
    sender_temp_obj->LSP_ID = tunnel_id;

    // Set destination (egress router)
    dest_addr->sin_family = AF_INET;
    dest_addr->sin_addr = hop_obj->next_hop;
    dest_addr->sin_port = 0;
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
    fetch_resv_data(100, resv_tree, &dest_addr, resv, session_obj, hop_obj, time_obj, label_obj);

    // Send RESV message
    if (sendto(sock, resv_packet, sizeof(resv_packet), 0, 
                (struct sockaddr*)&dest_addr, sizeof(dest_addr)) < 0) {
        perror("Send failed");
    } else {
        printf("Sent RESV message to %s with Label 1001\n", inet_ntoa(sender_ip));
    }
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
    char path_packet[256];

    struct rsvp_header *path = (struct rsvp_header*)path_packet;
    //struct class_obj *class_obj = (struct class_obj*)(path_packet + START_SENT_CLASS_OBJ); 
    struct session_object *session_obj = (struct session_object*)(path_packet + START_SENT_SESSION_OBJ);
    struct hop_object *hop_obj = (struct hop_object*)(path_packet + START_SENT_HOP_OBJ);
    struct time_object *time_obj = (struct time_object*)(path_packet + START_SENT_TIME_OBJ);
    struct label_req_object *label_req_obj = (struct label_req_object*)(path_packet + START_SENT_LABEL_REQ); 
    struct session_attr_object *session_attr_obj = (struct session_attr_object*)(path_packet + START_SENT_SESSION_ATTR_OBJ); 
    struct sender_temp_object *sender_temp_obj = (struct sender_temp_object*)(path_packet + START_SENT_SENDER_TEMP_OBJ);

    fill_path_tree(path_tree);
    fetch_path_data(100, path_tree, &dest_addr, path, session_obj, hop_obj, time_obj, label_req_obj, session_attr_obj, sender_temp_obj);

    printf(" sending message1 = %d\n",sock);
    // Send PATH message
    if (sendto(sock, path_packet, sizeof(path_packet), 0, 
                (struct sockaddr*)&dest_addr, sizeof(dest_addr)) < 0) {
        perror("Send failed");
    } else {
        printf("Sent PATH message to %s\n", inet_ntoa(receiver_ip));
    }
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

