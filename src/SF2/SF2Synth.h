#pragma once
#include <JuceHeader.h>
#include <vector>

struct tsf;

// ══════════════════════════════════════════════════════════════════════════════
// SF2Synth — Motor de reproducción SoundFont2 (envuelve TinySoundFont / tsf.h)
//
// Expone deliberadamente la misma forma de API que SFZSynth (loadXXX, prepare,
// processBlock, allNotesOff, resetRoundRobin, isLoaded, getLastError,
// meterLevelL/R, masterVolume/masterPan/globalTranspose/globalTune/maxVoices)
// para que DSKSFzProcessor pueda alternar entre los dos motores con un simple
// if en vez de necesitar una jerarquía de clases.
// ══════════════════════════════════════════════════════════════════════════════
class SF2Synth
{
public:
    struct Preset
    {
        int index;
        int bank;
        int presetNumber;
        juce::String name;
    };

    SF2Synth();
    ~SF2Synth();

    bool loadSF2(const juce::File& sf2File);
    void unload();

    void prepare(double sampleRate, int blockSize);
    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages);

    void allNotesOff();
    void resetRoundRobin() {} // no aplica a SF2

    bool isLoaded() const { return loaded; }
    juce::String getLastError() const { return lastError; }

    // ── Presets (banco/programa) ─────────────────────────────────────────────
    const std::vector<Preset>& getPresets() const { return presets; }
    bool selectPreset(int presetIndex);
    int  getCurrentPresetIndex() const { return currentPresetIndex; }

    // ── Parámetros globales (mismo significado que en SFZSynth) ──────────────
    float masterVolume   = 0.0f;   // dB
    float masterPan      = 0.0f;   // -100..+100
    float globalTranspose= 0.0f;   // semitonos
    float globalTune     = 0.0f;   // centésimas
    int   maxVoices       = 32;

    mutable std::atomic<float> meterLevelL { 0.0f };
    mutable std::atomic<float> meterLevelR { 0.0f };

private:
    tsf* font = nullptr;
    bool loaded = false;
    juce::String lastError;

    std::vector<Preset> presets;
    int currentPresetIndex = -1;

    double sampleRate = 44100.0;
    std::vector<float> renderScratch; // buffer TSF_STEREO_UNWEAVED: [L...][R...]

    void applyGlobals();
    void renderSegment(juce::AudioBuffer<float>& output, int startSample, int numSamples);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SF2Synth)
};
