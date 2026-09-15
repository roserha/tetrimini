FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

# resolving cmake being below 3.28 as a default by wgetting it
RUN apt-get update && apt-get install -y --no-install-recommends \
        ca-certificates wget && \
        rm -rf /var/lib/apt/lists/*

# download cmake
RUN wget -qO- "https://github.com/Kitware/CMake/releases/download/v3.31.12/cmake-3.31.12-linux-x86_64.tar.gz" \
        | tar --strip-components=1 -xz -C /usr/local/

# download neovim for lazyvim
RUN wget -qO- "https://github.com/neovim/neovim/releases/download/v0.12.5/nvim-linux-x86_64.tar.gz" \
        | tar --strip-components=1 -xz -C /usr/local/

# and downlaod ccache
RUN wget -qO- "https://github.com/ccache/ccache/releases/download/v4.14/ccache-4.14-linux-x86_64-glibc.tar.gz" \
        | tar --strip-components=1 -xz -C /usr/local/bin --wildcards '*/ccache'

# install dependencies
RUN \
        dpkg --add-architecture i386 && \
        apt-get update && \
        apt-get install -yq --no-install-recommends sudo build-essential git \
          libsdl2-dev libsdl2-dev:i386 \
          python3-yaml libncursesw6 libncursesw6:i386 \
          python3 bash libncurses-dev locales \
          libc6-dev-i386 libssl-dev \
          g++-multilib libssl-dev:i386 zlib1g-dev:i386 \
          picocom \
          ripgrep fd-find fzf \
          zstd \
          curl libtinfo6 unzip zip \
          ninja-build gperf device-tree-compiler \
          python3-dev python3-venv xz-utils file make \
          gcc gcc-multilib libsdl2-dev libmagic1 && \
        rm -rf /var/lib/apt/lists/* && \
        echo "dash dash/sh boolean false" | debconf-set-selections && \
        dpkg-reconfigure dash

# set up zephyr user
RUN usermod -l zephyr -d /home/zephyr -m ubuntu && \
    groupmod -n zephyr ubuntu && \
    usermod -aG sudo,dialout,plugdev zephyr && \
    echo "zephyr:zephyr" | chpasswd

RUN chown -R zephyr:zephyr /home/zephyr

RUN echo "en_US.UTF-8 UTF-8" > /etc/locale.gen && \
    locale-gen

# let's link the zephyr volume via this workspace and make sure zephyruser owns it!
RUN mkdir -p /workspace && chown zephyr:zephyr /workspace

# and run our bootstrap file to set volume up and stuff
COPY --chown=zephyr:zephyr bootstrap.sh /usr/local/bin/bootstrap-workspace
RUN chmod +x /usr/local/bin/bootstrap-workspace

USER zephyr
WORKDIR /home/zephyr
RUN git config --global user.email "rorha04@gmail.com" && git config --global user.name "Rose Rodrigues"

# activate venv!
RUN printf '%s\n' \
    'if [ -f /workspace/.venv/bin/activate ]; then' \
    '  . /workspace/.venv/bin/activate' \
    'else' \
    '  echo "workspace not bootstrapped -- run: bootstrap-workspace"' \
    'fi' \
    'cd /workspace/applications/tetrimini' >> /home/zephyr/.bashrc

# Add the future venv binaries directly to the system PATH.
# This permanently activates the virtual environment for all subsequent RUN commands.
ENV PATH="/workspace/.venv/bin:$PATH"

# Expose toolchain paths to the environment natively
ENV ZEPHYR_TOOLCHAIN_VARIANT=zephyr
ENV ZEPHYR_BASE=/workspace/zephyr
ENV ZEPHYR_SDK_INSTALL_DIR=/workspace/zephyr-sdk-1.0.1
ENV HOME=/home/zephyr

ENV LANG=en_US.utf8

WORKDIR /workspace