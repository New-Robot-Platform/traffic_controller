#!/bin/sh
set -eu

cd "$(dirname "$0")/.."
bazel fetch @asio//...
asio_include="$(bazel info output_base)/external/asio+/asio/include"
test -f "$asio_include/asio.hpp"
ln -sfn "$asio_include" .vscode/asio-include
