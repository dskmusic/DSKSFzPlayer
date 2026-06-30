#pragma once
#include <JuceHeader.h>

// ──────────────────────────────────────────────────────────────────────────────
// Modos de bucle para una región SFZ
// ──────────────────────────────────────────────────────────────────────────────
enum class SFZLoopMode
{
    NoLoop,       // sin bucle
    Continuous,   // bucle continuo mientras la nota está activa
    SustainOnly,  // bucle solo durante la fase de sustain
    OneShot       // reproduce hasta el final ignorando note-off
};

// ──────────────────────────────────────────────────────────────────────────────
// Tipos de filtro soportados
// ──────────────────────────────────────────────────────────────────────────────
enum class SFZFilterType
{
    LowPass,
    HighPass,
    BandPass,
    Notch
};

// ──────────────────────────────────────────────────────────────────────────────
// SFZRegion: todos los parámetros de una región SFZ
// Los valores aquí son los que provienen del archivo .sfz
// (ya con herencia global→group→region aplicada por el parser)
// ──────────────────────────────────────────────────────────────────────────────
struct SFZRegion
{
    // ── Sample ────────────────────────────────────────────────────────────────
    juce::String samplePath;            // ruta relativa al sample

    // ── Rango de teclas ───────────────────────────────────────────────────────
    int lokey          = 0;
    int hikey          = 127;
    int pitchKeycenter = 60;            // nota base del sample (C4 = 60)

    // ── Rango de velocidad ────────────────────────────────────────────────────
    int lovel = 0;
    int hivel = 127;

    // ── Afinación ─────────────────────────────────────────────────────────────
    float tune      = 0.0f;            // ajuste fino en centésimas de semitono
    int   transpose = 0;               // transposición en semitonos

    // ── Dinámica ──────────────────────────────────────────────────────────────
    float volume = 0.0f;               // en dB
    float pan    = 0.0f;               // -100 (izquierda) a +100 (derecha)

    // ── Reproducción ──────────────────────────────────────────────────────────
    int offset = 0;                    // muestra de inicio
    int end    = -1;                   // muestra de fin (-1 = hasta el final)

    // ── Loop ──────────────────────────────────────────────────────────────────
    SFZLoopMode loopMode         = SFZLoopMode::NoLoop;
    bool        loopModeExplicit = false; // true si loop_mode fue especificado en el SFZ
    int         loopStart        = 0;
    int         loopEnd          = -1;   // -1 = hasta el final

    // ── Round-robin ───────────────────────────────────────────────────────────
    int seqLength   = 1;               // número total de posiciones RR
    int seqPosition = 1;               // posición de esta región (1-based)

    // ── Aleatoriedad ──────────────────────────────────────────────────────────
    float lorand = 0.0f;
    float hirand = 1.0f;

    // ── Crossfade por nota ────────────────────────────────────────────────────
    int xfinLokey  = -1;
    int xfinHikey  = -1;
    int xfoutLokey = -1;
    int xfoutHikey = -1;

    // ── Crossfade por velocity ────────────────────────────────────────────────
    int xfinLovel  = -1;
    int xfinHivel  = -1;
    int xfoutLovel = -1;
    int xfoutHivel = -1;

    // ── Envolvente de amplitud (ampeg_*) ──────────────────────────────────────
    float ampAttack  = 0.001f;         // segundos
    float ampHold    = 0.0f;           // segundos
    float ampDecay   = 0.0f;           // segundos
    float ampSustain = 100.0f;         // porcentaje 0-100
    float ampRelease = 0.05f;          // segundos

    // ── Filtro por región ─────────────────────────────────────────────────────
    SFZFilterType filterType = SFZFilterType::LowPass;
    float         cutoff     = 20000.0f; // Hz
    float         resonance  = 0.707f;   // Q

    // ── Buffer de audio (cargado en memoria al preparar el instrumento) ───────
    std::shared_ptr<juce::AudioBuffer<float>> audioBuffer;
    double nativeSampleRate = 44100.0;   // sample rate del archivo de audio

    // ── Clave de grupo (para agrupar regiones RR) ─────────────────────────────
    juce::String group;

    // ─────────────────────────────────────────────────────────────────────────
    // Comprueba si esta región responde a una nota dada
    // ─────────────────────────────────────────────────────────────────────────
    bool matchesNote(int note, int velocity, float randomVal, int rrPos) const
    {
        if (note     < lokey || note     > hikey) return false;
        if (velocity < lovel || velocity > hivel) return false;
        if (randomVal < lorand || randomVal >= hirand) return false;
        if (seqLength > 1 && seqPosition != rrPos)    return false;
        return true;
    }

    // Calcula la ganancia de crossfade por nota (0.0-1.0)
    float xfadeGainByNote(int note) const
    {
        float gain = 1.0f;
        if (xfinLokey >= 0 && xfinHikey >= 0 && xfinHikey > xfinLokey)
        {
            if (note < xfinLokey) return 0.0f;
            if (note < xfinHikey)
                gain *= (float)(note - xfinLokey) / (float)(xfinHikey - xfinLokey);
        }
        if (xfoutLokey >= 0 && xfoutHikey >= 0 && xfoutHikey > xfoutLokey)
        {
            if (note > xfoutHikey) return 0.0f;
            if (note > xfoutLokey)
                gain *= 1.0f - (float)(note - xfoutLokey) / (float)(xfoutHikey - xfoutLokey);
        }
        return gain;
    }

    // Calcula la ganancia de crossfade por velocity (0.0-1.0)
    float xfadeGainByVel(int velocity) const
    {
        float gain = 1.0f;
        if (xfinLovel >= 0 && xfinHivel >= 0 && xfinHivel > xfinLovel)
        {
            if (velocity < xfinLovel) return 0.0f;
            if (velocity < xfinHivel)
                gain *= (float)(velocity - xfinLovel) / (float)(xfinHivel - xfinLovel);
        }
        if (xfoutLovel >= 0 && xfoutHivel >= 0 && xfoutHivel > xfoutLovel)
        {
            if (velocity > xfoutHivel) return 0.0f;
            if (velocity > xfoutLovel)
                gain *= 1.0f - (float)(velocity - xfoutLovel) / (float)(xfoutHivel - xfoutLovel);
        }
        return gain;
    }
};
