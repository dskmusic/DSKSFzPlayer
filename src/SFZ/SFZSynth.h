#pragma once
#include <JuceHeader.h>
#include "SFZRegion.h"

// ══════════════════════════════════════════════════════════════════════════════
// ADSR — Envolvente de amplitud con estados Ataque/Hold/Decaída/Sustain/Release
// ══════════════════════════════════════════════════════════════════════════════
class SFZADSR
{
public:
    enum class State { Idle, Attack, Hold, Decay, Sustain, Release };

    void setup(float attack, float hold, float decay,
               float sustainPct, float release, double sampleRate);
    void noteOn();
    void noteOff();
    double process();          // devuelve nivel 0.0-1.0
    bool   isActive()  const { return state != State::Idle; }
    bool   isIdle()    const { return state == State::Idle; }
    State  getState()  const { return state; }
    double getLevel()  const { return level; }

private:
    State  state        = State::Idle;
    double level        = 0.0;
    double attackRate   = 1.0;
    double holdSamples  = 0.0;
    double holdCounter  = 0.0;
    double decayRate    = 1.0;
    double sustainLevel = 1.0;
    double releaseRate  = 1.0;
};

// ══════════════════════════════════════════════════════════════════════════════
// BiquadFilter — Filtro biquad de doble precisión (un canal)
// ══════════════════════════════════════════════════════════════════════════════
class BiquadFilter
{
public:
    void reset();
    void setLowPass (double freq, double Q, double sampleRate);
    void setHighPass(double freq, double Q, double sampleRate);
    void setBandPass(double freq, double Q, double sampleRate);
    void setNotch   (double freq, double Q, double sampleRate);
    void setType    (SFZFilterType type, double freq, double Q, double sampleRate);

    double process(double x);

private:
    double b0 = 1.0, b1 = 0.0, b2 = 0.0;
    double a1 = 0.0, a2 = 0.0;
    double z1 = 0.0, z2 = 0.0;

    void setCoeffs(double b0, double b1, double b2,
                   double a0, double a1, double a2);
};

// ══════════════════════════════════════════════════════════════════════════════
// LFO — Oscilador de baja frecuencia asignable
// ══════════════════════════════════════════════════════════════════════════════
struct LFO
{
    enum class Shape { Sine, Triangle, Saw, Square, SampleHold };
    enum class Target { Pitch, Cutoff, Pan, Volume };

    Shape  shape     = Shape::Sine;
    Target target    = Target::Pitch;
    float  rate      = 1.0f;           // Hz
    float  amount    = 0.0f;           // 0.0-1.0
    double phase     = 0.0;
    float  lastRandom= 0.0f;

    void reset()  { phase = 0.0; }
    // numSamples: numero de muestras del bloque para avanzar la fase correctamente
    double process(double sampleRate, int numSamples = 1);
};

// ══════════════════════════════════════════════════════════════════════════════
// SFZVoice — Una voz polifónica del sampler
// ══════════════════════════════════════════════════════════════════════════════
class SFZVoice
{
public:
    static const int FADE_SAMPLES = 256;   // muestras de fade-out al robar voz

    void noteOn (const SFZRegion* region, int note, int velocity,
                 double sampleRate, float globalTranspose, float globalTune);
    void noteOff();
    void kill();                           // silencio inmediato con fade corto
    void resetInstant() { active = false; region = nullptr; } // Apagado forzoso

    // Renderiza 'numSamples' sobre los buffers L/R (se SUMA a lo que haya)
    // globalCutoff/Res/FiltType: parámetros de filtro globales del synth
    // cutoffMod: offset Hz aditivo (env + LFO);  pitchMod: semitonos de LFO
    bool render(float* leftOut, float* rightOut, int numSamples,
                float globalVolume, float globalPan,
                float globalCutoff, float globalRes, SFZFilterType globalFiltType,
                float cutoffMod, float ampMod, float pitchMod);

    bool  isActive()    const { return active; }
    bool  isReleasing() const { return releasing; }
    int   getNote()     const { return note; }
    int   getChannel()  const { return channel; }
    const SFZRegion* getRegion() const { return region; }

    // Override the voice ADSR with global master values after noteOn
    void setMasterADSR(float a, float d, float sPct, float r, double sr)
    {
        ampEnv.setup(a, 0.0f, d, sPct, r, sr);
        ampEnv.noteOn();
    }

    int   channel   = 0;

private:
    const SFZRegion* region = nullptr;
    int   note      = -1;
    int   velocity  = 0;
    bool  active    = false;
    bool  releasing = false;
    bool  killing   = false;

    double playbackPos  = 0.0;
    double playbackRate = 1.0;

    SFZADSR ampEnv;
    BiquadFilter filterL, filterR;

    float velGain    = 1.0f;
    float regionGain = 1.0f;
    float panL       = 0.7071f;
    float panR       = 0.7071f;

    double currentSR = 44100.0;

    // Fade para evitar clicks al robar voces
    int   fadeCounter = 0;
    bool  doFade      = false;

    void updateFilter(float baseCutoff, float baseRes,
                      SFZFilterType type, float modAmount);
};

// ══════════════════════════════════════════════════════════════════════════════
// SFZSynth — Motor polifónico completo: carga SFZ, gestiona voces, renderiza
// ══════════════════════════════════════════════════════════════════════════════
class SFZSynth
{
public:
    static const int MAX_VOICES = 64;

    SFZSynth();
    ~SFZSynth() = default;

    // ── Carga ─────────────────────────────────────────────────────────────────
    // Devuelve true si el instrumento se cargó correctamente
    bool loadSFZ(const juce::File& sfzFile,
                 std::function<void(float)> progressCallback = nullptr);
    void unload();

    // ── Preparación ───────────────────────────────────────────────────────────
    void prepare(double sampleRate, int blockSize);

    // ── Renderizado ───────────────────────────────────────────────────────────
    void processBlock(juce::AudioBuffer<float>& buffer,
                      juce::MidiBuffer& midiMessages);

    // ── MIDI ──────────────────────────────────────────────────────────────────
    void noteOn (int channel, int note, int velocity);
    void noteOff(int channel, int note, int velocity);
    void allNotesOff();
    void pitchBend(int channel, float semitones);  // ±2 semis por defecto
    void controlChange(int channel, int cc, int value);

    // ── Parámetros globales (van por encima de los valores SFZ) ───────────────
    float masterVolume   = 0.0f;    // dB
    float masterPan      = 0.0f;    // -100..+100
    float globalTranspose= 0.0f;    // semitonos
    float globalTune     = 0.0f;    // centésimas

    // ADSR de amplitud global (se SUMA sobre el ADSR por región)
    float ampAttack  = 0.0f;
    float ampDecay   = 0.0f;
    float ampSustain = 100.0f;
    float ampRelease = 0.0f;

    // Filtro global
    float filterCutoff    = 20000.0f;
    float filterResonance = 0.707f;
    int   filterTypeIdx   = 0;        // 0=LP 1=HP 2=BP 3=Notch
    float filterEnvAmt    = 0.0f;

    // ADSR de filtro global
    float fltAttack  = 0.001f;
    float fltDecay   = 0.1f;
    float fltSustain = 100.0f;
    float fltRelease = 0.1f;

    // LFOs
    LFO lfo1, lfo2;

    // Número de voces activo
    int maxVoices = 32;

    // ── Información del instrumento cargado ───────────────────────────────────
    juce::String getInstrumentName() const { return instrumentName; }
    int          getRegionCount()    const { return (int)regions.size(); }
    int          getSampleCount()    const { return (int)sampleCache.size(); }
    bool         isLoaded()          const { return loaded; }
    juce::String getLastError()      const { return lastError; }

    // ── Reset round-robin ─────────────────────────────────────────────────────
    void resetRoundRobin() { rrCounters.clear(); }

    // ── Output level meters (read by UI timer, written by audio thread) ───────
    mutable std::atomic<float> meterLevelL { 0.0f };
    mutable std::atomic<float> meterLevelR { 0.0f };

private:
    // Regiones parseadas del SFZ
    std::vector<SFZRegion> regions;

    // Caché de buffers de audio (clave = ruta absoluta)
    std::map<juce::String, std::shared_ptr<juce::AudioBuffer<float>>> sampleCache;
    // Sample rates nativos correspondientes a cada entrada del caché
    std::map<juce::String, double> nativeSRCache;

    // Pool de voces
    std::array<SFZVoice, MAX_VOICES> voices;

    // Formato y directorio base del SFZ cargado
    juce::AudioFormatManager formatManager;
    juce::File               sfzBaseDir;

    double sampleRate = 44100.0;
    int    blockSize  = 512;

    // Counters round-robin: clave = (lokey,hikey,lovel,hivel,seqLength)
    std::map<std::tuple<int,int,int,int,int>, int> rrCounters;

    // Estado
    bool         loaded         = false;
    juce::String instrumentName;
    juce::String lastError;

    // Pitch bend acumulado (en semitonos)
    float pitchBendSemis = 0.0f;
    // Sustain pedal
    bool  sustainPedal   = false;
    std::vector<int> sustainedNotes;  // notas retenidas por sustain

    // Filtro global para el render (ADSR de filtro)
    SFZADSR filterEnv;

    // Helpers
    SFZVoice* findFreeVoice(int note);
    void      loadSampleFiles(std::function<void(float)> progress);
    // Carga un archivo de audio y devuelve el buffer + sample rate nativo
    struct LoadedSample
    {
        std::shared_ptr<juce::AudioBuffer<float>> buffer;
        double  nativeSR     = 44100.0;
        int64_t embLoopStart = -1;   // loop point embebido en WAV (SMPL chunk), -1 si no hay
        int64_t embLoopEnd   = -1;
    };
    LoadedSample loadAudioFile(const juce::File& file);
    int       getNextRR(int lokey, int hikey, int lovel, int hivel, int seqLen);
    void      renderMidi(juce::MidiBuffer& midiMessages, int startSample, int numSamples,
                         juce::AudioBuffer<float>& output);
};
