#include "csiwindow.h"
#include "csireader.h"
#include <math.h>
#include <stdlib.h>

#define POOL_INIT 0
#define POOL_GET1 1
#define POOL_PUT1 2
#define gap(time2, time1) round((time2-time1)/RATE)

static int len;
static double curTime;
static CSIReader *reader;
static csi_list_node *head, *tail;


// this is the csi_list_node pool
// aim to reduce the overhead when we use 'new' and 'delete'
static
csi_list_node* pool_catchall(int op, csi_list_node* ptr = nullptr)
{
    static csi_list_node *head = nullptr, *tmp = nullptr;
    switch (op) {
    case POOL_INIT:
        head = (csi_list_node*)calloc(2*WINDOW_SIZE, sizeof(csi_list_node));
        for (int i = 0;i < 2*WINDOW_SIZE-1; i++) {
            head[i].next = &head[i+1];
        }
        return nullptr;
    case POOL_GET1:
        tmp = head;
        head = head->next;
        return tmp;
    case POOL_PUT1:
        memset(ptr, 0, sizeof(csi_list_node));
        ptr->next = head;
        head = ptr;
        return nullptr;
    }
    return nullptr;
}

static
void init_list(void)
{
    // init list head
    csi_list_node *tmp = pool_catchall(POOL_GET1);
    tmp->next = nullptr;
    reader->get_csi_from_shm(&tmp->csi, 1);
    head = tail = tmp;
}

void csi_window_init(void)
{
    len = 1;
    reader = new CSIReader();
    pool_catchall(POOL_INIT);
    init_list();
}

void csi_window_exit(void)
{
    if (reader != nullptr) {
        reader->~CSIReader();
        reader = nullptr;
    }
}

const csi_list_node* get_csi_within_window(void)
{
    csi_list_node *node = nullptr;

    // discard the first 10 csi packets and update startTime
    if (len >= WINDOW_SIZE) {
        for (int i = 0; i < 10; i ++) {
            node = head;
            head = head->next;
            pool_catchall(POOL_PUT1, node);
        }
        len -= 10;
    }

    // read a window-size of csi packets, add them to the list
    while (true) {
        node = pool_catchall(POOL_GET1);
        node->next = nullptr;

        if (!reader->get_csi_from_shm(&node->csi, 1))
            return nullptr;
        curTime = node->csi.timestamp_low;

        // deal with 2^32 overflow
        if (curTime < tail->csi.timestamp_low) {
            csi_list_node *tmp = head;
            while (tmp != nullptr) {
                tmp->csi.timestamp_low -= UINT32_MAX;
                tmp = tmp->next;
            }
        }

        // determine how many packets missed
        int mgap = gap(curTime, tail->csi.timestamp_low);

        // add the missed packets, init them with zeros
        for (int i = 0; i < mgap-1; i ++) {
            csi_list_node *tmp = pool_catchall(POOL_GET1);
            memset(tmp, 0, sizeof(csi_list_node));
            tail->next = tmp;
            tail = tmp;
        }

        // add the last one to the list
        tail->next = node;
        tail = node;
        len += mgap;

        // window-size packets received, exit
        if (len >= WINDOW_SIZE)
            break;
    }
    return head;
}
