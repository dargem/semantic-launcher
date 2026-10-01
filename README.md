# Semantic Launcher

A modern Wayland-native application launcher for Linux (primarily Arch Linux) that combines traditional fuzzy matching with semantic vector search to find and launch applications based on meaning and intent.

## Features

- **Semantic Search**: Powered by a local LLM embedding model (via `llama.cpp`) and a high-performance vector database (`usearch`). Search by intent or description—for example, typing _"web browser"_ will find Firefox or Chrome, and _"text editor"_ will find Micro.
- **Fuzzy Matching**: Integrated with `rapidfuzz` for instantaneous, traditional fuzzy matching when you know the exact name of the application.
- **Wayland Native**: Built with Qt6 and `LayerShellQt` to run as a lightweight, high-performance Wayland overlay window.
- **Multi-Source Aggregation**:
  - **Desktop Entries**: Parses standard system and user `.desktop` files.
  - **AppImages**: Scans your home directory for `.appimage` files and indexes them.
  - **Pacman**: Discovers explicitly installed packages if using the Pacman package manager.
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
# Example, this is a small, fast embedding model
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

### Standalone Mode

Run the compiled executable directly from the build directory or installation path:

```bash
./build/semantic-launcher
```

### Daemon Mode & Hyprland Integration

Running as a background daemon indexes all applications and models on startup once. Subsequent toggle commands will open and close the launcher instantly without startup or model-loading latency.

#### 1. CLI Commands

- `semantic-launcher --daemon` (or `-d`): Starts the daemon in the background with the UI hidden.
- `semantic-launcher --toggle` (or `-t`): Toggles UI visibility on the running daemon. (Running `semantic-launcher` with no flags will also toggle the running daemon).
- `semantic-launcher --show`: Shows the launcher UI.
- `semantic-launcher --hide`: Hides the launcher UI.
- `semantic-launcher --status` (or `-s`): Checks if the daemon is currently running.
- `semantic-launcher --quit` (or `-q`): Gracefully stops the daemon.
- `pkill -USR1 semantic-launcher`: Toggles the launcher via POSIX signal.

#### 2. Systemd User Service Setup

Copy the service file to your systemd user configuration directory:

```bash
mkdir -p ~/.config/systemd/user
cp semantic-launcher.service ~/.config/systemd/user/
systemctl --user daemon-reload
systemctl --user enable --now semantic-launcher.service
```

> **Note**: If `semantic-launcher` is installed in a custom directory (e.g. `~/.local/bin` or your build path), adjust `ExecStart` in `~/.config/systemd/user/semantic-launcher.service`.

#### 3. Hyprland Configuration (`hyprland.lua`)

Add the following to your `hyprland.lua`:

```lua
-- Keybinding to toggle the launcher
hl.bind(mainMod .. " + SPACE", hl.dsp.exec_cmd("semantic-launcher --toggle"))

-- (Optional) If not using systemd, start the daemon with Hyprland:
-- hl.on("hyprland.start", function()
--     hl.exec_cmd("semantic-launcher --daemon")
-- end)

-- (Optional) Layer rules for blur and smooth animations
hl.layer_rule({
    match = { namespace = "semantic-launcher" },
    blur = true,
    ignore_alpha = 0.5,
    animation = "popin 80%",
})
```

## Caveats

This launcher while compatible with any machine that meets the prerequisites is not necessarily as useful in all of them.
This project was built for my machine which runs Arch Linux, so it supports indexing Arch's package manager (pacman).
PR's are welcome to add indexing of other package managers.

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.
