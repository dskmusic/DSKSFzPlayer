#pragma once
#include <JuceHeader.h>
#include <mutex>
#include "SFZ/SFZSynth.h"
#include "Effects/EffectsChain.h"

class DSKSFzProcessor : public juce::AudioProcessor
{
public:
    DSKSFzProcessor();
    ~DSKSFzProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock  (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool                        hasEditor()    const override { return true; }

    const juce::String getName() const override { return "DSK SFz player"; }
    bool   acceptsMidi()          const override { return true; }
    bool   producesMidi()         const override { return false; }
    bool   isMidiEffect()         const override { return false; }
    double getTailLengthSeconds() const override { return 2.0; }

    int  getNumPrograms()    override { return 1; }
    int  getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName(int) override { return "Default"; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& dest) override;
    void setStateInformation(const void* data, int size) override;

    juce::AudioProcessorValueTreeState params;

    // Piano keyboard state — driven by UI keyboard + incoming MIDI
    juce::MidiKeyboardState keyboardState;

    SFZSynth&     getSynth() { return synth; }
    EffectsChain& getFX()    { return fx; }

    bool       loadSFZ(const juce::File& file);
    juce::File getCurrentSFZFile() const { return currentSFZFile; }

    juce::ApplicationProperties& getAppProperties() { return appProperties; }

    // True while a new instrument is loading — audio thread returns silence
    std::atomic<bool> isLoadingInstrument { false };

private:
    std::mutex   loadMutex;
    SFZSynth     synth;
    EffectsChain fx;
    juce::File   currentSFZFile;

    juce::ApplicationProperties appProperties;

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameters();
    void syncParametersToEngine();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DSKSFzProcessor)
};
