// Copyright 2021 AI Redefined Inc. <dev+cogment@ai-r.com>
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef AOM_DATALOG_STORAGE_INTERFACE_H
#define AOM_DATALOG_STORAGE_INTERFACE_H

#include "cogment/api/datalog.pb.h"

#include "yaml-cpp/yaml.h"

namespace cogment {

class Trial;
// Per trial datalog interface.
class TrialLogInterface {
public:
  virtual ~TrialLogInterface() {}

  virtual void add_sample(cogment::DatalogSample&& data) = 0;
};

// Orchestrator-wide datalog interface.
class DatalogStorageInterface {
public:
  virtual ~DatalogStorageInterface() {}

  virtual std::unique_ptr<TrialLogInterface> start_log(const Trial* trial) = 0;

  static std::unique_ptr<DatalogStorageInterface> create(const std::string& spec, const YAML::Node& cfg);
};

}  // namespace cogment

#endif