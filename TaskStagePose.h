#pragma once

#include "TaskStage.h"

namespace HG {

class TaskStagePose : public TaskStage {
public:
    TaskStagePose();
    TaskStagePose(const STProInfo& stProInfo);
    virtual ~TaskStagePose();

    virtual int start() override;
    virtual void stop() override;
    
private:

};

} // namespace HG
