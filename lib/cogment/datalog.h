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

#ifndef ORCHESTRATOR_DATALOG_H
#define ORCHESTRATOR_DATALOG_H

#include "cogment/actor.h"
#include "cogment/api/datalog.pb.h"
#include "cogment/api/datalog.grpc.pb.h"
#include "cogment/stub_pool.h"

namespace cogment {

class Trial;

class DatalogService {
public:
  virtual ~DatalogService() {}
  virtual void start(const std::string& trial_id, const std::string& user_id,
                     const cogmentAPI::TrialParams& params) = 0;
  virtual void add_sample(cogmentAPI::DatalogSample&& data) = 0;
};

class DatalogServiceNull : public DatalogService {
public:
  void start(const std::string& trial_id, const std::string& user_id, const cogmentAPI::TrialParams& params) override {}
  void add_sample(cogmentAPI::DatalogSample&& data) override {}
};

class DatalogServiceImpl : public DatalogService {
  using StubEntryType = std::shared_ptr<StubPool<cogmentAPI::LogExporterSP>::Entry>;

public:
  DatalogServiceImpl(StubEntryType stub_entry);
  ~DatalogServiceImpl();

  void start(const std::string& trial_id, const std::string& user_id, const cogmentAPI::TrialParams& params) override;
  void add_sample(cogmentAPI::DatalogSample&& data) override;

private:
  StubEntryType m_stub_entry;
  std::unique_ptr<grpc::ClientReaderWriter<cogmentAPI::LogExporterSampleRequest, cogmentAPI::LogExporterSampleReply>>
      m_stream;
  grpc::ClientContext m_context;
};

}  // namespace cogment

#endif
