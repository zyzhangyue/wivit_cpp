#ifndef CSIWINDOW_H
#define CSIWINDOW_H

#include "csi_packet.h"

#define RATE 5000
#define WINDOW_SIZE 60

void csi_window_init(void);
void csi_window_exit(void);
const csi_list_node* get_csi_within_window(void);

#endif // CSIWINDOW_H
