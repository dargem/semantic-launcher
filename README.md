# Semantic Launcher

A modern Wayland-native application launcher for Linux (primarily Arch Linux) that combines traditional fuzzy matching with semantic vector search to find and launch applications based on meaning and intent.

## Features

- **Semantic Search**: Powered by a local LLM embedding model (via `llama.cpp`) and a high-performance vector database (`usearch`). Search by intent or description—for example, typing _"web browser"_ will find Firefox or Chrome, and _"text editor"_ will find Micro.
- **Fuzzy Matching**: Integrated with `rapidfuzz` for instantaneous, traditional fuzzy matching when you know the exact name of the application.
- **Wayland Native**: Built with Qt6 and `LayerShellQt` to run as a lightweight, high-performance Wayland overlay window.
- **Multi-Source Aggregation**:
  - **Pacman**: Discovers explicitly installed packages on Arch Linux.
  - **Desktop Entries**: Parses standard system and user `.desktop` files.
  - **AppImages**: Scans your home directory for `.appimage` files and indexes them.
- **TUI Application Launching**: Supports launching TUI applications through a new terminal.

## Prerequisites

To build and run Semantic Launcher, you need:

- **Linux OS** (Wayland compositor with Layer Shell support, e.g., Hyprland, Sway)
- **Qt6** (Quick, Core, Gui)
- **LayerShellQt**
- **CMake** (version 3.21 or higher)
- **A GGUF Embedding Model**: You must place a GGUF-formatted embedding model (e.g., `nomic-embed-text` or `bge-micro`) named `model.gguf` inside the `models/` directory.

## Getting Started

### 1. Clone the Repository

```bash
git clone https://github.com/dargem/semantic-launcher.git
cd semantic-launcher
```

### 2. Download and set Embedding Model

Download a GGUF embedding model and place it in the `models/` directory:

```bash
mkdir -p models
# Example: Download a small, fast embedding model
wget -O models/model.gguf https://huggingface.co/second-state/BGE-Micro-v2-GGUF/resolve/main/bge-micro-v2-Q8_0.gguf
```

### 3. Build the Project

```bash
mkdir build
cd build
cmake ..
make -j$(nproc)
```

## Configuration

You can customize the launcher's behavior by editing `src/configs.hpp`. Key configuration options include:

- `MODEL_NAME`: The filename of your GGUF model inside the `models/` directory.
- `SEMANTIC_ACCEPTED_K` & `SEMANTIC_ACCEPTED_SCORE`: Control how many semantic results are returned and their minimum similarity threshold.
- `FUZZY_ACCEPTED_K` & `FUZZY_ACCEPTED_SCORE`: Control how many fuzzy results are returned and their minimum score threshold.
- `TERMINALS`: A prioritized list of terminal emulators to check for on startup when launching terminal-based applications.

## Running the Launcher

Run the compiled executable from the build directory:

```bash
./semantic-launcher
```

For the best experience, bind the launcher to a global hotkey in your Wayland compositor configuration (e.g., in `hyprland.conf` or `sway/config`).

## Caveats

This launcher while compatible with any machine that meets the prerequisites is not necessarily as useful in all of them.
This project was built for my machine which runs Arch Linux, so it supports indexing Arch's package manager (pacman).
PR's are welcome to add indexing of other package managers.

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.
