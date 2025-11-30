# Neve 1073 Channel Strip VST Plugin

A cross-platform VST3 plugin emulation of the classic Neve 1073 preamp and EQ, built with JUCE.

## Features

- **Input/Output Transformers**: Frequency-dependent saturation modeling with even-order harmonic generation
- **Class A Preamp**: Asymmetric clipping from DC bias point, adjustable saturation
- **3-Band EQ**:
  - Low Shelf: 35Hz, 60Hz, 110Hz, 220Hz (6dB/octave)
  - Mid Peak: 360Hz, 700Hz, 1.6kHz, 3.2kHz, 4.8kHz, 7.2kHz (inductor-based with frequency-dependent Q)
  - High Shelf: 12kHz (6dB/octave)
- **High-Pass Filter**: 18dB/octave at 50Hz, 80Hz, 160Hz, or 300Hz
- **4x Oversampling**: For clean saturation processing

## Building

### Prerequisites

- CMake 3.22+
- C++17 compatible compiler

#### Linux
```bash
sudo apt-get install libasound2-dev libjack-jackd2-dev libfreetype6-dev \
    libx11-dev libxcomposite-dev libxcursor-dev libxext-dev libxinerama-dev \
    libxrandr-dev libxrender-dev libwebkit2gtk-4.0-dev libglu1-mesa-dev \
    mesa-common-dev
```

#### macOS
```bash
xcode-select --install
```

#### Windows
- Visual Studio 2019 or later with C++ workload

### Build Instructions

```bash
# Clone the repository
git clone <repository-url>
cd neve-channel-1073

# Configure
cmake -B build -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build build --config Release
```

### Output Locations

After building:
- **VST3**: `build/Neve1073_artefacts/Release/VST3/`
- **Standalone**: `build/Neve1073_artefacts/Release/Standalone/`

## Installation

### Linux
```bash
cp -r build/Neve1073_artefacts/Release/VST3/*.vst3 ~/.vst3/
```

### macOS
```bash
cp -r build/Neve1073_artefacts/Release/VST3/*.vst3 ~/Library/Audio/Plug-Ins/VST3/
```

### Windows
Copy the `.vst3` folder to:
- `C:\Program Files\Common Files\VST3\`

## Parameters

| Parameter | Range | Description |
|-----------|-------|-------------|
| Input Gain | -20 to +20 dB | Input level |
| Input Drive | 0 to 12 dB | Input transformer saturation |
| Preamp Gain | -10 to +60 dB | Preamp amplification |
| Preamp Bias | 0.35 to 0.65 | DC bias point (affects asymmetry) |
| Low Freq | 35/60/110/220 Hz | Low shelf frequency |
| Low Gain | -16 to +16 dB | Low shelf gain |
| Mid Freq | 360Hz to 7.2kHz | Mid peak frequency |
| Mid Gain | -18 to +18 dB | Mid peak gain |
| High Gain | -16 to +16 dB | High shelf gain (12kHz) |
| HPF Freq | Off/50/80/160/300 Hz | High-pass filter |
| Output Gain | -20 to +20 dB | Output level |
| Output Drive | 0 to 12 dB | Output transformer saturation |
| EQ Enabled | On/Off | Bypass EQ section |

## Signal Flow

```
Input -> Input Transformer -> Preamp -> Low Shelf -> Mid Peak -> High Shelf -> HPF -> Output Transformer -> Output
```

## License

This project is provided as-is for educational purposes.

## Acknowledgments

Based on circuit analysis of the original Neve 1073 design by Rupert Neve (1970).
