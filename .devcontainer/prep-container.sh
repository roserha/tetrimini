#!/usr/bin/env bash
set -euo pipefail

# prep neovim folders for lazyvim!
mkdir -p ~/.config/nvim ~/.local/share/nvim ~/.local/state/nvim
xhost +SI:localuser:"$(id -un)" > /dev/null

# create volume if needed
docker volume inspect zephyr-workspace > /dev/null 2>&1 \
  || docker volume create zephyr-workspace

# build container if needed
if ! docker image inspect zephyrcontainer > /dev/null 2>&1; then
  echo "building zephyrcontainer (first run, a few minutes)..."
  docker build -t zephyrcontainer "$(dirname "$0")"
fi

# removable media for UF2 drag-drop flashing (pico w, feather bootloader).
# fedora uses /run/media/$USER, ubuntu/debian uses /media/$USER.
if [ -d /run/media ]; then
  MEDIA="/run/media/$(id -un)"
else
  MEDIA="/media/$(id -un)"
fi
mkdir -p "$MEDIA"
ln -sfn "$MEDIA" "$HOME/.devcontainer-media"