#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include "csi_packet.h"
#include "csiwindow.h"
#include "module_modify_csi_power.h"
#include "module_doppler_smooth_csi_mulpal.h"

void signal_handler(int signo)
{
    csi_window_exit();
    _exit(-signo);
}

int main()
{
    const csi_list_node *head;
    csi_window_init();

    signal(SIGINT, signal_handler);
    signal(SIGQUIT, signal_handler);
    signal(SIGSEGV, signal_handler);
    atexit(csi_window_exit);

    while (true) {
        head = get_csi_within_window();
        if (head == nullptr) {
            std::cout << "Error" << endl;
            break;
        }
        Mat<cx_double> csi_new = module_modify_csi_Power(head);
        csi_new = module_doppler_smooth_csi_mulpal(csi_new);
        std::cout << size(csi_new);
    }
    return 0;
}
