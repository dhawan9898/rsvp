#include "rsvp_db.h"
#include "rsvp_msg.h"
#include "timer_event.h"

struct session* sess = NULL;
struct session* head = NULL;
time_t now = 0;

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

void fill_resv_tree(avl_node *resv_tree) {
    /* Insert test data */
    resv_tree = insert_node(resv_tree, create_resv(100, "192.168.1.1", "192.168.1.2", "192.168.1.3", 5, 10), compare_resv);
    resv_tree = insert_node(resv_tree, create_resv(50, "192.168.2.1", "192.168.2.2", "192.168.2.3", 3, 20), compare_resv);
    resv_tree = insert_node(resv_tree, create_resv(150, "192.168.3.1", "192.168.3.2", "192.168.3.3", 7, 15), compare_resv);
    resv_tree = insert_node(resv_tree, create_resv(75, "192.168.4.1", "192.168.4.2", "192.168.4.3", 2, 25), compare_resv);
    resv_tree = insert_node(resv_tree, create_resv(125, "192.168.5.1", "192.168.5.2", "192.168.5.3", 8, 12), compare_resv);
}

void fill_path_tree(avl_node *path_tree) {
    /* Insert test data */
    path_tree = insert_node(path_tree, create_path(100, "192.168.1.1", "192.168.1.2", "192.168.1.3", 5, 10, 2, 3, 0, "Path A"), compare_path);
    path_tree = insert_node(path_tree, create_path(50, "192.168.2.1", "192.168.2.2", "192.168.2.3", 3, 20, 1, 4, 1, "Path B"), compare_path);
    path_tree = insert_node(path_tree, create_path(150, "192.168.3.1", "192.168.3.2", "192.168.3.3", 7, 15, 2, 2, 0, "Path C"), compare_path);
    path_tree = insert_node(path_tree, create_path(75, "192.168.4.1", "192.168.4.2", "192.168.4.3", 2, 25, 3, 1, 1, "Path D"), compare_path);
    path_tree = insert_node(path_tree, create_path(125, "192.168.5.1", "192.168.5.2", "192.168.5.3", 8, 12, 2, 3, 0, "Path E"), compare_path);
}

void fetch_resv_data(int tunnel_id, 
        avl_node *resv_tree, 
        char* nhip,
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
        char *nhip,
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

struct session* insert_session(struct session* sess, char sender[], char receiver[]) {
        now = time(NULL);
        printf("insert session\n");
        if(sess == NULL) {
                struct session *temp = (struct session*)malloc(sizeof(struct session));
                if(temp < 0)
                         printf("cannot allocate dynamic memory]n");

                temp->last_path_time = now;
                strcpy(temp->sender, sender);
                strcpy(temp->receiver, receiver);
                temp->next = NULL;
                return temp;
        } else {
		struct session *local = NULL;
                while(sess != NULL) {
                        if((strcmp(sess->sender, sender) == 0) &&
                           (strcmp(sess->receiver, receiver) == 0)) {
				sess->last_path_time = now;
                                return;
                        }
			local = sess;
                        sess=sess->next;
                }

                struct session *temp = (struct session*)malloc(sizeof(struct session));
                if(sess < 0)
                         printf("cannot allocate dynamic memory\n");

                temp->last_path_time = now;
                strcpy(temp->sender, sender);
                strcpy(temp->receiver, receiver);
                temp->next = NULL;

                local->next = temp;
        }
}


struct session* delete_session(struct session* sess, char sender[], char receiver[]) {

        struct session *temp = NULL;
	struct session *head = sess;

        printf("delete session\n");
        while(sess != NULL) {
                if((head == sess) &&
                   (strcmp(sess->sender, sender) == 0) &&
                   (strcmp(sess->receiver, receiver) == 0)) {
                        temp = head;
                        head = head->next;
                        free(temp);
                        return head;
                } else {
                        if((strcmp(sess->sender, sender) == 0) &&
                           (strcmp(sess->receiver, receiver) == 0)) {
				temp = sess->next;
                                *sess = *sess->next;
                                free(temp);
                        }else{
                                sess = sess->next;
                        }
                }
        }
}

