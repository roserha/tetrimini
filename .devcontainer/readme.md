# .devcontainer

zephyr dev container. builds zephyr apps, and `native_sim` renders to a real
SDL window on the desktop.

## files

- `Dockerfile` — the image (apt packages, cmake, ccache, neovim)
- `bootstrap.sh` — fills an empty volume. installed as `bootstrap-workspace`
- `run-container.sh` — plain docker run, no vscode
- `devcontainer.json` — same thing for vscode

## the split

| | holds | survives rebuild? |
|---|---|---|
| image `zephyrcontainer` | apt, cmake, ccache, nvim | no, rebuilt |
| volume `zephyr-workspace` → `/workspace` | zephyr, modules, SDK, venv | **yes** |

rule: **if the zephyr tree decides its version, it goes in the volume.** SDK
(version from `SDK_VERSION`), modules (revisions from the manifest), python
packages (from `west packages pip`).

put any of that in the image and every `docker build` quietly reverts it —
which shows up later as `ModuleNotFoundError: jsonschema` with no hint that a
rebuild caused it. keeping it in the volume makes the image disposable.

named volume instead of a bind mount so nothing zephyr-sized ends up in `~`.

## tarballs, not apt

ubuntu 24.04 ships these too old:

- cmake 3.31.12 — ubuntu has < 3.28, zephyr wants ≥ 3.28
- ccache 4.14 — ubuntu has 4.9.1, zephyr wants ≥ 4.12
- neovim 0.12.5 — ubuntu has 0.9.x, lazyvim wants newer

## don't strip these

- **the i386 stuff** (`gcc-multilib`, `libc6-dev-i386`, `*:i386`) — native_sim
  builds 32-bit on purpose
- **`libsdl2-dev`** — the window
- **`device-tree-compiler`** — SDK ships its own dtc but it's fragile

## setup from scratch

```bash
docker volume create zephyr-workspace
cd .devcontainer && docker build -t zephyrcontainer .
./run-container.sh
```

you'll get `workspace not bootstrapped -- run: bootstrap-workspace`. that's
right, the volume is empty.

```bash
bootstrap-workspace     # 15-30 min, several GB
```

does: venv → pip west → `west init` (instant, just writes `.west/config`) →
`west update` (the slow part) → `west packages pip --install` →
`west sdk install -b /workspace -t arm-zephyr-eabi`.

idempotent, safe to interrupt and rerun.

then check:

```bash
west build -b native_sim zephyr/samples/hello_world
./build/zephyr/zephyr.exe     # Hello World! native_sim/native
rm -rf /workspace/build
```

## daily

`./run-container.sh`, or vscode → reopen in container.

devcontainer.json points at the **prebuilt image**, not the Dockerfile, so it
opens in seconds. run `docker build` yourself after editing the Dockerfile —
the volume is untouched, that's the whole point.

xhost runs automatically in both. it only lasts as long as the X server, so
it's re-run every start.

## gotchas

**the SDK can't be moved.** `west sdk install` bakes absolute ELF interpreter
paths into its binaries. move it and you get `no such file or directory` on a
file that obviously exists. `setup.sh -c` doesn't fix it — that only registers
the cmake package. check with:

```bash
ldd /workspace/zephyr-sdk-*/hosttools/sysroots/x86_64-pokysdk-linux/usr/bin/dtc
```

if the interpreter path isn't the current location, delete and reinstall with
`-b`. no repath option exists. this is why bootstrap.sh passes `-b /workspace`.

**`docker system prune --volumes` nukes the workspace.** plain `prune -a` is
fine — images and cache regenerate. `--volumes` takes named volumes too.

**rm -rf build after toolchain changes.** cmake caches tool paths and you'll
re-debug something you already fixed. the tell: no `Found Dtc:` line in the
configure output.

**`zephyr.exe` is a linux ELF.** zephyr names host output `.exe` and embedded
output `.elf` to signal which one you can run directly. not a windows thing.

## misc

- user is `zephyr`, uid 1000, password `zephyr`, in sudo. matching host uid is
  what makes X and the bind mounts work without permission fights.
- `/workspace` is chowned before `USER zephyr` so docker seeds the empty volume
  with the right ownership on first mount.
- `ZEPHYR_BASE` / `ZEPHYR_SDK_INSTALL_DIR` are set in the Dockerfile instead of
  using `west zephyr-export` — that writes to a cmake registry in `~`, which
  lives in the image and dies on rebuild.