# VU Meter VST3

A minimal VU meter plugin for Windows. Displays input level as a vertical bar graph with no audio processing — signal passes through unchanged.

## Features

- Vertical bar graph with three colour zones:
  - **Green** — −60 dB to −6 dB
  - **Yellow** — −6 dB to 0 dB
  - **Red** — 0 dB to +6 dB
- White peak-hold line (holds 1.5 s, then decays)
- Optional dB scale on the right side
- Right-click menu with:
  - **Fall Time** — how fast the bar drops (Fast 60 dB/s / Medium 20 dB/s / Slow 10 dB/s / Very Slow 4 dB/s)
  - **Peak Mode** — toggle between decaying peak bar and smoothed RMS bar
  - **Show dB Scale** — show or hide the scale markings
- All settings persist with the DAW project

## Requirements

| Tool | Version |
|---|---|
| CMake | 3.22 or later |
| Visual Studio | 2019 or 2022 (Desktop C++ workload) |
| Internet connection | Required on first build (downloads JUCE) |
| OS | Windows 10 / 11 (64-bit) |

## Build

**1. Configure**

Visual Studio 2019:
```
cmake -B build -G "Visual Studio 16 2019" -A x64
```

Visual Studio 2022:
```
cmake -B build -G "Visual Studio 17 2022" -A x64
```

The first run downloads JUCE 7.0.9 automatically via CMake FetchContent. This takes a minute or two.

**2. Build**

```
cmake --build build --config Release --target VU_Meter_VST3
```

**Output:**
```
build\VU_Meter_artefacts\Release\VST3\VU Meter.vst3\
```

## Install

Copy the `VU Meter.vst3` folder to your system VST3 directory:

```
C:\Program Files\Common Files\VST3\
```

Then rescan plugins in your DAW. In Ableton Live: **Preferences → Plug-ins → Rescan**.

## Project structure

```
VU_Meter_VST/
├── CMakeLists.txt          # Build configuration, fetches JUCE
└── Source/
    ├── PluginProcessor.h   # Metering state and config
    ├── PluginProcessor.cpp # Audio thread: peak, RMS, hold ballistics
    ├── PluginEditor.h      # Editor class declaration
    └── PluginEditor.cpp    # UI: bar graph, scale, right-click menu
```
