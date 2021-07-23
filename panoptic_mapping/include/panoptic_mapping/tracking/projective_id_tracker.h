#ifndef PANOPTIC_MAPPING_TRACKING_PROJECTIVE_ID_TRACKER_H_
#define PANOPTIC_MAPPING_TRACKING_PROJECTIVE_ID_TRACKER_H_

#include <memory>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#include <opencv2/core/mat.hpp>

#include "panoptic_mapping/3rd_party/config_utilities.hpp"
#include "panoptic_mapping/common/camera.h"
#include "panoptic_mapping/common/label_handler.h"
#include "panoptic_mapping/tools/map_renderer.h"
#include "panoptic_mapping/tracking/id_tracker_base.h"
#include "panoptic_mapping/tracking/tracking_info.h"

namespace panoptic_mapping {

/**
 * @brief Uses the input segmentation images and compares them against the
 * rendered map to track associations.
 */
class ProjectiveIDTracker : public IDTrackerBase {
 public:
  /**
   * @brief Parameters used to track new frames against the map.
   *
   */
  struct Config : public config_utilities::Config<Config> {
    int verbosity = 4;

    // Tracking.
    float depth_tolerance = -1.0;  // m, negative for multiples of voxel size
    std::string tracking_metric = "IoU";  // IoU, overlap
    float match_acceptance_threshold = 0.5;
    bool use_class_data_for_matching = true;
    bool use_approximate_rendering = false;
    int rendering_subsampling = 1;

    // Allocation.
    int min_allocation_size = 0;  // #px required to allocate new submap.

    // System params.
    int rendering_threads = std::thread::hardware_concurrency();

    // Renderer settings.
    MapRenderer::Config renderer;

    Config() { setConfigName("ProjectiveIDTracker"); }

   protected:
    void setupParamsAndPrinting() override;
    void checkParams() const override;
  };

  ProjectiveIDTracker(const Config& config, std::shared_ptr<Globals> globals,
                      bool print_config = true);
  ~ProjectiveIDTracker() override = default;

  void processInput(SubmapCollection* submaps, InputData* input) override;

 protected:
  // Internal methods.
  virtual bool classesMatch(int input_id, int submap_class_id);
  virtual Submap* allocateSubmap(int input_id, SubmapCollection* submaps,
                                 InputData* input);
  TrackingInfoAggregator computeTrackingData(SubmapCollection* submaps,
                                             InputData* input);
  TrackingInfo renderTrackingInfo(const Submap& submap,
                                  const InputData& input) const;

  TrackingInfo renderTrackingInfoApproximate(const Submap& submap,
                                             const InputData& input) const;

  TrackingInfo renderTrackingInfoVertices(const Submap& submap,
                                          const InputData& input) const;

 private:
  static config_utilities::Factory::RegistrationRos<
      IDTrackerBase, ProjectiveIDTracker, std::shared_ptr<Globals>>
      registration_;

  // Members
  const Config config_;

 protected:
  MapRenderer renderer_;  // The renderer is only used if visualization is on.
  cv::Mat rendered_vis_;  // Store visualization data.
};

}  // namespace panoptic_mapping

#endif  // PANOPTIC_MAPPING_TRACKING_PROJECTIVE_ID_TRACKER_H_
