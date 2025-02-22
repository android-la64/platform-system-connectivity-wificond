/*
 * Copyright (C) 2016 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef WIFICOND_SCANNING_SINGLE_SCAN_SETTINGS_H_
#define WIFICOND_SCANNING_SINGLE_SCAN_SETTINGS_H_

#include <vector>

#include <binder/Parcel.h>
#include <binder/Parcelable.h>

#include "wificond/scanning/channel_settings.h"
#include "wificond/scanning/hidden_network.h"

namespace android {
namespace net {
namespace wifi {
namespace nl80211 {

class SingleScanSettings : public ::android::Parcelable {
 public:
  SingleScanSettings() = default;
  bool operator==(const SingleScanSettings& rhs) const {
    return (scan_type_ == rhs.scan_type_ &&
            channel_settings_ == rhs.channel_settings_ &&
            hidden_networks_ == rhs.hidden_networks_ &&
            vendor_ies_ == rhs.vendor_ies_);
  }
  ::android::status_t writeToParcel(::android::Parcel* parcel) const override;
  ::android::status_t readFromParcel(const ::android::Parcel* parcel) override;

  int32_t scan_type_;
  bool enable_6ghz_rnr_;
  std::vector<ChannelSettings> channel_settings_;
  std::vector<HiddenNetwork> hidden_networks_;
  std::vector<uint8_t> vendor_ies_;

 private:
  bool isValidScanType() const;
};

}  // namespace nl80211
}  // namespace wifi
}  // namespace net
}  // namespace android

#endif  // WIFICOND_SCANNING_SINGLE_SCAN_SETTINGS_H_
