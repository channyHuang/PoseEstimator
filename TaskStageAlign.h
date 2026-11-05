#pragma once

#include "TaskStage.h"

namespace HG {

class TaskStageAlign : public TaskStage {
public:
    TaskStageAlign();
    TaskStageAlign(const STProInfo& STProInfo);
    virtual ~TaskStageAlign();

    virtual int start() override;
    virtual void stop() override;

private:
};

} // namespace HG
