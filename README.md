# cogment-orchestrator

[![Retrieve from Docker Hub](https://img.shields.io/docker/v/cogment/orchestrator?sort=semver&style=flat-square)](https://hub.docker.com/repository/docker/cogment/orchestrator) [![Apache 2 License](https://img.shields.io/badge/license-Apache%202-green)](./LICENSE) [![Changelog](https://img.shields.io/badge/-Changelog%20-blueviolet)](./CHANGELOG.md)

[Cogment](https://cogment.ai) is an innovative open source AI platform designed to leverage the advent of AI to benefit humankind through human-AI collaboration developed by [AI Redefined](https://ai-r.com). Cogment enables AI researchers and engineers to build, train and operate AI agents in simulated or real environments shared with humans. For the full user documentation visit <https://docs.cogment.ai>

This module, `cogment-orchestrator`, is the central entity in the framework that ties all the services together. From the perspective of a Cogment user, it can be considered as the live interpreter of the cogment.yaml configuration file. It is the service that client applications will connect to in order to start and run trials.

## Developers

### Building the orchestrator

The orchestrator has a few dependecies:

- googletest
- grpc
- yaml-cpp
- spdlog

The easiest way to build the orchestrator is to make use of [cogment orchestrator build env](https://hub.docker.com/repository/docker/cogment/orchestrator-build-env) Docker image:

```
docker run --rm -it -v$(pwd):/workspace cogment/orchestrator-build-env:v2.0.0
mkdir _bld
cd _bld
cmake ..
make
```

### Used Cogment protobuf API

The version of the used cogment protobuf API is defined in the `.cogment-api.yml` file at the root of the repository.

To use a **published version**, set `cogment_api_version`, by default it retrieved the _latest_ version of cogment api, i.e. the one on the `develop` branch for cogment-api.

To use a **local version**, set `cogment_api_path` to the absolute path to the local cogment-api directory. When set, this local path overrides any `cogment_api_version` set.

> ⚠️ when building in docker you need to mount the path to cogment api in the docker image. you can for example add `-v$(pwd)/../cogment-api/:/workspace/cogment-api` to the above `docker run` command line.

> For a change to be taken into account, you'll need to re-run `cmake`.

> ⚠️ Due to the limited parsing capabilities of CMake, commented out fully defined variables won't be ignored...

### Auto-formatting code

The following will apply clang-format to all included source, with the exception of the third_party directory:

```
make format
```

### Release process

People having mainteners rights of the repository can follow these steps to release a version **MAJOR.MINOR.PATCH**. The versioning scheme follows [Semantic Versioning](http://semver.org/spec/v2.0.0.html).

1. Run `./scripts/create_release_branch.sh MAJOR.MINOR.PATCH` to create the release branch and update the version of the package,
2. On the release branch, check and update the changelog if needed,
3. Update .cogment-api.yaml to the latest version of cogment-api
4. Make sure everything's fine on CI,
5. Run `./scripts/tag_release.sh MAJOR.MINOR.PATCH` to create the specific version section in the changelog, merge the release branch in `main`, create the release tag and update the `develop` branch with those.

The rest, publishing the package to dockerhub and updating the mirror repositories, is handled directly by the CI.
