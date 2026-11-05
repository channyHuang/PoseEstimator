#include "TaskStageAlign.h"

#include <vector>
#include <filesystem>
#include <fstream>
#include <unordered_set>

#include <Eigen/Eigen>

#include <colmap/exe/model.h>
#include <colmap/scene/reconstruction.h>

#include "CommonFunc.h"
#include "Gis.h"

namespace HG {

TaskStageAlign::TaskStageAlign() {}

TaskStageAlign::TaskStageAlign(const STProInfo& stProInfo) 
    : TaskStage(stProInfo) {

}

TaskStageAlign::~TaskStageAlign() {}

struct STSubModel {
    std::filesystem::path sPath;
    Eigen::Vector3d vGPS;
};

// assert sub models in same database
int mergeSubModel(std::string sPath) {
    if (!std::filesystem::is_directory(sPath)) return EXIT_FAILURE;

    // sub models
    std::vector<STSubModel> vSubParts;
    for (const auto& entry : std::filesystem::directory_iterator(sPath)) {
        if (entry.is_directory()) {
            std::optional<Eigen::Vector3d> vGPS = HG::readGPS(entry.path().string());
            if (vGPS.has_value())
                vSubParts.push_back({entry.path(), vGPS.value()});
        }
    }
    if (vSubParts.empty()) return EXIT_FAILURE;
    std::sort(vSubParts.begin(), vSubParts.end(),
            [](const STSubModel& a, const STSubModel& b) {
                  return a.sPath.filename().string() < b.sPath.filename().string();
              });

    // select origin
    const STSubModel& stRef = vSubParts.front();
    std::filesystem::copy(stRef.sPath / "metadata.xml", sPath, std::filesystem::copy_options::overwrite_existing);
    HG::GISTranslate cGISTranslate(stRef.vGPS[1], stRef.vGPS[0], stRef.vGPS[2]);

    // merge
    colmap::Reconstruction stMergedRecon;
    for (const auto& stSubModel : vSubParts) {
        colmap::Reconstruction stSubRecon;
        stSubRecon.Read(stSubModel.sPath.string());
        // compute offset
        Eigen::Vector3d vOffset = cGISTranslate.gps2enuModelCoord(stSubModel.vGPS[1], stSubModel.vGPS[0], stSubModel.vGPS[2]);

        // camera_id, unique in same database
        for (const auto& [nIdCam, stCam] : stSubRecon.Cameras()) {
            if (stMergedRecon.ExistsCamera(nIdCam)) continue;

            colmap::Camera stCamNew(nIdCam,
                                   stCam.model_id,
                                   stCam.width,
                                   stCam.height,
                                   stCam.params);
            stMergedRecon.AddCameraWithTrivialRig(stCamNew);
        }

        // image_id, unique in same database
        std::unordered_set<colmap::image_t> setImageIdsCommon;
        std::unordered_set<colmap::image_t> setImageIdsMiss;
        for (const auto& [nIdImage, stImg] : stSubRecon.Images()) {
            if (stMergedRecon.ExistsImage(nIdImage)) {
                const std::string& sNameSrc = stImg.Name();
                const std::string& sNameDst = stMergedRecon.Image(nIdImage).Name();
                if (sNameDst != sNameSrc) {
                    return EXIT_FAILURE;
                }
                setImageIdsCommon.insert(nIdImage);
            } else {
                setImageIdsMiss.insert(nIdImage);
            }
        }

        for (const auto nIdImage : setImageIdsMiss) {
            const colmap::Image& img = stSubRecon.Image(nIdImage);
            colmap::Image new_img = img;

            new_img.ResetCameraPtr();
            
            new_img.ResetFramePtr();
            new_img.SetFrameId(colmap::kInvalidFrameId);

            for (size_t i = 0; i < new_img.NumPoints2D(); ++i) {
                new_img.ResetPoint3DForPoint2D(i);
            }

            auto cam_from_world = img.CamFromWorld();
            Eigen::Quaterniond q = cam_from_world.rotation();
            Eigen::Vector3d    t = cam_from_world.translation();
            colmap::Rigid3d new_pose(q, t - q * vOffset);

            stMergedRecon.AddImageWithTrivialFrame(new_img, new_pose);
        }

        // points3D
        for (const auto& [old_id, pt] : stSubRecon.Points3D()) {
            colmap::Track new_track; 
            colmap::Track old_track;
            std::unordered_set<colmap::point3D_t> old_point3D_ids;

            for (const auto& el : pt.track.Elements()) {
                colmap::TrackElement ne;
                ne.image_id    = el.image_id;
                ne.point2D_idx = el.point2D_idx;

                if (setImageIdsCommon.count(el.image_id)) {
                    const colmap::Image& tgt_img = stMergedRecon.Image(el.image_id);
                    printf("Image %d pts2d %d\n", el.image_id, tgt_img.NumPoints2D());
                    
                    if (el.point2D_idx >= tgt_img.NumPoints2D()) {
                        continue;
                    }
                    const auto& p2d = tgt_img.Point2D(el.point2D_idx);
                    if (p2d.HasPoint3D()) {
                        old_track.AddElement(ne);
                        old_point3D_ids.insert(p2d.point3D_id);
                    } else {
                        new_track.AddElement(ne);
                    }
                } else if (setImageIdsMiss.count(el.image_id)) {
                    new_track.AddElement(ne);
                }
            }

            const bool create_new_point = new_track.Length() >= 2;
            const bool merge_new_and_old =
                (new_track.Length() + old_track.Length()) >= 2 &&
                old_point3D_ids.size() == 1;

            if (create_new_point || merge_new_and_old) {
                Eigen::Vector3d xyz = pt.xyz + vOffset;
                auto new_pt_id =
                    stMergedRecon.AddPoint3D(xyz, new_track, pt.color);
                if (old_point3D_ids.size() == 1) {
                    stMergedRecon.MergePoints3D(new_pt_id, *old_point3D_ids.begin());
                }
            }
        }
    }
    
    // write merge result
    stMergedRecon.Write(sPath);

    try {
        for (const auto stSubModel : vSubParts) {
            std::filesystem::remove_all(stSubModel.sPath);
        }
    } catch (std::exception& e) {
    }

    return EXIT_SUCCESS;
}

int TaskStageAlign::start() {
    if ( !std::filesystem::exists(getWorkPath() + "/FeatureSaver") 
        || !std::filesystem::exists(getWorkPath() + "/cameraPose/0/cameras.bin") ) {
        return EXIT_FAILURE;
    }
        
    // gps
    std::string sPosePath = getWorkPath() + "/cameraPose/";
    // for each sub model
    const int nArgcModels = 11;
    std::string sArgvModels[nArgcModels] = { "HaigeReconstruct", 
                            "--input_path", sPosePath, 
                            "--output_path", sPosePath,
                            "--database_path", getWorkPath() + "/FeatureSaver",
                            "--alignment_max_error", "2.0",
                            "--image_path", getImagePath()
                        };

    try {
        size_t nSubModel = 0;
        std::vector<std::filesystem::path> vDropModel;
        for (const auto& entry : std::filesystem::directory_iterator(sPosePath)) {
            if (std::filesystem::is_directory(entry.status())) {
                std::string sFolderName = entry.path().filename().string();
                sArgvModels[2] = sPosePath + sFolderName;
                sArgvModels[4] = sPosePath + sFolderName;

                std::vector<char*> vArgvModels;
                for (size_t i = 0; i < nArgcModels; ++i) {
                    vArgvModels.push_back(const_cast<char*>(sArgvModels[i].c_str()));
                }

                if (colmap::RunModelAligner_HG(nArgcModels, vArgvModels.data()) != EXIT_SUCCESS) {
                    printf("SubModel [%s] align failed\n", sFolderName.c_str());
                }

                if (!std::filesystem::exists(sArgvModels[4] + "/metadata.xml")) {
                    vDropModel.emplace_back(entry.path());
                } else {
                    nSubModel++;
                }
            }
        }
        // remove failed part
        for (auto &path : vDropModel) {
            std::filesystem::remove_all(path);
        }

        if (nSubModel <= 0) {
            return EXIT_FAILURE;
        }
        printf("Valid parts of models %lu\n", nSubModel);

    } catch (std::exception& e) {
        printf("computeCameraPose catch exception when align %s", e.what());
        return EXIT_FAILURE;
    }

    // merge
    if (mergeSubModel(sPosePath) == EXIT_FAILURE) return EXIT_FAILURE;

    std::ofstream ofs(getWorkPath() + "/PoseDone");
    ofs.close();
    return EXIT_SUCCESS;
}

void TaskStageAlign::stop() {}

} // namespace HG
