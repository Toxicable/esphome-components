#!/usr/bin/env bash
set -euo pipefail

EXTENSIONS=(
  ms-python.python
  esphome.esphome-vscode
  llvm-vs-code-extensions.vscode-clangd
  openai.chatgpt
)

install_extensions() {
  local cli=""
  if command -v code-server >/dev/null 2>&1; then
    cli="code-server"
  elif command -v code >/dev/null 2>&1; then
    cli="code"
  else
    echo "No VS Code CLI found; relying on devcontainer customizations for extensions."
    return 0
  fi

  for extension in "${EXTENSIONS[@]}"; do
    "$cli" --install-extension "$extension" --force >/dev/null 2>&1 ||
      echo "Warning: failed to install VS Code extension $extension with $cli"
  done
}

fix_volume_owners() {
  mkdir -p \
    /home/vscode/.vscode-server \
    /home/vscode/.local/share/code-server \
    /home/vscode/.config/code-server \
    /home/vscode/.platformio

  if command -v sudo >/dev/null 2>&1; then
    sudo chown -R vscode:vscode \
      /home/vscode/.vscode-server \
      /home/vscode/.local/share/code-server \
      /home/vscode/.config/code-server \
      /home/vscode/.platformio || true
  else
    chown -R vscode:vscode \
      /home/vscode/.vscode-server \
      /home/vscode/.local/share/code-server \
      /home/vscode/.config/code-server \
      /home/vscode/.platformio || true
  fi
}

fix_volume_owners
install_extensions
python --version
esphome version
clangd --version
cmake --version
ninja --version
rg --version
omp --version
