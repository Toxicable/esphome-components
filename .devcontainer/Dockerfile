FROM mcr.microsoft.com/devcontainers/python:3.11-bookworm

ARG ESPHOME_VERSION=2026.5.3
ARG OMP_VERSION=v15.3.2

# Remove Yarn repo to avoid expired key during apt-get update; not needed here.
RUN rm -f /etc/apt/sources.list.d/yarn.list \
    && apt-get update \
    && apt-get install -y --no-install-recommends \
    clang-format \
    clangd \
    git \
    wget \
    flex \
    bison \
    gperf \
    cmake \
    ninja-build \
    ccache \
    libffi-dev \
    libssl-dev \
    dfu-util \
    libusb-1.0-0 \
    ripgrep \
    && rm -rf /var/lib/apt/lists/*

RUN python -m pip install --no-cache-dir --upgrade pip \
    && python -m pip install --no-cache-dir "esphome==${ESPHOME_VERSION}" pypdf

ENV IDF_PATH=/opt/esp-idf
ENV IDF_TOOLS_PATH=/opt/esp-idf-tools
RUN git clone --depth 1 --branch v5.1.2 https://github.com/espressif/esp-idf.git ${IDF_PATH} \
    && ${IDF_PATH}/install.sh \
    && printf 'source /opt/esp-idf/export.sh\n' > /etc/profile.d/esp-idf.sh \
    && . ${IDF_PATH}/export.sh \
    && python -m pip install --no-cache-dir pypdf


RUN set -eux; \
    apt-get update; \
    apt-get install -y --no-install-recommends curl ca-certificates; \
    rm -rf /var/lib/apt/lists/*; \
    export PI_INSTALL_DIR=/usr/local/bin; \
    curl -fsSL "https://raw.githubusercontent.com/can1357/oh-my-pi/${OMP_VERSION}/scripts/install.sh" \
      | sh -s -- --binary --ref "${OMP_VERSION}"; \
    command -v omp; \
    omp --version
