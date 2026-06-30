#pragma once
#include <JuceHeader.h>

// ══════════════════════════════════════════════════════════════════════════════
// DSKTheme — all UI colours in one place
// ══════════════════════════════════════════════════════════════════════════════
struct DSKTheme
{
    juce::String name;
    bool         isDark { true };

    // ── editor background / structure ──────────────────────────────────────────
    juce::Colour editorBg   { 0xFF141928 };
    juce::Colour headerBg   { 0xFF0D1828 };
    juce::Colour sectionBg  { 0xFF0F3460 };
    juce::Colour sectionBdr { 0xFF253A5E };
    juce::Colour divider    { 0xFF1A2A44 };
    juce::Colour footerBg   { 0xFF0D1828 };

    // ── accent / text ─────────────────────────────────────────────────────────
    juce::Colour accent       { 0xFF00D4FF };
    juce::Colour accent2      { 0xFFE94560 };
    juce::Colour text         { 0xFFCCDDEE };
    juce::Colour subtext      { 0xFF7799BB };
    juce::Colour textOnAccent { 0xFF0A1020 }; // text when drawn ON the accent bg

    // ── library panel ─────────────────────────────────────────────────────────
    juce::Colour libBg     { 0xFF16213E };
    juce::Colour libPanel  { 0xFF0F3460 };
    juce::Colour libTreeBg { 0xFF0D1B35 };
    juce::Colour libText   { 0xFFCCDDEE };
    juce::Colour libDim    { 0xFF667788 };
    juce::Colour libSel    { 0x330099FF };

    // ── favourites ────────────────────────────────────────────────────────────
    juce::Colour favGold  { 0xFFFFD700 };
    juce::Colour favAmber { 0xFFFFAA33 };
    juce::Colour favBg    { 0x18FFD700 };
    juce::Colour favSel   { 0x44FFD700 };

    // ── LookAndFeel draw colours (knobs, buttons …) ───────────────────────────
    juce::Colour bg     { 0xFF1A1A2E };
    juce::Colour panel  { 0xFF16213E };
    juce::Colour section{ 0xFF0F3460 };
    juce::Colour knobBg { 0xFF222240 };
    juce::Colour knobFg { 0xFF00D4FF };
    juce::Colour button { 0xFF0F3460 };
    juce::Colour border { 0xFF334466 };
    juce::Colour lcdText{ 0xFF00E6CC };
    juce::Colour lcdBg  { 0xFF0D1828 };
};

// ══════════════════════════════════════════════════════════════════════════════
// DSKLookAndFeel
// ══════════════════════════════════════════════════════════════════════════════
class DSKLookAndFeel : public juce::LookAndFeel_V4
{
public:
    // ── legacy constants (kept for compatibility) ─────────────────────────────
    static constexpr uint32_t COL_BG      = 0xFF1A1A2E;
    static constexpr uint32_t COL_PANEL   = 0xFF16213E;
    static constexpr uint32_t COL_SECTION = 0xFF0F3460;
    static constexpr uint32_t COL_ACCENT  = 0xFF00D4FF;
    static constexpr uint32_t COL_ACCENT2 = 0xFFE94560;
    static constexpr uint32_t COL_TEXT    = 0xFFE0E0FF;
    static constexpr uint32_t COL_SUBTEXT = 0xFF8888AA;
    static constexpr uint32_t COL_KNOB_BG = 0xFF222240;
    static constexpr uint32_t COL_KNOB_FG = 0xFF00D4FF;
    static constexpr uint32_t COL_BUTTON  = 0xFF0F3460;
    static constexpr uint32_t COL_BORDER  = 0xFF334466;

    // ── theme presets ─────────────────────────────────────────────────────────
    static constexpr int NUM_THEMES = 6;

    static DSKTheme themePreset(int idx)
    {
        DSKTheme t;
        switch (idx)
        {
        default:
        case 0: // Deep Navy (default)
            t.name = "Deep Navy";   t.isDark = true;
            t.editorBg   = juce::Colour(0xFF141928); t.headerBg  = juce::Colour(0xFF0D1828);
            t.sectionBg  = juce::Colour(0xFF0F3460); t.sectionBdr= juce::Colour(0xFF253A5E);
            t.divider    = juce::Colour(0xFF1A2A44); t.footerBg  = juce::Colour(0xFF0D1828);
            t.accent     = juce::Colour(0xFF00D4FF); t.accent2   = juce::Colour(0xFFE94560);
            t.text       = juce::Colour(0xFFCCDDEE); t.subtext   = juce::Colour(0xFF7799BB);
            t.textOnAccent = juce::Colour(0xFF0A1020);
            t.libBg      = juce::Colour(0xFF16213E); t.libPanel  = juce::Colour(0xFF0F3460);
            t.libTreeBg  = juce::Colour(0xFF0D1B35); t.libText   = juce::Colour(0xFFCCDDEE);
            t.libDim     = juce::Colour(0xFF667788); t.libSel    = juce::Colour(0x330099FF);
            t.favGold    = juce::Colour(0xFFFFD700); t.favAmber  = juce::Colour(0xFFFFAA33);
            t.favBg      = juce::Colour(0x18FFD700); t.favSel    = juce::Colour(0x44FFD700);
            t.bg         = juce::Colour(0xFF1A1A2E); t.panel     = juce::Colour(0xFF16213E);
            t.section    = juce::Colour(0xFF0F3460); t.knobBg    = juce::Colour(0xFF222240);
            t.knobFg     = juce::Colour(0xFF00D4FF); t.button    = juce::Colour(0xFF0F3460);
            t.border     = juce::Colour(0xFF334466); t.lcdText   = juce::Colour(0xFF00E6CC);
            t.lcdBg      = juce::Colour(0xFF0D1828);
            break;

        case 1: // Midnight (dark, orange)
            t.name = "Midnight";    t.isDark = true;
            t.editorBg   = juce::Colour(0xFF090810); t.headerBg  = juce::Colour(0xFF060508);
            t.sectionBg  = juce::Colour(0xFF1C1208); t.sectionBdr= juce::Colour(0xFF342210);
            t.divider    = juce::Colour(0xFF1A1410); t.footerBg  = juce::Colour(0xFF060508);
            t.accent     = juce::Colour(0xFFFF8833); t.accent2   = juce::Colour(0xFFFF3377);
            t.text       = juce::Colour(0xFFEEE8D5); t.subtext   = juce::Colour(0xFF998866);
            t.textOnAccent = juce::Colour(0xFF0A0608);
            t.libBg      = juce::Colour(0xFF0E0A06); t.libPanel  = juce::Colour(0xFF1C1208);
            t.libTreeBg  = juce::Colour(0xFF060404); t.libText   = juce::Colour(0xFFEEE8D5);
            t.libDim     = juce::Colour(0xFF776655); t.libSel    = juce::Colour(0x33FF8833);
            t.favGold    = juce::Colour(0xFFFFAA44); t.favAmber  = juce::Colour(0xFFFF8822);
            t.favBg      = juce::Colour(0x18FFAA44); t.favSel    = juce::Colour(0x44FFAA44);
            t.bg         = juce::Colour(0xFF0A0810); t.panel     = juce::Colour(0xFF0E0A06);
            t.section    = juce::Colour(0xFF1C1208); t.knobBg    = juce::Colour(0xFF1A1208);
            t.knobFg     = juce::Colour(0xFFFF8833); t.button    = juce::Colour(0xFF1C1208);
            t.border     = juce::Colour(0xFF3A2810); t.lcdText   = juce::Colour(0xFFFF9944);
            t.lcdBg      = juce::Colour(0xFF060508);
            break;

        case 2: // Forest (dark, green)
            t.name = "Forest";      t.isDark = true;
            t.editorBg   = juce::Colour(0xFF091208); t.headerBg  = juce::Colour(0xFF060C06);
            t.sectionBg  = juce::Colour(0xFF082010); t.sectionBdr= juce::Colour(0xFF144020);
            t.divider    = juce::Colour(0xFF0E1C0E); t.footerBg  = juce::Colour(0xFF060C06);
            t.accent     = juce::Colour(0xFF00DD88); t.accent2   = juce::Colour(0xFF88CC00);
            t.text       = juce::Colour(0xFFCCE8CC); t.subtext   = juce::Colour(0xFF556655);
            t.textOnAccent = juce::Colour(0xFF041008);
            t.libBg      = juce::Colour(0xFF0C1A10); t.libPanel  = juce::Colour(0xFF082010);
            t.libTreeBg  = juce::Colour(0xFF040E06); t.libText   = juce::Colour(0xFFCCE8CC);
            t.libDim     = juce::Colour(0xFF446644); t.libSel    = juce::Colour(0x3300DD88);
            t.favGold    = juce::Colour(0xFFAADD44); t.favAmber  = juce::Colour(0xFF88BB22);
            t.favBg      = juce::Colour(0x18AADD44); t.favSel    = juce::Colour(0x44AADD44);
            t.bg         = juce::Colour(0xFF0A1410); t.panel     = juce::Colour(0xFF0C1A10);
            t.section    = juce::Colour(0xFF082010); t.knobBg    = juce::Colour(0xFF0A180A);
            t.knobFg     = juce::Colour(0xFF00DD88); t.button    = juce::Colour(0xFF082010);
            t.border     = juce::Colour(0xFF154025); t.lcdText   = juce::Colour(0xFF00FF88);
            t.lcdBg      = juce::Colour(0xFF060C06);
            break;

        case 3: // Deep Purple
            t.name = "Deep Purple"; t.isDark = true;
            t.editorBg   = juce::Colour(0xFF120820); t.headerBg  = juce::Colour(0xFF0A0518);
            t.sectionBg  = juce::Colour(0xFF28104A); t.sectionBdr= juce::Colour(0xFF442068);
            t.divider    = juce::Colour(0xFF1E0E38); t.footerBg  = juce::Colour(0xFF0A0518);
            t.accent     = juce::Colour(0xFFCC66FF); t.accent2   = juce::Colour(0xFFFF55CC);
            t.text       = juce::Colour(0xFFEED8FF); t.subtext   = juce::Colour(0xFF997799);
            t.textOnAccent = juce::Colour(0xFF100420);
            t.libBg      = juce::Colour(0xFF1E0E34); t.libPanel  = juce::Colour(0xFF28104A);
            t.libTreeBg  = juce::Colour(0xFF0E0620); t.libText   = juce::Colour(0xFFEED8FF);
            t.libDim     = juce::Colour(0xFF775588); t.libSel    = juce::Colour(0x33CC66FF);
            t.favGold    = juce::Colour(0xFFCC88FF); t.favAmber  = juce::Colour(0xFFBB66EE);
            t.favBg      = juce::Colour(0x18CC88FF); t.favSel    = juce::Colour(0x44CC88FF);
            t.bg         = juce::Colour(0xFF150A28); t.panel     = juce::Colour(0xFF1E0E34);
            t.section    = juce::Colour(0xFF28104A); t.knobBg    = juce::Colour(0xFF1A0A30);
            t.knobFg     = juce::Colour(0xFFCC66FF); t.button    = juce::Colour(0xFF28104A);
            t.border     = juce::Colour(0xFF442068); t.lcdText   = juce::Colour(0xFFDD88FF);
            t.lcdBg      = juce::Colour(0xFF0A0518);
            break;

        case 4: // Light Gray
            t.name = "Light Gray";  t.isDark = false;
            t.editorBg   = juce::Colour(0xFFE4EBF8); t.headerBg  = juce::Colour(0xFFC8D4E8);
            t.sectionBg  = juce::Colour(0xFFB8CCDC); t.sectionBdr= juce::Colour(0xFF9AAABB);
            t.divider    = juce::Colour(0xFFAABCCC); t.footerBg  = juce::Colour(0xFFC8D4E8);
            t.accent     = juce::Colour(0xFF0055CC); t.accent2   = juce::Colour(0xFFCC2233);
            t.text       = juce::Colour(0xFF111830); t.subtext   = juce::Colour(0xFF446688);
            t.textOnAccent = juce::Colour(0xFFFFFFFF);
            t.libBg      = juce::Colour(0xFFD4DCF0); t.libPanel  = juce::Colour(0xFFB8CCDC);
            t.libTreeBg  = juce::Colour(0xFFEEF4FF); t.libText   = juce::Colour(0xFF111830);
            t.libDim     = juce::Colour(0xFF557799); t.libSel    = juce::Colour(0x330055CC);
            t.favGold    = juce::Colour(0xFF886600); t.favAmber  = juce::Colour(0xFFAA7700);
            t.favBg      = juce::Colour(0x18886600); t.favSel    = juce::Colour(0x44886600);
            t.bg         = juce::Colour(0xFFDDE4F0); t.panel     = juce::Colour(0xFFD4DCF0);
            t.section    = juce::Colour(0xFFB8CCDC); t.knobBg    = juce::Colour(0xFFC0CCE0);
            t.knobFg     = juce::Colour(0xFF0055CC); t.button    = juce::Colour(0xFFB8CCDC);
            t.border     = juce::Colour(0xFF9AAABB); t.lcdText   = juce::Colour(0xFF0044AA);
            t.lcdBg      = juce::Colour(0xFFC8D4E8);
            break;

        case 5: // Warm Cream
            t.name = "Warm Cream";  t.isDark = false;
            t.editorBg   = juce::Colour(0xFFF5F0E4); t.headerBg  = juce::Colour(0xFFE8E0CC);
            t.sectionBg  = juce::Colour(0xFFD4C8A8); t.sectionBdr= juce::Colour(0xFFC0AA88);
            t.divider    = juce::Colour(0xFFCCBBAA); t.footerBg  = juce::Colour(0xFFE8E0CC);
            t.accent     = juce::Colour(0xFF884400); t.accent2   = juce::Colour(0xFFCC2200);
            t.text       = juce::Colour(0xFF221400); t.subtext   = juce::Colour(0xFF776644);
            t.textOnAccent = juce::Colour(0xFFFFFFFF);
            t.libBg      = juce::Colour(0xFFEDE8D4); t.libPanel  = juce::Colour(0xFFD4C8A8);
            t.libTreeBg  = juce::Colour(0xFFF8F4E8); t.libText   = juce::Colour(0xFF221400);
            t.libDim     = juce::Colour(0xFF887766); t.libSel    = juce::Colour(0x33884400);
            t.favGold    = juce::Colour(0xFF884400); t.favAmber  = juce::Colour(0xFFAA6600);
            t.favBg      = juce::Colour(0x18884400); t.favSel    = juce::Colour(0x44884400);
            t.bg         = juce::Colour(0xFFF0EAD8); t.panel     = juce::Colour(0xFFEDE8D4);
            t.section    = juce::Colour(0xFFD4C8A8); t.knobBg    = juce::Colour(0xFFE0D4B4);
            t.knobFg     = juce::Colour(0xFF884400); t.button    = juce::Colour(0xFFD4C8A8);
            t.border     = juce::Colour(0xFFC0AA88); t.lcdText   = juce::Colour(0xFF662200);
            t.lcdBg      = juce::Colour(0xFFE8E0CC);
            break;
        }
        return t;
    }

    // ── constructor ───────────────────────────────────────────────────────────
    DSKLookAndFeel() { applyTheme(themePreset(0)); }

    const DSKTheme& getTheme() const { return currentTheme; }
    int  getThemeIndex()       const { return themeIndex;   }

    void setTheme(int idx)
    {
        themeIndex = juce::jlimit(0, NUM_THEMES - 1, idx);
        applyTheme(themePreset(themeIndex));
    }

    void applyTheme(const DSKTheme& t)
    {
        currentTheme = t;
        setColour(juce::ResizableWindow::backgroundColourId,      t.bg);
        setColour(juce::DocumentWindow::backgroundColourId,       t.bg);
        setColour(juce::Label::textColourId,                      t.text);
        setColour(juce::Label::backgroundColourId,                juce::Colours::transparentBlack);
        setColour(juce::TextButton::buttonColourId,               t.button);
        setColour(juce::TextButton::buttonOnColourId,             t.accent);
        setColour(juce::TextButton::textColourOffId,              t.text);
        setColour(juce::TextButton::textColourOnId,               t.textOnAccent);
        setColour(juce::ComboBox::backgroundColourId,             t.button);
        setColour(juce::ComboBox::textColourId,                   t.text);
        setColour(juce::ComboBox::arrowColourId,                  t.accent);
        setColour(juce::ComboBox::outlineColourId,                t.border);
        setColour(juce::PopupMenu::backgroundColourId,            t.panel);
        setColour(juce::PopupMenu::textColourId,                  t.text);
        setColour(juce::PopupMenu::highlightedBackgroundColourId, t.section);
        setColour(juce::PopupMenu::highlightedTextColourId,       t.accent);
        setColour(juce::TreeView::backgroundColourId,             t.libTreeBg);
        setColour(juce::TreeView::linesColourId,                  t.border);
        setColour(juce::ScrollBar::thumbColourId,                 t.border);
        setColour(juce::TextEditor::backgroundColourId,           t.button);
        setColour(juce::TextEditor::textColourId,                 t.text);
        setColour(juce::TextEditor::outlineColourId,              t.border);
        setColour(juce::TextEditor::focusedOutlineColourId,       t.accent);
        setColour(juce::Slider::thumbColourId,                    t.accent);
        setColour(juce::Slider::trackColourId,                    t.knobBg);
        setColour(juce::Slider::backgroundColourId,               t.knobBg);
        setColour(juce::Slider::rotarySliderFillColourId,         t.knobFg);
        setColour(juce::Slider::rotarySliderOutlineColourId,      t.knobBg);
        setColour(juce::ToggleButton::textColourId,               t.text);
        setColour(juce::ToggleButton::tickDisabledColourId,       t.border);
        setColour(juce::ToggleButton::tickColourId,               t.accent);
        setColour(juce::ListBox::backgroundColourId,              t.libTreeBg);
        setColour(juce::ListBox::textColourId,                    t.libText);
        setColour(juce::AlertWindow::backgroundColourId,          t.panel);
        setColour(juce::AlertWindow::textColourId,                t.text);
    }

    // ── Knob rotary ───────────────────────────────────────────────────────────
    void drawRotarySlider(juce::Graphics& g, int x, int y, int w, int h,
                          float sliderPos, float rotStart, float rotEnd,
                          juce::Slider& slider) override
    {
        const auto& t = currentTheme;
        const float radius  = (float)juce::jmin(w, h) / 2.0f - 4.0f;
        const float centreX = (float)x + (float)w / 2.0f;
        const float centreY = (float)y + (float)h / 2.0f;
        const float angle   = rotStart + sliderPos * (rotEnd - rotStart);

        g.setColour(t.knobBg);
        g.fillEllipse(centreX - radius, centreY - radius, radius * 2.0f, radius * 2.0f);
        g.setColour(t.border);
        g.drawEllipse(centreX - radius, centreY - radius, radius * 2.0f, radius * 2.0f, 1.5f);

        const bool isBipolar = (slider.getComponentID() == "bipolar");
        juce::Path arc;
        if (isBipolar)
        {
            const float centre = rotStart + 0.5f * (rotEnd - rotStart);
            arc.addArc(centreX - radius + 3, centreY - radius + 3,
                       (radius - 3) * 2.0f, (radius - 3) * 2.0f,
                       juce::jmin(centre, angle), juce::jmax(centre, angle), true);
        }
        else
        {
            arc.addArc(centreX - radius + 3, centreY - radius + 3,
                       (radius - 3) * 2.0f, (radius - 3) * 2.0f,
                       rotStart, angle, true);
        }
        g.setColour(t.knobFg);
        g.strokePath(arc, juce::PathStrokeType(2.5f, juce::PathStrokeType::curved,
                                                juce::PathStrokeType::rounded));

        const float len = radius * 0.55f;
        const float ix  = centreX + len * std::sin(angle);
        const float iy  = centreY - len * std::cos(angle);
        g.setColour(t.accent);
        g.drawLine(centreX, centreY, ix, iy, 2.0f);
        g.fillEllipse(centreX - 2.5f, centreY - 2.5f, 5.0f, 5.0f);
    }

    // ── Buttons ───────────────────────────────────────────────────────────────
    void drawButtonBackground(juce::Graphics& g, juce::Button& btn,
                               const juce::Colour& /*bgColour*/,
                               bool isHighlighted, bool isDown) override
    {
        const auto& t = currentTheme;
        auto bounds   = btn.getLocalBounds().toFloat().reduced(0.5f);
        auto baseCol  = btn.getToggleState()
            ? btn.findColour(juce::TextButton::buttonOnColourId)
            : btn.findColour(juce::TextButton::buttonColourId);
        if (isHighlighted) baseCol = baseCol.brighter(0.15f);
        if (isDown)        baseCol = baseCol.darker(0.2f);
        g.setColour(baseCol);
        g.fillRoundedRectangle(bounds, 4.0f);
        g.setColour(t.border);
        g.drawRoundedRectangle(bounds, 4.0f, 1.0f);
    }

    // ── Linear sliders ────────────────────────────────────────────────────────
    void drawLinearSlider(juce::Graphics& g, int x, int y, int w, int h,
                          float sliderPos, float minSliderPos, float maxSliderPos,
                          const juce::Slider::SliderStyle style, juce::Slider& slider) override
    {
        if (style == juce::Slider::LinearVertical)
        {
            const auto& t  = currentTheme;
            float trackX   = x + w / 2.0f - 2.0f;
            g.setColour(t.knobBg);
            g.fillRoundedRectangle(trackX, (float)y, 4.0f, (float)h, 2.0f);
            float fillH = (float)y + (float)h - sliderPos;
            if (fillH > 0)
            {
                g.setColour(t.accent);
                g.fillRoundedRectangle(trackX, sliderPos, 4.0f, fillH, 2.0f);
            }
            g.setColour(t.accent);
            g.fillEllipse(x + w / 2.0f - 7.0f, sliderPos - 7.0f, 14.0f, 14.0f);
            g.setColour(t.bg);
            g.fillEllipse(x + w / 2.0f - 4.0f, sliderPos - 4.0f, 8.0f, 8.0f);
        }
        else
        {
            LookAndFeel_V4::drawLinearSlider(g, x, y, w, h, sliderPos,
                                              minSliderPos, maxSliderPos, style, slider);
        }
    }

    // ── ComboBox ──────────────────────────────────────────────────────────────
    void drawComboBox(juce::Graphics& g, int w, int h, bool /*isDown*/,
                      int buttonX, int buttonY, int buttonW, int buttonH,
                      juce::ComboBox& /*box*/) override
    {
        const auto& t = currentTheme;
        auto bounds   = juce::Rectangle<float>(0, 0, (float)w, (float)h);
        g.setColour(t.button);
        g.fillRoundedRectangle(bounds, 4.0f);
        g.setColour(t.border);
        g.drawRoundedRectangle(bounds.reduced(0.5f), 4.0f, 1.0f);
        g.setColour(t.accent);
        juce::Path arrow;
        float arrowX = (float)buttonX + (float)buttonW * 0.5f;
        float arrowY = (float)buttonY + (float)buttonH * 0.5f;
        arrow.addTriangle(arrowX - 4.0f, arrowY - 2.0f,
                          arrowX + 4.0f, arrowY - 2.0f,
                          arrowX,        arrowY + 3.0f);
        g.fillPath(arrow);
    }

private:
    DSKTheme currentTheme;
    int      themeIndex { 0 };
};
