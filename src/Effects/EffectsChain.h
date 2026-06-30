#pragma once
#include <JuceHeader.h>

// ══════════════════════════════════════════════════════════════════════════════
// EffectsChain — Cadena de efectos master en serie
//   Drive → Chorus → Delay → Reverb → EQ
// Cada efecto tiene bypass y mezcla wet/dry independientes.
// ══════════════════════════════════════════════════════════════════════════════
class EffectsChain
{
public:
    EffectsChain();
    ~EffectsChain() = default;

    void prepare(double sampleRate, int blockSize);
    void process(juce::AudioBuffer<float>& buffer);
    void reset();

    // ── Drive / Saturación ────────────────────────────────────────────────────
    bool  driveBypass  = false;
    float driveAmount  = 0.0f;     // 0.0 (limpio) a 1.0 (máximo)
    float driveMix     = 1.0f;     // mezcla wet/dry

    // ── Chorus ────────────────────────────────────────────────────────────────
    bool  chorusBypass = false;
    float chorusRate   = 0.5f;     // Hz
    float chorusDepth  = 0.02f;    // 0.0-1.0
    float chorusMix    = 0.5f;

    // ── Delay ─────────────────────────────────────────────────────────────────
    bool  delayBypass    = false;
    float delayTimeMs    = 250.0f; // ms
    float delayFeedback  = 0.35f;  // 0.0-0.95
    float delayMix       = 0.3f;

    // ── Reverb ────────────────────────────────────────────────────────────────
    bool  reverbBypass   = false;
    float reverbSize     = 0.5f;   // 0.0-1.0
    float reverbDamping  = 0.5f;
    float reverbMix      = 0.3f;

    // ── EQ de 3 bandas ────────────────────────────────────────────────────────
    bool  eqBypass  = false;
    float eqLowGain = 0.0f;        // dB (-12 a +12)
    float eqMidGain = 0.0f;
    float eqHighGain= 0.0f;
    float eqLowFreq = 200.0f;      // Hz
    float eqMidFreq = 1000.0f;
    float eqHighFreq= 4000.0f;

private:
    double currentSampleRate = 44100.0;
    int    currentBlockSize  = 512;

    // ── Drive interno ─────────────────────────────────────────────────────────
    void processDrive(juce::AudioBuffer<float>& buf);

    // ── Chorus ────────────────────────────────────────────────────────────────
    juce::dsp::Chorus<float> chorus;
    bool chorusPrepared = false;

    // ── Delay: línea de retardo con feedback ──────────────────────────────────
    static const int MAX_DELAY_SAMPLES = 192000; // 2 s a 96 kHz
    std::vector<float> delayBufL, delayBufR;
    int delayWritePos = 0;

    void processDelay(juce::AudioBuffer<float>& buf);

    // ── Reverb ────────────────────────────────────────────────────────────────
    juce::dsp::Reverb reverb;

    // ── EQ ────────────────────────────────────────────────────────────────────
    using IIRCoeffs = juce::dsp::IIR::Coefficients<float>;

    juce::dsp::IIR::Filter<float> eqLowL,  eqLowR;
    juce::dsp::IIR::Filter<float> eqMidL,  eqMidR;
    juce::dsp::IIR::Filter<float> eqHighL, eqHighR;

    void updateEQ();
    void processEQ(juce::AudioBuffer<float>& buf);

    // Caché para evitar recalcular EQ en cada bloque
    float prevEqLow = -999.0f, prevEqMid = -999.0f, prevEqHigh = -999.0f;
};
