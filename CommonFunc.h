#pragma once

#include <fstream>
#include <optional>
#include <regex>

#include <Eigen/Eigen>

namespace HG {
inline std::optional<Eigen::Vector3d> readGPS(const std::string& sWorkPath) {
    std::ifstream ifs(sWorkPath + "/metadata.xml");
    if (ifs.is_open()) {
        std::string sLine;
        std::regex pattern(R"(^<SRS>ENU:\s*([+-]?\d*\.?\d+)\s*[, ]\s*([+-]?\d*\.?\d+)\s*[, ]\s*([+-]?\d*\.?\d+))");
        while (std::getline(ifs, sLine)) {
            std::smatch matches;
            if (std::regex_search(sLine, matches, pattern)) {
                if (matches.size() == 4) {  
                    Eigen::Vector3d vGPS;
                    try {
                        vGPS[0] = std::stod(matches[1].str());
                        vGPS[1] = std::stod(matches[2].str());
                        vGPS[2] = std::stod(matches[3].str());
                    } catch (const std::exception& e) {
                        printf("error: %s\n", e.what() );
                        return std::nullopt;
                    }
                    ifs.close();
                    return vGPS;
                }
            }
        }
        ifs.close();
    }
    return std::nullopt;
}
}