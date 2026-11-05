#pragma once

#include "TaskStage.h"

class StageProcess {
public:
    static StageProcess& getInstance() {
        static StageProcess m_pInstance;
        return m_pInstance; 
    }

    virtual ~StageProcess();

    int run(const std::string& sImagePath, const std::string& sOutputPath);

private:
    StageProcess();
};
