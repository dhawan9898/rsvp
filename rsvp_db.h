#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include"avl_tree.h"

#define PACKET_SIZE 256

struct session {
        char sender[16];
        char receiver[16];
        time_t last_path_time;
        struct session *next;
};

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

path_msg* create_path(int, char*, char*, char*, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, char*);
resv_msg* create_resv(int, char*, char*, char*, uint8_t, uint8_t);
void fill_resv_tree(avl_node*);
void fill_path_tree(avl_node*);
struct session* insert_session(struct session* , char[], char[]);
struct session* delete_session(struct session* , char[], char[]);

