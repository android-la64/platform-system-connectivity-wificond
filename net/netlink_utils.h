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

#ifndef WIFICOND_NET_NETLINK_UTILS_H_
#define WIFICOND_NET_NETLINK_UTILS_H_

#include <string>
#include <vector>

#include <linux/nl80211.h>

#include <android-base/macros.h>

#include "wificond/net/netlink_manager.h"

namespace android {
namespace wificond {

struct BandInfo {
  BandInfo() = default;
  BandInfo(std::vector<uint32_t>& band_2g_,
           std::vector<uint32_t>& band_5g_,
           std::vector<uint32_t>& band_dfs_)
      : band_2g(band_2g_),
        band_5g(band_5g_),
        band_dfs(band_dfs_) {}
  // Frequencies for 2.4 GHz band.
  std::vector<uint32_t> band_2g;
  // Frequencies for 5 GHz band without DFS.
  std::vector<uint32_t> band_5g;
  // Frequencies for DFS.
  std::vector<uint32_t> band_dfs;
};

struct ScanCapabilities {
  ScanCapabilities() = default;
  ScanCapabilities(uint8_t max_num_scan_ssids_,
                   uint8_t max_num_sched_scan_ssids_,
                   uint8_t max_match_sets_)
      : max_num_scan_ssids(max_num_scan_ssids_),
        max_num_sched_scan_ssids(max_num_sched_scan_ssids_),
        max_match_sets(max_match_sets_) {}
  // Number of SSIDs you can scan with a single scan request.
  uint8_t max_num_scan_ssids;
  // Number of SSIDs you can scan with a single scheduled scan request.
  uint8_t max_num_sched_scan_ssids;
  // Maximum number of sets that can be used with NL80211_ATTR_SCHED_SCAN_MATCH.
  uint8_t max_match_sets;
};

struct WiphyFeatures {
  WiphyFeatures()
      : supports_random_mac_oneshot_scan(false),
        supports_random_mac_sched_scan(false) {}
  WiphyFeatures(uint32_t feature_flags)
      : supports_random_mac_oneshot_scan(
            feature_flags & NL80211_FEATURE_SCAN_RANDOM_MAC_ADDR),
        supports_random_mac_sched_scan(
            feature_flags & NL80211_FEATURE_SCHED_SCAN_RANDOM_MAC_ADDR) {}
  // This device/driver supports using a random MAC address during scan
  // (while not associated).
  bool supports_random_mac_oneshot_scan;
  // This device/driver supports using a random MAC address for every
  // scan iteration during scheduled scan (while not associated).
  bool supports_random_mac_sched_scan;
  // There are other flags included in NL80211_ATTR_FEATURE_FLAGS.
  // We will add them once we find them useful.
};

struct StationInfo {
  StationInfo() = default;
  StationInfo(uint32_t station_tx_packets_,
              uint32_t station_tx_failed_,
              uint32_t station_tx_bitrate_,
              int8_t current_rssi_)
      : station_tx_packets(station_tx_packets_),
        station_tx_failed(station_tx_failed_),
        station_tx_bitrate(station_tx_bitrate_),
        current_rssi(current_rssi_) {}
  // Number of successfully transmitted packets.
  int32_t station_tx_packets;
  // Number of tramsmission failures.
  int32_t station_tx_failed;
  // Transimission bit rate in 100kbit/s.
  uint32_t station_tx_bitrate;
  // Current signal strength.
  int8_t current_rssi;
  // There are many other counters/parameters included in station info.
  // We will add them once we find them useful.
};

class MlmeEventHandler;
class NetlinkManager;
class NL80211Packet;

// Provides NL80211 helper functions.
class NetlinkUtils {
 public:
  // Currently we only support setting the interface to STATION mode.
  // This is used for cleaning up interface after KILLING hostapd.
  enum InterfaceMode{
      STATION_MODE
  };

  explicit NetlinkUtils(NetlinkManager* netlink_manager);
  virtual ~NetlinkUtils();

  // Get the wiphy index from kernel.
  // |*out_wiphy_index| returns the wiphy index from kernel.
  // Returns true on success.
  virtual bool GetWiphyIndex(uint32_t* out_wiphy_index);

  // Get wifi interface info from kernel.
  // |wiphy_index| is the wiphy index we get using GetWiphyIndex().
  // Returns true on success.
  virtual bool GetInterfaceInfo(uint32_t wiphy_index,
                                std::string* name,
                                uint32_t* index,
                                std::vector<uint8_t>* mac_addr);

  // Set the mode of interface.
  // |interface_index| is the interface index.
  // |mode| is one of the values in |enum InterfaceMode|.
  // Returns true on success.
  virtual bool SetInterfaceMode(uint32_t interface_index,
                                InterfaceMode mode);

  // Get wiphy capability information from kernel.
  // Returns true on success.
  virtual bool GetWiphyInfo(uint32_t wiphy_index,
                            BandInfo* out_band_info,
                            ScanCapabilities* out_scan_capabilities,
                            WiphyFeatures* out_wiphy_features);

  // Get station info from kernel.
  // |*out_station_info]| is the struct of available station information.
  // Returns true on success.
  virtual bool GetStationInfo(uint32_t interface_index,
                              const std::vector<uint8_t>& mac_address,
                              StationInfo* out_station_info);

  // Sign up to be notified when there is MLME event.
  // Only one handler can be registered per interface index.
  // New handler will replace the registered handler if they are for the
  // same interface index.
  // NetlinkUtils is not going to take ownership of this pointer, and that it
  // is the caller's responsibility to make sure that the object exists for the
  // duration of the subscription.
  virtual void SubscribeMlmeEvent(uint32_t interface_index,
                                  MlmeEventHandler* handler);

  // Cancel the sign-up of receiving MLME event notification
  // from interface with index |interface_index|.
  virtual void UnsubscribeMlmeEvent(uint32_t interface_index);

  // Sign up to be notified when there is an regulatory domain change.
  // Only one handler can be registered per wiphy index.
  // New handler will replace the registered handler if they are for the
  // same wiphy index.
  virtual void SubscribeRegDomainChange(uint32_t wiphy_index,
                                        OnRegDomainChangedHandler handler);

  // Cancel the sign-up of receiving regulatory domain change notification
  // from wiphy with index |wiphy_index|.
  virtual void UnsubscribeRegDomainChange(uint32_t wiphy_index);

 private:
  bool ParseBandInfo(const NL80211Packet* const packet,
                     BandInfo* out_band_info);
  bool ParseScanCapabilities(const NL80211Packet* const packet,
                             ScanCapabilities* out_scan_capabilities);
  NetlinkManager* netlink_manager_;

  DISALLOW_COPY_AND_ASSIGN(NetlinkUtils);
};

}  // namespace wificond
}  // namespace android

#endif  // WIFICOND_NET_NETLINK_UTILS_H_
