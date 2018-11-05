#include <signal.h>
#include "csi_packet.h"
#include "csireader.h"

static CSIReader *reader;

void sigint_handler(int signo)
{
    if (reader != nullptr) {
        reader->~CSIReader();
    }
    exit(-signo);
}

int main()
{
    csi_packet packet;
    reader = new CSIReader();

    signal(SIGINT, sigint_handler);
    signal(SIGTERM, sigint_handler);
    signal(SIGSEGV, sigint_handler);

    while (true) {
        if (!reader->get_csi_from_shm(&packet, 1)) {
            break;
        }
        reader->print_csi_packet(&packet);
    }
    return 0;
}
