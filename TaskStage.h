#pragma once

#include <string>

namespace HG {

struct STProInfo {
    std::string sImagePath;
    std::string sWorkPath;
};

class TaskStage {
public:
    TaskStage() {}
    TaskStage(const STProInfo& stProInfo) {
        m_stProInfo = stProInfo;
    }
    virtual ~TaskStage() {}

    virtual int start() { return EXIT_SUCCESS; }
    virtual void stop() {}

    const std::string& getImagePath() { return m_stProInfo.sImagePath; } 
    const std::string& getWorkPath() { return m_stProInfo.sWorkPath; } 

protected:
    STProInfo m_stProInfo;
    const std::string m_sDatabase = "FeatureSaver";

private:
};

} // namespace HG
