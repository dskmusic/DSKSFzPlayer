#pragma once
#include <JuceHeader.h>
#include <map>
#include <vector>
#include "../SF2/SF2Synth.h"

// ══════════════════════════════════════════════════════════════════════════════
// SF2PresetPicker — contenido de un CallOutBox para elegir preset SF2.
// Lista de altura fija con scroll (nunca "enorme" aunque haya cientos de
// presets), selector de banco (solo si hay más de uno) y buscador dinámico
// que filtra en tiempo real sobre todos los bancos del archivo.
// ══════════════════════════════════════════════════════════════════════════════
class SF2PresetPicker : public juce::Component,
                        private juce::TextEditor::Listener,
                        private juce::ListBoxModel
{
public:
    SF2PresetPicker(const std::vector<SF2Synth::Preset>& presets, int currentPresetIndex);

    std::function<void(int presetIndex)> onPresetChosen;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    // juce::TextEditor::Listener
    void textEditorTextChanged(juce::TextEditor&) override;

    // juce::ListBoxModel
    int  getNumRows() override;
    void paintListBoxItem(int row, juce::Graphics& g, int width, int height, bool rowIsSelected) override;
    void listBoxItemClicked(int row, const juce::MouseEvent&) override;

    void rebuildFilteredList();
    void choose(int presetIndex);

    const std::vector<SF2Synth::Preset>& allPresets;
    int currentPresetIndex;
    std::vector<int> filtered; // indices into allPresets
    bool hasBankSelector = false;
    std::map<int, int> bankIdToBank; // ComboBox item id -> bank number

    juce::TextEditor searchBox;
    juce::ComboBox   bankBox;
    juce::ListBox    listBox;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SF2PresetPicker)
};
