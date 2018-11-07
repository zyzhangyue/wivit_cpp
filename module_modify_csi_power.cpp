#include "module_modify_csi_power.h"

static unsigned long long len = 0;
static double antenna1_I[60][30];
static double antenna1_R[60][30];
static double antenna2_I[60][30];
static double antenna2_R[60][30];

Mat<cx_double> module_modify_csi_Power(const csi_list_node* head)
{
    len = 0;
    // to store the csi matrix within a time-window into local varibles
    for (int i = 0; i < 60; i ++) {
        if (head->csi.Nrx != 0) {
            memcpy(antenna1_I[len], head->csi.csiI[0], sizeof(double)*30);
            memcpy(antenna1_R[len], head->csi.csiR[0], sizeof(double)*30);
            memcpy(antenna2_I[len], head->csi.csiI[2], sizeof(double)*30);
            memcpy(antenna2_R[len], head->csi.csiR[2], sizeof(double)*30);
            len++;
        }
        head = head->next;
    }
    Mat<double> csi1_I((double*)antenna1_I, 30, len, false, false);
    Mat<double> csi1_R((double*)antenna1_R, 30, len, false, false);
    Mat<double> csi2_I((double*)antenna2_I, 30, len, false, false);
    Mat<double> csi2_R((double*)antenna2_R, 30, len, false, false);

    // these are the csi matrix of two antennas
    Mat<cx_double> csi1(csi1_R, csi1_I);
    Mat<cx_double> csi2(csi2_R, csi2_I);

    Mat<double> amp1 = abs(csi1)+0.001;
    Mat<double> amp2 = abs(csi2)+0.001;

    double meanAmp1 = mean(mean(amp1));
    double meanAmp2 = mean(mean(amp2));

    double minAmp1 = min(min(amp1));
    double addAmp2 = (meanAmp2*minAmp1)/(meanAmp1-minAmp1);
    amp1 = (amp1 - minAmp1*1) / amp1;
    amp2 = (amp2 + addAmp2*10000) / amp2;

    csi1 = csi1 % amp1;
    csi2 = csi2 % amp2;
    return csi1 % conj(csi2);
}
