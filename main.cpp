#include "StageProcess.h"

// ./PoseEstimator "/home/channy/Documents/datasets/20260728160521949" "/home/channy/Documents/datasets/data_imagecollection_1023"
int main(int argc, char** argv) {
    if (argc != 3) {
        printf("Usage:\n [App] [image_path] [result_path]\n");
        return 0;
    }
    StageProcess::getInstance().run(argv[1], argv[2] );

    return 0;
}
