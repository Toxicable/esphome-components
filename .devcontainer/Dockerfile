FROM mcr.microsoft.com/devcontainers/python:3.11-bookworm

# Remove Yarn repo to avoid expired key during apt-get update; not needed here.
RUN rm -f /etc/apt/sources.list.d/yarn.list \
    && apt-get update \
    && apt-get install -y --no-install-recommends \
    clang-format \
    clangd \
    && rm -rf /var/lib/apt/lists/*

RUN python -m pip install --no-cache-dir --upgrade pip \
    && python -m pip install --no-cache-dir esphome
