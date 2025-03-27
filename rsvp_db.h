#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include"avl_tree.h"

#define PACKET_SIZE 256

struct session {
        char sender[16];
        char receiver[16];
        time_t last_path_time;
        uint8_t tunnel_id;
        struct session *next;
};

/* Define path_msg structure */
typedef struct path_msg {
    uint8_t tunnel_id;
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
    uint8_t tunnel_id;
    struct in_addr src_ip;
    struct in_addr dest_ip;
    struct in_addr next_hop_ip;
    uint8_t IFH;
    uint8_t time_interval;
} resv_msg;

int compare_path(void*, void*);
int compare_resv(void*, void*);
void free_path(void*);
void free_resv(void*);
path_msg* create_path(int, char*, char*, char*, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, char*);
resv_msg* create_resv(int, char*, char*, char*, uint8_t, uint8_t);
avl_node* fill_resv_tree();
avl_node* fill_path_tree();
avl_node* insert_path_node(uint8_t, char[], char[]);
avl_node* insert_resv_node(uint8_t, char[], char[]);
struct session* insert_session(struct session* , char[], char[]);
struct session* delete_session(struct session* , char[], char[]);

