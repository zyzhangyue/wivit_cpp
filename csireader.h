#ifndef CSIREADER_H
#define CSIREADER_H

#include "csi_packet.h"
/*
 * user R/W
 * group R/W
 * others R/W
 */
#define SHM_MODE 0600
#define SHM_SEM_TOKEN "/home/zhangyue/sem.shm"
#define SHM_CSI_TOKEN "/home/zhangyue/csi.shm"

class CSIReader
{
private:
    int slots, index;
    int semid, shmid;
    csi_packet *csi_array;

private:
    bool init_shm_get();
    bool get_single_csi(csi_packet*);

public:
    CSIReader();
    ~CSIReader();
    void print_csi_packet(csi_packet*);
    bool get_csi_from_shm(csi_packet*, int);
};

#endif // CSIREADER_H
