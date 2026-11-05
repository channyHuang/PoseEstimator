
#include "TaskStagePose.h"

#include <vector>

#include <colmap/exe/feature.h>
#include <colmap/exe/sfm.h>

namespace HG {

TaskStagePose::TaskStagePose() {}

TaskStagePose::TaskStagePose(const STProInfo& stProInfo) 
    : TaskStage(stProInfo) {

}

TaskStagePose::~TaskStagePose() {}

int TaskStagePose::start() {
    std::vector<char*> vArgv;
    if (false) {
        const int nargc = 5;
        std::string sArgv[nargc] = {"spatial_matcher",
                                    "--database_path", getWorkPath() + "/FeatureSaver",
                                    "--FeatureMatching.use_gpu", "0"
        };

        vArgv.reserve(nargc);
        for (int i = 0; i < nargc; i++) {
            vArgv.push_back(const_cast<char*>(sArgv[i].c_str()));
        }

        try {
            if (colmap::RunSpatialMatcher(nargc, vArgv.data()) != EXIT_SUCCESS) {
                return EXIT_FAILURE;
            }
        } catch (...) {
            printf("Exception in matcher \n");
            return EXIT_FAILURE;
        }
    }
    {
        const int nargc = 7;
        std::string sArgv[nargc] = {"mapper",
                                    "--database_path", getWorkPath() + "/FeatureSaver",
                                    "--image_path", getImagePath(),
                                    "--output_path", getWorkPath() + "/cameraPose"
        };

        vArgv.reserve(nargc);
        for (int i = 0; i < nargc; i++) {
            vArgv.push_back(const_cast<char*>(sArgv[i].c_str()));
        }

        try {
            if (colmap::RunMapper(nargc, vArgv.data()) != EXIT_SUCCESS) {
                return EXIT_FAILURE;
            }
        } catch (...) {
            printf("Exception in mapper \n");
            return EXIT_FAILURE;
        }
    }
    return EXIT_SUCCESS;
}

void TaskStagePose::stop() {}

}
