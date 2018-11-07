#include "module_doppler_smooth_csi_mulpal.h"

Mat<cx_double> module_doppler_smooth_csi_mulpal(Mat<cx_double> &csi)
{
    Col<cx_double> meanval = mean(csi, 1);
    for (int i = 0; i < csi.n_rows; i++) {
//        for (int j = 0; j < csi.n_cols; j++) {
//            csi.at(i, j) = csi.at(i, j) - meanval.at(i);
//        }
        int index;
        std::cin >> index;
        std::cout << csi.row(i);
    }
    return strans(csi);
}
