#pragma once
#include <JuceHeader.h>

// ══════════════════════════════════════════════════════════════════════════════
// FavoritesManager  — pure data, no UI
// ══════════════════════════════════════════════════════════════════════════════
class FavoritesManager
{
public:
    struct Entry  { juce::String name; juce::File file; };
    struct Folder { juce::String name; std::vector<Entry> entries; };

    std::vector<Entry>  rootEntries; // entries not inside any folder
    std::vector<Folder> folders;

    void save(juce::PropertiesFile& props) const;
    void load(juce::PropertiesFile& props);

    // root-level entries
    bool addRootEntry   (const juce::String& name, const juce::File& file);
    bool removeRootEntry(int idx);

    // folder CRUD
    bool addFolder   (const juce::String& name);
    bool removeFolder(int idx);
    bool renameFolder(int idx, const juce::String& newName);
    bool addEntry    (int folderIdx, const juce::String& name, const juce::File& file);
    bool removeEntry (int folderIdx, int entryIdx);
    bool moveEntry   (int fromFolder, int entryIdx, int toFolder);
    int  findFolder  (const juce::String& name) const;
};

// ══════════════════════════════════════════════════════════════════════════════
// LibraryTreeItem
// ══════════════════════════════════════════════════════════════════════════════
class LibraryTreeItem : public juce::TreeViewItem
{
public:
    enum class Type { RootFolder, SubFolder, SFZFile, SF2File, ZIPFile };

    std::function<void(const juce::File&, bool)>                    onFileActivated;
    std::function<void(const juce::File&, const juce::MouseEvent&)> onSFZRightClicked;
    std::function<void(const juce::File&)>                          onSFZPreview;

    LibraryTreeItem(Type t, const juce::File& f);

    bool         mightContainSubItems()                       override;
    void         itemOpennessChanged(bool isOpen)             override;
    void         paintItem(juce::Graphics& g, int w, int h)  override;
    void         itemDoubleClicked(const juce::MouseEvent& e) override;
    void         itemClicked      (const juce::MouseEvent& e) override;
    juce::String getUniqueName() const                        override;

    Type       getType() const { return type; }
    juce::File getFile() const { return file; }

private:
    Type       type;
    juce::File file;
    void populateChildren();
};

// ══════════════════════════════════════════════════════════════════════════════
// FavoritesFileItem
// folderIdx == -1 means root entry (in FavoritesManager::rootEntries)
// ══════════════════════════════════════════════════════════════════════════════
class FavoritesFileItem : public juce::TreeViewItem
{
public:
    FavoritesManager*                      favorites;
    int                                    folderIdx; // -1 = root
    int                                    entryIdx;
    std::function<void(const juce::File&)> onActivated;
    std::function<void()>                  onChanged;
    std::function<void(const juce::File&)> onSFZPreview;

    FavoritesFileItem(FavoritesManager* fm, int fi, int ei);

    bool         mightContainSubItems()                       override { return false; }
    void         paintItem(juce::Graphics& g, int w, int h)  override;
    void         itemDoubleClicked(const juce::MouseEvent&)   override;
    void         itemClicked      (const juce::MouseEvent& e) override;
    juce::String getUniqueName() const                        override;
};

// ══════════════════════════════════════════════════════════════════════════════
// FavoritesFolderItem
// ══════════════════════════════════════════════════════════════════════════════
class FavoritesFolderItem : public juce::TreeViewItem
{
public:
    FavoritesManager*                      favorites;
    int                                    folderIdx;
    std::function<void(const juce::File&)> onActivated;
    std::function<void()>                  onChanged;
    std::function<void(const juce::File&)> onSFZPreview;

    FavoritesFolderItem(FavoritesManager* fm, int fi);

    bool         mightContainSubItems()                       override { return true; }
    void         itemOpennessChanged(bool isOpen)             override;
    void         paintItem(juce::Graphics& g, int w, int h)  override;
    void         itemClicked      (const juce::MouseEvent& e) override;
    void         itemDoubleClicked(const juce::MouseEvent&)   override;
    juce::String getUniqueName() const                        override;

    void rebuild();

private:
    void populateChildren();
};

// ══════════════════════════════════════════════════════════════════════════════
// FavoritesRootItem
// ══════════════════════════════════════════════════════════════════════════════
class FavoritesRootItem : public juce::TreeViewItem
{
public:
    FavoritesManager*                      favorites;
    std::function<void(const juce::File&)> onActivated;
    std::function<void()>                  onChanged;
    std::function<void(const juce::File&)> onSFZPreview;

    FavoritesRootItem(FavoritesManager* fm);

    bool         mightContainSubItems()                       override { return true; }
    void         paintItem(juce::Graphics& g, int w, int h)  override;
    void         itemClicked      (const juce::MouseEvent& e) override;
    void         itemDoubleClicked(const juce::MouseEvent&)   override;
    juce::String getUniqueName() const override { return "__FAVORITES_ROOT__"; }

    void rebuild();
};

// ══════════════════════════════════════════════════════════════════════════════
// SearchResultsModel
// ══════════════════════════════════════════════════════════════════════════════
class SearchResultsModel : public juce::ListBoxModel
{
public:
    struct Result { juce::String name; juce::String location; juce::File file; };

    std::vector<Result>                  results;
    std::function<void(const juce::File&)> onFileActivated;

    int  getNumRows() override { return (int)results.size(); }
    void paintListBoxItem(int row, juce::Graphics& g, int w, int h, bool selected) override;
    void listBoxItemDoubleClicked(int row, const juce::MouseEvent&) override;
};

// ══════════════════════════════════════════════════════════════════════════════
// Evitamos que el TreeView robe el Drag & Drop
// ══════════════════════════════════════════════════════════════════════════════
class LibraryTreeView : public juce::TreeView
{
public:
    LibraryTreeView() {}
    bool isInterestedInFileDrag(const juce::StringArray&) override { return false; }
};

// ══════════════════════════════════════════════════════════════════════════════
// LibraryPanel
// ══════════════════════════════════════════════════════════════════════════════
class LibraryPanel : public juce::Component,
    public juce::TextEditor::Listener
{
public:
    struct Listener
    {
        virtual ~Listener() = default;
        virtual void libraryFileRequested(const juce::File& sfzFile) = 0;
        virtual void libraryZipRequested (const juce::File& zipFile) = 0;
    };

    LibraryPanel();
    ~LibraryPanel() override;

    void paint    (juce::Graphics& g)          override;
    void resized  ()                            override;
    void mouseDown(const juce::MouseEvent& e)   override;
    void mouseMove(const juce::MouseEvent& e)   override;
    void mouseExit(const juce::MouseEvent& e)   override;
    void lookAndFeelChanged()                   override;

    void addListener   (Listener* l) { listeners.add(l); }
    void removeListener(Listener* l) { listeners.remove(l); }

    void saveState(juce::PropertiesFile& props);
    void loadState(juce::PropertiesFile& props);

    void addFolder(const juce::File& folder);
    void refreshTree();

    // called by editor drag & drop handler
    void addFolderDirect(const juce::File& folder) { addFolder(folder); }

    // JSON export / import (shows its own file chooser)
    void startExport();
    void startImport();

    // Called when an import restores a saved theme index (set by PluginEditor)
    std::function<void(int themeIndex)> onThemeChanged;

    // Called when user clicks [S] badge — editor handles load + preview note
    std::function<void(const juce::File&)> onSFZPreviewRequested;

    void textEditorTextChanged(juce::TextEditor& editor) override;
    void handleFileActivated(const juce::File& file, bool isZip);
    void showAddToFavoritesMenu(const juce::File& sfzFile, const juce::MouseEvent& e);

private:
    juce::ListenerList<Listener> listeners;

    juce::TextEditor searchBox;
    juce::TextButton clearSearchBtn{ "X" };
    juce::TextButton addFolderBtn{ "+" };
    juce::TextButton removeFolderBtn{ "-" };
    juce::TextButton refreshBtn{ "+/-" };
    LibraryTreeView  treeView;

    std::unique_ptr<LibraryTreeItem> invisibleRoot;
    juce::StringArray                libraryFolderPaths;

    FavoritesManager   favorites;
    FavoritesRootItem* favoritesRootItem { nullptr };

    SearchResultsModel searchModel;
    juce::ListBox      searchResultsList;

    void removeSelectedFolder();
    void runSearch(const juce::String& query);
    void rebuildFavoritesSection();

    // JSON helpers (internal, no file chooser)
    juce::var buildConfigJson() const;
    int       applyConfigJson(const juce::var& root); // returns theme index or -1

    juce::Rectangle<int> getBannerBounds() const;

    std::unique_ptr<juce::FileChooser> fileChooser;
    juce::Image bannerImage;
    juce::String bannerClickUrl { "https://www.dskmusic.com/high-quality-instruments/" }; // fallback si no hay banner remoto
    std::shared_ptr<std::atomic<bool>> bannerStillAlive { std::make_shared<std::atomic<bool>>(true) };
    void fetchRemoteBanner();
    juce::String savedTreeOpenState;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LibraryPanel)
};
