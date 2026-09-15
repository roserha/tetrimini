#!/usr/bin/env bash
set -euo pipefail

cd /workspace

if [ ! -d .venv ]; then
  python3 -m venv .venv
fi
# shellcheck disable=SC1091
. .venv/bin/activate
pip install --upgrade pip west

if [ ! -d .west ]; then
  west init -m https://github.com/zephyrproject-rtos/zephyr .
fi
west update
west packages pip --install

# -b sets the install path. the SDK bakes absolute ELF interpreter paths
# into its binaries at install time, so it CANNOT be moved afterward --
# setup.sh -c only re-registers the cmake package, it doesn't repath.
# install it where it will live.
if ! compgen -G "/workspace/zephyr-sdk-*" > /dev/null; then
  west sdk install -b /workspace -t arm-zephyr-eabi
fi

rm -rf /workspace/tmp*
echo "done. try: west build -b native_sim zephyr/samples/hello_world"