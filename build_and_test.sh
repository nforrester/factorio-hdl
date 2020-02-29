#!/bin/bash

set -ex

# Implies build, even for things that aren't tests.
bazel test //src/... //circuits/...

bazel build //target/... --platforms=//target:factorio-riscv
