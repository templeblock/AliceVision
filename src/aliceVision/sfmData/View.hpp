// This file is part of the AliceVision project.
// Copyright (c) 2016 AliceVision contributors.
// Copyright (c) 2012 openMVG contributors.
// This Source Code Form is subject to the terms of the Mozilla Public License,
// v. 2.0. If a copy of the MPL was not distributed with this file,
// You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include <aliceVision/types.hpp>

#include <boost/algorithm/string.hpp>

#include <string>
#include <utility>

namespace aliceVision {
namespace sfmData {

/**
 * @brief EXIF Orientation to names
 */
enum class EEXIFOrientation
{
  NONE = 1
  , REVERSED = 2
  , UPSIDEDOWN = 3
  , UPSIDEDOWN_REVERSED = 4
  , LEFT_REVERSED = 5
  , LEFT = 6  
  , RIGHT_REVERSED = 7
  , RIGHT = 8
  , UNKNOWN = -1
};

/**
 * @brief A view define an image by a string and unique indexes for
 * the view, the camera intrinsic, the pose and the subpose if the camera is part of a rig
 */
class View
{
public:

  /**
   * @brief View Constructor
   * @param[in] imagePath The image path on disk
   * @param[in] viewId The view id (use unique index)
   * @param[in] intrinsicId The intrinsic id
   * @param[in] poseId The pose id (or the rig pose id)
   * @param[in] width The image width
   * @param[in] height The image height
   * @param[in] rigId The rig id (or undefined)
   * @param[in] subPoseId The sub-pose id (or undefined)
   * @param[in] metadata The image metadata
   */
  View(const std::string& imagePath = "",
       IndexT viewId = UndefinedIndexT,
       IndexT intrinsicId = UndefinedIndexT,
       IndexT poseId = UndefinedIndexT,
       std::size_t width = 0,
       std::size_t height = 0,
       IndexT rigId = UndefinedIndexT,
       IndexT subPoseId = UndefinedIndexT,
       const std::map<std::string, std::string>& metadata = std::map<std::string, std::string>())
    : _imagePath(imagePath)
    , _width(width)
    , _height(height)
    , _viewId(viewId)
    , _intrinsicId(intrinsicId)
    , _poseId(poseId)
    , _rigId(rigId)
    , _subPoseId(subPoseId)
    , _metadata(metadata)
  {}

  bool operator==(const View& other) const
  {
    // image paths can be different
    return _viewId == other._viewId &&
           _intrinsicId == other._intrinsicId &&
           _poseId == other._poseId &&
           _width == other._width &&
           _height == other._height &&
           _rigId == other._rigId &&
           _subPoseId == other._subPoseId;
  }

  /**
   * @brief Get view image path
   * @return image path
   */
  const std::string& getImagePath() const
  {
    return _imagePath;
  }

  /**
   * @brief Get view image width
   * @return image width
   */
  std::size_t getWidth() const
  {
    return _width;
  }

  /**
   * @brief Get view image height
   * @return image height
   */
  std::size_t getHeight() const
  {
    return _height;
  }

  /**
   * @brief Get the view id
   * @return view id
   */
  IndexT getViewId() const
  {
    return _viewId;
  }

  /**
   * @brief Get the intrinsic id
   * @return intrinsic id
   */
  IndexT getIntrinsicId() const
  {
    return _intrinsicId;
  }

  /**
   * @brief Get the pose id
   * @return pose id
   */
  IndexT getPoseId() const
  {
    return _poseId;
  }

  /**
   * @brief Get the rig id
   * @return rig id or undefined
   */
  IndexT getRigId() const
  {
    return _rigId;
  }

  /**
   * @brief Get the sub-pose id
   * @return sup-pose id or undefined
   */
  IndexT getSubPoseId() const
  {
    return _subPoseId;
  }

  /**
   * @brief Get the frame id
   * @return frame id
   */
  IndexT getFrameId() const
  {
    return _frameId;
  }

  /**
   * @brief Get the resection id
   * @return resection id
   */
  IndexT getResectionId() const
  {
    return _resectionId;
  }

  /**
   * @brief Return if true or false the view is part of a rig
   * @return true if the view is part of a rig
   */
  bool isPartOfRig() const
  {
    return _rigId != UndefinedIndexT;
  }

  /**
   * @brief xxx
   * @return true if the view is xxx
   */
  bool isPoseIndependant() const
  {
    return (!isPartOfRig() || _isIndependantPose);
  }

  /**
   * 
  */
  float getCameraExposureSetting() const
  {
    const float shutter = getMetadataShutter();
    const float fnumber = getMetadataFNumber();
    if (shutter < 0 || fnumber < 0)
        return -1.f;

    const float iso = getMetadataISO();
    const float isoRatio = (iso < 0.f) ? 1.0 : (iso / 100.f);

    float cameraExposure = (shutter * isoRatio) / (fnumber * fnumber);
    return cameraExposure;
  }

  float getEv() const
  {
    return std::log2(1.f/getCameraExposureSetting());
  }

  std::map<std::string, std::string>::const_iterator findMetadataIterator(const std::string& name) const
  {
    auto it = _metadata.find(name);
    if(it != _metadata.end())
      return it;
    std::string nameLower = name;
    boost::algorithm::to_lower(nameLower);
    for(auto mIt = _metadata.begin(); mIt != _metadata.end(); ++mIt)
    {
      std::string key = mIt->first;
      boost::algorithm::to_lower(key);
      if(key.size() > name.size())
      {
        auto delimiterIt = key.find_last_of("/:");
        if(delimiterIt != std::string::npos)
          key = key.substr(delimiterIt + 1);
      }
      if(key == nameLower)
      {
        if(it == _metadata.end())
        {
          // we found one candidate match
          it = mIt;
        }
        else
        {
          // multiple entries match the input
          return _metadata.end();
        }
      }
    }
    return it;
  }

  /**
   * @brief Return true if the given metadata name exists
   * @param[in] name The metadata name
   * @return true if the corresponding metadata value exists
   */
  bool hasMetadata(const std::vector<std::string>& names) const
  {
    for(const std::string& name: names)
    {
      const auto it = findMetadataIterator(name);
      if(it != _metadata.end())
        return true;
    }
    return false;
  }

  /**
   * @brief Return true if the given metadata name exists and is a digit
   * @param[in] name The metadata name
   * @param[in] isPositive true if the metadata must be positive
   * @return true if the corresponding metadata value exists
   */
  bool hasDigitMetadata(const std::string& name, bool isPositive = true) const
  {
    double value = -1.0;
    const auto it = findMetadataIterator(name);

    if(it == _metadata.end() || it->second.empty())
      return false;

    try
    {
      value = std::stod(it->second);
    }
    catch(std::exception &)
    {
      return false;
    }
    return (!isPositive || (value > 0));
  }

  const std::string& getMetadata(const std::vector<std::string>& names) const
  {
    static const std::string emptyString;
    for(const std::string& name: names)
    {
      const auto it = findMetadataIterator(name);
      if(it != _metadata.end())
        return it->second;
    }
    return emptyString;
  }

  double getDoubleMetadata(const std::vector<std::string>& names) const
  {
    const std::string value = getMetadata(names);
    if(value.empty())
      return -1.0;
    try
    {
      return std::stod(value);
    }
    catch(std::exception&)
    {
      return -1.0;
    }
  }

  int getIntMetadata(const std::vector<std::string>& names) const
  {
    const std::string value = getMetadata(names);
    if(value.empty())
      return -1;
    try
    {
      return std::stoi(value);
    }
    catch(std::exception&)
    {
      return -1;
    }
  }

  /**
   * @brief Get the corresponding "Make" metadata value
   * @return the metadata value string or "" if no corresponding value
   */
  const std::string& getMetadataMake() const
  {
    return getMetadata({"Make", "cameraMake"});
  }

  /**
   * @brief Get the corresponding "Model" metadata value
   * @return the metadata value string or "" if no corresponding value
   */
  const std::string& getMetadataModel() const
  {
    return getMetadata({"Model", "cameraModel"});
  }

  /**
   * @brief Get the corresponding "BodySerialNumber" metadata value
   * @return the metadata value string or "" if no corresponding value
   */
  const std::string& getMetadataBodySerialNumber() const
  {
    return getMetadata({"Exif:BodySerialNumber", "cameraSerialNumber"});
  }

  /**
   * @brief Get the corresponding "LensSerialNumber" metadata value
   * @return the metadata value string or "" if no corresponding value
   */
  const std::string& getMetadataLensSerialNumber() const
  {
    return getMetadata({"Exif:LensSerialNumber", "lensSerialNumber"});
  }

  /**
   * @brief Get the corresponding "FocalLength" metadata value
   * @return the metadata value float or -1 if no corresponding value
   */
  double getMetadataFocalLength() const
  {
    return getDoubleMetadata({"Exif:FocalLength", "focalLength"});
  }

  /**
     * @brief Get the corresponding "ExposureTime" (shutter) metadata value
     * @return the metadata value float or -1 if no corresponding value
     */
  double getMetadataShutter() const
  {
      return getDoubleMetadata({"ExposureTime"});
  }

  /**
   * @brief Get the corresponding "FNumber" (relative aperture) metadata value
   * @return the metadata value float or -1 if no corresponding value
   */
  double getMetadataFNumber() const
  {
      if(hasDigitMetadata("FNumber"))
      {
          return getDoubleMetadata({"FNumber"});
      }
      if (hasDigitMetadata("ApertureValue"))
      {
          const double aperture = getDoubleMetadata({"ApertureValue"});
          // fnumber = 2^(aperture/2)
          return std::pow(2.0, aperture / 2.0);
      }
      return -1;
  }

  /**
     * @brief Get the corresponding "PhotographicSensitivity" (ISO) metadata value
     * @return the metadata value int or -1 if no corresponding value
     */
  double getMetadataISO() const
  {
      return getDoubleMetadata({"Exif:PhotographicSensitivity", "PhotographicSensitivity"});
  }

  /**
   * @brief Get the corresponding "Orientation" metadata value
   * @return the enum EEXIFOrientation
   */
  EEXIFOrientation getMetadataOrientation() const
  {
    const int orientation = getIntMetadata({"Exif:Orientation", "Orientation"});
    if(orientation < 0)
      return EEXIFOrientation::UNKNOWN;
    return static_cast<EEXIFOrientation>(orientation);
  }

  const std::string& getMetadataDateTimeOriginal() const
  {
    return getMetadata({"Exif:DateTimeOriginal", "DateTimeOriginal"});
  }

  /**
   * @brief Get the view metadata structure
   * @return the view metadata
   */
  const std::map<std::string, std::string>& getMetadata() const
  {
    return _metadata;
  }

  /**
   * @brief Set the given view image path
   * @param[in] imagePath The given view image path
   */
  void setImagePath(const std::string& imagePath)
  {
    _imagePath = imagePath;
  }

  /**
   * @brief  Set the given view image width
   * @param[in] width The given view image width
   */
  void setWidth(std::size_t width)
  {
    _width = width;
  }

  /**
   * @brief  Set the given view image height
   * @param[in] height The given view image height
   */
  void setHeight(std::size_t height)
  {
    _height = height;
  }

  /**
   * @brief Set the given view id
   * @param[in] viewId The given view id
   */
  void setViewId(IndexT viewId)
  {
    _viewId = viewId;
  }

  /**
   * @brief Set the given intrinsic id
   * @param[in] intrinsicId The given intrinsic id
   */
  void setIntrinsicId(IndexT intrinsicId)
  {
    _intrinsicId = intrinsicId;
  }

  /**
   * @brief Set the given pose id
   * @param[in] poseId The given pose id
   */
  void setPoseId(IndexT poseId)
  {
    _poseId = poseId;
  }

  /**
   * @brief setIndependantPose
   * @param independant
   */
  void setIndependantPose(bool independant)
  {
    _isIndependantPose = independant;
  }

  /**
   * @brief Set the given rig id and the given sub-pose id
   * @param[in] rigId The given rig id
   * @param[in] subPoseId The given sub-pose id
   */
  void setRigAndSubPoseId(IndexT rigId, IndexT subPoseId)
  {
    _rigId = rigId;
    _subPoseId = subPoseId;
  }


  /**
   * @brief Set the given frame id
   * @param[in] frame The given frame id
   */
  void setFrameId(IndexT frameId)
  {
    _frameId = frameId;
  }

  /**
   * @brief Set the given resection id
   * @param[in] resectionId The given resection id
   */
  void setResectionId(IndexT resectionId)
  {
    _resectionId = resectionId;
  }

  /**
   * @brief Set view metadata
   * @param[in] metadata The metadata map
   */
  void setMetadata(const std::map<std::string, std::string>& metadata)
  {
    _metadata = metadata;
  }

  /**
   * @brief Add view metadata
   * @param[in] key The metadata key
   * @param[in] value The metadata value
   */
  void addMetadata(const std::string& key, const std::string& value)
  {
    _metadata.emplace(key, value);
  }

private:

  /// image path on disk
  std::string _imagePath;
  /// image width
  std::size_t _width;
  /// image height
  std::size_t _height;
  /// view id
  IndexT _viewId;
  /// intrinsics id
  IndexT _intrinsicId;
  /// either the pose of the rig or the pose of the camera if there's no rig
  IndexT _poseId;
  /// corresponding rig id or undefined
  IndexT _rigId;
  /// corresponding sub-pose id or undefined
  IndexT _subPoseId;
  /// corresponding frame id for synchronized views
  IndexT _frameId = UndefinedIndexT;
  /// resection id
  IndexT _resectionId = UndefinedIndexT;
  /// pose independant of other view(s)
  bool _isIndependantPose = true;
  /// map for metadata
  std::map<std::string, std::string> _metadata;
};

} // namespace sfmData
} // namespace aliceVision
