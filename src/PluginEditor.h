#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "UI/DSKLookAndFeel.h"
#include "UI/LibraryPanel.h"

// ══════════════════════════════════════════════════════════════════════════════
// ADSRDisplay — live ADSR envelope visualiser
// ══════════════════════════════════════════════════════════════════════════════
class ADSRDisplay : public juce::Component
{
public:
    float attack = 0.001f;
    float decay = 0.1f;
    float sustain = 100.0f;
    float release = 0.05f;
    void paint(juce::Graphics& g) override;
};

// ══════════════════════════════════════════════════════════════════════════════
// DSKSFzEditor
// ══════════════════════════════════════════════════════════════════════════════
class DSKSFzEditor : public juce::AudioProcessorEditor,
    public LibraryPanel::Listener,
    public juce::FileDragAndDropTarget,
    public juce::Timer
{
public:
    explicit DSKSFzEditor(DSKSFzProcessor& p);
    ~DSKSFzEditor() override;

    void paint(juce::Graphics& g) override;
    void resized()                   override;
    void timerCallback()              override;
    void lookAndFeelChanged()         override;

    void mouseMove(const juce::MouseEvent& e) override;
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseUp(const juce::MouseEvent& e) override;

    // PC keyboard → MIDI note forwarding
    bool keyPressed(const juce::KeyPress& key) override;
    bool keyStateChanged(bool isKeyDown)            override;

    // LibraryPanel::Listener
    void libraryFileRequested(const juce::File& f) override;
    void libraryZipRequested(const juce::File& f) override;

    // FileDragAndDropTarget
    bool isInterestedInFileDrag(const juce::StringArray& files) override;
    void fileDragEnter(const juce::StringArray& files, int x, int y) override;
    void fileDragExit(const juce::StringArray& files) override;
    void filesDropped(const juce::StringArray& files, int x, int y) override;

private:
    DSKSFzProcessor& proc;
    DSKLookAndFeel   lf;

    bool isDragOver{ false };

    // Dimensiones y estados de arrastre
    int  libWidth{ 220 };
    int  pluginHeight{ 388 }; // Altura por defecto (se calcula dinámicamente en el cpp)
    bool isDraggingDivider{ false };
    bool isDraggingHDivider{ false };
    int  libWidthAtDragStart{ 220 };
    int  pluginHeightAtDragStart{ 388 };
    int  dragStartX{ 0 };
    int  dragStartY{ 0 };
    int  previewCountdown{ 0 };

    // ── Library ───────────────────────────────────────────────────────────────
    LibraryPanel libraryPanel;

    // ── Header ────────────────────────────────────────────────────────────────
    juce::HyperlinkButton titleLink;
    juce::HyperlinkButton siteLink;
    juce::Label           instrNameLabel;
    juce::Label           statusLabel;
    juce::TextButton      openBtn{ "Open SFZ / ZIP" };
    juce::TextButton      optionsBtn{ "Options" };

    // ── AMP section ───────────────────────────────────────────────────────────
    juce::Slider masterVolKnob, masterPanKnob;
    juce::Slider pitchTransKnob, pitchFineKnob;
    juce::Slider ampAKnob, ampDKnob, ampSKnob, ampRKnob;
    ADSRDisplay  adsrDisplay;

    // ── FILTER section ────────────────────────────────────────────────────────
    juce::ComboBox filtTypeBox;
    juce::Slider   filtCutKnob, filtResKnob, filtEnvKnob;
    juce::Slider   filtAKnob, filtDKnob, filtSKnob, filtRKnob;

    // ── MOD section ───────────────────────────────────────────────────────────
    juce::ComboBox lfo1ShapeBox, lfo1TgtBox, lfo2ShapeBox, lfo2TgtBox;
    juce::Slider   lfo1RateKnob, lfo1AmtKnob, lfo2RateKnob, lfo2AmtKnob;

    // ── FX section ────────────────────────────────────────────────────────────
    juce::TextButton driveOnBtn, chorusOnBtn, delayOnBtn, reverbOnBtn, eqOnBtn;
    juce::Slider driveAmtKnob, driveMixKnob;
    juce::Slider choRateKnob, choDepthKnob, choMixKnob;
    juce::Slider delTimeKnob, delFbkKnob, delMixKnob;
    juce::Slider revSizeKnob, revDampKnob, revMixKnob;
    juce::Slider eqLowKnob, eqMidKnob, eqHighKnob;

    // ── Footer ────────────────────────────────────────────────────────────────
    juce::Slider     voicesKnob;
    juce::TextButton rrResetBtn{ "RR Reset" };

    // ── Piano keyboard ────────────────────────────────────────────────────────
    juce::MidiKeyboardComponent keyboard;

    // ── Meters ────────────────────────────────────────────────────────────────
    std::atomic<float> meterL{ 0.0f }, meterR{ 0.0f };

    // ── File chooser ──────────────────────────────────────────────────────────
    std::unique_ptr<juce::FileChooser> fileChooser;

    // ── Knob labels ───────────────────────────────────────────────────────────
    struct KnobLabel { juce::Slider* s; std::unique_ptr<juce::Label> lbl; };
    std::vector<KnobLabel> kLabels;

    // ── APVTS attachments ─────────────────────────────────────────────────────
    using SA = juce::AudioProcessorValueTreeState::SliderAttachment;
    using BA = juce::AudioProcessorValueTreeState::ButtonAttachment;
    using CA = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    std::vector<std::unique_ptr<SA>> sAttach;
    std::vector<std::unique_ptr<BA>> bAttach;
    std::vector<std::unique_ptr<CA>> cAttach;

    // ── Build helpers ─────────────────────────────────────────────────────────
    void buildUI();
    void addKL(juce::Slider& s, const juce::String& text);
    void mk(juce::Slider& s);
    void mkFXBtn(juce::TextButton& b, const juce::String& name);
    void sa(juce::Slider& s, const juce::String& id);
    void ba(juce::TextButton& b, const juce::String& id);
    void ca(juce::ComboBox& c, const juce::String& id);
    void applyWindowConstraints();
    void triggerPreviewNote();
    void loadSFZForPreview(const juce::File& f);

    // Funciones de detección de tiradores
    bool isNearDivider(int x) const;
    bool isNearHDivider(int y) const;

    // ── Paint helpers ─────────────────────────────────────────────────────────
    void secBg(juce::Graphics& g, juce::Rectangle<int> r, const juce::String& t);
    void drawMeter(juce::Graphics& g, int x, int y, int w, int h, float lvl, const char* ch);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DSKSFzEditor)
};