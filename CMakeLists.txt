cmake_minimum_required(VERSION 3.22)
project(DSKSFzPlayer VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ──────────────────────────────────────────────────────────────────────────────
# Descarga automática de JUCE vía FetchContent
# ──────────────────────────────────────────────────────────────────────────────
include(FetchContent)

FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        7.0.12
    GIT_SHALLOW    TRUE
    GIT_PROGRESS   TRUE
)

FetchContent_MakeAvailable(JUCE)

# ──────────────────────────────────────────────────────────────────────────────
# Definición del plugin
# ──────────────────────────────────────────────────────────────────────────────
juce_add_plugin(DSKSFzPlayer
    COMPANY_NAME                "DSK"
    PLUGIN_MANUFACTURER_CODE    Dsk1
    PLUGIN_CODE                 Dsfz
    FORMATS                     VST3 Standalone
    PRODUCT_NAME                "DSK SFz player"
    IS_SYNTH                    TRUE
    NEEDS_MIDI_INPUT            TRUE
    NEEDS_MIDI_OUTPUT           FALSE
    IS_MIDI_EFFECT              FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD     FALSE
    VST3_CATEGORIES             "Instrument|Synth|Sampler"
    # ── App icon: replace Resources/icon.png with your own 256x256 PNG ──────
    ICON_BIG   "${CMAKE_CURRENT_SOURCE_DIR}/Resources/icon.png"
    ICON_SMALL "${CMAKE_CURRENT_SOURCE_DIR}/Resources/icon.png"
)

# Genera JuceHeader.h con todos los módulos enlazados
juce_generate_juce_header(DSKSFzPlayer)

# ──────────────────────────────────────────────────────────────────────────────
# Archivos fuente
# ──────────────────────────────────────────────────────────────────────────────
target_sources(DSKSFzPlayer
    PRIVATE
        src/PluginProcessor.cpp
        src/PluginEditor.cpp
        src/SFZ/SFZParser.cpp
        src/SFZ/SFZSynth.cpp
        src/SF2/SF2Synth.cpp
        src/Effects/EffectsChain.cpp
        src/UI/LibraryPanel.cpp
        src/UI/SF2PresetPicker.cpp
        src/UI/RemoteBanner.cpp
)

target_include_directories(DSKSFzPlayer
    PRIVATE
        src
        # TinySoundFont (tsf.h) vendorizado directamente en el repo:
        # librería de un único header, no requiere descarga por CMake.
        "${CMAKE_CURRENT_SOURCE_DIR}/libs/tinysoundfont"
)

# ──────────────────────────────────────────────────────────────────────────────
# Definiciones de compilación
# ──────────────────────────────────────────────────────────────────────────────
target_compile_definitions(DSKSFzPlayer
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_DISPLAY_SPLASH_SCREEN=0
        JUCE_USE_OGGVORBIS=1
        JUCE_USE_FLAC=1
        JUCE_REPORT_APP_USAGE=0
        JUCE_MODAL_LOOPS_PERMITTED=1
)

# ──────────────────────────────────────────────────────────────────────────────
# Módulos JUCE necesarios
# ──────────────────────────────────────────────────────────────────────────────
target_link_libraries(DSKSFzPlayer
    PRIVATE
        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_audio_formats
        juce::juce_audio_plugin_client
        juce::juce_audio_processors
        juce::juce_audio_utils
        juce::juce_core
        juce::juce_dsp
        juce::juce_events
        juce::juce_graphics
        juce::juce_gui_basics
        juce::juce_gui_extra
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_warning_flags
)


juce_add_binary_data(MyResources
    HEADER_NAME "BinaryData.h"
    SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/Resources/banner.png"
        "${CMAKE_CURRENT_SOURCE_DIR}/Resources/manual.md"
)
target_link_libraries(DSKSFzPlayer PRIVATE MyResources)

# ──────────────────────────────────────────────────────────────────────────────
# Copy manual.md alongside built artefacts
# ──────────────────────────────────────────────────────────────────────────────
add_custom_command(TARGET DSKSFzPlayer_Standalone POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        "${CMAKE_CURRENT_SOURCE_DIR}/Resources/manual.md"
        "$<TARGET_FILE_DIR:DSKSFzPlayer_Standalone>/manual.md"
    COMMAND ${CMAKE_COMMAND} -E copy
        "${CMAKE_CURRENT_SOURCE_DIR}/Resources/install_notes.md"
        "$<TARGET_FILE_DIR:DSKSFzPlayer_Standalone>/install_notes.md"
    COMMENT "Copying docs alongside Standalone executable"
)

add_custom_command(TARGET DSKSFzPlayer_VST3 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        "${CMAKE_CURRENT_SOURCE_DIR}/Resources/manual.md"
        "${CMAKE_CURRENT_BINARY_DIR}/DSKSFzPlayer_artefacts/$<CONFIG>/VST3/manual.md"
    COMMAND ${CMAKE_COMMAND} -E copy
        "${CMAKE_CURRENT_SOURCE_DIR}/Resources/install_notes.md"
        "${CMAKE_CURRENT_BINARY_DIR}/DSKSFzPlayer_artefacts/$<CONFIG>/VST3/install_notes.md"
    COMMENT "Copying docs alongside VST3 bundle"
)