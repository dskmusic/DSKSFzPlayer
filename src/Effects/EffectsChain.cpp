#include <JuceHeader.h>
#include "EffectsChain.h"

EffectsChain::EffectsChain()
{
    delayBufL.resize(MAX_DELAY_SAMPLES, 0.0f);
    delayBufR.resize(MAX_DELAY_SAMPLES, 0.0f);
}

// ──────────────────────────────────────────────────────────────────────────────
// prepare(): inicializar todos los efectos con el sample rate del host
// ──────────────────────────────────────────────────────────────────────────────
void EffectsChain::prepare(double sampleRate, int blockSize)
{
    currentSampleRate = sampleRate;
    currentBlockSize  = blockSize;

    juce::dsp::ProcessSpec spec;
    spec.sampleRate       = sampleRate;
    spec.maximumBlockSize = (juce::uint32)blockSize;
    spec.numChannels      = 2;

    // Chorus
    chorus.prepare(spec);
    chorus.setRate     (chorusRate);
    chorus.setDepth    (chorusDepth);
    chorus.setCentreDelay(7.0f);
    chorus.setFeedback (0.0f);
    chorus.setMix      (chorusMix);
    chorusPrepared = true;

    // Reverb
    reverb.prepare(spec);
    juce::dsp::Reverb::Parameters rp;
    rp.roomSize   = reverbSize;
    rp.damping    = reverbDamping;
    rp.wetLevel   = 0.0f;         // controlamos mezcla manualmente
    rp.dryLevel   = 1.0f;
    rp.width      = 1.0f;
    reverb.setParameters(rp);

    // EQ
    eqLowL .prepare(spec);  eqLowR .prepare(spec);
    eqMidL .prepare(spec);  eqMidR .prepare(spec);
    eqHighL.prepare(spec);  eqHighR.prepare(spec);
    updateEQ();

    // Reiniciar buffer de delay
    std::fill(delayBufL.begin(), delayBufL.end(), 0.0f);
    std::fill(delayBufR.begin(), delayBufR.end(), 0.0f);
    delayWritePos = 0;
}

void EffectsChain::reset()
{
    std::fill(delayBufL.begin(), delayBufL.end(), 0.0f);
    std::fill(delayBufR.begin(), delayBufR.end(), 0.0f);
    delayWritePos = 0;
    chorus.reset();
    reverb.reset();
}

// ──────────────────────────────────────────────────────────────────────────────
// process(): aplica la cadena completa de efectos al buffer
// ──────────────────────────────────────────────────────────────────────────────
void EffectsChain::process(juce::AudioBuffer<float>& buffer)
{
    if (buffer.getNumSamples() == 0) return;

    if (!driveBypass  && driveAmount  > 0.001f) processDrive(buffer);
    if (!chorusBypass && chorusMix    > 0.001f)
    {
        // Actualizar parámetros del chorus si cambiaron
        chorus.setRate (chorusRate);
        chorus.setDepth(chorusDepth);
        chorus.setMix  (chorusMix);

        juce::dsp::AudioBlock<float> block(buffer);
        juce::dsp::ProcessContextReplacing<float> ctx(block);
        chorus.process(ctx);
    }
    if (!delayBypass  && delayMix     > 0.001f) processDelay(buffer);
    if (!reverbBypass && reverbMix    > 0.001f)
    {
        // Actualizar parámetros de reverb
        juce::dsp::Reverb::Parameters rp;
        rp.roomSize   = reverbSize;
        rp.damping    = reverbDamping;
        rp.wetLevel   = reverbMix;
        rp.dryLevel   = 1.0f - reverbMix;
        rp.width      = 1.0f;
        reverb.setParameters(rp);

        juce::dsp::AudioBlock<float> block(buffer);
        juce::dsp::ProcessContextReplacing<float> ctx(block);
        reverb.process(ctx);
    }
    if (!eqBypass) processEQ(buffer);
}

// ──────────────────────────────────────────────────────────────────────────────
// processDrive(): saturación suave con tanh (función sigmoide)
// ──────────────────────────────────────────────────────────────────────────────
void EffectsChain::processDrive(juce::AudioBuffer<float>& buf)
{
    float driveGain = 1.0f + driveAmount * 15.0f;  // 1x a 16x

    for (int ch = 0; ch < buf.getNumChannels(); ++ch)
    {
        float* data = buf.getWritePointer(ch);
        for (int i = 0; i < buf.getNumSamples(); ++i)
        {
            float dry = data[i];
            float wet = std::tanh(dry * driveGain) / std::tanh(driveGain);
            data[i] = dry * (1.0f - driveMix) + wet * driveMix;
        }
    }
}

// ──────────────────────────────────────────────────────────────────────────────
// processDelay(): delay estéreo con feedback
// ──────────────────────────────────────────────────────────────────────────────
void EffectsChain::processDelay(juce::AudioBuffer<float>& buf)
{
    int delaySamples = juce::jlimit(1, MAX_DELAY_SAMPLES - 1,
        (int)(delayTimeMs * 0.001 * currentSampleRate));

    float* L = buf.getWritePointer(0);
    float* R = buf.getWritePointer(buf.getNumChannels() > 1 ? 1 : 0);
    int    n = buf.getNumSamples();

    for (int i = 0; i < n; ++i)
    {
        // Leer de la línea de delay
        int readPos = (delayWritePos - delaySamples + MAX_DELAY_SAMPLES) % MAX_DELAY_SAMPLES;
        float dL = delayBufL[readPos];
        float dR = delayBufR[readPos];

        // Escribir (dry + feedback) en la línea de delay
        delayBufL[delayWritePos] = L[i] + dL * delayFeedback;
        delayBufR[delayWritePos] = R[i] + dR * delayFeedback;

        // Mezclar wet/dry
        L[i] = L[i] * (1.0f - delayMix) + dL * delayMix;
        R[i] = R[i] * (1.0f - delayMix) + dR * delayMix;

        delayWritePos = (delayWritePos + 1) % MAX_DELAY_SAMPLES;
    }
}

// ──────────────────────────────────────────────────────────────────────────────
// updateEQ(): recalcula coeficientes de los filtros EQ
// ──────────────────────────────────────────────────────────────────────────────
void EffectsChain::updateEQ()
{
    float sr = (float)currentSampleRate;

    // Low shelf
    auto lowCoeffs = IIRCoeffs::makeLowShelf(sr, eqLowFreq, 0.707f,
                                              juce::Decibels::decibelsToGain(eqLowGain));
    *eqLowL.coefficients = *lowCoeffs;
    *eqLowR.coefficients = *lowCoeffs;

    // Peak mid
    auto midCoeffs = IIRCoeffs::makePeakFilter(sr, eqMidFreq, 0.707f,
                                                juce::Decibels::decibelsToGain(eqMidGain));
    *eqMidL.coefficients = *midCoeffs;
    *eqMidR.coefficients = *midCoeffs;

    // High shelf
    auto highCoeffs = IIRCoeffs::makeHighShelf(sr, eqHighFreq, 0.707f,
                                                juce::Decibels::decibelsToGain(eqHighGain));
    *eqHighL.coefficients = *highCoeffs;
    *eqHighR.coefficients = *highCoeffs;

    prevEqLow  = eqLowGain;
    prevEqMid  = eqMidGain;
    prevEqHigh = eqHighGain;
}

// ──────────────────────────────────────────────────────────────────────────────
// processEQ(): aplica EQ de 3 bandas sample a sample
// ──────────────────────────────────────────────────────────────────────────────
void EffectsChain::processEQ(juce::AudioBuffer<float>& buf)
{
    // Recalcular solo si cambiaron los valores
    if (std::abs(eqLowGain  - prevEqLow)  > 0.01f ||
        std::abs(eqMidGain  - prevEqMid)  > 0.01f ||
        std::abs(eqHighGain - prevEqHigh) > 0.01f)
        updateEQ();

    // Si todos los gains son 0, no procesar
    if (std::abs(eqLowGain) < 0.01f &&
        std::abs(eqMidGain) < 0.01f &&
        std::abs(eqHighGain)< 0.01f) return;

    int nc = buf.getNumChannels();
    int ns = buf.getNumSamples();

    // Canal L (siempre existe)
    {
        float* d = buf.getWritePointer(0);
        for (int i = 0; i < ns; ++i)
        {
            float s = d[i];
            if (std::abs(eqLowGain)  > 0.01f) s = eqLowL .processSample(s);
            if (std::abs(eqMidGain)  > 0.01f) s = eqMidL .processSample(s);
            if (std::abs(eqHighGain) > 0.01f) s = eqHighL.processSample(s);
            d[i] = s;
        }
    }

    // Canal R (si existe)
    if (nc > 1)
    {
        float* d = buf.getWritePointer(1);
        for (int i = 0; i < ns; ++i)
        {
            float s = d[i];
            if (std::abs(eqLowGain)  > 0.01f) s = eqLowR .processSample(s);
            if (std::abs(eqMidGain)  > 0.01f) s = eqMidR .processSample(s);
            if (std::abs(eqHighGain) > 0.01f) s = eqHighR.processSample(s);
            d[i] = s;
        }
    }
}
