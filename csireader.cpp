#include "csireader.h"
#include <sys/sem.h>
#include <sys/ipc.h>
#include <errno.h>
#include <strings.h>
#include <sys/shm.h>
#include <stdio.h>

CSIReader::CSIReader()
{
    index = 0;
    semid = -1;
    shmid = -1;
    csi_array = (csi_packet*)-1;
    init_shm_get();
}

CSIReader::~CSIReader()
{
    if (csi_array != (void*)-1) {
        shmdt(csi_array);
    }
}

bool CSIReader::init_shm_get()
{
    // converting a global token to XSI keys
    key_t key_sem = ftok(SHM_SEM_TOKEN, 0);
    key_t key_csi = ftok(SHM_CSI_TOKEN, 0);
    if (key_sem == -1 || key_csi == -1) {
        fprintf(stderr, "Error converting shm keys: %s\n", strerror(errno));
        return false;
    }

sem_retry:
    // referencing to an existing semaphore set
    if ((semid = semget(key_sem, 0, SHM_MODE)) < 0) {
        fprintf(stderr, "Error referencing to the semaphore set: %s\n", strerror(errno));
        if (errno == ENOENT || errno == EINTR) {
            sleep(3);
            goto sem_retry;
        }
        return false;
    }
    fprintf(stderr, "XSI semaphore set referenced\n");

shm_retry:
    // referencing to an existing shared memory segment
    if ((shmid = shmget(key_csi, 0, SHM_MODE)) < 0) {
        fprintf(stderr, "Error referencing to the shared memory: %s\n", strerror(errno));
        if (errno == ENOENT || errno == EINTR) {
            sleep(3);
            goto shm_retry;
        }
        return false;
    }

    csi_array = (csi_packet*)shmat(shmid, nullptr, 0);
    if (csi_array == (void*)-1) {
        fprintf(stderr, "Error attaching to shared meory segments: %s\n", strerror(errno));
        return false;
    }

    struct shmid_ds ds;
    if (shmctl(shmid, IPC_STAT, &ds) < 0) {
        fprintf(stderr, "Error retrieving shared memory struct: %s\n", strerror(errno));
        return false;
    }
    slots = ds.shm_segsz / sizeof(csi_packet);
    fprintf(stderr, "XSI shared memory referenced\n");
    return true;
}

bool CSIReader::get_single_csi(csi_packet *packet)
{
    struct sembuf op;
    op.sem_num =index;
    op.sem_op = -1;
    op.sem_flg = 0;

    // waiting semaphore for greater than 0, decrement it by 1
    if (semop(semid, &op, 1) < 0) {
        fprintf(stderr, "get_csi_from_shm: %s\n", strerror(errno));
        return false;
    }

    // write csi to local buffer
    memcpy(packet, csi_array+index, sizeof(csi_packet));

    // decrement it by 1 again, let the semaphore to become 0
    if (semop(semid, &op, 1) < 0) {
        fprintf(stderr, "get_csi_from_shm: %s\n", strerror(errno));
        return false;
    }
    index = (index+1) % slots;
    return true;
}

bool CSIReader::get_csi_from_shm(csi_packet *packet, int downsampling)
{
    if (packet == nullptr) {
        fprintf(stderr, "get_all_csi: null pointer\n");
        return false;
    }
    for (int i = 0; i < downsampling; i++) {
        if (!get_single_csi(packet)) {
            return false;
        }
    }
    return true;
}

void CSIReader::print_csi_packet(csi_packet *packet)
{
    fprintf(stderr, "csi_packet:\n{\n");
    fprintf(stderr, "    timestamp: %u\n", packet->timestamp_low);
    fprintf(stderr, "    bfee_count: %u\n", packet->bfee_count);
    fprintf(stderr, "    Ntx: %u\n", packet->Ntx);
    fprintf(stderr, "    Nrx: %u\n", packet->Nrx);
    fprintf(stderr, "    rssi_a: %u\n", packet->rssi_a);
    fprintf(stderr, "    rssi_b: %u\n", packet->rssi_b);
    fprintf(stderr, "    rssi_c: %u\n", packet->rssi_c);
    fprintf(stderr, "    noise: %d\n", packet->noise);
    fprintf(stderr, "    agc: %u\n", packet->agc);
    fprintf(stderr, "    perm: [%u %u %u]\n", packet->perm[0], packet->perm[1], packet->perm[2]);
    fprintf(stderr, "    rate: %u\n", packet->rate);
    fprintf(stderr, "    csi:\n");
    Cube<double> *csiI = new Cube<double>((double*)packet->csiI, 1, 3, 30, false, false);
    Cube<double> *csiR = new Cube<double>((double*)packet->csiR, 1, 3, 30, false, false);
    Cube<cx_double> csi(*csiR, *csiI);
    std::cout << csi;
    fprintf(stderr, "}\n");
    delete csiI;
    delete csiR;
}
