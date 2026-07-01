#include <JuceHeader.h>
#include "PluginEditor.h"

// ── Help dialog ───────────────────────────────────────────────────────────────
class HelpDialog : public juce::Component
{
public:
    HelpDialog()
    {
        text.setReadOnly(true);
        text.setMultiLine(true, false);
        text.setScrollbarsShown(true);
        text.setFont(juce::Font(juce::Font::getDefaultMonospacedFontName(), 12.5f, 0));
        text.setColour(juce::TextEditor::backgroundColourId, juce::Colour(0xFF0D1828));
        text.setColour(juce::TextEditor::textColourId, juce::Colour(0xFFCCDDEE));
        text.setColour(juce::TextEditor::outlineColourId, juce::Colour(0xFF334466));
        text.setColour(juce::TextEditor::focusedOutlineColourId, juce::Colour(0xFF334466));

        // Load from embedded binary resource
        const juce::String content = juce::String::fromUTF8(
            reinterpret_cast<const char*>(BinaryData::manual_md),
            BinaryData::manual_mdSize);
        text.setText(content, juce::dontSendNotification);
        text.moveCaretToTop(false);

        closeBtn.setButtonText("Close");
        closeBtn.setColour(juce::TextButton::buttonColourId, juce::Colour(0xFF0F3460));
        closeBtn.setColour(juce::TextButton::textColourOffId, juce::Colour(0xFF00D4FF));
        closeBtn.onClick = [this]()
        {
            if (auto* dw = findParentComponentOfClass<juce::DialogWindow>())
                dw->closeButtonPressed();
        };

        addAndMakeVisible(text);
        addAndMakeVisible(closeBtn);
        setSize(720, 580);
    }

    void resized() override
    {
        auto b = getLocalBounds().reduced(6);
        closeBtn.setBounds(b.removeFromBottom(28).removeFromRight(90));
        b.removeFromBottom(6);
        text.setBounds(b);
    }

private:
    juce::TextEditor text;
    juce::TextButton closeBtn;
};

// ── Layout constants ──────────────────────────────────────────────────────────
static constexpr int LIB_W = 220;
static constexpr int HDR_H = 48;
static constexpr int KSZ = 42;
static constexpr int KSLOT = 52;
static constexpr int KNOB_Y = 18;
static constexpr int ROW_H = KNOB_Y + KSZ + 16 + 10;
static constexpr int FTR_H = 30;
static constexpr int KB_H = 56;
static constexpr int VDIV_H = 6; // Altura del tirador horizontal
static constexpr int DEF_H = HDR_H + 3 * ROW_H + FTR_H + VDIV_H + KB_H;

static int colX(int rx, int col) { return rx + 8 + col * KSLOT; }

// ══════════════════════════════════════════════════════════════════════════════
// ADSRDisplay
// ══════════════════════════════════════════════════════════════════════════════
void ADSRDisplay::paint(juce::Graphics& g)
{
    auto* dskLF = dynamic_cast<const DSKLookAndFeel*>(&getLookAndFeel());
    const juce::Colour accent = dskLF ? dskLF->getTheme().accent : juce::Colour(0xFF00D4FF);
    const juce::Colour bgCol = dskLF ? dskLF->getTheme().lcdBg : juce::Colour(0xFF0D1828);
    const juce::Colour bdCol = dskLF ? dskLF->getTheme().border : juce::Colour(0xFF334466);

    auto b = getLocalBounds().toFloat().reduced(2.0f);
    g.setColour(bgCol);
    g.fillRoundedRectangle(b, 3.0f);
    g.setColour(bdCol);
    g.drawRoundedRectangle(b, 3.0f, 1.0f);

    float total = attack + decay + 0.25f + release;
    if (total < 0.001f) return;

    float xA = attack / total;
    float xD = xA + decay / total;
    float xS = xD + 0.25f / total;
    float yS = juce::jlimit(0.0f, 1.0f, sustain / 100.0f);

    float pw = b.getWidth() - 4.0f;
    float ph = b.getHeight() - 4.0f;
    float ox = b.getX() + 2.0f;
    float oy = b.getY() + 2.0f;

    auto xp = [&](float t) { return ox + t * pw; };
    auto yp = [&](float v) { return oy + ph * (1.0f - v); };

    juce::Path curve;
    curve.startNewSubPath(xp(0.f), yp(0.f));
    curve.lineTo(xp(xA), yp(1.f));
    curve.lineTo(xp(xD), yp(yS));
    curve.lineTo(xp(xS), yp(yS));
    curve.lineTo(xp(1.f), yp(0.f));

    juce::Path fill = curve;
    fill.lineTo(xp(1.f), yp(0.f));
    fill.lineTo(xp(0.f), yp(0.f));
    fill.closeSubPath();
    g.setColour(accent.withAlpha(0.18f));
    g.fillPath(fill);
    g.setColour(accent);
    g.strokePath(curve, juce::PathStrokeType(1.5f));
}

// ══════════════════════════════════════════════════════════════════════════════
// DSKSFzEditor
// ══════════════════════════════════════════════════════════════════════════════
DSKSFzEditor::DSKSFzEditor(DSKSFzProcessor& p)
    : AudioProcessorEditor(p),
    proc(p),
    titleLink("DSK SFz player", juce::URL("https://www.dskmusic.com")),
    siteLink("www.dskmusic.com", juce::URL("https://www.dskmusic.com")),
    keyboard(p.keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard)
{
    setLookAndFeel(&lf);

    pluginHeight = DEF_H;

    // Restore saved theme before buildUI so colours are applied immediately
    if (auto* s = proc.getAppProperties().getUserSettings())
    {
        lf.setTheme(s->getIntValue("uiTheme", 0));
        libWidth = juce::jlimit(120, 450, s->getIntValue("libWidth", 220));
        pluginHeight = juce::jlimit(DEF_H, 1200, s->getIntValue("pluginHeight", DEF_H));
        libraryPanel.loadState(*s);
    }

    buildUI();

    libraryPanel.addListener(this);
    libraryPanel.onThemeChanged = [this](int idx)
    {
        lf.setTheme(idx);
        sendLookAndFeelChange();
        repaint();
        if (auto* s = proc.getAppProperties().getUserSettings())
            s->setValue("uiTheme", idx);
    };
    libraryPanel.onSFZPreviewRequested = [this](const juce::File& f)
    {
        if (f == proc.getCurrentSFZFile())
            triggerPreviewNote();
        else
            loadSFZForPreview(f);
    };

    setSize(libWidth + 780, pluginHeight);

    // Habilitar redimensión/maximizado SOLO en Standalone
    if (proc.wrapperType == juce::AudioProcessor::wrapperType_Standalone)
    {
        setResizable(true, true);

        // Pedimos al hilo principal que espere a que el editor se acople a la ventana...
        juce::MessageManager::callAsync([this]()
            {
                // ...buscamos la ventana principal contenedora (DocumentWindow)
                if (auto* dw = findParentComponentOfClass<juce::DocumentWindow>())
                {
                    // ...y le forzamos a mostrar todos los botones (Minimizar, Maximizar y Cerrar)
                    dw->setTitleBarButtonsRequired(juce::DocumentWindow::allButtons, false);
                }
            });
    }
    else
    {
        setResizable(false, false);
    }

    applyWindowConstraints();

    startTimerHz(12);
}

DSKSFzEditor::~DSKSFzEditor()
{
    stopTimer();
    libraryPanel.removeListener(this);
    if (auto* s = proc.getAppProperties().getUserSettings())
    {
        s->setValue("uiTheme", lf.getThemeIndex());
        s->setValue("libWidth", libWidth);
        s->setValue("pluginHeight", pluginHeight);
        libraryPanel.saveState(*s);
        s->saveIfNeeded();
    }
    setLookAndFeel(nullptr);
}

// ── LookAndFeel changed ───────────────────────────────────────────────────────
void DSKSFzEditor::lookAndFeelChanged()
{
    const auto& t = lf.getTheme();

    instrNameLabel.setColour(juce::Label::textColourId, t.lcdText);
    statusLabel.setColour(juce::Label::textColourId, t.subtext);
    titleLink.setColour(juce::HyperlinkButton::textColourId, t.accent);
    siteLink.setColour(juce::HyperlinkButton::textColourId, t.subtext);

    openBtn.setColour(juce::TextButton::buttonColourId, t.sectionBg);
    openBtn.setColour(juce::TextButton::textColourOffId, t.accent);
    optionsBtn.setColour(juce::TextButton::buttonColourId, t.sectionBg);
    optionsBtn.setColour(juce::TextButton::textColourOffId, t.subtext);
    addFavBtn.setColour(juce::TextButton::buttonColourId, t.sectionBg);
    addFavBtn.setColour(juce::TextButton::textColourOffId, t.favGold);
    rrResetBtn.setColour(juce::TextButton::buttonColourId, t.sectionBg);
    rrResetBtn.setColour(juce::TextButton::textColourOffId, t.subtext);
    voicesKnob.setColour(juce::Slider::textBoxTextColourId, t.text);

    const juce::Colour wKey = t.isDark ? juce::Colour(0xFFD8E8F0) : juce::Colour(0xFFF8F8F8);
    const juce::Colour bKey = t.isDark ? juce::Colour(0xFF0D1828) : juce::Colour(0xFF1A1A2A);
    keyboard.setColour(juce::MidiKeyboardComponent::whiteNoteColourId, wKey);
    keyboard.setColour(juce::MidiKeyboardComponent::blackNoteColourId, bKey);
    keyboard.setColour(juce::MidiKeyboardComponent::mouseOverKeyOverlayColourId, t.accent.withAlpha(0.2f));
    keyboard.setColour(juce::MidiKeyboardComponent::keyDownOverlayColourId, t.accent.withAlpha(0.5f));
    keyboard.setColour(juce::MidiKeyboardComponent::keySeparatorLineColourId, t.border);

    const juce::Colour fxBtnBg = t.isDark ? juce::Colour(0xFF1A2438) : t.sectionBg;
    const juce::Colour fxBtnBgOn = t.sectionBg.brighter(0.2f);
    for (auto* b : { &driveOnBtn, &chorusOnBtn, &delayOnBtn, &reverbOnBtn, &eqOnBtn })
    {
        b->setColour(juce::TextButton::buttonColourId, fxBtnBg);
        b->setColour(juce::TextButton::buttonOnColourId, fxBtnBgOn);
        b->setColour(juce::TextButton::textColourOffId, t.subtext);
        b->setColour(juce::TextButton::textColourOnId, t.accent);
    }

    // update knob label colours
    for (auto& kl : kLabels)
        kl.lbl->setColour(juce::Label::textColourId, t.text.withAlpha(0.75f));

    repaint();
}

// ── Helpers ───────────────────────────────────────────────────────────────────
void DSKSFzEditor::mk(juce::Slider& s)
{
    s.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    s.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    s.setPopupDisplayEnabled(true, true, this);
    addAndMakeVisible(s);
}

void DSKSFzEditor::addKL(juce::Slider& s, const juce::String& text)
{
    auto lbl = std::make_unique<juce::Label>();
    lbl->setText(text, juce::dontSendNotification);
    lbl->setFont(juce::Font(11.0f, juce::Font::bold));
    lbl->setColour(juce::Label::textColourId, juce::Colour(0xFFAABBCC));
    lbl->setColour(juce::Label::backgroundColourId, juce::Colours::transparentBlack);
    lbl->setJustificationType(juce::Justification::centred);
    lbl->setInterceptsMouseClicks(false, false);
    addAndMakeVisible(*lbl);
    kLabels.push_back({ &s, std::move(lbl) });
}

void DSKSFzEditor::mkFXBtn(juce::TextButton& b, const juce::String& name)
{
    b.setButtonText(name);
    b.setClickingTogglesState(true);
    b.setColour(juce::TextButton::buttonColourId, juce::Colour(0xFF1A2438));
    b.setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xFF00556E));
    b.setColour(juce::TextButton::textColourOffId, juce::Colour(0xFFAABBCC));
    b.setColour(juce::TextButton::textColourOnId, juce::Colour(0xFF00D4FF));
    b.setTooltip(name + ": click to enable / disable");
    addAndMakeVisible(b);
}

void DSKSFzEditor::sa(juce::Slider& s, const juce::String& id)
{
    sAttach.push_back(std::make_unique<SA>(proc.params, id, s));
}

void DSKSFzEditor::ba(juce::TextButton& b, const juce::String& id)
{
    bAttach.push_back(std::make_unique<BA>(proc.params, id, b));
}

void DSKSFzEditor::ca(juce::ComboBox& c, const juce::String& id)
{
    cAttach.push_back(std::make_unique<CA>(proc.params, id, c));
}

// ── Funciones de detección para tiradores ─────────────────────────────────────
bool DSKSFzEditor::isNearDivider(int x) const
{
    return x >= libWidth && x <= libWidth + 5;
}

bool DSKSFzEditor::isNearHDivider(int y) const
{
    return y >= getHeight() - KB_H - VDIV_H && y <= getHeight() - KB_H;
}

// ── buildUI ───────────────────────────────────────────────────────────────────
void DSKSFzEditor::buildUI()
{
    titleLink.setFont(juce::Font(15.0f, juce::Font::bold), false);
    titleLink.setColour(juce::HyperlinkButton::textColourId, juce::Colour(0xFF00D4FF));
    titleLink.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(titleLink);

    siteLink.setFont(juce::Font(10.0f), false);
    siteLink.setColour(juce::HyperlinkButton::textColourId, juce::Colour(0xFF4488AA));
    siteLink.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(siteLink);

    instrNameLabel.setFont(juce::Font(16.0f, juce::Font::bold)); // Aumentamos la fuente de 13 a 16
    instrNameLabel.setColour(juce::Label::textColourId, juce::Colour(0xFF00E6CC));
    instrNameLabel.setJustificationType(juce::Justification::centred);
    instrNameLabel.setText("No instrument loaded", juce::dontSendNotification);
    addAndMakeVisible(instrNameLabel);

    statusLabel.setFont(juce::Font(10.0f));
    statusLabel.setColour(juce::Label::textColourId, juce::Colour(0xFF7799BB));
    statusLabel.setJustificationType(juce::Justification::centred);
    statusLabel.setText("Ready", juce::dontSendNotification);
    addAndMakeVisible(statusLabel);

    addFavBtn.setTooltip("Add current instrument to Favorites / Añadir a favoritos");
    addFavBtn.onFavClick = [this](const juce::MouseEvent& e)
    {
        auto f = proc.getCurrentSFZFile();
        if (f.existsAsFile())
        {
            libraryPanel.showAddToFavoritesMenu(f, e); // Ahora sí le pasamos el evento real
        }
        else
        {
            juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,
                "Favorites", "No instrument loaded to add to favorites.");
        }
    };
    addAndMakeVisible(addFavBtn);

    openBtn.setColour(juce::TextButton::buttonColourId, juce::Colour(0xFF0F3460));
    openBtn.setColour(juce::TextButton::textColourOffId, juce::Colour(0xFF00D4FF));
    openBtn.setTooltip("Open an SFZ file or ZIP instrument pack");
    openBtn.onClick = [this]()
    {
        fileChooser = std::make_unique<juce::FileChooser>(
            "Open SFZ instrument", proc.getCurrentSFZFile().getParentDirectory(),
            "*.sfz;*.SFZ;*.zip;*.ZIP");
        fileChooser->launchAsync(
            juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
            [this](const juce::FileChooser& fc)
            {
                auto f = fc.getResult();
        if (!f.existsAsFile()) return;
        if (f.hasFileExtension(".zip")) libraryZipRequested(f);
        else                            libraryFileRequested(f);
            });
    };
    addAndMakeVisible(openBtn);

    optionsBtn.setColour(juce::TextButton::buttonColourId, juce::Colour(0xFF0F3460));
    optionsBtn.setColour(juce::TextButton::textColourOffId, juce::Colour(0xFF8899AA));
    optionsBtn.setTooltip("Plugin options");
    optionsBtn.onClick = [this]()
    {
        // ── Theme submenu ─────────────────────────────────────────────────────
        const char* themeNames[DSKLookAndFeel::NUM_THEMES] = {
            "Deep Navy", "Midnight", "Forest", "Deep Purple", "Light Gray", "Warm Cream"
        };
        juce::PopupMenu themeMenu;
        for (int i = 0; i < DSKLookAndFeel::NUM_THEMES; ++i)
        {
            bool isCurrent = (lf.getThemeIndex() == i);
            juce::String label = juce::String(themeNames[i]);
            if (i < 4) label += " (dark)"; else label += " (light)";
            themeMenu.addItem(50 + i, label, true, isCurrent);
        }

        juce::PopupMenu menu;
        menu.addSectionHeader("DSK SFz player");
        menu.addItem(1, "All notes off");
        menu.addItem(2, "Reset round-robin counters");
        menu.addSeparator();
        menu.addItem(3, "Reset all parameters");
        menu.addSeparator();
        menu.addItem(10, "Export config...");
        menu.addItem(11, "Import config...");
        menu.addSeparator();
        menu.addSubMenu("Theme", themeMenu);
        menu.addSeparator();
        menu.addItem(5, "Help...");
        menu.addItem(4, "About...");

        menu.showMenuAsync(
            juce::PopupMenu::Options()
            .withParentComponent(this)
            .withTargetComponent(&optionsBtn),
            [this](int r)
            {
                if (r == 1)
                {
                    proc.getSynth().allNotesOff();
                }
                else if (r == 2)
                {
                    proc.getSynth().resetRoundRobin();
                }
                else if (r == 3)
                {
                    juce::AlertWindow::showAsync(
                        juce::MessageBoxOptions()
                        .withIconType(juce::MessageBoxIconType::QuestionIcon)
                        .withTitle("Reset All Parameters")
                        .withMessage("Are you sure you want to reset all parameters and interface dimensions to their default values?")
                        .withButton("Reset")
                        .withButton("Cancel"),
                        [this](int result)
                        {
                            if (result == 1) // Si el usuario hace clic en "Reset"
                            {
                                // 1. Restaurar todos los parámetros de audio
                                for (auto* p : proc.getParameters())
                                    p->setValueNotifyingHost(p->getDefaultValue());

                                // 2. Restaurar las dimensiones por defecto de la interfaz
                                libWidth = LIB_W;
                                pluginHeight = DEF_H;
                                setSize(libWidth + 780, pluginHeight);
                                applyWindowConstraints();
                            }
                        });
                }
                else if (r == 10)
                {
                    libraryPanel.startExport();
                }
                else if (r == 11)
                {
                    libraryPanel.startImport();
                }
                else if (r >= 50 && r < 50 + DSKLookAndFeel::NUM_THEMES)
                {
                    lf.setTheme(r - 50);
                    sendLookAndFeelChange();
                    repaint();
                    if (auto* s = proc.getAppProperties().getUserSettings())
                        s->setValue("uiTheme", r - 50);
                }
                else if (r == 5) // Help
                {
                    juce::DialogWindow::LaunchOptions opts;
                    opts.dialogTitle = "DSK SFz Player  —  Help / Ayuda";
                    opts.content.setOwned(new HelpDialog());
                    opts.resizable = true;
                    opts.useNativeTitleBar = true;
                    opts.componentToCentreAround = this;
                    opts.launchAsync();
                }
                else if (r == 4)
                {
                    auto* dlg = new juce::AlertWindow(
                        "DSK SFz player",
                        "Version 1.0  |  DSK Music\n\n"
                        "64-voice polyphonic SFZ sampler.\n"
                        "Supports WAV, FLAC, and OGG formats.\n\n"
                        "Drop SFZ, folder, or ZIP onto the plugin window to load.\n\n"
                        "Click \"Visit Website\" to open www.dskmusic.com",
                        juce::AlertWindow::InfoIcon, this);
                    dlg->addButton("Visit Website", 1);
                    dlg->addButton("Close", 0);
                    dlg->enterModalState(true,
                        juce::ModalCallbackFunction::create([](int res)
                            {
                                if (res == 1)
                                juce::URL("https://www.dskmusic.com").launchInDefaultBrowser();
                            }), true);
                }
            });
    };
    addAndMakeVisible(optionsBtn);

    addAndMakeVisible(libraryPanel);

    // ── AMP section ───────────────────────────────────────────────────────────
    masterVolKnob.setRange(-60.0, 6.0, 0.1);
    masterVolKnob.setTooltip("Master Volume (-60 to +6 dB)");
    mk(masterVolKnob); addKL(masterVolKnob, "Volume");
    sa(masterVolKnob, "masterVolume");

    masterPanKnob.setRange(-100.0, 100.0, 1.0);
    masterPanKnob.setComponentID("bipolar");
    masterPanKnob.setTooltip("Master Pan (-100 = left, +100 = right)");
    mk(masterPanKnob); addKL(masterPanKnob, "Pan");
    sa(masterPanKnob, "masterPan");

    pitchTransKnob.setRange(-24.0, 24.0, 1.0);
    pitchTransKnob.setComponentID("bipolar");
    pitchTransKnob.setTooltip("Transpose in semitones");
    mk(pitchTransKnob); addKL(pitchTransKnob, "Semit.");
    sa(pitchTransKnob, "transpose");

    pitchFineKnob.setRange(-100.0, 100.0, 0.5);
    pitchFineKnob.setComponentID("bipolar");
    pitchFineKnob.setTooltip("Fine tune in cents");
    mk(pitchFineKnob); addKL(pitchFineKnob, "Cents");
    sa(pitchFineKnob, "fineTune");

    ampAKnob.setRange(0.001, 10.0, 0.001);
    ampAKnob.setTooltip("Amp Attack (seconds)");
    mk(ampAKnob); addKL(ampAKnob, "A");
    sa(ampAKnob, "ampAttack");

    ampDKnob.setRange(0.001, 10.0, 0.001);
    ampDKnob.setTooltip("Amp Decay (seconds)");
    mk(ampDKnob); addKL(ampDKnob, "D");
    sa(ampDKnob, "ampDecay");

    ampSKnob.setRange(0.0, 100.0, 0.1);
    ampSKnob.setTooltip("Amp Sustain (%)");
    mk(ampSKnob); addKL(ampSKnob, "S");
    sa(ampSKnob, "ampSustain");

    ampRKnob.setRange(0.001, 10.0, 0.001);
    ampRKnob.setTooltip("Amp Release (seconds)");
    mk(ampRKnob); addKL(ampRKnob, "R");
    sa(ampRKnob, "ampRelease");

    addAndMakeVisible(adsrDisplay);

    // ── FILTER section ────────────────────────────────────────────────────────
    filtTypeBox.addItem("LP", 1); filtTypeBox.addItem("HP", 2);
    filtTypeBox.addItem("BP", 3); filtTypeBox.addItem("Notch", 4);
    filtTypeBox.setTooltip("Filter type: Low-Pass / High-Pass / Band-Pass / Notch");
    addAndMakeVisible(filtTypeBox);
    ca(filtTypeBox, "filterType");

    filtCutKnob.setRange(20.0, 20000.0, 1.0);
    filtCutKnob.setSkewFactorFromMidPoint(1200.0);
    filtCutKnob.setTooltip("Cutoff frequency (Hz)");
    mk(filtCutKnob); addKL(filtCutKnob, "Cutoff");
    sa(filtCutKnob, "filterCutoff");

    filtResKnob.setRange(0.1, 20.0, 0.01);
    filtResKnob.setTooltip("Resonance / Q");
    mk(filtResKnob); addKL(filtResKnob, "Res");
    sa(filtResKnob, "filterRes");

    filtEnvKnob.setRange(-1.0, 1.0, 0.001);
    filtEnvKnob.setComponentID("bipolar");
    filtEnvKnob.setTooltip("Filter envelope amount");
    mk(filtEnvKnob); addKL(filtEnvKnob, "Env");
    sa(filtEnvKnob, "filterEnvAmt");

    filtAKnob.setRange(0.001, 10.0, 0.001); mk(filtAKnob); addKL(filtAKnob, "A");
    filtDKnob.setRange(0.001, 10.0, 0.001); mk(filtDKnob); addKL(filtDKnob, "D");
    filtSKnob.setRange(0.0, 100.0, 0.1);   mk(filtSKnob); addKL(filtSKnob, "S");
    filtRKnob.setRange(0.001, 10.0, 0.001); mk(filtRKnob); addKL(filtRKnob, "R");
    filtAKnob.setTooltip("Filter env Attack");  sa(filtAKnob, "filterAttack");
    filtDKnob.setTooltip("Filter env Decay");   sa(filtDKnob, "filterDecay");
    filtSKnob.setTooltip("Filter env Sustain"); sa(filtSKnob, "filterSustain");
    filtRKnob.setTooltip("Filter env Release"); sa(filtRKnob, "filterRelease");

    // ── MOD section ───────────────────────────────────────────────────────────
    auto buildLFO = [&](juce::ComboBox& shp, juce::ComboBox& tgt,
        juce::Slider& rate, juce::Slider& amt,
        const juce::String& shpId, const juce::String& tgtId,
        const juce::String& rateId, const juce::String& amtId)
    {
        for (auto* c : { &shp, &tgt }) addAndMakeVisible(*c);
        shp.addItem("Sine", 1); shp.addItem("Tri", 2);
        shp.addItem("Saw", 3); shp.addItem("Sq", 4); shp.addItem("S&H", 5);
        tgt.addItem("Pitch", 1); tgt.addItem("Cutoff", 2);
        tgt.addItem("Pan", 3);  tgt.addItem("Vol", 4);
        shp.setTooltip("LFO waveform"); tgt.setTooltip("LFO modulation target");
        rate.setRange(0.1, 20.0, 0.01); rate.setTooltip("LFO rate (Hz)");
        amt.setRange(0.0, 1.0, 0.001); amt.setTooltip("LFO depth");
        mk(rate); addKL(rate, "Rate");
        mk(amt);  addKL(amt, "Depth");
        ca(shp, shpId); ca(tgt, tgtId);
        sa(rate, rateId); sa(amt, amtId);
    };
    buildLFO(lfo1ShapeBox, lfo1TgtBox, lfo1RateKnob, lfo1AmtKnob,
        "lfo1Shape", "lfo1Target", "lfo1Rate", "lfo1Amount");
    buildLFO(lfo2ShapeBox, lfo2TgtBox, lfo2RateKnob, lfo2AmtKnob,
        "lfo2Shape", "lfo2Target", "lfo2Rate", "lfo2Amount");

    // ── FX section ────────────────────────────────────────────────────────────
    mkFXBtn(driveOnBtn, "DRIVE");
    mkFXBtn(chorusOnBtn, "CHORUS");
    mkFXBtn(delayOnBtn, "DELAY");
    mkFXBtn(reverbOnBtn, "REVERB");
    mkFXBtn(eqOnBtn, "EQ");
    ba(driveOnBtn, "driveOn"); ba(chorusOnBtn, "chorusOn");
    ba(delayOnBtn, "delayOn"); ba(reverbOnBtn, "reverbOn");
    ba(eqOnBtn, "eqOn");

    driveAmtKnob.setRange(0.0, 1.0, 0.001); mk(driveAmtKnob); addKL(driveAmtKnob, "Drive");
    driveMixKnob.setRange(0.0, 1.0, 0.001); mk(driveMixKnob); addKL(driveMixKnob, "Mix");
    sa(driveAmtKnob, "driveAmount"); sa(driveMixKnob, "driveMix");

    choRateKnob.setRange(0.1, 10.0, 0.01);  mk(choRateKnob);  addKL(choRateKnob, "Rate");
    choDepthKnob.setRange(0.0, 1.0, 0.001); mk(choDepthKnob); addKL(choDepthKnob, "Depth");
    choMixKnob.setRange(0.0, 1.0, 0.001);   mk(choMixKnob);   addKL(choMixKnob, "Mix");
    sa(choRateKnob, "chorusRate"); sa(choDepthKnob, "chorusDepth"); sa(choMixKnob, "chorusMix");

    delTimeKnob.setRange(1.0, 2000.0, 1.0); mk(delTimeKnob); addKL(delTimeKnob, "Time");
    delFbkKnob.setRange(0.0, 0.95, 0.001);  mk(delFbkKnob);  addKL(delFbkKnob, "Fbk");
    delMixKnob.setRange(0.0, 1.0, 0.001);   mk(delMixKnob);  addKL(delMixKnob, "Mix");
    sa(delTimeKnob, "delayTime"); sa(delFbkKnob, "delayFeedback"); sa(delMixKnob, "delayMix");

    revSizeKnob.setRange(0.0, 1.0, 0.001); mk(revSizeKnob); addKL(revSizeKnob, "Size");
    revDampKnob.setRange(0.0, 1.0, 0.001); mk(revDampKnob); addKL(revDampKnob, "Damp");
    revMixKnob.setRange(0.0, 1.0, 0.001);  mk(revMixKnob);  addKL(revMixKnob, "Mix");
    sa(revSizeKnob, "reverbSize"); sa(revDampKnob, "reverbDamping"); sa(revMixKnob, "reverbMix");

    eqLowKnob.setRange(-12.0, 12.0, 0.1);  eqLowKnob.setComponentID("bipolar");
    eqMidKnob.setRange(-12.0, 12.0, 0.1);  eqMidKnob.setComponentID("bipolar");
    eqHighKnob.setRange(-12.0, 12.0, 0.1); eqHighKnob.setComponentID("bipolar");
    mk(eqLowKnob); addKL(eqLowKnob, "Low");
    mk(eqMidKnob); addKL(eqMidKnob, "Mid");
    mk(eqHighKnob); addKL(eqHighKnob, "High");
    sa(eqLowKnob, "eqLow"); sa(eqMidKnob, "eqMid"); sa(eqHighKnob, "eqHigh");

    // ── Footer ────────────────────────────────────────────────────────────────
    voicesKnob.setRange(1, 64, 1);
    voicesKnob.setSliderStyle(juce::Slider::LinearHorizontal);
    voicesKnob.setTextBoxStyle(juce::Slider::TextBoxRight, false, 28, 16);
    voicesKnob.setColour(juce::Slider::textBoxTextColourId, juce::Colour(0xFFCCDDEE));
    voicesKnob.setTooltip("Max polyphony (voices)");
    addAndMakeVisible(voicesKnob);
    sa(voicesKnob, "maxVoices");

    rrResetBtn.setColour(juce::TextButton::buttonColourId, juce::Colour(0xFF0F3460));
    rrResetBtn.setColour(juce::TextButton::textColourOffId, juce::Colour(0xFF8899AA));
    rrResetBtn.setTooltip("Reset round-robin counters");
    rrResetBtn.onClick = [this]() { proc.getSynth().resetRoundRobin(); };
    addAndMakeVisible(rrResetBtn);

    // ── Piano keyboard ────────────────────────────────────────────────────────
    keyboard.setColour(juce::MidiKeyboardComponent::whiteNoteColourId, juce::Colour(0xFFD8E8F0));
    keyboard.setColour(juce::MidiKeyboardComponent::blackNoteColourId, juce::Colour(0xFF0D1828));
    keyboard.setColour(juce::MidiKeyboardComponent::mouseOverKeyOverlayColourId, juce::Colour(0x3300D4FF));
    keyboard.setColour(juce::MidiKeyboardComponent::keyDownOverlayColourId, juce::Colour(0x7700D4FF));
    keyboard.setColour(juce::MidiKeyboardComponent::keySeparatorLineColourId, juce::Colour(0xFF334466));
    keyboard.setAvailableRange(21, 108);
    keyboard.setLowestVisibleKey(36);
    keyboard.setScrollButtonsVisible(false);
    addAndMakeVisible(keyboard);

    setWantsKeyboardFocus(true);

    // Apply current theme colours to all per-component overrides
    lookAndFeelChanged();
}

// ── Paint ─────────────────────────────────────────────────────────────────────
void DSKSFzEditor::secBg(juce::Graphics& g, juce::Rectangle<int> r, const juce::String& title)
{
    const auto& t = lf.getTheme();
    g.setColour(t.sectionBg);
    g.fillRoundedRectangle(r.toFloat(), 3.0f);
    g.setColour(t.sectionBdr);
    g.drawRoundedRectangle(r.toFloat(), 3.0f, 1.0f);
    if (title.isNotEmpty())
    {
        g.setColour(t.accent);
        g.setFont(juce::Font(9.0f, juce::Font::bold));
        g.drawText(title, r.getX() + 5, r.getY() + 2, 80, 14, juce::Justification::centredLeft);
    }
}

void DSKSFzEditor::drawMeter(juce::Graphics& g, int x, int y, int w, int h,
    float lvl, const char* ch)
{
    const auto& t = lf.getTheme();
    g.setColour(t.headerBg);
    g.fillRect(x, y, w, h);

    float l = juce::jlimit(0.0f, 1.0f, lvl);
    int   gH = (int)(juce::jmin(l, 0.75f) / 0.75f * h * 0.75f);
    int   yH = (int)(juce::jmax(0.0f, l - 0.75f) / 0.15f * h * 0.15f);
    int   rH = (int)(juce::jmax(0.0f, l - 0.90f) / 0.10f * h * 0.10f);

    g.setColour(juce::Colour(0xFF00BB44));
    g.fillRect(x, y + h - gH, w, gH);
    g.setColour(juce::Colour(0xFFCCBB00));
    g.fillRect(x, y + (int)(h * 0.25f) - yH, w, yH);
    g.setColour(juce::Colour(0xFFCC2200));
    g.fillRect(x, y + (int)(h * 0.10f) - rH, w, rH);

    g.setColour(t.border);
    g.drawRect(x, y, w, h, 1);
    g.setColour(t.subtext);
    g.setFont(juce::Font(8.5f));
    g.drawText(ch, x, y - 12, w, 12, juce::Justification::centred);
}

void DSKSFzEditor::paint(juce::Graphics& g)
{
    const auto& t = lf.getTheme();

    g.fillAll(t.editorBg);

    const int W = getWidth();
    const int H = getHeight();
    const int rx = libWidth;
    const int rw = 780; // Fijo visualmente a 780px para no desparramar controles

    // Header bar (Extendemos el fondo hasta W para ocultar el hueco)
    g.setColour(t.headerBg);
    g.fillRect(rx, 0, W - rx, HDR_H);
    g.setColour(t.divider);
    g.drawLine((float)rx, (float)HDR_H, (float)W, (float)HDR_H, 1.5f);

    

    const int S0 = HDR_H;
    const int S1 = S0 + ROW_H;
    const int S2 = S1 + ROW_H;
    const int SF = S2 + ROW_H;

    auto secR = [&](int sy) { return juce::Rectangle<int>(rx + 3, sy + 2, rw - 6, ROW_H - 4); };
    secBg(g, secR(S0), "AMP");

    {
        auto r = secR(S1);
        secBg(g, r, "");
        const int divX = rx + rw * 54 / 100;
        g.setColour(t.sectionBdr);
        g.drawLine((float)divX, (float)(r.getY() + 4), (float)divX, (float)(r.getBottom() - 4), 1.0f);
        g.setColour(t.accent);
        g.setFont(juce::Font(10.5f, juce::Font::bold));
        g.drawText("FILTER", r.getX() + 5, r.getY() + 2, 80, 14, juce::Justification::centredLeft);
        g.drawText("MOD", divX + 6, r.getY() + 2, 80, 14, juce::Justification::centredLeft);
    }

    secBg(g, secR(S2), "FX");

    // Footer strip (Extendemos el fondo hasta W)
    g.setColour(t.footerBg);
    g.fillRect(rx, SF, W - rx, FTR_H);
    g.setColour(t.text.withAlpha(0.75f));
    g.setFont(juce::Font(11.0f, juce::Font::bold));
    g.drawText("Voices:", rx + 10, SF + 7, 56, 16, juce::Justification::centredLeft);

    // Library divider (drawn slightly wider so user can grab it)
    g.setColour(t.accent.withAlpha(0.25f));
    g.fillRect(libWidth, 0, 2, H - KB_H - VDIV_H);
    g.setColour(t.divider);
    g.drawLine((float)libWidth, 0.0f, (float)libWidth, (float)(H - KB_H - VDIV_H), 1.0f);

    // Horizontal divider (Separador de teclado / redimensionador vertical)
    const int divY = H - KB_H - VDIV_H;
    g.setColour(t.accent.withAlpha(0.12f));
    g.fillRect(0, divY, W, VDIV_H);
    g.setColour(t.divider);
    g.drawLine(0.0f, (float)divY, (float)W, (float)divY, 1.0f);
    g.drawLine(0.0f, (float)(divY + VDIV_H), (float)W, (float)(divY + VDIV_H), 1.0f);

    // Meters
    const int mW = 10, mGap = 4;
    const int mX = rx + rw - 2 * (mW + mGap) - 4;
    const int mY = S0 + KNOB_Y;
    const int mH = KSZ + 16;
    drawMeter(g, mX, mY, mW, mH, meterL.load(), "L");
    drawMeter(g, mX + mW + mGap, mY, mW, mH, meterR.load(), "R");
}

// ── overlay visual sobre TODOS los componentes ────────────────────────────────
void DSKSFzEditor::paintOverChildren(juce::Graphics& g)
{
    if (isDragOver)
    {
        const auto& t = lf.getTheme();

        // Oscurecemos toda la interfaz, incluyendo la librería y el teclado
        g.setColour(t.accent.withAlpha(0.20f));
        g.fillAll();

        // Borde exterior
        g.setColour(t.accent);
        g.drawRect(getLocalBounds(), 3);

        // Fondo semi-transparente para asegurar que el texto se lea perfecto
        auto textBounds = getLocalBounds().withSizeKeepingCentre(360, 40);
        g.setColour(juce::Colour(0xD9000000));
        g.fillRoundedRectangle(textBounds.toFloat(), 6.0f);

        g.setColour(t.accent);
        g.setFont(juce::Font(18.0f, juce::Font::bold));
        g.drawText("Drop SFZ file, folder, or ZIP pack",
            getLocalBounds(), juce::Justification::centred);
    }
}

// ── resized ───────────────────────────────────────────────────────────────────
void DSKSFzEditor::resized()
{
    // Si la ventana fue redimensionada por el OS (ej. al maximizar en Standalone)
    // calculamos libWidth para que la parte derecha NUNCA pase de 780px
    if (getWidth() != libWidth + 780 || getHeight() != pluginHeight)
    {
        libWidth = juce::jmax(120, getWidth() - 780);
        pluginHeight = juce::jmax(DEF_H, getHeight());
    }

    const int W = getWidth();
    const int H = getHeight();
    const int rx = libWidth;
    const int rw = 780; // Fijo a 780px para que los knobs se queden agrupados

    // Los controles de la derecha conservan su posición Y anclada respecto a la parte superior
    const int S0 = HDR_H;
    const int S1 = S0 + ROW_H;
    const int S2 = S1 + ROW_H;
    const int SF = S2 + ROW_H;

    // La librería ocupa todo el alto hasta el tirador horizontal
    libraryPanel.setBounds(0, 0, libWidth, H - KB_H - VDIV_H);
    keyboard.setBounds(0, H - KB_H, W, KB_H);
    keyboard.setKeyWidth(juce::jmax(10.0f, (float)W / 52.0f));

    const int hdrLabelX = rx + 8;
    titleLink.setBounds(hdrLabelX, 4, 160, 24);
    siteLink.setBounds(hdrLabelX, 28, 120, 16);
    const int lcdX = hdrLabelX + 170;
    const int lcdW = 380;
    instrNameLabel.setBounds(lcdX, 6, lcdW - 24, 24);
    statusLabel.setBounds(lcdX, 30, lcdW - 24, 16);
    addFavBtn.setBounds(lcdX + lcdW - 20, 16, 18, 18); // Botón bajado a Y=15 para centrado vertical perfecto
    openBtn.setBounds(rx + 780 - 210, 10, 116, 28);
    optionsBtn.setBounds(rx + rw - 90, 10, 82, 28);

    auto K = [&](juce::Slider& s, int col, int secY)
    { s.setBounds(colX(rx, col), secY + KNOB_Y, KSZ, KSZ); };

    K(masterVolKnob, 0, S0); K(masterPanKnob, 1, S0);
    K(pitchTransKnob, 2, S0); K(pitchFineKnob, 3, S0);
    K(ampAKnob, 5, S0); K(ampDKnob, 6, S0);
    K(ampSKnob, 7, S0); K(ampRKnob, 8, S0);

    const int mW = 10, mGap = 4;
    const int mX = rx + rw - 2 * (mW + mGap) - 4;
    const int adsrX = colX(rx, 9) + 4;
    adsrDisplay.setBounds(adsrX, S0 + KNOB_Y, mX - adsrX - 4, KSZ);

    {
        const int filterZoneW = rw * 54 / 100;
        const int divX = rx + filterZoneW;
        const int KSM = 36, SLOT = 42, comboW = 74;
        const int comboY = S1 + KNOB_Y + (KSM - 20) / 2;
        int fx0 = rx + 8;
        filtTypeBox.setBounds(fx0, comboY, comboW, 20);
        int kx = fx0 + comboW + 6;
        auto KF = [&](juce::Slider& s) { s.setBounds(kx, S1 + KNOB_Y, KSM, KSM); kx += SLOT; };
        KF(filtCutKnob); KF(filtResKnob); KF(filtEnvKnob);
        kx += 4;
        KF(filtAKnob); KF(filtDKnob); KF(filtSKnob); KF(filtRKnob);

        const int modZoneW = rw - filterZoneW;
        const int lfoComboW = juce::jmin(72, (modZoneW / 2 - 8 - 2 * KSLOT));
        const int lfoComboH = 18;
        int lfo1X = divX + 8;
        lfo1ShapeBox.setBounds(lfo1X, S1 + KNOB_Y, lfoComboW, lfoComboH);
        lfo1TgtBox.setBounds(lfo1X, S1 + KNOB_Y + 22, lfoComboW, lfoComboH);
        lfo1RateKnob.setBounds(lfo1X + lfoComboW + 4, S1 + KNOB_Y, KSZ, KSZ);
        lfo1AmtKnob.setBounds(lfo1X + lfoComboW + 4 + KSLOT, S1 + KNOB_Y, KSZ, KSZ);

        int lfo2X = divX + modZoneW / 2 + 4;
        lfo2ShapeBox.setBounds(lfo2X, S1 + KNOB_Y, lfoComboW, lfoComboH);
        lfo2TgtBox.setBounds(lfo2X, S1 + KNOB_Y + 22, lfoComboW, lfoComboH);
        lfo2RateKnob.setBounds(lfo2X + lfoComboW + 4, S1 + KNOB_Y, KSZ, KSZ);
        lfo2AmtKnob.setBounds(lfo2X + lfoComboW + 4 + KSLOT, S1 + KNOB_Y, KSZ, KSZ);
    }

    const int fxSlot = (rw - 6) / 5;
    struct FXSlot { juce::TextButton* btn; std::vector<juce::Slider*> knobs; };
    FXSlot fxSlots[] = {
        { &driveOnBtn,  { &driveAmtKnob, &driveMixKnob } },
        { &chorusOnBtn, { &choRateKnob, &choDepthKnob, &choMixKnob } },
        { &delayOnBtn,  { &delTimeKnob, &delFbkKnob, &delMixKnob } },
        { &reverbOnBtn, { &revSizeKnob, &revDampKnob, &revMixKnob } },
        { &eqOnBtn,     { &eqLowKnob, &eqMidKnob, &eqHighKnob } },
    };
    for (int i = 0; i < 5; ++i)
    {
        const int x0 = rx + 3 + i * fxSlot;
        fxSlots[i].btn->setBounds(x0 + 2, S2 + 4, fxSlot - 4, 14);
        const int nk = (int)fxSlots[i].knobs.size();
        const int avail = fxSlot - 4;
        const int kw = juce::jmin(KSZ, (avail - (nk - 1) * 2) / nk);
        const int kx0 = x0 + 2 + (avail - nk * kw - (nk - 1) * 2) / 2;
        for (int j = 0; j < nk; ++j)
            fxSlots[i].knobs[j]->setBounds(kx0 + j * (kw + 2), S2 + KNOB_Y + 2, kw, kw);
    }

    voicesKnob.setBounds(rx + 66, SF + 5, 130, 20);
    rrResetBtn.setBounds(rx + rw - 80, SF + 4, 74, 22);

    for (auto& kl : kLabels)
        kl.lbl->setBounds(kl.s->getX(), kl.s->getBottom() + 2, kl.s->getWidth(), 14);
}

// ── timerCallback ─────────────────────────────────────────────────────────────
void DSKSFzEditor::timerCallback()
{
    auto sfz = proc.getCurrentSFZFile();
    instrNameLabel.setText(
        sfz.existsAsFile() ? sfz.getFileNameWithoutExtension() : "No instrument loaded",
        juce::dontSendNotification);
    statusLabel.setText(
        proc.isLoadingInstrument.load() ? "Loading..." : "Ready",
        juce::dontSendNotification);

    auto gf = [this](const char* id) -> float
    {
        auto* p = proc.params.getRawParameterValue(id);
        return p ? p->load() : 0.0f;
    };
    adsrDisplay.attack = gf("ampAttack");
    adsrDisplay.decay = gf("ampDecay");
    adsrDisplay.sustain = gf("ampSustain");
    adsrDisplay.release = gf("ampRelease");
    adsrDisplay.repaint();

    auto& syn = proc.getSynth();
    float pl = syn.meterLevelL.exchange(0.0f);
    float pr = syn.meterLevelR.exchange(0.0f);
    meterL.store(juce::jmax(meterL.load() * 0.88f, pl));
    meterR.store(juce::jmax(meterR.load() * 0.88f, pr));

    if (previewCountdown > 0)
        if (--previewCountdown == 0)
            proc.keyboardState.noteOff(1, 60, 0.0f);

    repaint();
}

// ── PC keyboard → MIDI note forwarding ───────────────────────────────────────
bool DSKSFzEditor::keyPressed(const juce::KeyPress& key)
{
    return keyboard.keyPressed(key);
}

bool DSKSFzEditor::keyStateChanged(bool isKeyDown)
{
    return keyboard.keyStateChanged(isKeyDown);
}

// ── Library callbacks ─────────────────────────────────────────────────────────
void DSKSFzEditor::libraryFileRequested(const juce::File& sfzFile)
{
    statusLabel.setText("Loading...", juce::dontSendNotification);
    const int currentGen = ++loadGeneration;
    juce::Thread::launch([this, sfzFile, currentGen]()
        {
            if (currentGen != loadGeneration.load()) return; // Abortar carga si hay un clic más reciente
    proc.loadSFZ(sfzFile);
    juce::MessageManager::callAsync([this, currentGen]()
        {
            if (currentGen == loadGeneration.load())
            statusLabel.setText("Ready", juce::dontSendNotification);
        });
        });
}

void DSKSFzEditor::libraryZipRequested(const juce::File& zipFile)
{
    libraryPanel.handleFileActivated(zipFile, true);
}

// ── Drag and Drop ─────────────────────────────────────────────────────────────
bool DSKSFzEditor::isInterestedInFileDrag(const juce::StringArray& files)
{
    for (auto& f : files)
    {
        juce::File file(f);
        if (file.hasFileExtension(".sfz") || file.hasFileExtension(".zip") || file.isDirectory())
            return true;
    }
    return false;
}

void DSKSFzEditor::fileDragEnter(const juce::StringArray&, int, int)
{
    isDragOver = true;
    repaint();
}

void DSKSFzEditor::fileDragExit(const juce::StringArray&)
{
    isDragOver = false;
    repaint();
}

void DSKSFzEditor::filesDropped(const juce::StringArray& files, int, int)
{
    isDragOver = false;
    repaint();

    bool sfzLoaded = false;
    bool zipLoaded = false;

    for (auto& f : files)
    {
        juce::File file(f);
        if (file.isDirectory())
        {
            libraryPanel.addFolderDirect(file);
        }
        else if (!sfzLoaded && file.hasFileExtension(".sfz"))
        {
            libraryFileRequested(file);
            sfzLoaded = true;
        }
        else if (!zipLoaded && file.hasFileExtension(".zip"))
        {
            libraryZipRequested(file);
            zipLoaded = true;
        }
    }
}

// ── Divider drag (adjustable library width and window height) ─────────────────
void DSKSFzEditor::mouseMove(const juce::MouseEvent& e)
{
    bool nearV = isNearDivider(e.x) && e.y < getHeight() - KB_H - VDIV_H;
    bool nearH = isNearHDivider(e.y);

    if (nearV)       setMouseCursor(juce::MouseCursor::LeftRightResizeCursor);
    else if (nearH)  setMouseCursor(juce::MouseCursor::UpDownResizeCursor);
    else             setMouseCursor(juce::MouseCursor::NormalCursor);
}

void DSKSFzEditor::mouseDown(const juce::MouseEvent& e)
{
    bool nearV = isNearDivider(e.x) && e.y < getHeight() - KB_H - VDIV_H;
    bool nearH = isNearHDivider(e.y);

    if (!nearV && !nearH) return;

    if (e.getNumberOfClicks() >= 2)
    {
        if (nearV) libWidth = LIB_W;
        if (nearH) pluginHeight = DEF_H;

        setSize(libWidth + 780, pluginHeight);
        applyWindowConstraints();
        return;
    }

    if (nearV)
    {
        isDraggingDivider = true;
        libWidthAtDragStart = libWidth;
        dragStartX = e.x;
    }

    if (nearH)
    {
        isDraggingHDivider = true;
        pluginHeightAtDragStart = pluginHeight;
        dragStartY = e.y;
    }

    if (auto* c = getConstrainer())
    {
        if (proc.wrapperType == juce::AudioProcessor::wrapperType_Standalone)
        {
            // Permite a la ventana crecer libremente para que nuestro paint() tape el fondo
            c->setSizeLimits(120 + 780, DEF_H, 4000, 4000);
        }
        else
        {
            c->setSizeLimits(120 + 780, DEF_H, 450 + 780, 1200);
        }
    }
}

void DSKSFzEditor::mouseDrag(const juce::MouseEvent& e)
{
    if (!isDraggingDivider && !isDraggingHDivider) return;

    if (isDraggingDivider)
        libWidth = juce::jlimit(120, 450, libWidthAtDragStart + (e.x - dragStartX));

    if (isDraggingHDivider)
        pluginHeight = juce::jlimit(DEF_H, 4000, pluginHeightAtDragStart + (e.y - dragStartY));

    // Si es Standalone, actualizamos el limitador al vuelo con tu nuevo ancho manual
    // justo antes de aplicar el cambio de tamaño, evitando que se quede trabado
    if (auto* c = getConstrainer())
    {
        if (proc.wrapperType == juce::AudioProcessor::wrapperType_Standalone)
        {
            int nextW = libWidth + 780;
            c->setSizeLimits(nextW, DEF_H, nextW, 4000);
        }
    }

    setSize(libWidth + 780, pluginHeight);
}

void DSKSFzEditor::mouseUp(const juce::MouseEvent&)
{
    if (!isDraggingDivider && !isDraggingHDivider) return;
    isDraggingDivider = false;
    isDraggingHDivider = false;
    applyWindowConstraints();
}

// ── Window constraints ────────────────────────────────────────────────────────
void DSKSFzEditor::applyWindowConstraints()
{
    if (auto* c = getConstrainer())
    {
        if (proc.wrapperType == juce::AudioProcessor::wrapperType_Standalone)
            c->setSizeLimits(120 + 780, DEF_H, 4000, 4000);
        else
            c->setSizeLimits(getWidth(), getHeight(), getWidth(), getHeight());
    }
}

// ── SFZ preview (click [S] badge) ────────────────────────────────────────────
void DSKSFzEditor::triggerPreviewNote()
{
    proc.keyboardState.noteOn(1, 60, 0.8f);
    previewCountdown = 10; // ~830 ms at 12 Hz
}

void DSKSFzEditor::loadSFZForPreview(const juce::File& f)
{
    statusLabel.setText("Loading...", juce::dontSendNotification);
    const int currentGen = ++loadGeneration;
    juce::Thread::launch([this, f, currentGen]()
        {
            if (currentGen != loadGeneration.load()) return; // Abortar carga si hay un clic más reciente
    proc.loadSFZ(f);
    juce::MessageManager::callAsync([this, currentGen]()
        {
            if (currentGen == loadGeneration.load())
            {
                statusLabel.setText("Ready", juce::dontSendNotification);
                triggerPreviewNote();
            }
        });
        });
}
