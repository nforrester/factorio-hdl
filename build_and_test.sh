#!/bin/bash

set -ex

bazel build //src/... //circuits/...
bazel test //src/... //circuits/...

bazel build //target/... --platforms=//target:factorio-riscv
