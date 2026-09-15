#!/usr/bin/env bash
set -euo pipefail

# manage permissions for sdl window simulation for phase 1 of project
xhost +SI:localuser:"$(id -un)" >/dev/null

docker run \
  --rm \
  --privileged \
  --cap-add NET_ADMIN \
  --hostname zephyrserver \
  --name zephyrcontainer \
  -it \
  -v /tftpboot:/tftpboot \
  -v "$(pwd):/home/zephyr/applications" \
  -v /dev:/dev \
  -v "/run/media/$(whoami):/run/media/zephyr" \
  -v ~/.config/nvim:/home/zephyr/.config/nvim \
  -v ~/.local/share/nvim:/home/zephyr/.local/share/nvim \
  -v ~/.local/state/nvim:/home/zephyr/.local/state/nvim \
  -e DISPLAY=$DISPLAY \
  -v /tmp/.X11-unix:/tmp/.X11-unix \
  -v zephyr-workspace:/workspace \
  zephyrcontainer