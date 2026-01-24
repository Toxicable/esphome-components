FROM mcr.microsoft.com/devcontainers/python:3.11-bookworm

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
    && rm -rf /var/lib/apt/lists/*

RUN python -m pip install --no-cache-dir --upgrade pip \
    && python -m pip install --no-cache-dir esphome

ENV IDF_PATH=/opt/esp-idf
ENV IDF_TOOLS_PATH=/opt/esp-idf-tools
RUN git clone --depth 1 --branch v5.1.2 https://github.com/espressif/esp-idf.git ${IDF_PATH} \
    && ${IDF_PATH}/install.sh \
    && printf 'source /opt/esp-idf/export.sh\n' > /etc/profile.d/esp-idf.sh
