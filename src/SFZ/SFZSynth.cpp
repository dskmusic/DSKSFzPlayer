#include <JuceHeader.h>
#include "SFZSynth.h"
#include "SFZParser.h"

// ══════════════════════════════════════════════════════════════════════════════
// SFZADSR
// ══════════════════════════════════════════════════════════════════════════════
void SFZADSR::setup(float attack, float hold, float decay,
                    float sustainPct, float release, double sampleRate)
{
    attackRate   = (attack  > 0.0001f) ? 1.0 / (attack  * sampleRate) : 1.0;
    holdSamples  = hold  * sampleRate;
    decayRate    = (decay   > 0.0001f) ? 1.0 / (decay   * sampleRate) : 1.0;
    sustainLevel = juce::jlimit(0.0, 1.0, (double)sustainPct / 100.0);
    releaseRate  = (release > 0.0001f) ? 1.0 / (release * sampleRate) : 1.0;
}

void SFZADSR::noteOn()
{
    state = State::Attack;
    holdCounter = 0.0;
}

void SFZADSR::noteOff()
{
    if (state != State::Idle)
        state = State::Release;
}

double SFZADSR::process()
{
    switch (state)
    {
        case State::Attack:
            level += attackRate;
            if (level >= 1.0)
            {
                level = 1.0;
                if (holdSamples > 0.0) { state = State::Hold; holdCounter = 0.0; }
                else if (sustainLevel < 1.0 - 1e-6) { state = State::Decay; }
                else state = State::Sustain;
            }
            break;

        case State::Hold:
            holdCounter++;
            if (holdCounter >= holdSamples)
            {
                if (sustainLevel < 1.0 - 1e-6) state = State::Decay;
                else state = State::Sustain;
            }
            break;

        case State::Decay:
            level -= decayRate;
            if (level <= sustainLevel)
            {
                level = sustainLevel;
                state = State::Sustain;
            }
            break;

        case State::Sustain:
            level = sustainLevel;
            break;

        case State::Release:
            level -= releaseRate;
            if (level <= 0.0) { level = 0.0; state = State::Idle; }
            break;

        case State::Idle:
            level = 0.0;
            break;
    }
    return level;
}

// ══════════════════════════════════════════════════════════════════════════════
// BiquadFilter
// ══════════════════════════════════════════════════════════════════════════════
void BiquadFilter::reset() { z1 = z2 = 0.0; }

void BiquadFilter::setCoeffs(double _b0, double _b1, double _b2,
                              double a0,  double _a1,  double _a2)
{
    b0 = _b0 / a0;  b1 = _b1 / a0;  b2 = _b2 / a0;
    a1 = _a1 / a0;  a2 = _a2 / a0;
}

void BiquadFilter::setLowPass(double freq, double Q, double sr)
{
    double w0    = juce::MathConstants<double>::twoPi * freq / sr;
    double cosw  = std::cos(w0);
    double alpha = std::sin(w0) / (2.0 * Q);
    setCoeffs((1.0 - cosw) / 2.0, 1.0 - cosw, (1.0 - cosw) / 2.0,
              1.0 + alpha, -2.0 * cosw, 1.0 - alpha);
}

void BiquadFilter::setHighPass(double freq, double Q, double sr)
{
    double w0    = juce::MathConstants<double>::twoPi * freq / sr;
    double cosw  = std::cos(w0);
    double alpha = std::sin(w0) / (2.0 * Q);
    setCoeffs((1.0 + cosw) / 2.0, -(1.0 + cosw), (1.0 + cosw) / 2.0,
              1.0 + alpha, -2.0 * cosw, 1.0 - alpha);
}

void BiquadFilter::setBandPass(double freq, double Q, double sr)
{
    double w0    = juce::MathConstants<double>::twoPi * freq / sr;
    double sinw  = std::sin(w0);
    double cosw  = std::cos(w0);
    double alpha = sinw / (2.0 * Q);
    setCoeffs(sinw / 2.0, 0.0, -sinw / 2.0,
              1.0 + alpha, -2.0 * cosw, 1.0 - alpha);
}

void BiquadFilter::setNotch(double freq, double Q, double sr)
{
    double w0    = juce::MathConstants<double>::twoPi * freq / sr;
    double cosw  = std::cos(w0);
    double alpha = std::sin(w0) / (2.0 * Q);
    setCoeffs(1.0, -2.0 * cosw, 1.0,
              1.0 + alpha, -2.0 * cosw, 1.0 - alpha);
}

void BiquadFilter::setType(SFZFilterType type, double freq, double Q, double sr)
{
    double f = juce::jlimit(20.0, sr * 0.499, freq);
    double q = juce::jlimit(0.1,  20.0, Q);
    switch (type)
    {
        case SFZFilterType::HighPass:  setHighPass(f, q, sr); break;
        case SFZFilterType::BandPass:  setBandPass(f, q, sr); break;
        case SFZFilterType::Notch:     setNotch   (f, q, sr); break;
        default:                       setLowPass  (f, q, sr); break;
    }
}

double BiquadFilter::process(double x)
{
    double y = b0 * x + z1;
    z1 = b1 * x - a1 * y + z2;
    z2 = b2 * x - a2 * y;
    return y;
}

// ══════════════════════════════════════════════════════════════════════════════
// LFO
// ══════════════════════════════════════════════════════════════════════════════
double LFO::process(double sampleRate, int numSamples)
{
    double phaseInc = (double)rate * numSamples / sampleRate;
    double value    = 0.0;

    switch (shape)
    {
        case Shape::Sine:
            value = std::sin(phase * juce::MathConstants<double>::twoPi);
            break;
        case Shape::Triangle:
            value = 1.0 - 4.0 * std::abs(std::fmod(phase, 1.0) - 0.5);
            break;
        case Shape::Saw:
            value = 2.0 * phase - 1.0;
            break;
        case Shape::Square:
            value = (phase < 0.5) ? 1.0 : -1.0;
            break;
        case Shape::SampleHold:
            if (phase < phaseInc)
                lastRandom = juce::Random::getSystemRandom().nextFloat() * 2.0f - 1.0f;
            value = lastRandom;
            break;
    }

    phase += phaseInc;
    if (phase >= 1.0) phase -= std::floor(phase);

    return value * amount;
}

// ══════════════════════════════════════════════════════════════════════════════
// SFZVoice
// ══════════════════════════════════════════════════════════════════════════════
void SFZVoice::noteOn(const SFZRegion* r, int n, int vel,
                      double sampleRate, float globalTranspose, float globalTune)
{
    region       = r;
    note         = n;
    velocity     = vel;
    active       = true;
    releasing    = false;
    killing      = false;
    doFade       = false;
    fadeCounter  = 0;
    currentSR    = sampleRate;
    playbackPos  = (double)r->offset;

    // Calcular pitch: nota → tasa de reproducción relativa al sample
    double semiDiff = (double)(n - r->pitchKeycenter)
                    + (double)r->transpose
                    + (double)(globalTranspose)
                    + (double)(r->tune + globalTune) / 100.0;
    playbackRate = std::pow(2.0, semiDiff / 12.0)
                 * (r->nativeSampleRate / sampleRate);

    // Ganancia por velocity (lineal simple)
    velGain    = (float)vel / 127.0f;
    // Ganancia por volumen de región (dB → lineal)
    regionGain = juce::Decibels::decibelsToGain(r->volume);

    // Paneo de región: convertir -100..+100 a coeficientes L/R (ley de seno)
    double panRad = juce::jlimit(-1.0, 1.0, (double)r->pan / 100.0)
                  * juce::MathConstants<double>::pi / 4.0;
    panL = (float)std::cos(panRad);
    panR = (float)std::sin(panRad + juce::MathConstants<double>::pi / 2.0);

    // Configurar ADSR de amplitud con valores de la región
    ampEnv.setup(r->ampAttack, r->ampHold, r->ampDecay, r->ampSustain, r->ampRelease, sampleRate);
    ampEnv.noteOn();

    // Configurar filtro
    double cutoffClamped = juce::jlimit(20.0f, 20000.0f, r->cutoff);
    double resClamped    = juce::jlimit(0.1f,  20.0f,    r->resonance);
    filterL.setType(r->filterType, cutoffClamped, resClamped, sampleRate);
    filterR.setType(r->filterType, cutoffClamped, resClamped, sampleRate);
    filterL.reset();
    filterR.reset();
}

void SFZVoice::noteOff()
{
    if (!active) return;
    if (region && region->loopMode == SFZLoopMode::OneShot) return; // one-shot ignora noteOff
    releasing = true;
    ampEnv.noteOff();
}

void SFZVoice::kill()
{
    // Inicia fade-out corto para evitar click
    killing = true;
    doFade  = true;
    fadeCounter = 0;
}

bool SFZVoice::render(float* leftOut, float* rightOut, int numSamples,
                      float globalVolume, float globalPan,
                      float globalCutoff, float globalRes, SFZFilterType globalFiltType,
                      float cutoffMod, float ampMod, float pitchMod)
{
    if (!active || !region || !region->audioBuffer) return false;

    auto* buf       = region->audioBuffer.get();
    int   bufLen    = buf->getNumSamples();
    int   nChannels = buf->getNumChannels();

    // Paneo global
    double globalPanRad = juce::jlimit(-1.0, 1.0, (double)globalPan / 100.0)
                        * juce::MathConstants<double>::pi / 4.0;
    float gpL = (float)std::cos(globalPanRad);
    float gpR = (float)std::sin(globalPanRad + juce::MathConstants<double>::pi / 2.0);

    // Gain total = velocity × región × volumen master
    float masterGainLinear = juce::Decibels::decibelsToGain(globalVolume);

    // Crossfade
    float xfGain = region->xfadeGainByNote(note) * region->xfadeGainByVel(velocity);

    int endSample  = (region->end >= 0 && region->end < bufLen) ? region->end : (bufLen - 1);
    int loopStart  = juce::jlimit(0, endSample, region->loopStart);
    int loopEnd    = (region->loopEnd > 0 && region->loopEnd <= endSample)
                   ? region->loopEnd : endSample;

    bool doLoop = (region->loopMode == SFZLoopMode::Continuous)
               || (region->loopMode == SFZLoopMode::SustainOnly && !releasing);

    // Filtro: usa parámetros globales del synth como base; el SFZ override si lo especifica
    // region->cutoff == 20000 Hz significa "no filtra" (valor por defecto SFZ)
    float baseCutoff = (region->cutoff < 19990.0f) ? region->cutoff : globalCutoff;
    float baseRes    = (region->resonance > 0.71f)  ? region->resonance : globalRes;
    SFZFilterType filtType = (region->filterType != SFZFilterType::LowPass)
                             ? region->filterType : globalFiltType;
    float cutoffModded = juce::jlimit(20.0f, 20000.0f, baseCutoff + cutoffMod);
    updateFilter(cutoffModded, baseRes, filtType, 0.0f);

    // Pitch mod: ratio de reproducción modificado por LFO (en semitonos)
    double effectiveRate = playbackRate;
    if (pitchMod != 0.0f)
        effectiveRate *= std::pow(2.0, (double)pitchMod / 12.0);

    for (int i = 0; i < numSamples; ++i)
    {
        // Fade-out al robar voz
        float fadeMul = 1.0f;
        if (doFade)
        {
            fadeMul = 1.0f - (float)fadeCounter / (float)FADE_SAMPLES;
            fadeCounter++;
            if (fadeCounter >= FADE_SAMPLES) { active = false; return false; }
        }

        // Comprobar fin de muestra
        if (doLoop)
        {
            if (playbackPos >= loopEnd)
                playbackPos = loopStart + std::fmod(playbackPos - loopStart,
                                                    (double)(loopEnd - loopStart));
        }
        else
        {
            if (playbackPos >= endSample)
            {
                active = false;
                return false;
            }
        }

        // Interpolación Hermite de 4 puntos para mejor calidad
        auto hermiteInterp = [&](int ch) -> float
        {
            int p1 = (int)playbackPos;
            float t = (float)(playbackPos - p1);

            int p0 = juce::jmax(0, p1 - 1);
            int p2 = juce::jmin(bufLen - 1, p1 + 1);
            int p3 = juce::jmin(bufLen - 1, p1 + 2);

            float y0 = buf->getSample(ch, p0);
            float y1 = buf->getSample(ch, juce::jmin(bufLen-1, p1));
            float y2 = buf->getSample(ch, p2);
            float y3 = buf->getSample(ch, p3);

            float c0 = y1;
            float c1 = 0.5f * (y2 - y0);
            float c2 = y0 - 2.5f * y1 + 2.0f * y2 - 0.5f * y3;
            float c3 = 0.5f * (y3 - y0) + 1.5f * (y1 - y2);
            return ((c3 * t + c2) * t + c1) * t + c0;
        };

        float sL = 0.0f, sR = 0.0f;
        if (nChannels >= 2)
        {
            sL = hermiteInterp(0);
            sR = hermiteInterp(1);
        }
        else
        {
            sL = sR = hermiteInterp(0);
        }

        // Aplicar filtro (coeficientes ya calculados fuera del bucle)
        sL = (float)filterL.process(sL);
        sR = (float)filterR.process(sR);

        // ADSR de amplitud
        double envGain = ampEnv.process();
        if (!ampEnv.isActive()) { active = false; return false; }

        float finalGain = (float)envGain * velGain * regionGain
                        * masterGainLinear * xfGain * fadeMul
                        * (1.0f + ampMod);

        leftOut [i] += sL * finalGain * panL * gpL;
        rightOut[i] += sR * finalGain * panR * gpR;

        // Avanzar posición de reproducción (pitch bend + pitch LFO)
        playbackPos += effectiveRate;
    }

    return true;
}

void SFZVoice::updateFilter(float baseCutoff, float baseRes,
                             SFZFilterType type, float /*modAmount*/)
{
    // Solo reconfigura si el cutoff cambió significativamente
    filterL.setType(type, baseCutoff, baseRes, currentSR);
    filterR.setType(type, baseCutoff, baseRes, currentSR);
}

// ══════════════════════════════════════════════════════════════════════════════
// SFZSynth
// ══════════════════════════════════════════════════════════════════════════════
SFZSynth::SFZSynth()
{
    // Registrar formatos de audio soportados
    formatManager.registerBasicFormats();  // WAV, AIFF
#if JUCE_USE_FLAC
    formatManager.registerFormat(new juce::FlacAudioFormat(), true);
#endif
#if JUCE_USE_OGGVORBIS
    formatManager.registerFormat(new juce::OggVorbisAudioFormat(), true);
#endif
}

bool SFZSynth::loadSFZ(const juce::File& sfzFile,
                        std::function<void(float)> progressCallback)
{
    unload();
    instrumentName = sfzFile.getFileNameWithoutExtension();
    sfzBaseDir     = sfzFile.getParentDirectory();

    SFZParser parser;
    if (!parser.parse(sfzFile, regions))
    {
        lastError = "Could not parse the SFZ file: " + sfzFile.getFullPathName();
        return false;
    }

    if (regions.empty())
    {
        lastError = "The SFZ file does not contain valid regions.";
        return false;
    }

    loadSampleFiles(progressCallback);
    loaded = true;
    return true;
}

void SFZSynth::unload()
{
    for (auto& v : voices) v.resetInstant(); // APAGADO INSTANTÁNEO, corta la lectura de memoria
    regions.clear();
    sampleCache.clear();
    nativeSRCache.clear();
    rrCounters.clear();
    loaded = false;
    instrumentName = {};
    lastError = {};
}

void SFZSynth::prepare(double sr, int bs)
{
    sampleRate = sr;
    blockSize  = bs;
    allNotesOff();
    lfo1.reset();
    lfo2.reset();
    filterEnv.setup(fltAttack, 0.0f, fltDecay, fltSustain, fltRelease, sampleRate);
}

void SFZSynth::processBlock(juce::AudioBuffer<float>& buffer,
                             juce::MidiBuffer& midiMessages)
{
    buffer.clear();
    if (!loaded) return;

    const int numSamples = buffer.getNumSamples();

    // Procesar eventos MIDI sample-accurate
    int samplePos = 0;
    for (const auto meta : midiMessages)
    {
        int eventSample = juce::jlimit(0, numSamples - 1, meta.samplePosition);
        // Renderizar hasta el evento
        if (eventSample > samplePos)
            renderMidi(midiMessages, samplePos, eventSample - samplePos, buffer);

        auto msg = meta.getMessage();
        if      (msg.isNoteOn (true))  noteOn (msg.getChannel(), msg.getNoteNumber(), msg.getVelocity());
        else if (msg.isNoteOff(true))  noteOff(msg.getChannel(), msg.getNoteNumber(), msg.getVelocity());
        else if (msg.isPitchWheel())
        {
            float pw = (float)(msg.getPitchWheelValue() - 8192) / 8192.0f;
            pitchBend(msg.getChannel(), pw * 2.0f); // ±2 semitonos
        }
        else if (msg.isController())
            controlChange(msg.getChannel(), msg.getControllerNumber(), msg.getControllerValue());

        samplePos = eventSample;
    }
    // Renderizar el resto del bloque
    if (samplePos < numSamples)
        renderMidi(midiMessages, samplePos, numSamples - samplePos, buffer);

    midiMessages.clear();

    // Update output meters
    if (buffer.getNumChannels() > 0)
        meterLevelL.store(buffer.getMagnitude(0, 0, numSamples));
    if (buffer.getNumChannels() > 1)
        meterLevelR.store(buffer.getMagnitude(1, 0, numSamples));
}

// ──────────────────────────────────────────────────────────────────────────────
// renderMidi(): renderiza audio para un segmento del bloque
// ──────────────────────────────────────────────────────────────────────────────
void SFZSynth::renderMidi(juce::MidiBuffer& /*midi*/, int startSample,
                           int numSamples, juce::AudioBuffer<float>& output)
{
    if (numSamples <= 0) return;

    float* L = output.getWritePointer(0, startSample);
    float* R = output.getWritePointer(output.getNumChannels() > 1 ? 1 : 0, startSample);

    // LFO avanza numSamples por bloque para una tasa correcta
    double lfo1Val = lfo1.process(sampleRate, numSamples);
    double lfo2Val = lfo2.process(sampleRate, numSamples);

    // Filter envelope: additive Hz offset — fltEnvLevel scales filterCutoff range
    double fltEnvLevel = filterEnv.process();
    float  cutoffMod   = (float)fltEnvLevel
                       * juce::jlimit(-1.0f, 1.0f, filterEnvAmt)
                       * filterCutoff;
    if (lfo1.target == LFO::Target::Cutoff) cutoffMod += (float)lfo1Val * filterCutoff;
    if (lfo2.target == LFO::Target::Cutoff) cutoffMod += (float)lfo2Val * filterCutoff;

    // Amp mod desde LFO (±50 % de ganancia)
    float ampMod = 0.0f;
    if (lfo1.target == LFO::Target::Volume) ampMod += (float)lfo1Val * 0.5f;
    if (lfo2.target == LFO::Target::Volume) ampMod += (float)lfo2Val * 0.5f;

    // Pitch mod desde LFO (en semitonos; lfoVal ya incluye el depth)
    float pitchMod = 0.0f;
    if (lfo1.target == LFO::Target::Pitch) pitchMod += (float)lfo1Val * 12.0f;
    if (lfo2.target == LFO::Target::Pitch) pitchMod += (float)lfo2Val * 12.0f;

    // Renderizar cada voz activa
    const SFZFilterType globalFT = static_cast<SFZFilterType>(filterTypeIdx);
    for (auto& v : voices)
    {
        if (!v.isActive()) continue;
        v.render(L, R, numSamples,
                 masterVolume, masterPan,
                 filterCutoff, filterResonance, globalFT,
                 cutoffMod, ampMod, pitchMod);
    }
}

// ──────────────────────────────────────────────────────────────────────────────
// noteOn
// ──────────────────────────────────────────────────────────────────────────────
void SFZSynth::noteOn(int channel, int note, int velocity)
{
    if (!loaded || velocity == 0) { noteOff(channel, note, 0); return; }

    float randVal = juce::Random::getSystemRandom().nextFloat();

    // Calcular la próxima posición RR para cada grupo y buscar regiones que coincidan
    // Se usa un mapa temporal para no avanzar el contador hasta confirmar que hay match
    std::map<std::tuple<int,int,int,int,int>, int> tempRR = rrCounters;

    std::vector<const SFZRegion*> matching;
    for (auto& r : regions)
    {
        if (!r.audioBuffer) continue;
        auto key  = std::make_tuple(r.lokey, r.hikey, r.lovel, r.hivel, r.seqLength);
        int  rrPos = (r.seqLength <= 1) ? 1 : ((tempRR[key] % r.seqLength) + 1);
        if (r.matchesNote(note, velocity, randVal, rrPos))
            matching.push_back(&r);
    }

    if (matching.empty()) return;

    // Avanzar contadores RR solo para los grupos de las regiones que suenan
    for (auto* r : matching)
    {
        if (r->seqLength <= 1) continue;
        auto key = std::make_tuple(r->lokey, r->hikey, r->lovel, r->hivel, r->seqLength);
        rrCounters[key] = (rrCounters[key] % r->seqLength) + 1;
    }

    // Activar filtro env
    filterEnv.setup(fltAttack, 0.0f, fltDecay, fltSustain, fltRelease, sampleRate);
    filterEnv.noteOn();

    // Asignar voces
    for (auto* r : matching)
    {
        SFZVoice* v = findFreeVoice(note);
        if (!v) break;
        v->channel = channel;
        v->noteOn(r, note, velocity, sampleRate,
                  globalTranspose + pitchBendSemis, globalTune);
        // Always apply master ADSR — it overrides per-region SFZ envelope
        v->setMasterADSR(ampAttack, ampDecay, ampSustain, ampRelease, sampleRate);
    }
}

// ──────────────────────────────────────────────────────────────────────────────
// noteOff
// ──────────────────────────────────────────────────────────────────────────────
void SFZSynth::noteOff(int channel, int note, int /*velocity*/)
{
    if (sustainPedal) { sustainedNotes.push_back(note); return; }

    filterEnv.noteOff();

    for (auto& v : voices)
        if (v.isActive() && v.getNote() == note && v.channel == channel)
            v.noteOff();
}

void SFZSynth::allNotesOff()
{
    for (auto& v : voices) v.noteOff();
    sustainedNotes.clear();
    sustainPedal = false;
    filterEnv.noteOff();
}

void SFZSynth::pitchBend(int /*channel*/, float semitones)
{
    pitchBendSemis = semitones;
    // Actualizar playbackRate de voces activas no es trivial sin recalcular;
    // el efecto se aplica en el próximo noteOn. Para pitch bend en tiempo real
    // sería necesario modificar las voces activas — simplificamos aquí.
}

void SFZSynth::controlChange(int channel, int cc, int value)
{
    if (cc == 64) // Sustain pedal
    {
        sustainPedal = (value >= 64);
        if (!sustainPedal)
        {
            // Liberar notas retenidas
            for (int n : sustainedNotes) noteOff(channel, n, 0);
            sustainedNotes.clear();
        }
    }
}

// ──────────────────────────────────────────────────────────────────────────────
// findFreeVoice(): encuentra voz libre o roba la más vieja en release
// ──────────────────────────────────────────────────────────────────────────────
SFZVoice* SFZSynth::findFreeVoice(int /*note*/)
{
    int activeCount = 0;
    for (auto& v : voices) if (v.isActive()) ++activeCount;

    // 1. Voz completamente libre
    for (auto& v : voices)
        if (!v.isActive()) return &v;

    // 2. Respetar límite de polifonía: robar si se excede
    if (activeCount >= maxVoices)
    {
        // Robar la voz más antigua en releasing
        for (auto& v : voices)
            if (v.isReleasing()) { v.kill(); return &v; }
        // Si no hay en release, robar la primera activa
        for (auto& v : voices)
            if (v.isActive()) { v.kill(); return &v; }
    }

    return nullptr;
}

// ──────────────────────────────────────────────────────────────────────────────
// getNextRR(): devuelve la posición round-robin esperada para este grupo
// ──────────────────────────────────────────────────────────────────────────────
int SFZSynth::getNextRR(int lo, int hi, int lv, int hv, int seqLen)
{
    if (seqLen <= 1) return 1;
    auto key = std::make_tuple(lo, hi, lv, hv, seqLen);
    auto it  = rrCounters.find(key);
    if (it == rrCounters.end()) return 1;       // primera vez: posición 1
    return (it->second % seqLen) + 1;
}

// ──────────────────────────────────────────────────────────────────────────────
// loadSampleFiles(): carga en RAM todos los samples del instrumento
// ──────────────────────────────────────────────────────────────────────────────
void SFZSynth::loadSampleFiles(std::function<void(float)> progress)
{
    int total    = (int)regions.size();
    int loaded_n = 0;

    for (auto& r : regions)
    {
        if (r.samplePath.isEmpty()) { ++loaded_n; continue; }

        // Resolver ruta absoluta
        juce::File sampleFile = sfzBaseDir.getChildFile(r.samplePath);
        if (!sampleFile.existsAsFile())
        {
            // Búsqueda insensible a mayúsculas: recorrer la ruta parte a parte
            juce::StringArray parts;
            parts.addTokens(r.samplePath.replaceCharacter('\\', '/'), "/", "");
            juce::File f = sfzBaseDir;
            bool found  = true;
            for (int pi = 0; pi < parts.size(); ++pi)
            {
                juce::Array<juce::File> children;
                f.findChildFiles(children, juce::File::findFilesAndDirectories, false);
                bool matched = false;
                for (auto& c : children)
                {
                    if (c.getFileName().equalsIgnoreCase(parts[pi]))
                    {
                        f = c; matched = true; break;
                    }
                }
                if (!matched) { found = false; break; }
            }
            if (found && f.existsAsFile()) sampleFile = f;
        }

        juce::String absPath = sampleFile.getFullPathName();

        // Reutilizar buffer si ya está cargado
        auto it = sampleCache.find(absPath);
        if (it != sampleCache.end())
        {
            r.audioBuffer = it->second;
            // Recuperar sample rate nativo del mapa de tasas
            if (nativeSRCache.count(absPath))
                r.nativeSampleRate = nativeSRCache[absPath];
        }
        else if (sampleFile.existsAsFile())
        {
            auto sample = loadAudioFile(sampleFile);
            if (sample.buffer)
            {
                sampleCache[absPath]   = sample.buffer;
                nativeSRCache[absPath] = sample.nativeSR;
                r.audioBuffer          = sample.buffer;
                r.nativeSampleRate     = sample.nativeSR;

                // Aplica loop points embebidos del WAV si el SFZ no los especificó
                int64_t bufLen = (int64_t)sample.buffer->getNumSamples();
                if (sample.embLoopEnd > 0 && sample.embLoopEnd < bufLen)
                {
                    if (!r.loopModeExplicit)
                    {
                        // SFZ no especificó loop_mode → activar loop con datos del WAV
                        r.loopMode  = SFZLoopMode::Continuous;
                        r.loopStart = (int)juce::jmax((int64_t)0, sample.embLoopStart);
                        r.loopEnd   = (int)sample.embLoopEnd;
                    }
                    else if (r.loopMode != SFZLoopMode::NoLoop && r.loopEnd < 0)
                    {
                        // SFZ quiere loop pero sin puntos explícitos → usar los del WAV
                        r.loopStart = (int)juce::jmax((int64_t)0, sample.embLoopStart);
                        r.loopEnd   = (int)sample.embLoopEnd;
                    }
                }
            }
        }

        ++loaded_n;
        if (progress) progress((float)loaded_n / (float)total);
    }
}

// ──────────────────────────────────────────────────────────────────────────────
// loadAudioFile(): carga un archivo de audio a un AudioBuffer<float>
// ──────────────────────────────────────────────────────────────────────────────
SFZSynth::LoadedSample SFZSynth::loadAudioFile(const juce::File& file)
{
    LoadedSample result;

    std::unique_ptr<juce::AudioFormatReader> reader(
        formatManager.createReaderFor(file));

    if (!reader) return result;

    result.nativeSR = reader->sampleRate;
    result.buffer   = std::make_shared<juce::AudioBuffer<float>>(
        (int)reader->numChannels,
        (int)reader->lengthInSamples);

    reader->read(result.buffer.get(), 0,
                 (int)reader->lengthInSamples, 0, true, true);

    // Lee loop points del chunk SMPL (WAV) si existen
    // JUCE nombra las claves "Loop0Start", "Loop0End" (índice antes del campo)
    auto& meta = reader->metadataValues;
    int numLoops = meta.getValue("NumSampleLoops", "0").getIntValue();
    if (numLoops > 0)
    {
        int64_t ls = meta.getValue("Loop0Start", "-1").getLargeIntValue();
        int64_t le = meta.getValue("Loop0End",   "-1").getLargeIntValue();
        if (le > 0 && le > ls)
        {
            result.embLoopStart = juce::jmax((int64_t)0, ls);
            result.embLoopEnd   = le;
        }
    }

    return result;
}
