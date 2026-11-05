#pragma once

#include "TaskStage.h"

namespace HG {

class TaskStageFeature : public TaskStage {
public:
    TaskStageFeature();
    TaskStageFeature(const STProInfo& stProInfo);
    virtual ~TaskStageFeature();

    virtual int start() override;
    virtual void stop() override;

private:
};

} // namespace HG