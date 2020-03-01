#!/bin/bash

bazel test --platforms=//target:factorio-riscv "$@"
