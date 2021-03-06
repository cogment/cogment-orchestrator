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

#ifndef COGMENT_ORCHESTRATOR_ACTOR_SERVICE_H
#define COGMENT_ORCHESTRATOR_ACTOR_SERVICE_H

#include "cogment/api/orchestrator.egrpc.pb.h"

namespace cogment {
class Orchestrator;
class ActorService {
  Orchestrator* m_orchestrator;

public:
  using service_type = cogment::ClientActor;

  ActorService(Orchestrator* orch);

  ::cogment::TrialJoinReply JoinTrial(::cogment::TrialJoinRequest, easy_grpc::Context ctx);

  ::easy_grpc::Stream_future<::cogment::TrialActionReply> ActionStream(
      ::easy_grpc::Stream_future<::cogment::TrialActionRequest>, easy_grpc::Context ctx);
  ::easy_grpc::Future<::cogment::TrialHeartbeatReply> Heartbeat(::cogment::TrialHeartbeatRequest,
                                                                easy_grpc::Context ctx);
  ::easy_grpc::Future<::cogment::TrialRewardReply> SendReward(::cogment::TrialRewardRequest, easy_grpc::Context ctx);
  ::easy_grpc::Future<::cogment::TrialMessageReply> SendMessage(::cogment::TrialMessageRequest, easy_grpc::Context ctx);

  ::cogment::VersionInfo Version(::cogment::VersionRequest, easy_grpc::Context ctx);
};
}  // namespace cogment

#endif