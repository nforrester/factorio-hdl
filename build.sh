#!/bin/bash

bazel build --platforms=//target:factorio-riscv "$@"
