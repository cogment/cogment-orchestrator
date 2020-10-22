// Copyright 2020 Artificial Intelligence Redefined <dev+cogment@ai-r.com>
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

#include "cogment/trial_params.h"
#include "cogment/base64.h"
#include "cogment/trial_spec.h"

#include <google/protobuf/util/json_util.h>
#include "spdlog/spdlog.h"

namespace cogment {

namespace {

std::string yaml_to_json(YAML::Node yaml) {
  yaml.SetStyle(YAML::EmitterStyle::Flow);
  YAML::Emitter emmiter;
  emmiter << YAML::DoubleQuoted << yaml;

  return emmiter.c_str();
}

// This function takes a config node, containing a yaml object describing
// a protobuf of the "proto" type, and replaces it with an object with a single
// "content" member that holds the base64 encoded version of that object.
void encode_user_config(YAML::Node config_node, const google::protobuf::Message* proto) {
  // This will happen if the cogment.yaml does not specify that config type,
  // but still provides a value for it.
  if (proto == nullptr) {
    throw std::runtime_error("Unexpected user config");
  }

  auto config_as_json = yaml_to_json(config_node);

  // Convert from the json to protobuf
  std::unique_ptr<google::protobuf::Message> user_msg(proto->New());
  auto status = google::protobuf::util::JsonStringToMessage(config_as_json, user_msg.get());

  if (!status.ok()) {
    spdlog::error("{}", status.error_message().as_string());
    throw std::runtime_error("Problem interpreting user config");
  }

  // Build the replacement node
  YAML::Node content_node;
  content_node["content"] = base64_encode(user_msg->SerializeAsString());

  // Replace the node
  config_node = content_node;
}
}  // namespace

// Loads and interprets the default params structure from the root cogment.yaml
cogment::TrialParams load_params(const YAML::Node& yaml, const Trial_spec& spec) {
  cogment::TrialParams result;

  if (yaml["trial_params"] != nullptr) {
    YAML::Node yaml_params = yaml["trial_params"];

    // The user specifies his own protocol buffers in the yaml, but the actual
    // forat for TrialParams uses bytes fields instead, so we translate

    if (yaml_params["trial_config"] != nullptr) {
      encode_user_config(yaml_params["trial_config"], spec.get_trial_config_proto());
    }

    if (yaml_params["environment"]["config"] != nullptr) {
      encode_user_config(yaml_params["environment"]["config"], spec.get_env_config_prototype());
    }

    for (auto actor : yaml_params["actors"]) {
      if (actor["config"] != nullptr) {
        const auto& actor_class = spec.get_actor_class(actor["actor_class"].as<std::string>());

        encode_user_config(actor["config"], actor_class.config_prototype);
      }
    }

    auto status = google::protobuf::util::JsonStringToMessage(yaml_to_json(yaml_params), &result);

    if (!status.ok()) {
      spdlog::error("{}", status.error_message().as_string());
      spdlog::error("{}", yaml_to_json(yaml_params));
      throw std::runtime_error("Problem rebuilding trial params");
    }
  }

  spdlog::info("default trial params: {}", result.DebugString());
  return result;
}
}  // namespace cogment
