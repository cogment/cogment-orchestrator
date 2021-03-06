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

#ifndef AOM_ORCHESTRATOR_AGENT_ACTOR_H
#define AOM_ORCHESTRATOR_AGENT_ACTOR_H

#include "cogment/actor.h"
#include "cogment/api/agent.egrpc.pb.h"
#include "cogment/stub_pool.h"

#include <optional>

namespace cogment {

class Trial;

class Agent : public Actor {
public:
  using StubEntryType = std::shared_ptr<Stub_pool<cogment::AgentEndpoint>::Entry>;
  Agent(Trial* owner, const std::string& actor_name, const ActorClass* actor_class, const std::string& impl,
        StubEntryType stub_entry, std::optional<std::string> config_data);

  ~Agent();

  aom::Future<void> init() override;

  bool is_active() const override;

protected:
  void dispatch_observation(cogment::Observation&& obs) override;
  void dispatch_final_data(cogment::ActorPeriodData&& data) override;
  void dispatch_reward(cogment::Reward&& reward) override;
  void dispatch_message(cogment::Message&& message) override;

private:
  void lazy_start_decision_stream();

  StubEntryType m_stub_entry;
  std::vector<grpc_metadata> m_headers;
  easy_grpc::client::Call_options m_options;

  cogment::Action m_latest_action;
  std::optional<std::string> m_config_data;

  std::optional<::easy_grpc::Stream_promise<::cogment::AgentObservationRequest>> m_outgoing_observations;

  std::string m_impl;
};

}  // namespace cogment

#endif
