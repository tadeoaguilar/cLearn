# A ready-to-use Linux C++ toolchain with PostgreSQL client libraries.
# Used by `docker compose run --rm dev` (see compose.yaml and docs/00-setup.md).
FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y --no-install-recommends \
        gcc-14 g++-14 cmake ninja-build git ca-certificates curl \
        libpq-dev postgresql-client \
    && rm -rf /var/lib/apt/lists/*

ENV CC=gcc-14 CXX=g++-14
WORKDIR /workspace
CMD ["bash"]
