#pragma once
#include <JuceHeader.h>
#include "SFZRegion.h"

// ──────────────────────────────────────────────────────────────────────────────
// SFZParser: lee y analiza un archivo .sfz, devuelve una lista de regiones
// ──────────────────────────────────────────────────────────────────────────────
class SFZParser
{
public:
    SFZParser() = default;

    // Parsea el archivo sfzFile y devuelve las regiones resultantes.
    // baseDir se usa para resolver rutas relativas a samples.
    bool parse(const juce::File& sfzFile, std::vector<SFZRegion>& outRegions);

    // Devuelve mensajes de error/aviso generados durante el parseo
    const juce::StringArray& getWarnings() const { return warnings; }

private:
    juce::StringArray warnings;

    // Tipos de sección activa
    enum class Section { None, Global, Group, Region };


    // Procesa un archivo SFZ (puede llamarse recursivamente para #include)
    void parseFile(const juce::File& file,
                   const juce::File& baseDir,
                   std::vector<SFZRegion>& outRegions,
                   int depth);

    // Divide un string de opcodes "key=value key2=value2 ..." y los aplica
    void processOpcodeString(const juce::String& str, SFZRegion& region);

    // Aplica un opcode a una región
    void applyOpcode(const juce::String& key,
                     const juce::String& value,
                     SFZRegion& region);

    // Convierte nombre de nota a número MIDI (ej: "C4" → 60)
    static int noteNameToMidi(const juce::String& name);

    // Convierte nombre de tipo de filtro
    static SFZFilterType parseFilterType(const juce::String& s);
};
