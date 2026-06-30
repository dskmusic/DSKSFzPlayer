#include <JuceHeader.h>
#include "SFZParser.h"

// ──────────────────────────────────────────────────────────────────────────────
// parse(): punto de entrada público
// ──────────────────────────────────────────────────────────────────────────────
bool SFZParser::parse(const juce::File& sfzFile, std::vector<SFZRegion>& outRegions)
{
    warnings.clear();

    if (!sfzFile.existsAsFile())
    {
        warnings.add("Archivo SFZ no encontrado: " + sfzFile.getFullPathName());
        return false;
    }

    parseFile(sfzFile, sfzFile.getParentDirectory(), outRegions, 0);
    return true;
}

// ──────────────────────────────────────────────────────────────────────────────
// parseFile(): procesa un archivo línea a línea
// ──────────────────────────────────────────────────────────────────────────────
void SFZParser::parseFile(const juce::File& file,
                          const juce::File& baseDir,
                          std::vector<SFZRegion>& outRegions,
                          int depth)
{
    if (depth > 8)
    {
        warnings.add("Demasiados niveles de #include anidado, posible bucle.");
        return;
    }

    juce::String content = file.loadFileAsString();

    // Eliminar comentarios de bloque /* ... */
    {
        int start = 0;
        juce::String cleaned;
        while (start < content.length())
        {
            int blockStart = content.indexOf(start, "/*");
            if (blockStart < 0) { cleaned += content.substring(start); break; }
            cleaned += content.substring(start, blockStart);
            int blockEnd = content.indexOf(blockStart + 2, "*/");
            if (blockEnd < 0) break;
            start = blockEnd + 2;
        }
        content = cleaned;
    }

    // Contextos de herencia (global, group, region)
    SFZRegion globalCtx, groupCtx, regionCtx;
    bool hasGlobal = false, hasGroup = false;

    Section currentSection = Section::None;

    // El parser va carácter a carácter buscando headers <xxx> y opcodes key=value
    // Primero dividimos en tokens por línea
    juce::StringArray lines;
    lines.addTokens(content, "\n", "");

    for (auto& rawLine : lines)
    {
        // Eliminar comentario de línea //
        juce::String line = rawLine;
        int commentPos = line.indexOf("//");
        if (commentPos >= 0)
            line = line.substring(0, commentPos);
        line = line.trim();

        if (line.isEmpty()) continue;

        // Directiva #include
        if (line.startsWithIgnoreCase("#include"))
        {
            juce::String rest = line.substring(8).trim();
            rest = rest.trimCharactersAtStart("\"").trimCharactersAtEnd("\"");
            juce::File included = baseDir.getChildFile(rest);
            if (included.existsAsFile())
                parseFile(included, included.getParentDirectory(), outRegions, depth + 1);
            else
                warnings.add("Include no encontrado: " + rest);
            continue;
        }

        // Procesar la línea buscando headers <...> mezclados con opcodes
        // Un mismo línea puede tener: <region> sample=xx.wav lokey=60
        juce::String remaining = line;

        while (remaining.isNotEmpty())
        {
            int headerStart = remaining.indexOfChar('<');
            if (headerStart >= 0)
            {
                // Procesar opcodes antes del header
                juce::String before = remaining.substring(0, headerStart).trim();
                if (before.isNotEmpty() && currentSection == Section::Region)
                    processOpcodeString(before, regionCtx);
                else if (before.isNotEmpty() && currentSection == Section::Group)
                    processOpcodeString(before, groupCtx);
                else if (before.isNotEmpty() && currentSection == Section::Global)
                    processOpcodeString(before, globalCtx);

                int headerEnd = remaining.indexOf(headerStart, ">");
                if (headerEnd < 0) break;
                juce::String headerName = remaining.substring(headerStart + 1, headerEnd).trim().toLowerCase();
                remaining = remaining.substring(headerEnd + 1).trim();

                // Si cerramos una región, la guardamos
                if (currentSection == Section::Region)
                {
                    if (regionCtx.samplePath.isNotEmpty())
                        outRegions.push_back(regionCtx);
                }

                if (headerName == "global")
                {
                    globalCtx = SFZRegion();
                    hasGlobal = true;
                    currentSection = Section::Global;
                }
                else if (headerName == "group")
                {
                    groupCtx = hasGlobal ? globalCtx : SFZRegion();
                    hasGroup = true;
                    currentSection = Section::Group;
                }
                else if (headerName == "region")
                {
                    if (hasGroup)       regionCtx = groupCtx;
                    else if (hasGlobal) regionCtx = globalCtx;
                    else                regionCtx = SFZRegion();
                    currentSection = Section::Region;
                }
                else
                {
                    // Sección desconocida (curve, effect, etc.), ignorar
                    currentSection = Section::None;
                }
            }
            else
            {
                // Sin más headers en esta línea: son opcodes puros
                if (currentSection == Section::Region)
                    processOpcodeString(remaining, regionCtx);
                else if (currentSection == Section::Group)
                    processOpcodeString(remaining, groupCtx);
                else if (currentSection == Section::Global)
                    processOpcodeString(remaining, globalCtx);
                break;
            }
        }
    }

    // Guardar la última región pendiente
    if (currentSection == Section::Region && regionCtx.samplePath.isNotEmpty())
        outRegions.push_back(regionCtx);
}

// ──────────────────────────────────────────────────────────────────────────────
// processOpcodeString(): divide un string en pares key=value y los aplica
// Maneja rutas con espacios: sample=some file.wav lokey=60
// ──────────────────────────────────────────────────────────────────────────────
void SFZParser::processOpcodeString(const juce::String& str, SFZRegion& region)
{
    // Buscamos patrones "word=" y usamos sus posiciones como delimitadores
    // para encontrar el valor de cada opcode
    juce::String s = str.trim();
    if (s.isEmpty()) return;

    // Encontrar todas las posiciones donde aparece "key="
    struct OpcodePos { int start; juce::String key; };
    std::vector<OpcodePos> positions;

    int i = 0;
    while (i < s.length())
    {
        // Buscar inicio de identificador (letras, _, dígitos)
        if (juce::CharacterFunctions::isLetterOrDigit(s[i]) || s[i] == '_')
        {
            int keyStart = i;
            while (i < s.length() && (juce::CharacterFunctions::isLetterOrDigit(s[i]) || s[i] == '_'))
                ++i;
            // Comprobar si le sigue un '=' (ignorando espacios)
            int j = i;
            while (j < s.length() && s[j] == ' ') ++j;
            if (j < s.length() && s[j] == '=')
            {
                positions.push_back({ keyStart, s.substring(keyStart, i) });
                i = j + 1; // saltar el '='
            }
        }
        else
        {
            ++i;
        }
    }

    // Extraer los valores usando las posiciones de los siguientes opcodes
    for (int k = 0; k < (int)positions.size(); ++k)
    {
        int valueStart = positions[k].start + positions[k].key.length() + 1;
        // Saltar el '=' y espacios
        while (valueStart < s.length() && s[valueStart] == ' ') ++valueStart;
        while (valueStart < s.length() && s[valueStart] == '=') ++valueStart;
        while (valueStart < s.length() && s[valueStart] == ' ') ++valueStart;

        int valueEnd = (k + 1 < (int)positions.size()) ? positions[k + 1].start : s.length();
        juce::String value = s.substring(valueStart, valueEnd).trim();

        applyOpcode(positions[k].key.toLowerCase(), value, region);
    }
}

// ──────────────────────────────────────────────────────────────────────────────
// applyOpcode(): aplica un par key=value a la región
// ──────────────────────────────────────────────────────────────────────────────
void SFZParser::applyOpcode(const juce::String& key, const juce::String& value, SFZRegion& region)
{
    if (key == "sample")
    {
        // Normalizar separadores de ruta
        region.samplePath = value.replaceCharacter('\\', '/');
        return;
    }

    // ── Rango de teclas ───────────────────────────────────────────────────────
    if (key == "lokey")   { region.lokey = noteNameToMidi(value); return; }
    if (key == "hikey")   { region.hikey = noteNameToMidi(value); return; }
    if (key == "key")
    {
        int n = noteNameToMidi(value);
        region.lokey = region.hikey = region.pitchKeycenter = n;
        return;
    }
    if (key == "pitch_keycenter") { region.pitchKeycenter = noteNameToMidi(value); return; }

    // ── Velocidad ─────────────────────────────────────────────────────────────
    if (key == "lovel") { region.lovel = value.getIntValue(); return; }
    if (key == "hivel") { region.hivel = value.getIntValue(); return; }

    // ── Afinación ─────────────────────────────────────────────────────────────
    if (key == "tune")      { region.tune      = value.getFloatValue(); return; }
    if (key == "transpose") { region.transpose = value.getIntValue();   return; }

    // ── Dinámica ──────────────────────────────────────────────────────────────
    if (key == "volume") { region.volume = value.getFloatValue(); return; }
    if (key == "pan")    { region.pan    = value.getFloatValue(); return; }

    // ── Reproducción ──────────────────────────────────────────────────────────
    if (key == "offset") { region.offset = value.getIntValue(); return; }
    if (key == "end")    { region.end    = value.getIntValue(); return; }

    // ── Loop ──────────────────────────────────────────────────────────────────
    if (key == "loop_mode")
    {
        juce::String v = value.trim().toLowerCase();
        if (v == "no_loop")          region.loopMode = SFZLoopMode::NoLoop;
        else if (v == "loop_continuous" || v == "continuous") region.loopMode = SFZLoopMode::Continuous;
        else if (v == "loop_sustain" || v == "sustain")       region.loopMode = SFZLoopMode::SustainOnly;
        else if (v == "one_shot")    region.loopMode = SFZLoopMode::OneShot;
        region.loopModeExplicit = true;
        return;
    }
    if (key == "loop_start") { region.loopStart = value.getIntValue(); return; }
    if (key == "loop_end")   { region.loopEnd   = value.getIntValue(); return; }

    // ── Round-robin ───────────────────────────────────────────────────────────
    if (key == "seq_length")   { region.seqLength   = value.getIntValue(); return; }
    if (key == "seq_position") { region.seqPosition = value.getIntValue(); return; }

    // ── Aleatoriedad ──────────────────────────────────────────────────────────
    if (key == "lorand") { region.lorand = value.getFloatValue(); return; }
    if (key == "hirand") { region.hirand = value.getFloatValue(); return; }

    // ── Crossfade por nota ────────────────────────────────────────────────────
    if (key == "xfin_lokey")  { region.xfinLokey  = noteNameToMidi(value); return; }
    if (key == "xfin_hikey")  { region.xfinHikey  = noteNameToMidi(value); return; }
    if (key == "xfout_lokey") { region.xfoutLokey = noteNameToMidi(value); return; }
    if (key == "xfout_hikey") { region.xfoutHikey = noteNameToMidi(value); return; }

    // ── Crossfade por velocity ────────────────────────────────────────────────
    if (key == "xfin_lovel")  { region.xfinLovel  = value.getIntValue(); return; }
    if (key == "xfin_hivel")  { region.xfinHivel  = value.getIntValue(); return; }
    if (key == "xfout_lovel") { region.xfoutLovel = value.getIntValue(); return; }
    if (key == "xfout_hivel") { region.xfoutHivel = value.getIntValue(); return; }

    // ── Envolvente de amplitud ────────────────────────────────────────────────
    if (key == "ampeg_attack"  || key == "amp_attack")   { region.ampAttack  = value.getFloatValue(); return; }
    if (key == "ampeg_hold"    || key == "amp_hold")     { region.ampHold    = value.getFloatValue(); return; }
    if (key == "ampeg_decay"   || key == "amp_decay")    { region.ampDecay   = value.getFloatValue(); return; }
    if (key == "ampeg_sustain" || key == "amp_sustain")  { region.ampSustain = value.getFloatValue(); return; }
    if (key == "ampeg_release" || key == "amp_release")  { region.ampRelease = value.getFloatValue(); return; }

    // ── Filtro ────────────────────────────────────────────────────────────────
    if (key == "fil_type" || key == "filter_type")
    {
        region.filterType = parseFilterType(value.trim().toLowerCase());
        return;
    }
    if (key == "cutoff")     { region.cutoff    = value.getFloatValue(); return; }
    if (key == "resonance")  { region.resonance = value.getFloatValue(); return; }

    // Opcodes desconocidos: ignorar silenciosamente
}

// ──────────────────────────────────────────────────────────────────────────────
// noteNameToMidi(): convierte nombre de nota a MIDI (ej: "C#4" → 61)
// ──────────────────────────────────────────────────────────────────────────────
int SFZParser::noteNameToMidi(const juce::String& name)
{
    // Si es número directo, devolver tal cual
    if (name.containsOnly("0123456789-"))
        return name.getIntValue();

    juce::String n = name.trim().toUpperCase();
    if (n.isEmpty()) return 60;

    // Nombre de nota
    static const char* noteNames[] = { "C", "D", "E", "F", "G", "A", "B" };
    static const int   semitones[] = {  0,   2,   4,   5,   7,   9,  11  };

    int noteIdx = -1;
    for (int i = 0; i < 7; ++i)
        if (n.startsWith(noteNames[i])) { noteIdx = i; break; }

    if (noteIdx < 0) return 60; // nota desconocida

    int semi = semitones[noteIdx];
    int pos = 1;

    // Accidental
    if (pos < n.length() && n[pos] == '#') { semi++; pos++; }
    else if (pos < n.length() && n[pos] == 'B') { semi--; pos++; }

    // Octava (puede ser negativa, ej: C-1 = 0 en convención MIDI)
    juce::String octStr = n.substring(pos);
    int octave = octStr.isEmpty() ? 4 : octStr.getIntValue();

    return juce::jlimit(0, 127, (octave + 1) * 12 + semi);
}

// ──────────────────────────────────────────────────────────────────────────────
// parseFilterType(): convierte string a SFZFilterType
// ──────────────────────────────────────────────────────────────────────────────
SFZFilterType SFZParser::parseFilterType(const juce::String& s)
{
    if (s.contains("hp") || s.contains("high")) return SFZFilterType::HighPass;
    if (s.contains("bp") || s.contains("band")) return SFZFilterType::BandPass;
    if (s.contains("no") || s.contains("notch"))return SFZFilterType::Notch;
    return SFZFilterType::LowPass;
}
