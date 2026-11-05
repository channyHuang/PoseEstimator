#include "StageProcess.h"

#include <vector>
#include <memory>

#include "TaskStage.h"
#include "TaskStageFeature.h"
#include "TaskStagePose.h"
#include "TaskStageAlign.h"

StageProcess::StageProcess() {}

StageProcess::~StageProcess() {}

int StageProcess::run(const std::string& sImagePath, const std::string& sOutputPath) {
    HG::STProInfo stProInfo{sImagePath, sOutputPath};

    std::vector<std::shared_ptr<HG::TaskStage>> vStages;
    vStages.push_back(std::make_shared<HG::TaskStageFeature>(stProInfo));
    vStages.push_back(std::make_shared<HG::TaskStagePose>(stProInfo));
    vStages.emplace_back(std::make_shared<HG::TaskStageAlign>(stProInfo));

    std::shared_ptr<HG::TaskStage> pCurrentStage;
    for (size_t i = 0; i < vStages.size(); ++i) {
        pCurrentStage = vStages[i];
        if (pCurrentStage->start() != EXIT_SUCCESS) {
            printf("Stage [%lu] failed\n", i);
            return EXIT_FAILURE;
        }
    }
    return EXIT_SUCCESS;
}
