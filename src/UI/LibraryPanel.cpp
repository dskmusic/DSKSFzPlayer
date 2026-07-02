#include <JuceHeader.h>
#include "LibraryPanel.h"
#include "DSKLookAndFeel.h"
#include "RemoteBanner.h"

// ── theme helper —————————————————————————————————————————————————————————————
static DSKTheme libTheme(const juce::Component* c)
{
    if (c)
        if (auto* lf = dynamic_cast<const DSKLookAndFeel*>(&c->getLookAndFeel()))
            return lf->getTheme();
    return DSKLookAndFeel::themePreset(0);
}

static DSKTheme libTheme(const juce::TreeViewItem& item)
{
    return libTheme(item.getOwnerView());
}

// helper: fire a callback deferred to the next message loop tick
static void callLater(std::function<void()> fn)
{
    juce::MessageManager::callAsync(std::move(fn));
}

// ══════════════════════════════════════════════════════════════════════════════
// FavoritesManager
// ══════════════════════════════════════════════════════════════════════════════
void FavoritesManager::save(juce::PropertiesFile& props) const
{
    // root entries
    props.setValue("fav_root_count", (int)rootEntries.size());
    for (int i = 0; i < (int)rootEntries.size(); ++i)
    {
        juce::String p = "fav_root_" + juce::String(i);
        props.setValue(p + "_name", rootEntries[i].name);
        props.setValue(p + "_path", rootEntries[i].file.getFullPathName());
    }

    // folders
    props.setValue("fav_count", (int)folders.size());
    for (int i = 0; i < (int)folders.size(); ++i)
    {
        const auto& f          = folders[i];
        const juce::String pfx = "fav_" + juce::String(i);
        props.setValue(pfx + "_name",  f.name);
        props.setValue(pfx + "_count", (int)f.entries.size());
        for (int j = 0; j < (int)f.entries.size(); ++j)
        {
            const juce::String ep = pfx + "_" + juce::String(j);
            props.setValue(ep + "_name", f.entries[j].name);
            props.setValue(ep + "_path", f.entries[j].file.getFullPathName());
        }
    }
}

void FavoritesManager::load(juce::PropertiesFile& props)
{
    rootEntries.clear();
    const int rc = props.getIntValue("fav_root_count", 0);
    for (int i = 0; i < rc; ++i)
    {
        juce::String p = "fav_root_" + juce::String(i);
        Entry e;
        e.name = props.getValue(p + "_name", "");
        e.file = juce::File(props.getValue(p + "_path", ""));
        if (e.name.isNotEmpty() && e.file != juce::File())
            rootEntries.push_back(e);
    }

    folders.clear();
    const int count = props.getIntValue("fav_count", 0);
    for (int i = 0; i < count; ++i)
    {
        const juce::String pfx = "fav_" + juce::String(i);
        Folder f;
        f.name = props.getValue(pfx + "_name", "");
        if (f.name.isEmpty()) continue;
        const int ec = props.getIntValue(pfx + "_count", 0);
        for (int j = 0; j < ec; ++j)
        {
            const juce::String ep = pfx + "_" + juce::String(j);
            Entry e;
            e.name = props.getValue(ep + "_name", "");
            e.file = juce::File(props.getValue(ep + "_path", ""));
            if (e.name.isNotEmpty() && e.file != juce::File())
                f.entries.push_back(e);
        }
        folders.push_back(f);
    }
}

bool FavoritesManager::addRootEntry(const juce::String& name, const juce::File& file)
{
    for (auto& e : rootEntries) if (e.file == file) return false;
    rootEntries.push_back({ name, file });
    return true;
}

bool FavoritesManager::removeRootEntry(int idx)
{
    if (idx < 0 || idx >= (int)rootEntries.size()) return false;
    rootEntries.erase(rootEntries.begin() + idx);
    return true;
}

bool FavoritesManager::addFolder(const juce::String& name)
{
    if (name.isEmpty() || findFolder(name) >= 0) return false;
    folders.push_back({ name, {} });
    return true;
}

bool FavoritesManager::removeFolder(int idx)
{
    if (idx < 0 || idx >= (int)folders.size()) return false;
    folders.erase(folders.begin() + idx);
    return true;
}

bool FavoritesManager::renameFolder(int idx, const juce::String& newName)
{
    if (idx < 0 || idx >= (int)folders.size() || newName.isEmpty()) return false;
    if (findFolder(newName) >= 0 && findFolder(newName) != idx) return false;
    folders[idx].name = newName;
    return true;
}

bool FavoritesManager::addEntry(int folderIdx, const juce::String& name, const juce::File& file)
{
    if (folderIdx < 0 || folderIdx >= (int)folders.size()) return false;
    for (auto& e : folders[folderIdx].entries) if (e.file == file) return false;
    folders[folderIdx].entries.push_back({ name, file });
    return true;
}

bool FavoritesManager::removeEntry(int folderIdx, int entryIdx)
{
    if (folderIdx < 0 || folderIdx >= (int)folders.size()) return false;
    auto& v = folders[folderIdx].entries;
    if (entryIdx < 0 || entryIdx >= (int)v.size()) return false;
    v.erase(v.begin() + entryIdx);
    return true;
}

bool FavoritesManager::moveEntry(int fromFolder, int entryIdx, int toFolder)
{
    if (fromFolder < 0 || fromFolder >= (int)folders.size()) return false;
    if (toFolder   < 0 || toFolder   >= (int)folders.size()) return false;
    if (fromFolder == toFolder) return false;
    auto& src = folders[fromFolder].entries;
    if (entryIdx < 0 || entryIdx >= (int)src.size()) return false;
    Entry e = src[entryIdx];
    src.erase(src.begin() + entryIdx);
    folders[toFolder].entries.push_back(e);
    return true;
}

int FavoritesManager::findFolder(const juce::String& name) const
{
    for (int i = 0; i < (int)folders.size(); ++i)
        if (folders[i].name == name) return i;
    return -1;
}

// ══════════════════════════════════════════════════════════════════════════════
// LibraryTreeItem
// ══════════════════════════════════════════════════════════════════════════════
LibraryTreeItem::LibraryTreeItem(Type t, const juce::File& f)
    : type(t), file(f) {}

juce::String LibraryTreeItem::getUniqueName() const { return file.getFullPathName(); }

bool LibraryTreeItem::mightContainSubItems()
{
    return type == Type::RootFolder || type == Type::SubFolder;
}

void LibraryTreeItem::itemOpennessChanged(bool isOpen)
{
    if (file == juce::File()) return;
    if (isOpen) populateChildren();
    else        clearSubItems();
}

void LibraryTreeItem::populateChildren()
{
    clearSubItems();
    juce::Array<juce::File> children;
    file.findChildFiles(children,
        juce::File::findFilesAndDirectories | juce::File::ignoreHiddenFiles, false);

    std::sort(children.begin(), children.end(),
        [](const juce::File& a, const juce::File& b)
        {
            if (a.isDirectory() && !b.isDirectory()) return true;
    if (!a.isDirectory() && b.isDirectory()) return false;
    return a.getFileName().compareIgnoreCase(b.getFileName()) < 0;
        });

    for (auto& child : children)
    {
        if (child.isDirectory())
        {
            // Escanear si hay algún archivo jugable dentro de esta carpeta (o sus subcarpetas)
            juce::Array<juce::File> playableFiles;
            child.findChildFiles(playableFiles, juce::File::findFiles, true, "*.sfz;*.sf2;*.zip;*.SFZ;*.SF2;*.ZIP");

            // Solo mostramos la carpeta si tiene al menos un SFZ o ZIP dentro
            if (!playableFiles.isEmpty())
            {
                auto* item = new LibraryTreeItem(Type::SubFolder, child);
                item->onFileActivated = onFileActivated;
                item->onSFZRightClicked = onSFZRightClicked;
                item->onSFZPreview = onSFZPreview;
                addSubItem(item);
            }
        }
        else
        {
            juce::String ext = child.getFileExtension().toLowerCase();
            if (ext == ".sfz" || ext == ".sf2" || ext == ".zip")
            {
                auto* item = new LibraryTreeItem(
                    ext == ".sfz" ? Type::SFZFile : (ext == ".sf2" ? Type::SF2File : Type::ZIPFile), child);
                item->onFileActivated = onFileActivated;
                item->onSFZRightClicked = onSFZRightClicked;
                item->onSFZPreview = onSFZPreview;
                addSubItem(item);
            }
        }
    }
}

void LibraryTreeItem::paintItem(juce::Graphics& g, int w, int h)
{
    const auto t = libTheme(*this);
    if (isSelected()) g.fillAll(t.libSel);

    juce::String icon;
    juce::Colour col = t.libText;
    switch (type)
    {
    case Type::RootFolder: icon = "[R]"; col = t.accent;   break;
    case Type::SubFolder:  icon = "[F]"; col = t.libDim;   break;
    case Type::SFZFile:    icon = "[S]"; col = t.accent2;  break;
    case Type::SF2File:    icon = "[2]"; col = t.favAmber; break;
    case Type::ZIPFile:    icon = "[Z]"; col = t.accent;   break;
    }

    g.setColour(col);
    g.setFont(juce::Font(11.5f, juce::Font::bold));
    g.drawText(icon, 2, 0, 24, h, juce::Justification::centred);

    g.setColour(t.libText);
    g.setFont(juce::Font(12.5f));
    g.drawText(file.getFileName(), 28, 0, w - 30, h, juce::Justification::centredLeft);
}

void LibraryTreeItem::itemDoubleClicked(const juce::MouseEvent&)
{
    if (type == Type::RootFolder || type == Type::SubFolder)
        setOpen(!isOpen());
    else if (onFileActivated)
        onFileActivated(file, type == Type::ZIPFile);
}

void LibraryTreeItem::itemClicked(const juce::MouseEvent& e)
{
    if (e.mods.isRightButtonDown())
    {
        if ((type == Type::SFZFile || type == Type::SF2File) && onSFZRightClicked)
            onSFZRightClicked(file, e);
        return;
    }
    if ((type == Type::SFZFile || type == Type::SF2File) && e.x < 28 && onSFZPreview)
        onSFZPreview(file);
}

// ══════════════════════════════════════════════════════════════════════════════
// SearchResultsModel
// ══════════════════════════════════════════════════════════════════════════════
void SearchResultsModel::paintListBoxItem(int row, juce::Graphics& g,
                                           int w, int h, bool selected)
{
    // model has no reference to a Component, use default theme for search results
    const auto t = DSKLookAndFeel::themePreset(0);
    if (selected) g.fillAll(t.libSel);
    if (row < 0 || row >= (int)results.size()) return;
    auto& r = results[(size_t)row];

    g.setColour(t.accent2);
    g.setFont(juce::Font(11.5f, juce::Font::bold));
    g.drawText(r.name, 8, 1, w - 10, h / 2, juce::Justification::centredLeft);
    g.setColour(t.libDim);
    g.setFont(juce::Font(10.0f));
    g.drawText(r.location, 8, h / 2, w - 10, h / 2 - 1, juce::Justification::centredLeft);
}

void SearchResultsModel::listBoxItemDoubleClicked(int row, const juce::MouseEvent&)
{
    if (row >= 0 && row < (int)results.size() && onFileActivated)
        onFileActivated(results[(size_t)row].file);
}

// ══════════════════════════════════════════════════════════════════════════════
// FavoritesFileItem  (folderIdx == -1 → root entry)
// ══════════════════════════════════════════════════════════════════════════════
FavoritesFileItem::FavoritesFileItem(FavoritesManager* fm, int fi, int ei)
    : favorites(fm), folderIdx(fi), entryIdx(ei) {}

juce::String FavoritesFileItem::getUniqueName() const
{
    return "__FAV_FILE_" + juce::String(folderIdx) + "_" + juce::String(entryIdx);
}

static bool favGetEntry(FavoritesManager* fav, int fi, int ei,
                        FavoritesManager::Entry& out)
{
    if (fi == -1)
    {
        if (ei < 0 || ei >= (int)fav->rootEntries.size()) return false;
        out = fav->rootEntries[ei];
        return true;
    }
    if (fi < 0 || fi >= (int)fav->folders.size()) return false;
    auto& folder = fav->folders[fi];
    if (ei < 0 || ei >= (int)folder.entries.size()) return false;
    out = folder.entries[ei];
    return true;
}

void FavoritesFileItem::paintItem(juce::Graphics& g, int w, int h)
{
    const auto t = libTheme(*this);
    if (isSelected()) g.fillAll(t.favSel);

    FavoritesManager::Entry entry;
    if (!favGetEntry(favorites, folderIdx, entryIdx, entry)) return;

    g.setColour(t.favGold);
    g.setFont(juce::Font(12.0f, juce::Font::bold));
    g.drawText("[*]", 4, 0, 18, h, juce::Justification::centred); // Cambiado a un asterisco estándar

    g.setColour(t.libText);
    g.setFont(juce::Font(12.5f));
    g.drawText(entry.name, 28, 0, w - 30, h, juce::Justification::centredLeft);
}

void FavoritesFileItem::itemDoubleClicked(const juce::MouseEvent&)
{
    FavoritesManager::Entry e;
    if (favGetEntry(favorites, folderIdx, entryIdx, e) && onActivated)
        onActivated(e.file);
}

void FavoritesFileItem::itemClicked(const juce::MouseEvent& e)
{
    if (!e.mods.isRightButtonDown())
    {
        if (e.x < 28 && onSFZPreview)
        {
            FavoritesManager::Entry entry;
            if (favGetEntry(favorites, folderIdx, entryIdx, entry))
                onSFZPreview(entry.file);
        }
        return;
    }

    FavoritesManager::Entry entry;
    if (!favGetEntry(favorites, folderIdx, entryIdx, entry)) return;

    const juce::String instrName = entry.name;
    FavoritesManager*  fav       = favorites;
    const int          fi        = folderIdx;
    const int          ei        = entryIdx;
    auto               onChange  = onChanged;
    auto               onAct     = onActivated;

    juce::PopupMenu moveMenu;
    // Can move to root if in a folder, or to a folder if at root
    if (fi != -1)
        moveMenu.addItem(198, "Favorites (no folder)");
    for (int i = 0; i < (int)fav->folders.size(); ++i)
        if (i != fi)
            moveMenu.addItem(200 + i, fav->folders[i].name);

    juce::PopupMenu menu;
    menu.addItem(1, "Load \"" + instrName + "\"");
    menu.addSeparator();
    menu.addSubMenu("Move to...", moveMenu, moveMenu.getNumItems() > 0);
    menu.addSeparator();
    menu.addItem(2, "Remove from favorites");

    menu.showMenuAsync(
        juce::PopupMenu::Options(),
        [fav, fi, ei, onAct, onChange](int result)
        {
            if (result == 1)
            {
                FavoritesManager::Entry entry2;
                if (favGetEntry(fav, fi, ei, entry2) && onAct)
                    onAct(entry2.file);
            }
            else if (result == 2)
            {
                FavoritesManager::Entry entry2;
                if (!favGetEntry(fav, fi, ei, entry2)) return;
                const juce::String nm = entry2.name;
                juce::AlertWindow::showAsync(
                    juce::MessageBoxOptions()
                        .withIconType(juce::MessageBoxIconType::QuestionIcon)
                        .withTitle("Remove from Favorites")
                        .withMessage("Remove \"" + nm + "\" from favorites?")
                        .withButton("Remove")
                        .withButton("Cancel"),
                    [fav, fi, ei, onChange](int r)
                    {
                        if (r == 1)
                        {
                            if (fi == -1) fav->removeRootEntry(ei);
                            else          fav->removeEntry(fi, ei);
                            if (onChange) callLater(onChange);
                        }
                    });
            }
            else if (result == 198)
            {
                // move from folder to root
                FavoritesManager::Entry entry2;
                if (!favGetEntry(fav, fi, ei, entry2)) return;
                if (fi >= 0) fav->removeEntry(fi, ei);
                fav->addRootEntry(entry2.name, entry2.file);
                if (onChange) callLater(onChange);
            }
            else if (result >= 200)
            {
                const int toFolder = result - 200;
                FavoritesManager::Entry entry2;
                if (!favGetEntry(fav, fi, ei, entry2)) return;
                if (fi == -1)
                {
                    fav->addEntry(toFolder, entry2.name, entry2.file);
                    fav->removeRootEntry(ei);
                }
                else
                {
                    fav->moveEntry(fi, ei, toFolder);
                }
                if (onChange) callLater(onChange);
            }
        });
}

// ══════════════════════════════════════════════════════════════════════════════
// FavoritesFolderItem
// ══════════════════════════════════════════════════════════════════════════════
FavoritesFolderItem::FavoritesFolderItem(FavoritesManager* fm, int fi)
    : favorites(fm), folderIdx(fi) {}

juce::String FavoritesFolderItem::getUniqueName() const
{
    juce::String name;
    if (folderIdx >= 0 && folderIdx < (int)favorites->folders.size())
        name = favorites->folders[folderIdx].name;
    return "__FAV_FOLDER_" + juce::String(folderIdx) + "_" + name;
}

void FavoritesFolderItem::paintItem(juce::Graphics& g, int w, int h)
{
    const auto t = libTheme(*this);
    if (isSelected()) g.fillAll(t.favSel);
    if (folderIdx < 0 || folderIdx >= (int)favorites->folders.size()) return;

    g.setColour(t.favAmber);
    g.setFont(juce::Font(11.5f, juce::Font::bold));
    g.drawText("[F]", 2, 0, 24, h, juce::Justification::centred);

    g.setColour(t.favGold);
    g.setFont(juce::Font(12.5f, juce::Font::bold));
    g.drawText(favorites->folders[folderIdx].name, 28, 0, w - 30, h,
               juce::Justification::centredLeft);
}

void FavoritesFolderItem::itemOpennessChanged(bool isOpen)
{
    if (isOpen) populateChildren();
    else        clearSubItems();
}

void FavoritesFolderItem::populateChildren()
{
    clearSubItems();
    if (folderIdx < 0 || folderIdx >= (int)favorites->folders.size()) return;
    auto& folder = favorites->folders[folderIdx];
    for (int i = 0; i < (int)folder.entries.size(); ++i)
    {
        auto* fi   = new FavoritesFileItem(favorites, folderIdx, i);
        fi->onActivated  = onActivated;
        fi->onChanged    = onChanged;
        fi->onSFZPreview = onSFZPreview;
        addSubItem(fi);
    }
}

void FavoritesFolderItem::itemDoubleClicked(const juce::MouseEvent&)
{
    setOpen(!isOpen());
}

void FavoritesFolderItem::rebuild()
{
    clearSubItems();
    if (isOpen()) populateChildren();
}

void FavoritesFolderItem::itemClicked(const juce::MouseEvent& e)
{
    if (!e.mods.isRightButtonDown()) return;
    if (folderIdx < 0 || folderIdx >= (int)favorites->folders.size()) return;

    const juce::String folderName = favorites->folders[folderIdx].name;
    FavoritesManager*  fav        = favorites;
    const int          fi         = folderIdx;
    auto               onChange   = onChanged;

    juce::PopupMenu menu;
    menu.addItem(1, "Rename...");
    menu.addItem(2, "Delete folder");

    menu.showMenuAsync(
        juce::PopupMenu::Options(),
        [fav, fi, folderName, onChange](int result)
        {
            if (result == 1)
            {
                auto* aw = new juce::AlertWindow("Rename Folder",
                    "Enter a new name for \"" + folderName + "\":",
                    juce::MessageBoxIconType::NoIcon);
                aw->addTextEditor("name", folderName, "New name:");
                aw->addButton("Rename", 1, juce::KeyPress(juce::KeyPress::returnKey));
                aw->addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey));
                aw->enterModalState(false,
                    juce::ModalCallbackFunction::create([aw, fav, fi, onChange](int r)
                    {
                        if (r == 1)
                        {
                            juce::String nm = aw->getTextEditorContents("name").trim();
                            if (nm.isNotEmpty() && fav->renameFolder(fi, nm))
                                if (onChange) callLater(onChange);
                        }
                    }), true);
            }
            else if (result == 2)
            {
                juce::AlertWindow::showAsync(
                    juce::MessageBoxOptions()
                        .withIconType(juce::MessageBoxIconType::QuestionIcon)
                        .withTitle("Delete Folder")
                        .withMessage("Delete folder \"" + folderName + "\" and all its entries?")
                        .withButton("Delete")
                        .withButton("Cancel"),
                    [fav, fi, onChange](int r)
                    {
                        if (r == 1)
                        {
                            fav->removeFolder(fi);
                            if (onChange) callLater(onChange);
                        }
                    });
            }
        });
}

// ══════════════════════════════════════════════════════════════════════════════
// FavoritesRootItem
// ══════════════════════════════════════════════════════════════════════════════
FavoritesRootItem::FavoritesRootItem(FavoritesManager* fm)
    : favorites(fm) {}

void FavoritesRootItem::paintItem(juce::Graphics& g, int w, int h)
{
    const auto t = libTheme(*this);
    g.fillAll(t.favBg);
    if (isSelected()) g.fillAll(t.favSel);

    g.setColour(t.favGold);
    g.setFont(juce::Font(12.5f, juce::Font::bold));
    g.drawText("FAVORITES", 8, 0, w - 10, h, juce::Justification::centredLeft);

    g.setColour(t.libDim);
    g.setFont(juce::Font(10.0f));
    g.drawText("right-click", 24, 0, w - 28, h, juce::Justification::centredRight);

    g.setColour(t.favGold.withAlpha(0.25f));
    g.drawLine(0.0f, (float)h - 1.0f, (float)w, (float)h - 1.0f, 1.0f);
}

void FavoritesRootItem::itemDoubleClicked(const juce::MouseEvent&)
{
    setOpen(!isOpen());
}

void FavoritesRootItem::rebuild()
{
    // capture open-state of folder items before clearing
    int rootFileCount = 0;
    std::vector<bool> folderWasOpen;
    for (int i = 0; i < getNumSubItems(); ++i)
    {
        auto* sub = getSubItem(i);
        if (dynamic_cast<FavoritesFolderItem*>(sub))
            folderWasOpen.push_back(sub->isOpen());
        else
            ++rootFileCount;
    }

    clearSubItems();

    // root entries first
    for (int i = 0; i < (int)favorites->rootEntries.size(); ++i)
    {
        auto* fi = new FavoritesFileItem(favorites, -1, i);
        fi->onActivated  = onActivated;
        fi->onChanged    = onChanged;
        fi->onSFZPreview = onSFZPreview;
        addSubItem(fi);
    }

    // then folders
    for (int i = 0; i < (int)favorites->folders.size(); ++i)
    {
        auto* fi = new FavoritesFolderItem(favorites, i);
        fi->onActivated  = onActivated;
        fi->onChanged    = onChanged;
        fi->onSFZPreview = onSFZPreview;
        addSubItem(fi);
        if (i < (int)folderWasOpen.size() && folderWasOpen[i])
            fi->setOpen(true);
    }
}

void FavoritesRootItem::itemClicked(const juce::MouseEvent& e)
{
    if (!e.mods.isRightButtonDown()) return;

    FavoritesManager* fav      = favorites;
    auto              onChange = onChanged;

    juce::PopupMenu menu;
    menu.addItem(1, "New folder...");

    menu.showMenuAsync(
        juce::PopupMenu::Options(),
        [fav, onChange](int result)
        {
            if (result != 1) return;
            auto* aw = new juce::AlertWindow("New Favorites Folder",
                "Enter a name for the new folder:",
                juce::MessageBoxIconType::NoIcon);
            aw->addTextEditor("name", "", "Folder name:");
            aw->addButton("Create", 1, juce::KeyPress(juce::KeyPress::returnKey));
            aw->addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey));
            aw->enterModalState(false,
                juce::ModalCallbackFunction::create([aw, fav, onChange](int r)
                {
                    if (r == 1)
                    {
                        juce::String nm = aw->getTextEditorContents("name").trim();
                        if (nm.isNotEmpty() && fav->addFolder(nm))
                            if (onChange) callLater(onChange);
                    }
                }), true);
        });
}

// ══════════════════════════════════════════════════════════════════════════════
// LibraryPanel
// ══════════════════════════════════════════════════════════════════════════════
LibraryPanel::LibraryPanel()
{
    searchBox.setTextToShowWhenEmpty("Search instruments...", juce::Colour(0xFF667788));
    searchBox.setColour(juce::TextEditor::backgroundColourId, juce::Colour(0xFF0D1B35));
    searchBox.setColour(juce::TextEditor::textColourId,       juce::Colour(0xFFCCDDEE));
    searchBox.setColour(juce::TextEditor::outlineColourId,    juce::Colour(0xFF334466));
    searchBox.addListener(this);
    addAndMakeVisible(searchBox);

    clearSearchBtn.setColour(juce::TextButton::buttonColourId,  juce::Colours::transparentBlack);
    clearSearchBtn.setColour(juce::TextButton::textColourOffId, juce::Colour(0xFF667788));
    clearSearchBtn.setColour(juce::TextButton::textColourOnId,  juce::Colour(0xFFCCDDEE));
    clearSearchBtn.setVisible(false);
    clearSearchBtn.onClick = [this]()
    {
        searchBox.clear();
        searchResultsList.setVisible(false);
        treeView.setVisible(true);
        searchBox.grabKeyboardFocus();
    };
    addAndMakeVisible(clearSearchBtn);

    for (auto* btn : { &addFolderBtn, &removeFolderBtn, &refreshBtn })
    {
        btn->setColour(juce::TextButton::buttonColourId,  juce::Colour(0xFF1A2A4A));
        btn->setColour(juce::TextButton::textColourOffId, juce::Colour(0xFF00D4FF));
        addAndMakeVisible(btn);
    }
    addFolderBtn.setTooltip("Add library folder");
    removeFolderBtn.setTooltip("Remove selected folder");
    refreshBtn.setTooltip("Expand / Collapse all folders");

    addFolderBtn.onClick = [this]()
    {
        fileChooser = std::make_unique<juce::FileChooser>(
            "Select a folder containing SFZ instruments",
            juce::File::getSpecialLocation(juce::File::userMusicDirectory));
        fileChooser->launchAsync(
            juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectDirectories,
            [this](const juce::FileChooser& fc)
            {
                auto result = fc.getResult();
                if (result.isDirectory()) addFolder(result);
            });
    };

    removeFolderBtn.onClick = [this]() { removeSelectedFolder(); };

    refreshBtn.onClick = [this]()
    {
        if (!invisibleRoot) return;
        bool anyOpen = false;
        for (int i = 0; i < invisibleRoot->getNumSubItems(); ++i)
        {
            auto* sub = invisibleRoot->getSubItem(i);
            if (sub && sub->getUniqueName() != "__FAVORITES_ROOT__" && sub->isOpen())
            { anyOpen = true; break; }
        }
        bool newState = !anyOpen;
        for (int i = 0; i < invisibleRoot->getNumSubItems(); ++i)
        {
            auto* sub = invisibleRoot->getSubItem(i);
            if (sub && sub->getUniqueName() != "__FAVORITES_ROOT__")
                sub->setOpen(newState);
        }
        invisibleRoot->treeHasChanged();
    };

    treeView.setColour(juce::TreeView::backgroundColourId, juce::Colour(0xFF0D1B35));
    treeView.setRootItemVisible(false);
    addAndMakeVisible(treeView);

    searchResultsList.setModel(&searchModel);
    searchResultsList.setColour(juce::ListBox::backgroundColourId, juce::Colour(0xFF0D1B35));
    searchResultsList.setRowHeight(36);
    addAndMakeVisible(searchResultsList);
    searchResultsList.setVisible(false);

    searchModel.onFileActivated = [this](const juce::File& f)
    {
        handleFileActivated(f, f.hasFileExtension(".zip"));
    };

    bannerImage = juce::ImageCache::getFromMemory(BinaryData::banner_png,
        BinaryData::banner_pngSize);

    fetchRemoteBanner();

    refreshTree();
}

LibraryPanel::~LibraryPanel()
{
    bannerStillAlive->store(false);
    favoritesRootItem = nullptr;
    treeView.setRootItem(nullptr);
}

// Descarga banner.json + imagen en un hilo aparte; si tiene éxito, sustituye el
// banner embebido. Si falla por cualquier motivo (red, JSON inválido, imagen
// corrupta) el banner/URL embebidos de fallback quedan como están, sin más.
void LibraryPanel::fetchRemoteBanner()
{
    auto aliveFlag = bannerStillAlive;
    juce::Thread::launch([this, aliveFlag]
    {
        auto result = RemoteBanner::fetchBlocking();
        if (!result.ok)
            return;

        juce::MessageManager::callAsync([this, aliveFlag, result]
        {
            if (!aliveFlag->load())
                return; // el LibraryPanel ya se destruyó
            bannerImage    = result.image;
            bannerClickUrl = result.url;
            repaint();
        });
    });
}

void LibraryPanel::lookAndFeelChanged()
{
    const auto t = libTheme(this);
    searchBox.setTextToShowWhenEmpty("Search instruments...", t.libDim);
    searchBox.setColour(juce::TextEditor::backgroundColourId, t.libTreeBg);
    searchBox.setColour(juce::TextEditor::textColourId,       t.libText);
    searchBox.setColour(juce::TextEditor::outlineColourId,    t.border);
    treeView.setColour(juce::TreeView::backgroundColourId, t.libTreeBg);
    searchResultsList.setColour(juce::ListBox::backgroundColourId, t.libTreeBg);
    for (auto* btn : { &addFolderBtn, &removeFolderBtn, &refreshBtn })
    {
        btn->setColour(juce::TextButton::buttonColourId,  t.libPanel);
        btn->setColour(juce::TextButton::textColourOffId, t.accent);
    }
    clearSearchBtn.setColour(juce::TextButton::textColourOffId, t.libDim);
    treeView.repaint();
    repaint();
}

void LibraryPanel::paint(juce::Graphics& g)
{
    const auto t = libTheme(this);
    g.fillAll(t.libBg);

    g.setColour(t.libPanel);
    g.fillRect(0, 0, getWidth(), 26);

    g.setColour(t.accent);
    g.setFont(juce::Font(12.0f, juce::Font::bold));
    g.drawText("LIBRARY", 6, 4, 80, 18, juce::Justification::centredLeft);

    if (bannerImage.isValid())
    {
        int bannerH = 40;
        g.drawImageWithin(bannerImage, 5, getHeight() - bannerH - 5,
                          getWidth() - 10, bannerH, juce::RectanglePlacement::stretchToFit);
    }
}

juce::Rectangle<int> LibraryPanel::getBannerBounds() const
{
    return { 5, getHeight() - 45, getWidth() - 10, 40 };
}

void LibraryPanel::mouseDown(const juce::MouseEvent& e)
{
    if (bannerImage.isValid() && getBannerBounds().contains(e.getPosition()))
    {
        RemoteBanner::trackClickAsync(bannerClickUrl);
        juce::URL(bannerClickUrl).launchInDefaultBrowser();
    }
}

void LibraryPanel::mouseMove(const juce::MouseEvent& e)
{
    if (bannerImage.isValid() && getBannerBounds().contains(e.getPosition()))
        setMouseCursor(juce::MouseCursor::PointingHandCursor);
    else
        setMouseCursor(juce::MouseCursor::NormalCursor);
}

void LibraryPanel::mouseExit(const juce::MouseEvent& e)
{
    juce::ignoreUnused(e);
    setMouseCursor(juce::MouseCursor::NormalCursor);
}

void LibraryPanel::resized()
{
    auto area = getLocalBounds();

    auto header = area.removeFromTop(26);
    refreshBtn.setBounds     (header.removeFromRight(30).reduced(2));
    removeFolderBtn.setBounds(header.removeFromRight(26).reduced(2));
    addFolderBtn.setBounds   (header.removeFromRight(26).reduced(2));

    searchBox.setBounds(area.removeFromTop(28).reduced(4, 3));
    clearSearchBtn.setBounds(searchBox.getRight() - 24, searchBox.getY() + 2,
                             20, searchBox.getHeight() - 4);

    area.removeFromBottom(45);  // banner footer
    treeView.setBounds(area);
    searchResultsList.setBounds(area);
}

void LibraryPanel::addFolder(const juce::File& folder)
{
    if (!libraryFolderPaths.contains(folder.getFullPathName()))
        libraryFolderPaths.add(folder.getFullPathName());
    refreshTree();
}

void LibraryPanel::refreshTree()
{
    favoritesRootItem = nullptr;
    treeView.setRootItem(nullptr);

    invisibleRoot = std::make_unique<LibraryTreeItem>(
        LibraryTreeItem::Type::RootFolder, juce::File());

    auto* favRoot = new FavoritesRootItem(&favorites);
    favRoot->onActivated  = [this](const juce::File& f) { handleFileActivated(f, false); };
    favRoot->onChanged    = [this]() { rebuildFavoritesSection(); };
    favRoot->onSFZPreview = [this](const juce::File& f) { if (onSFZPreviewRequested) onSFZPreviewRequested(f); };
    invisibleRoot->addSubItem(favRoot);
    favoritesRootItem = favRoot;

    for (auto& path : libraryFolderPaths)
    {
        juce::File folder(path);
        if (!folder.isDirectory()) continue;
        auto* item = new LibraryTreeItem(LibraryTreeItem::Type::RootFolder, folder);
        item->onFileActivated   = [this](const juce::File& f, bool iz) { handleFileActivated(f, iz); };
        item->onSFZRightClicked = [this](const juce::File& f, const juce::MouseEvent& ev) { showAddToFavoritesMenu(f, ev); };
        item->onSFZPreview      = [this](const juce::File& f) { if (onSFZPreviewRequested) onSFZPreviewRequested(f); };
        invisibleRoot->addSubItem(item);
    }

    treeView.setRootItem(invisibleRoot.get());
    invisibleRoot->setOpen(true);
    favoritesRootItem->setOpen(false);
    favoritesRootItem->rebuild();

    juce::MessageManager::callAsync([this, treeState = savedTreeOpenState]()
    {
        if (!invisibleRoot) return;
        savedTreeOpenState.clear();
        if (treeState.isNotEmpty())
        {
            if (auto xml = juce::parseXML(treeState))
                treeView.restoreOpennessState(*xml, false);
        }
        else
        {
            for (int i = 0; i < invisibleRoot->getNumSubItems(); ++i)
            {
                auto* sub = invisibleRoot->getSubItem(i);
                if (sub)
                    sub->setOpen(false);
            }
        }
        invisibleRoot->treeHasChanged();
    });
}

void LibraryPanel::rebuildFavoritesSection()
{
    if (favoritesRootItem)
        favoritesRootItem->rebuild();
}

void LibraryPanel::showAddToFavoritesMenu(const juce::File& sfzFile,
                                           const juce::MouseEvent& /*e*/)
{
    const juce::String instrName = sfzFile.getFileNameWithoutExtension();

    juce::PopupMenu folderMenu;
    // Option: add directly to Favorites root (no folder)
    folderMenu.addItem(98, "Favorites (no folder)");
    if (!favorites.folders.empty())
    {
        folderMenu.addSeparator();
        for (int i = 0; i < (int)favorites.folders.size(); ++i)
            folderMenu.addItem(100 + i, favorites.folders[i].name);
    }
    folderMenu.addSeparator();
    folderMenu.addItem(99, "New folder...");

    juce::PopupMenu menu;
    menu.addSubMenu("Add to Favorites", folderMenu);

    menu.showMenuAsync(
        juce::PopupMenu::Options(),
        [this, sfzFile, instrName](int result)
        {
            if (result == 98)
            {
                // add to root
                if (favorites.addRootEntry(instrName, sfzFile))
                    rebuildFavoritesSection();
                else
                    juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::InfoIcon,
                        "Already in Favorites",
                        "\"" + instrName + "\" is already in Favorites.");
            }
            else if (result == 99)
            {
                auto* aw = new juce::AlertWindow("New Favorites Folder",
                    "Enter a name for the new folder:",
                    juce::MessageBoxIconType::NoIcon);
                aw->addTextEditor("name", "", "Folder name:");
                aw->addButton("Create", 1, juce::KeyPress(juce::KeyPress::returnKey));
                aw->addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey));
                aw->enterModalState(false,
                    juce::ModalCallbackFunction::create([this, aw, sfzFile, instrName](int r)
                    {
                        if (r == 1)
                        {
                            juce::String nm = aw->getTextEditorContents("name").trim();
                            if (nm.isNotEmpty())
                            {
                                favorites.addFolder(nm);
                                int idx = favorites.findFolder(nm);
                                if (idx >= 0) favorites.addEntry(idx, instrName, sfzFile);
                                rebuildFavoritesSection();
                            }
                        }
                    }), true);
            }
            else if (result >= 100)
            {
                int idx = result - 100;
                if (favorites.addEntry(idx, instrName, sfzFile))
                    rebuildFavoritesSection();
                else
                    juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::InfoIcon,
                        "Already in Favorites",
                        "\"" + instrName + "\" is already in that folder.");
            }
        });
}

void LibraryPanel::removeSelectedFolder()
{
    auto* sel = dynamic_cast<LibraryTreeItem*>(treeView.getSelectedItem(0));
    if (!sel || sel->getType() != LibraryTreeItem::Type::RootFolder) return;

    juce::String folderPath = sel->getFile().getFullPathName();

    juce::AlertWindow::showAsync(
        juce::MessageBoxOptions()
            .withIconType(juce::MessageBoxIconType::QuestionIcon)
            .withTitle("Remove Folder")
            .withMessage("Remove this folder from the library?")
            .withButton("Remove")
            .withButton("Cancel"),
        [this, folderPath](int result)
        {
            if (result == 1)
            {
                libraryFolderPaths.removeString(folderPath);
                refreshTree();
            }
        });
}

void LibraryPanel::textEditorTextChanged(juce::TextEditor& editor)
{
    juce::String query = editor.getText().trim();
    clearSearchBtn.setVisible(query.isNotEmpty());

    if (query.isEmpty())
    {
        searchResultsList.setVisible(false);
        treeView.setVisible(true);
    }
    else
    {
        treeView.setVisible(false);
        searchResultsList.setVisible(true);
        runSearch(query);
    }
}

void LibraryPanel::runSearch(const juce::String& query)
{
    juce::String lq = query.toLowerCase();
    searchModel.results.clear();

    for (auto& path : libraryFolderPaths)
    {
        juce::File folder(path);
        if (!folder.isDirectory()) continue;
        juce::Array<juce::File> found;
        folder.findChildFiles(found, juce::File::findFiles, true, "*.sfz;*.sf2");
        for (auto& f : found)
        {
            if (f.getFileNameWithoutExtension().toLowerCase().contains(lq) ||
                f.getFullPathName().toLowerCase().contains(lq))
            {
                SearchResultsModel::Result r;
                r.name     = f.getFileNameWithoutExtension();
                r.location = f.getParentDirectory().getFullPathName();
                r.file     = f;
                searchModel.results.push_back(r);
            }
        }
    }
    searchResultsList.updateContent();
}

void LibraryPanel::handleFileActivated(const juce::File& file, bool isZip)
{
    if (isZip)
    {
        fileChooser = std::make_unique<juce::FileChooser>(
            "Extract ZIP to folder",
            juce::File::getSpecialLocation(juce::File::userDocumentsDirectory));

        auto zipPtr = std::make_shared<juce::ZipFile>(file);
        fileChooser->launchAsync(
            juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectDirectories,
            [this, zipPtr](const juce::FileChooser& fc)
            {
                auto dest = fc.getResult();
        if (!dest.isDirectory()) return;

        // 1. Crear ventana de progreso
        // Usamos un puntero raw para el progressBar para que viva mientras la ventana esté abierta
        auto progressWindow = std::make_unique<juce::AlertWindow>("Extracting...",
            "Please wait...",
            juce::AlertWindow::NoIcon);

        double progress = 0.0;
        // CORRECTO: ProgressBar solo acepta la variable de referencia
        auto progressBar = std::make_unique<juce::ProgressBar>(progress);

        progressWindow->addCustomComponent(progressBar.get());
        progressWindow->enterModalState(false, nullptr, false);

        bool hasFilesInRoot = false;
        juce::Array<juce::File> extractedTopLevelFolders;
        int totalEntries = (int)zipPtr->getNumEntries();

        // 2. Bucle de extracción
        for (int i = 0; i < totalEntries; ++i)
        {
            auto* entry = zipPtr->getEntry(i);
            if (entry != nullptr)
            {
                juce::String path = entry->filename;
                int firstSlash = path.indexOfChar('/');
                if (firstSlash < 0) firstSlash = path.indexOfChar('\\');

                if (firstSlash < 0) {
                    if (!path.endsWith("/") && !path.endsWith("\\")) hasFilesInRoot = true;
                }
                else {
                    juce::String topLevel = path.substring(0, firstSlash);
                    juce::File f = dest.getChildFile(topLevel);
                    if (!extractedTopLevelFolders.contains(f)) extractedTopLevelFolders.add(f);
                }
            }

            zipPtr->uncompressEntry(i, dest, true);

            // 3. Actualizar progreso
            progress = (double)(i + 1) / (double)totalEntries;
            progressBar->repaint();

            // Forzar el procesado de eventos de UI para mover la barra
            juce::MessageManager::getInstance()->runDispatchLoopUntil(1);
        }

        progressWindow->exitModalState(0);
        progressWindow = nullptr; // Cerramos ventana

        // 4. Lógica final
        if (hasFilesInRoot) {
            addFolder(dest);
        }
        else {
            for (auto& folder : extractedTopLevelFolders) addFolder(folder);
            if (extractedTopLevelFolders.isEmpty()) addFolder(dest);
        }

        juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::InfoIcon, "ZIP Extracted", "Done!");
            });
    }
    else
    {
        listeners.call([&file](Listener& l) { l.libraryFileRequested(file); });
    }
}

// ── JSON export / import ──────────────────────────────────────────────────────
juce::var LibraryPanel::buildConfigJson() const
{
    auto* root = new juce::DynamicObject();
    root->setProperty("version", 1);

    // save current theme index
    if (auto* lf = dynamic_cast<const DSKLookAndFeel*>(&getLookAndFeel()))
        root->setProperty("uiTheme", lf->getThemeIndex());

    juce::Array<juce::var> libFolders;
    for (auto& p : libraryFolderPaths) libFolders.add(p);
    root->setProperty("library_folders", libFolders);

    // favorites
    auto* favsObj = new juce::DynamicObject();

    juce::Array<juce::var> rootArr;
    for (auto& e : favorites.rootEntries)
    {
        auto* eo = new juce::DynamicObject();
        eo->setProperty("name", e.name);
        eo->setProperty("path", e.file.getFullPathName());
        rootArr.add(juce::var(eo));
    }
    favsObj->setProperty("root_entries", rootArr);

    juce::Array<juce::var> foldersArr;
    for (auto& f : favorites.folders)
    {
        auto* fo = new juce::DynamicObject();
        fo->setProperty("name", f.name);
        juce::Array<juce::var> entriesArr;
        for (auto& e : f.entries)
        {
            auto* eo = new juce::DynamicObject();
            eo->setProperty("name", e.name);
            eo->setProperty("path", e.file.getFullPathName());
            entriesArr.add(juce::var(eo));
        }
        fo->setProperty("entries", entriesArr);
        foldersArr.add(juce::var(fo));
    }
    favsObj->setProperty("folders", foldersArr);
    root->setProperty("favorites", juce::var(favsObj));

    return juce::var(root);
}

int LibraryPanel::applyConfigJson(const juce::var& parsed)
{
    auto* obj = parsed.getDynamicObject();
    if (!obj) return -1;

    int themeIdx = -1;
    if (obj->hasProperty("uiTheme"))
        themeIdx = (int)obj->getProperty("uiTheme");

    if (auto* arr = obj->getProperty("library_folders").getArray())
    {
        libraryFolderPaths.clear();
        for (auto& v : *arr) libraryFolderPaths.add(v.toString());
    }

    auto favsVar = obj->getProperty("favorites");
    if (auto* favsObj = favsVar.getDynamicObject())
    {
        favorites.rootEntries.clear();
        if (auto* arr = favsObj->getProperty("root_entries").getArray())
        {
            for (auto& v : *arr)
                if (auto* eo = v.getDynamicObject())
                {
                    FavoritesManager::Entry e;
                    e.name = eo->getProperty("name").toString();
                    e.file = juce::File(eo->getProperty("path").toString());
                    if (e.name.isNotEmpty() && e.file != juce::File())
                        favorites.rootEntries.push_back(e);
                }
        }

        favorites.folders.clear();
        if (auto* arr = favsObj->getProperty("folders").getArray())
        {
            for (auto& v : *arr)
                if (auto* fo = v.getDynamicObject())
                {
                    FavoritesManager::Folder f;
                    f.name = fo->getProperty("name").toString();
                    if (auto* entries = fo->getProperty("entries").getArray())
                        for (auto& ev : *entries)
                            if (auto* eo = ev.getDynamicObject())
                            {
                                FavoritesManager::Entry e;
                                e.name = eo->getProperty("name").toString();
                                e.file = juce::File(eo->getProperty("path").toString());
                                if (e.name.isNotEmpty() && e.file != juce::File())
                                    f.entries.push_back(e);
                            }
                    favorites.folders.push_back(f);
                }
        }
    }

    refreshTree();
    return themeIdx;
}

void LibraryPanel::startExport()
{
    fileChooser = std::make_unique<juce::FileChooser>(
        "Export Config",
        juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
            .getChildFile("dsk_sfz_config.json"),
        "*.json");

    fileChooser->launchAsync(
        juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::warnAboutOverwriting,
        [this](const juce::FileChooser& fc)
        {
            auto dest = fc.getResult();
            if (dest == juce::File()) return;
            if (!dest.hasFileExtension(".json"))
                dest = dest.withFileExtension(".json");
            dest.replaceWithText(juce::JSON::toString(buildConfigJson(), true));
            juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::InfoIcon,
                "Config Exported",
                "Config saved to:\n" + dest.getFullPathName());
        });
}

void LibraryPanel::startImport()
{
    fileChooser = std::make_unique<juce::FileChooser>(
        "Import Config",
        juce::File::getSpecialLocation(juce::File::userDocumentsDirectory),
        "*.json");

    fileChooser->launchAsync(
        juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
        [this](const juce::FileChooser& fc)
        {
            auto src = fc.getResult();
            if (!src.existsAsFile()) return;
            auto parsed = juce::JSON::parse(src.loadFileAsString());
            if (parsed.getDynamicObject() == nullptr)
            {
                juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,
                    "Import Failed",
                    "Could not parse the config file.");
            }
            else
            {
                int themeIdx = applyConfigJson(parsed);
                if (themeIdx >= 0 && onThemeChanged)
                    onThemeChanged(themeIdx);
                juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::InfoIcon,
                    "Config Imported",
                    "Library and favorites restored from:\n" + src.getFullPathName());
            }
        });
}

void LibraryPanel::saveState(juce::PropertiesFile& props)
{
    props.setValue("libraryFolders", libraryFolderPaths.joinIntoString("|"));
    favorites.save(props);
    if (auto xml = treeView.getOpennessState(true))
        props.setValue("treeOpenState", xml->toString());
}

void LibraryPanel::loadState(juce::PropertiesFile& props)
{
    juce::String saved = props.getValue("libraryFolders", "");
    if (saved.isNotEmpty())
    {
        libraryFolderPaths.clear();
        libraryFolderPaths.addTokens(saved, "|", "");
    }
    favorites.load(props);
    savedTreeOpenState = props.getValue("treeOpenState", "");
    refreshTree();
}
