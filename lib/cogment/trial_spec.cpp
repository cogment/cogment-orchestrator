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

#ifndef NDEBUG
  #define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
#endif

#include "cogment/trial_spec.h"
#include "cogment/config_file.h"
#include "cogment/utils.h"
#include "spdlog/spdlog.h"

namespace {
class ProtoErrorCollector : public google::protobuf::compiler::MultiFileErrorCollector {
public:
  void AddError(const google::protobuf::string& filename, int line, int column,
                const google::protobuf::string& message) override {
    (void)filename;
    (void)line;
    (void)column;
    spdlog::error("e: {}", message);
  }

  void AddWarning(const google::protobuf::string& filename, int line, int column,
                  const google::protobuf::string& message) override {
    (void)filename;
    (void)line;
    (void)column;
    spdlog::warn("w: {}", message);
  }
};
}  // namespace

namespace cogment {

Trial_spec::Trial_spec(const YAML::Node& root) {
  ProtoErrorCollector error_collector;

  m_source_tree = std::make_unique<google::protobuf::compiler::DiskSourceTree>();
  m_source_tree->MapPath("", ".");

  m_importer = std::make_unique<google::protobuf::compiler::Importer>(m_source_tree.get(), &error_collector);

  for (const auto& i : root[cfg_file::import_key][cfg_file::i_proto_key]) {
    spdlog::info("Importing protobuf: {}", i.as<std::string>());
    auto fd = m_importer->Import(i.as<std::string>());

    if (!fd) {
      throw MakeException("Init failure: Failed to load proto file [%s]", i.as<std::string>().c_str());
    }
  }

  m_message_factory = std::make_unique<google::protobuf::DynamicMessageFactory>(m_importer->pool());

  if (root[cfg_file::environment_key] != nullptr &&
      root[cfg_file::environment_key][cfg_file::e_config_type_key] != nullptr) {
    auto type = root[cfg_file::environment_key][cfg_file::e_config_type_key].as<std::string>();
    const auto* config_type = m_importer->pool()->FindMessageTypeByName(type);
    if (config_type == nullptr) {
      throw MakeException("Init failure (1): Failed to lookup message type [%s]", type.c_str());
    }

    env_config_prototype = m_message_factory->GetPrototype(config_type);
  }

  if (root[cfg_file::trial_key] != nullptr && root[cfg_file::trial_key][cfg_file::t_config_type_key] != nullptr) {
    auto type = root[cfg_file::trial_key][cfg_file::t_config_type_key].as<std::string>();
    const auto* config_type = m_importer->pool()->FindMessageTypeByName(type);
    if (config_type == nullptr) {
      throw MakeException("Init failure (2): Failed to lookup message type [%s]", type.c_str());
    }

    trial_config_prototype = m_message_factory->GetPrototype(config_type);
  }

  for (const auto& a_class : root[cfg_file::actors_key]) {
    actor_classes.push_back({});

    auto& actor_class = actor_classes.back();
    actor_class.name = a_class[cfg_file::ac_name_key].as<std::string>();
    spdlog::info("Adding actor class {}", actor_class.name);

    if (a_class[cfg_file::ac_config_type_key] != nullptr) {
      auto type = a_class[cfg_file::ac_config_type_key].as<std::string>();
      const auto* config_type = m_importer->pool()->FindMessageTypeByName(type);
      if (config_type == nullptr) {
        throw MakeException("Init failure (3): Failed to lookup message type [%s]", type.c_str());
      }

      actor_class.config_prototype = m_message_factory->GetPrototype(config_type);
    }

    auto obs_space = a_class[cfg_file::ac_observation_key][cfg_file::ac_obs_space_key].as<std::string>();
    const auto* observation_space = m_importer->pool()->FindMessageTypeByName(obs_space);
    if (observation_space == nullptr) {
      throw MakeException("Init failure (4): Failed to lookup message type [%s]", obs_space.c_str());
    }

    actor_class.observation_space_prototype = m_message_factory->GetPrototype(observation_space);

    if (root[cfg_file::datalog_key] != nullptr && root[cfg_file::datalog_key][cfg_file::d_fields_key] != nullptr &&
        root[cfg_file::datalog_key][cfg_file::d_fields_key][cfg_file::d_fld_exclude_key] != nullptr) {
      for (const auto& f : root[cfg_file::datalog_key][cfg_file::d_fields_key][cfg_file::d_fld_exclude_key]) {
        auto field_name = f.as<std::string>();
        if (field_name.find(observation_space->full_name()) == 0) {
          field_name = field_name.substr(observation_space->full_name().size() + 1);
        }
        else {
          continue;
        }

        const auto* x = observation_space->FindFieldByName(field_name);
        if (x != nullptr) {
          actor_class.cleared_observation_fields.push_back(x);
        }
      }
    }
    if (a_class[cfg_file::ac_observation_key][cfg_file::ac_obs_delta_key] != nullptr) {
      auto delta = a_class[cfg_file::ac_observation_key][cfg_file::ac_obs_delta_key].as<std::string>();
      const auto* observation_delta = m_importer->pool()->FindMessageTypeByName(delta);
      if (observation_delta == nullptr) {
        throw MakeException("Init failure (5): Failed to lookup message type [%s]", delta.c_str());
      }

      actor_class.observation_delta_prototype = m_message_factory->GetPrototype(observation_delta);
      if (root[cfg_file::datalog_key] != nullptr && root[cfg_file::datalog_key][cfg_file::d_fields_key] != nullptr &&
          root[cfg_file::datalog_key][cfg_file::d_fields_key][cfg_file::d_fld_exclude_key] != nullptr) {
        for (const auto& f : root[cfg_file::datalog_key][cfg_file::d_fields_key][cfg_file::d_fld_exclude_key]) {
          auto field_name = f.as<std::string>();
          if (field_name.find(observation_delta->full_name()) == 0 &&
              field_name.size() > observation_delta->full_name().size() &&
              field_name[observation_delta->full_name().size()] == '.') {
            field_name = field_name.substr(observation_delta->full_name().size() + 1);
          }
          else {
            continue;
          }

          const auto* x = observation_delta->FindFieldByName(field_name);
          if (x != nullptr) {
            actor_class.cleared_delta_fields.push_back(x);
          }
        }
      }
    }
    else {
      actor_class.observation_delta_prototype = actor_class.observation_space_prototype;
      actor_class.cleared_delta_fields = actor_class.cleared_observation_fields;
    }

    spdlog::debug("Clearing {} delta fields", actor_class.cleared_delta_fields.size());

    auto act_space = a_class[cfg_file::ac_action_key][cfg_file::ac_act_space_key].as<std::string>();
    const auto* action_space = m_importer->pool()->FindMessageTypeByName(act_space);
    if (action_space == nullptr) {
      throw MakeException("Init failure (6): Failed to lookup message type [%s]", act_space.c_str());
    }
    actor_class.action_space_prototype = m_message_factory->GetPrototype(action_space);
  }
}

const ActorClass& Trial_spec::get_actor_class(const std::string& class_name) const {
  for (const auto& actor_class : actor_classes) {
    if (actor_class.name == class_name) {
      return actor_class;
    }
  }

  throw MakeException("Trying to use unregistered actor class [{}]", class_name.c_str());
}
}  // namespace cogment
