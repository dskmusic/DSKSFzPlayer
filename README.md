# DSK SFz Player

![Version 1.0](https://img.shields.io/badge/version-1.0-blue)
![Platform](https://img.shields.io/badge/platform-VST3%20%7C%20Standalone-green)

**DSK SFz Player** is a high-performance, 64-voice polyphonic SFZ/SF2 sampler. Designed for music producers and sound designers, it provides a seamless workflow for loading SFZ and SoundFont2 (SF2) instrument libraries with full MIDI control, flexible modulation, and professional-grade effects.


<div align="center">
  <img src="screenshots/screenshot01.png" alt="DSK SFz Player Interface" width="600"/>
</div>

---

## 🚀 Key Features

* **Polyphony:** 64-voice polyphonic engine for rich, layered soundscapes.
* **Versatile Formats:** Works as a **VST3 Plugin** or as a **Standalone Application**.
* **Dual Sampler Engine:** Loads both **SFZ** libraries and **SF2 (SoundFont2)** files with the same Library, Favorites, and drag & drop workflow.
* **Audio Support:** Compatible with WAV, FLAC, and OGG audio formats inside SFZ files.
* **Drag & Drop Workflow:** Effortlessly load `.sfz`, `.sf2` files, folders, or `.zip` packs directly into the plugin.
* **SF2 Bank/Preset Picker:** Multi-bank SoundFont files automatically show a searchable bank/preset selector — reachable at any time by clicking the instrument name in the header.
* **Sound Sculpting:**
    * Amplitude & Filter envelopes, two versatile LFOs — available for both SFZ and SF2, replacing each preset's own envelope/filter with the plugin's knobs.
    * Five built-in effects, available for both SFZ and SF2.
* **Automation:** Real-time parameter automation for dynamic performances.
* **Intuitive Library Management:** Effortlessly organize, browse, and manage your growing collection of instrument libraries.
* **Favorites System:** Quickly tag and access your most-used instruments for instant recall.
* **Customizable Interface:** Fully resizeable window with multiple theme options and tooltips.


<div align="center">
  <img src="screenshots/dsk_sfz_player_themes.png" alt="DSK SFz Player themes" width="600"/>
</div>

---

## 🎹 Quick Start

### Loading Instruments
Getting started with DSK SFz Player is intuitive:

1.  **Drag & Drop:** Simply drag an `.sfz` or `.sf2` file, a folder containing instruments, or a `.zip` library pack onto the plugin window.
2.  **Open Button:** Use the "Open SFZ/SF2/ZIP" button to browse your local files.
3.  **Library Tree:** Organize your collections using the built-in Library panel. Double-click any entry to load it immediately.
4.  **SF2 Presets:** If an SF2 file has more than one bank/preset, a picker appears automatically — click the instrument name in the header to reopen it anytime.

### Playability
* **MIDI Ready:** Connect your favorite MIDI controller or use your PC keyboard to trigger notes.
* **Parameter Control:** Right-click on any knob to enter precise values or reset parameters.
* **Global Config:** Export and import your library paths, favorites, and theme settings as `.json` files.

---

## 🛠 Tech Stack
Built with C++ and the [JUCE Framework](https://juce.com/).

---

## 📦 Installation & System Requirements
* **Formats:** VST3, Standalone.
* **Operating Systems:** macOS, Windows, Linux.
* *Check the latest releases for binary downloads.*

---

## 📄 Documentation
For a detailed guide on all features, MIDI mapping, and configuration, please refer to the `manual.md` file included in this repository.

---

## Downloads
[Click here to download the latest version](https://github.com/dskmusic/DSKSFzPlayer/releases/latest)
---


## 🔗 Links
* **Official Website:** [www.dskmusic.com](http://www.dskmusic.com)

---
*Developed by DSK Music.*

*Made with ❤️ in Gran Canaria, Spain.*
