# PoseEstimator
Estimate camera pose base on colmap and GPS info in images

## input
1. 多视角图像集
2. 每张图像的Exif信息中需要包含有GPS，用于当特征匹配不佳导致colmap生成多个子模型时合并多个子模型 

## steps
1. 调用colmap的feature_extractor进行特征检测
2. 调用colmap的spatial_matcher根据GPS信息进行特征匹配
3. 调用colmap的mapper进行三角恢复
4. 调用修改的colmap的model_align对齐到enu坐标系，然后根据GPS信息合并子模型（如果有的话）
5. 最终生成整体模型的摄像机参数{cameras.bin, points3D.bin, images.bin}
