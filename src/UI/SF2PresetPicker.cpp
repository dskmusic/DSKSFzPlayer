#include "SF2PresetPicker.h"
#include <set>

SF2PresetPicker::SF2PresetPicker(const std::vector<SF2Synth::Preset>& presets, int currentIndex)
    : allPresets(presets), currentPresetIndex(currentIndex)
{
    searchBox.setTextToShowWhenEmpty("Search presets...", juce::Colours::grey);
    searchBox.addListener(this);
    addAndMakeVisible(searchBox);

    std::set<int> banks;
    for (auto& p : allPresets) banks.insert(p.bank);
    hasBankSelector = banks.size() > 1;

    if (hasBankSelector)
    {
        bankBox.addItem("All banks", 1);
        int id = 2;
        for (auto b : banks)
        {
            bankBox.addItem("Bank " + juce::String(b), id);
            bankIdToBank[id] = b;
            ++id;
        }
        bankBox.setSelectedId(1, juce::dontSendNotification);
        bankBox.onChange = [this] { rebuildFilteredList(); };
        addAndMakeVisible(bankBox);
    }

    listBox.setModel(this);
    listBox.setRowHeight(22);
    listBox.setColour(juce::ListBox::backgroundColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(listBox);

    rebuildFilteredList();

    const int visibleRows = juce::jlimit(6, 14, (int)allPresets.size());
    const int headerH = 8 + 26 + 4 + (hasBankSelector ? 26 + 4 : 0);
    setSize(300, headerH + visibleRows * 22 + 8);
}

void SF2PresetPicker::paint(juce::Graphics& g)
{
    g.fillAll(findColour(juce::PopupMenu::backgroundColourId));
}

void SF2PresetPicker::resized()
{
    auto r = getLocalBounds().reduced(6);
    searchBox.setBounds(r.removeFromTop(26));
    r.removeFromTop(4);
    if (hasBankSelector)
    {
        bankBox.setBounds(r.removeFromTop(26));
        r.removeFromTop(4);
    }
    listBox.setBounds(r);
}

void SF2PresetPicker::textEditorTextChanged(juce::TextEditor&)
{
    rebuildFilteredList();
}

void SF2PresetPicker::rebuildFilteredList()
{
    const juce::String query = searchBox.getText().trim().toLowerCase();
    int wantedBank = -1;
    if (hasBankSelector && bankBox.getSelectedId() > 1)
    {
        auto it = bankIdToBank.find(bankBox.getSelectedId());
        if (it != bankIdToBank.end()) wantedBank = it->second;
    }

    filtered.clear();
    for (int i = 0; i < (int)allPresets.size(); ++i)
    {
        auto& p = allPresets[(size_t)i];
        if (wantedBank >= 0 && p.bank != wantedBank) continue;
        if (query.isNotEmpty() && !p.name.toLowerCase().contains(query)) continue;
        filtered.push_back(i);
    }

    listBox.updateContent();
    listBox.repaint();
}

int SF2PresetPicker::getNumRows()
{
    return (int)filtered.size();
}

void SF2PresetPicker::paintListBoxItem(int row, juce::Graphics& g, int width, int height, bool rowIsSelected)
{
    if (row < 0 || row >= (int)filtered.size()) return;
    auto& p = allPresets[(size_t)filtered[(size_t)row]];
    const bool isCurrent = (p.index == currentPresetIndex);

    if (rowIsSelected)
        g.fillAll(findColour(juce::PopupMenu::highlightedBackgroundColourId));

    g.setColour(isCurrent ? findColour(juce::PopupMenu::highlightedTextColourId)
                          : findColour(juce::PopupMenu::textColourId));
    g.setFont(juce::Font(13.0f, isCurrent ? juce::Font::bold : juce::Font::plain));
    juce::String label = juce::String::formatted("%03d:%03d  ", p.bank, p.presetNumber) + p.name;
    g.drawText(label, 6, 0, width - 8, height, juce::Justification::centredLeft);
}

void SF2PresetPicker::listBoxItemClicked(int row, const juce::MouseEvent&)
{
    if (row < 0 || row >= (int)filtered.size()) return;
    choose(allPresets[(size_t)filtered[(size_t)row]].index);
}

void SF2PresetPicker::choose(int presetIndex)
{
    if (onPresetChosen) onPresetChosen(presetIndex);
    if (auto* cb = findParentComponentOfClass<juce::CallOutBox>())
        cb->dismiss();
}
