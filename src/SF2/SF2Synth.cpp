#include <JuceHeader.h>
#include "SF2Synth.h"

#define TSF_IMPLEMENTATION
#include <tsf.h>

// ══════════════════════════════════════════════════════════════════════════════
// SF2Synth
// ══════════════════════════════════════════════════════════════════════════════
SF2Synth::SF2Synth() {}

SF2Synth::~SF2Synth()
{
    unload();
}

bool SF2Synth::loadSF2(const juce::File& sf2File)
{
    unload();

    juce::MemoryBlock data;
    if (!sf2File.loadFileAsData(data))
    {
        lastError = "Could not read the SF2 file: " + sf2File.getFullPathName();
        return false;
    }

    font = tsf_load_memory(data.getData(), (int)data.getSize());
    if (font == nullptr)
    {
        lastError = "Could not parse the SF2 file: " + sf2File.getFullPathName();
        return false;
    }

    const int count = tsf_get_presetcount(font);
    if (count <= 0)
    {
        lastError = "The SF2 file does not contain any preset.";
        tsf_close(font);
        font = nullptr;
        return false;
    }

    // Enumerar (banco, número de programa, nombre) de cada preset.
    // tsf no expone esto directamente por índice, así que se recorre asignando
    // temporalmente cada preset al canal 0 y leyendo de vuelta banco/número.
    presets.clear();
    presets.reserve((size_t)count);
    for (int i = 0; i < count; ++i)
    {
        tsf_channel_set_presetindex(font, 0, i);
        Preset p;
        p.index        = i;
        p.bank         = tsf_channel_get_preset_bank(font, 0);
        p.presetNumber = tsf_channel_get_preset_number(font, 0);
        p.name         = juce::String::fromUTF8(tsf_get_presetname(font, i));
        presets.push_back(p);
    }

    tsf_set_output(font, TSF_STEREO_UNWEAVED, (int)sampleRate, 0.0f);
    tsf_set_max_voices(font, maxVoices);

    loaded = true;
    lastError = {};
    selectPreset(0);
    return true;
}

void SF2Synth::unload()
{
    if (font != nullptr)
    {
        tsf_close(font);
        font = nullptr;
    }
    presets.clear();
    currentPresetIndex = -1;
    loaded = false;
    lastError = {};
}

void SF2Synth::prepare(double sr, int blockSize)
{
    sampleRate = sr;
    renderScratch.assign((size_t)blockSize * 2, 0.0f);
    if (font != nullptr)
        tsf_set_output(font, TSF_STEREO_UNWEAVED, (int)sampleRate, 0.0f);
    allNotesOff();
}

bool SF2Synth::selectPreset(int presetIndex)
{
    if (font == nullptr || presetIndex < 0 || presetIndex >= (int)presets.size())
        return false;

    tsf_channel_set_presetindex(font, 0, presetIndex);
    tsf_channel_set_pitchrange(font, 0, 2.0f); // ±2 semitonos, igual que SFZSynth::pitchBend
    currentPresetIndex = presetIndex;
    return true;
}

void SF2Synth::applyGlobals()
{
    if (font == nullptr) return;
    tsf_set_volume(font, juce::Decibels::decibelsToGain(masterVolume));
    tsf_channel_set_tuning(font, 0, globalTranspose + globalTune / 100.0f);
    tsf_set_max_voices(font, juce::jmax(1, maxVoices));
}

void SF2Synth::renderSegment(juce::AudioBuffer<float>& output, int startSample, int numSamples)
{
    if (numSamples <= 0) return;
    if ((size_t)(numSamples * 2) > renderScratch.size())
        renderScratch.assign((size_t)numSamples * 2, 0.0f);

    tsf_render_float(font, renderScratch.data(), numSamples, 0);

    // TSF_STEREO_UNWEAVED: primera mitad = canal L, segunda mitad = canal R
    const float* srcL = renderScratch.data();
    const float* srcR = renderScratch.data() + numSamples;

    // Pan/volumen master como post-proceso sobre el buffer ya renderizado
    // (misma ley seno usada en SFZVoice para el pan)
    double panRad = juce::jlimit(-1.0, 1.0, (double)masterPan / 100.0)
                  * juce::MathConstants<double>::pi / 4.0;
    float panL = (float)std::cos(panRad);
    float panR = (float)std::sin(panRad + juce::MathConstants<double>::pi / 2.0);

    float* outL = output.getWritePointer(0, startSample);
    float* outR = output.getWritePointer(output.getNumChannels() > 1 ? 1 : 0, startSample);
    for (int i = 0; i < numSamples; ++i)
    {
        outL[i] = srcL[i] * panL;
        if (outR != outL)
            outR[i] = srcR[i] * panR;
    }
}

void SF2Synth::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    buffer.clear();
    if (!loaded || font == nullptr) return;

    applyGlobals();

    const int numSamples = buffer.getNumSamples();
    int samplePos = 0;

    for (const auto meta : midiMessages)
    {
        int eventSample = juce::jlimit(0, numSamples - 1, meta.samplePosition);
        if (eventSample > samplePos)
            renderSegment(buffer, samplePos, eventSample - samplePos);

        auto msg = meta.getMessage();
        if (msg.isNoteOn(true))
            tsf_channel_note_on(font, 0, msg.getNoteNumber(), (float)msg.getVelocity() / 127.0f);
        else if (msg.isNoteOff(true))
            tsf_channel_note_off(font, 0, msg.getNoteNumber());
        else if (msg.isPitchWheel())
            tsf_channel_set_pitchwheel(font, 0, msg.getPitchWheelValue());
        else if (msg.isController())
            tsf_channel_midi_control(font, 0, msg.getControllerNumber(), msg.getControllerValue());

        samplePos = eventSample;
    }
    if (samplePos < numSamples)
        renderSegment(buffer, samplePos, numSamples - samplePos);

    midiMessages.clear();

    if (buffer.getNumChannels() > 0)
        meterLevelL.store(buffer.getMagnitude(0, 0, numSamples));
    if (buffer.getNumChannels() > 1)
        meterLevelR.store(buffer.getMagnitude(1, 0, numSamples));
}

void SF2Synth::allNotesOff()
{
    if (font != nullptr)
        tsf_note_off_all(font);
}
