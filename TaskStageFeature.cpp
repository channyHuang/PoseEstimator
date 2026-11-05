#include "TaskStageFeature.h"

#include <colmap/exe/feature.h>

namespace HG {

TaskStageFeature::TaskStageFeature() {}

TaskStageFeature::TaskStageFeature(const STProInfo& stProInfo) 
    : TaskStage(stProInfo) {

}

TaskStageFeature::~TaskStageFeature() {}

int TaskStageFeature::start() {
    const int nargc = 11;
    // num threads depends on memory
    std::string sArgv[nargc] = {"feature_extractor", 
                                "--database_path", getWorkPath() + "/" + m_sDatabase, 
                                "--image_path", getImagePath(),
                                "--image_list_path", "",
                                "--FeatureExtraction.use_gpu", "0",
                                "--FeatureExtraction.num_threads", "16"
                            };

    std::vector<char*> vArgv;
    vArgv.reserve(nargc);
    for (int i = 0; i < nargc; i++) {
        vArgv.push_back(const_cast<char*>(sArgv[i].c_str()));
    }
    
    try {
        if (colmap::RunFeatureExtractor(nargc, vArgv.data()) != EXIT_SUCCESS) {
            return EXIT_FAILURE;
        }
    } catch (...) {
        printf("Exception in feature \n");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

void TaskStageFeature::stop() {

}

} // namespace HG
