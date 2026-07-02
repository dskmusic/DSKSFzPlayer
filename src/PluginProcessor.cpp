#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "PluginEditor.h"

juce::AudioProcessorValueTreeState::ParameterLayout DSKSFzProcessor::createParameters()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    auto makeFloat = [](const juce::String& id, const juce::String& name,
                        float lo, float hi, float def)
    {
        return std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID(id, 1), name,
            juce::NormalisableRange<float>(lo, hi), def);
    };

    auto makeInt = [](const juce::String& id, const juce::String& name,
                      int lo, int hi, int def)
    {
        return std::make_unique<juce::AudioParameterInt>(
            juce::ParameterID(id, 1), name, lo, hi, def);
    };

    auto makeBool = [](const juce::String& id, const juce::String& name, bool def)
    {
        return std::make_unique<juce::AudioParameterBool>(
            juce::ParameterID(id, 1), name, def);
    };

    params.push_back(makeFloat("masterVolume",  "Master Volume",   -60.0f,  6.0f,   0.0f));
    params.push_back(makeFloat("masterPan",     "Master Pan",     -100.0f, 100.0f,  0.0f));

    params.push_back(makeFloat("ampAttack",  "Amp Attack",  0.001f, 10.0f, 0.003f));
    params.push_back(makeFloat("ampDecay",   "Amp Decay",   0.001f, 10.0f, 0.5f));
    params.push_back(makeFloat("ampSustain", "Amp Sustain", 0.0f,  100.0f, 70.0f));
    params.push_back(makeFloat("ampRelease", "Amp Release", 0.001f, 10.0f, 0.5f));

    params.push_back(makeInt  ("filterType",    "Filter Type",    0,   3,        0));
    params.push_back(makeFloat("filterCutoff",  "Cutoff",        20.0f, 20000.0f, 20000.0f));
    params.push_back(makeFloat("filterRes",     "Resonance",      0.1f,   20.0f,   0.707f));
    params.push_back(makeFloat("filterEnvAmt",  "Filter Env",    -1.0f,    1.0f,   0.0f));
    params.push_back(makeFloat("filterAttack",  "Flt Attack",    0.001f, 10.0f,   0.001f));
    params.push_back(makeFloat("filterDecay",   "Flt Decay",     0.001f, 10.0f,   0.1f));
    params.push_back(makeFloat("filterSustain", "Flt Sustain",   0.0f,  100.0f,  100.0f));
    params.push_back(makeFloat("filterRelease", "Flt Release",   0.001f, 10.0f,   0.1f));

    params.push_back(makeFloat("transpose", "Transpose", -24.0f,  24.0f,  0.0f));
    params.push_back(makeFloat("fineTune",  "Fine Tune", -100.0f, 100.0f, 0.0f));

    params.push_back(makeFloat("lfo1Rate",   "LFO1 Rate",   0.1f, 20.0f, 1.0f));
    params.push_back(makeFloat("lfo1Amount", "LFO1 Depth",  0.0f,  1.0f, 0.0f));
    params.push_back(makeInt  ("lfo1Shape",  "LFO1 Shape",  0,     4,    0));
    params.push_back(makeInt  ("lfo1Target", "LFO1 Target", 0,     3,    0));

    params.push_back(makeFloat("lfo2Rate",   "LFO2 Rate",   0.1f, 20.0f, 1.0f));
    params.push_back(makeFloat("lfo2Amount", "LFO2 Depth",  0.0f,  1.0f, 0.0f));
    params.push_back(makeInt  ("lfo2Shape",  "LFO2 Shape",  0,     4,    0));
    params.push_back(makeInt  ("lfo2Target", "LFO2 Target", 0,     3,    0));

    // FX enable buttons: ON = effect active, OFF (default) = bypassed
    params.push_back(makeBool ("driveOn",    "Drive Enable",  false));
    params.push_back(makeFloat("driveAmount","Drive",         0.0f, 1.0f, 0.5f));
    params.push_back(makeFloat("driveMix",   "Drive Mix",     0.0f, 1.0f, 0.35f));

    params.push_back(makeBool ("chorusOn",   "Chorus Enable", false));
    params.push_back(makeFloat("chorusRate", "Chorus Rate",   0.1f, 10.0f, 0.5f));
    params.push_back(makeFloat("chorusDepth","Chorus Depth",  0.0f,  1.0f, 0.02f));
    params.push_back(makeFloat("chorusMix",  "Chorus Mix",    0.0f,  1.0f, 0.4f));

    params.push_back(makeBool ("delayOn",      "Delay Enable",    false));
    params.push_back(makeFloat("delayTime",    "Delay Time (ms)", 1.0f, 2000.0f, 250.0f));
    params.push_back(makeFloat("delayFeedback","Delay Feedback",  0.0f, 0.95f,   0.35f));
    params.push_back(makeFloat("delayMix",     "Delay Mix",       0.0f, 1.0f,    0.3f));

    params.push_back(makeBool ("reverbOn",    "Reverb Enable",  false));
    params.push_back(makeFloat("reverbSize",  "Reverb Size",    0.0f, 1.0f, 0.6f));
    params.push_back(makeFloat("reverbDamping","Reverb Damping",0.0f, 1.0f, 0.4f));
    params.push_back(makeFloat("reverbMix",   "Reverb Mix",     0.0f, 1.0f, 0.3f));

    params.push_back(makeBool ("eqOn",    "EQ Enable",  false));
    params.push_back(makeFloat("eqLow",   "EQ Low",    -12.0f, 12.0f, 0.0f));
    params.push_back(makeFloat("eqMid",   "EQ Mid",    -12.0f, 12.0f, 0.0f));
    params.push_back(makeFloat("eqHigh",  "EQ High",   -12.0f, 12.0f, 0.0f));

    params.push_back(makeInt  ("maxVoices", "Polyphony", 1, 64, 32));

    return { params.begin(), params.end() };
}

DSKSFzProcessor::DSKSFzProcessor()
    : AudioProcessor(BusesProperties()
                     .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      params(*this, nullptr, "DSKSFzParams", createParameters())
{
    juce::PropertiesFile::Options opts;
    opts.applicationName     = "DSKSFzPlayer";
    opts.filenameSuffix      = ".settings";
    opts.folderName          = "DSK SFz player";
    opts.osxLibrarySubFolder = "Application Support";
    appProperties.setStorageParameters(opts);
}

DSKSFzProcessor::~DSKSFzProcessor() {}

void DSKSFzProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    synth.prepare(sampleRate, samplesPerBlock);
    sf2synth.prepare(sampleRate, samplesPerBlock);
    fx.prepare(sampleRate, samplesPerBlock);
    keyboardState.reset();
}

void DSKSFzProcessor::releaseResources()
{
    allNotesOff();
}

void DSKSFzProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                    juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    // Silence output while loading to avoid audio thread / load thread race
    if (isLoadingInstrument.load(std::memory_order_acquire))
    {
        buffer.clear();
        midiMessages.clear();
        return;
    }

    // Process on-screen keyboard MIDI events (adds to the incoming buffer)
    keyboardState.processNextMidiBuffer(midiMessages, 0, buffer.getNumSamples(), true);

    syncParametersToEngine();

    if (currentFormat == InstrumentFormat::SF2)
        sf2synth.processBlock(buffer, midiMessages);
    else
        synth.processBlock(buffer, midiMessages);
    fx.process(buffer);
}

void DSKSFzProcessor::allNotesOff()
{
    synth.allNotesOff();
    sf2synth.allNotesOff();
}

void DSKSFzProcessor::resetRoundRobin()
{
    synth.resetRoundRobin();
    sf2synth.resetRoundRobin();
}

bool DSKSFzProcessor::selectSF2Preset(int presetIndex)
{
    isLoadingInstrument.store(true, std::memory_order_release);
    juce::Thread::sleep(30);
    sf2synth.allNotesOff();
    bool ok = sf2synth.selectPreset(presetIndex);
    isLoadingInstrument.store(false, std::memory_order_release);
    return ok;
}

void DSKSFzProcessor::syncParametersToEngine()
{
    auto getF = [this](const char* id) -> float
    {
        auto* p = params.getRawParameterValue(id);
        return p ? p->load() : 0.0f;
    };
    auto getI = [this](const char* id) -> int
    {
        auto* p = params.getRawParameterValue(id);
        return p ? (int)p->load() : 0;
    };
    auto getB = [this](const char* id) -> bool
    {
        auto* p = params.getRawParameterValue(id);
        return p ? p->load() > 0.5f : false;
    };

    synth.masterVolume    = getF("masterVolume");
    synth.masterPan       = getF("masterPan");
    synth.ampAttack       = getF("ampAttack");
    synth.ampDecay        = getF("ampDecay");
    synth.ampSustain      = getF("ampSustain");
    synth.ampRelease      = getF("ampRelease");
    synth.filterCutoff    = getF("filterCutoff");
    synth.filterResonance = getF("filterRes");
    synth.filterTypeIdx   = getI("filterType");
    synth.filterEnvAmt    = getF("filterEnvAmt");
    synth.fltAttack       = getF("filterAttack");
    synth.fltDecay        = getF("filterDecay");
    synth.fltSustain      = getF("filterSustain");
    synth.fltRelease      = getF("filterRelease");
    synth.globalTranspose = getF("transpose");
    synth.globalTune      = getF("fineTune");
    synth.maxVoices       = getI("maxVoices");

    // El motor SF2 comparte los mismos parámetros globales que SFZSynth (ver
    // SF2Synth.h): vol/pan/transpose/tune/polifonía, ADSR de amplitud, filtro
    // (aplicado en bus) y LFOs — se mantienen sincronizados con los mismos knobs.
    sf2synth.masterVolume    = synth.masterVolume;
    sf2synth.masterPan       = synth.masterPan;
    sf2synth.globalTranspose = synth.globalTranspose;
    sf2synth.globalTune      = synth.globalTune;
    sf2synth.maxVoices       = synth.maxVoices;
    sf2synth.ampAttack       = synth.ampAttack;
    sf2synth.ampDecay        = synth.ampDecay;
    sf2synth.ampSustain      = synth.ampSustain;
    sf2synth.ampRelease      = synth.ampRelease;
    sf2synth.filterCutoff    = synth.filterCutoff;
    sf2synth.filterResonance = synth.filterResonance;
    sf2synth.filterTypeIdx   = synth.filterTypeIdx;
    sf2synth.filterEnvAmt    = synth.filterEnvAmt;
    sf2synth.fltAttack       = synth.fltAttack;
    sf2synth.fltDecay        = synth.fltDecay;
    sf2synth.fltSustain      = synth.fltSustain;
    sf2synth.fltRelease      = synth.fltRelease;

    synth.lfo1.rate   = getF("lfo1Rate");
    synth.lfo1.amount = getF("lfo1Amount");
    synth.lfo1.shape  = static_cast<LFO::Shape>(getI("lfo1Shape"));
    synth.lfo1.target = static_cast<LFO::Target>(getI("lfo1Target"));
    synth.lfo2.rate   = getF("lfo2Rate");
    synth.lfo2.amount = getF("lfo2Amount");
    synth.lfo2.shape  = static_cast<LFO::Shape>(getI("lfo2Shape"));
    synth.lfo2.target = static_cast<LFO::Target>(getI("lfo2Target"));

    // Solo se copian los ajustes (no la fase en curso: cada motor corre su
    // propio LFO de forma independiente, ya que solo uno de los dos procesa
    // audio a la vez según currentFormat).
    sf2synth.lfo1.rate   = synth.lfo1.rate;
    sf2synth.lfo1.amount = synth.lfo1.amount;
    sf2synth.lfo1.shape  = synth.lfo1.shape;
    sf2synth.lfo1.target = synth.lfo1.target;
    sf2synth.lfo2.rate   = synth.lfo2.rate;
    sf2synth.lfo2.amount = synth.lfo2.amount;
    sf2synth.lfo2.shape  = synth.lfo2.shape;
    sf2synth.lfo2.target = synth.lfo2.target;

    // Enable buttons: ON = effect active (bypass = NOT enabled)
    fx.driveBypass  = !getB("driveOn");
    fx.driveAmount  = getF("driveAmount");
    fx.driveMix     = getF("driveMix");

    fx.chorusBypass = !getB("chorusOn");
    fx.chorusRate   = getF("chorusRate");
    fx.chorusDepth  = getF("chorusDepth");
    fx.chorusMix    = getF("chorusMix");

    fx.delayBypass   = !getB("delayOn");
    fx.delayTimeMs   = getF("delayTime");
    fx.delayFeedback = getF("delayFeedback");
    fx.delayMix      = getF("delayMix");

    fx.reverbBypass  = !getB("reverbOn");
    fx.reverbSize    = getF("reverbSize");
    fx.reverbDamping = getF("reverbDamping");
    fx.reverbMix     = getF("reverbMix");

    fx.eqBypass   = !getB("eqOn");
    fx.eqLowGain  = getF("eqLow");
    fx.eqMidGain  = getF("eqMid");
    fx.eqHighGain = getF("eqHigh");
}

bool DSKSFzProcessor::loadInstrument(const juce::File& file)
{
    std::lock_guard<std::mutex> lock(loadMutex);

    // Signal audio thread to output silence during loading
    isLoadingInstrument.store(true, std::memory_order_release);

    // Wait long enough for the audio thread to complete its current block
    juce::Thread::sleep(30);

    allNotesOff();

    bool ok = false;
    if (file.hasFileExtension(".sf2"))
    {
        ok = sf2synth.loadSF2(file);
        if (ok) currentFormat = InstrumentFormat::SF2;
    }
    else
    {
        ok = synth.loadSFZ(file);
        if (ok) currentFormat = InstrumentFormat::SFZ;
    }
    if (ok) currentSFZFile = file;

    isLoadingInstrument.store(false, std::memory_order_release);
    return ok;
}

void DSKSFzProcessor::getStateInformation(juce::MemoryBlock& dest)
{
    auto state = params.copyState();
    state.setProperty("sfzPath", currentSFZFile.getFullPathName(), nullptr);
    if (currentFormat == InstrumentFormat::SF2)
        state.setProperty("sf2PresetIndex", sf2synth.getCurrentPresetIndex(), nullptr);

    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, dest);
}

void DSKSFzProcessor::setStateInformation(const void* data, int size)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, size));
    if (!xml) return;

    auto state = juce::ValueTree::fromXml(*xml);
    if (state.isValid()) params.replaceState(state);

    juce::String sfzPath = state.getProperty("sfzPath", "").toString();
    if (sfzPath.isNotEmpty())
    {
        juce::File sfzFile(sfzPath);
        if (sfzFile.existsAsFile() && loadInstrument(sfzFile))
        {
            int presetIndex = state.getProperty("sf2PresetIndex", -1);
            if (currentFormat == InstrumentFormat::SF2 && presetIndex >= 0)
                selectSF2Preset(presetIndex);
        }
    }
}

juce::AudioProcessorEditor* DSKSFzProcessor::createEditor()
{
    return new DSKSFzEditor(*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DSKSFzProcessor();
}