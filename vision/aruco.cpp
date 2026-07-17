#include "aruco.h"

#include <plog/Log.h>
#include <opencv2/imgproc.hpp>

namespace vision {

Aruco::Aruco() {
    auto params = cv::aruco::DetectorParameters();
    const auto dict =
        cv::aruco::getPredefinedDictionary(cv::aruco::DICT_5X5_1000);
    _detector = cv::aruco::ArucoDetector(dict, params);

    auto dict2 = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_5X5_1000);

    cv::Mat marker;
    cv::aruco::generateImageMarker(dict, 1, 100, marker, 1);
    cv::imwrite("marker.png", marker);
}

void Aruco::detect(const cv::Mat& image) {
    _ids.clear();
    _corners.clear();
    _rejected.clear();

    _detector.detectMarkers(image, _corners, _ids, _rejected);
}

std::vector<cv::Point2f> Aruco::corners(int id) const {
    assert(_corners.size() == _ids.size() &&
           "ArucoDetector: ids and corners arrays are different size");

    const auto it = std::find(std::begin(_ids), std::end(_ids), id);

    if (it == std::end(_ids)) {
        return {};
    } else {
        const auto index = std::distance(std::begin(_ids), it);
        return _corners.at(index);
    }
}

std::vector<int> Aruco::ids() const { return _ids; }

std::vector<Aruco::Detection> Aruco::detections() const {
    assert(_corners.size() == _ids.size() &&
           "ArucoDetector: ids and corners arrays are different size");

    std::vector<Detection> result;
    result.reserve(_ids.size());

    for (auto i = 0u; i < _ids.size(); ++i) {
        result.emplace_back(_ids[i], _corners[i]);
    }

    return result;
}

}  // namespace vision
