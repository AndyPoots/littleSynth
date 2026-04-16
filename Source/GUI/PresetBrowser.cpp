#include "PresetBrowser.h"

// ---------------------------------------------------------------
// Content component — the actual UI inside the browser overlay
// ---------------------------------------------------------------

class PresetBrowser::ContentComponent : public juce::Component,
                                        public juce::ListBoxModel,
                                        public juce::Button::Listener
{
public:
    ContentComponent(PresetManager& pm, PresetBrowser& owner)
        : presetManager_(pm), owner_(owner)
    {
        // Category list
        addAndMakeVisible(categoryBox_);
        categoryBox_.setModel(&categoryModel_);
        categoryBox_.setColour(juce::ListBox::backgroundColourId, juce::Colour(CustomLookAndFeel::kPanelBg));
        categoryBox_.setColour(juce::ListBox::textColourId, juce::Colour(CustomLookAndFeel::kText));
        categoryBox_.setColour(juce::ListBox::outlineColourId, juce::Colour(CustomLookAndFeel::kAccent));
        categoryBox_.selectRow(0);

        // Preset list
        addAndMakeVisible(presetBox_);
        presetBox_.setModel(this);
        presetBox_.setColour(juce::ListBox::backgroundColourId, juce::Colour(CustomLookAndFeel::kPanelBg));
        presetBox_.setColour(juce::ListBox::textColourId, juce::Colour(CustomLookAndFeel::kText));
        presetBox_.setColour(juce::ListBox::outlineColourId, juce::Colour(CustomLookAndFeel::kAccent));

        // Load button
        addAndMakeVisible(loadButton_);
        loadButton_.setButtonText("Load");
        loadButton_.addListener(this);
        loadButton_.setColour(juce::TextButton::buttonColourId, juce::Colour(CustomLookAndFeel::kAccent));
        loadButton_.setColour(juce::TextButton::textColourOffId, juce::Colour(CustomLookAndFeel::kText));

        // Delete button
        addAndMakeVisible(deleteButton_);
        deleteButton_.setButtonText("Delete");
        deleteButton_.addListener(this);
        deleteButton_.setColour(juce::TextButton::buttonColourId, juce::Colour(CustomLookAndFeel::kPanelBg));
        deleteButton_.setColour(juce::TextButton::textColourOffId, juce::Colour(CustomLookAndFeel::kTextDim));

        // Cancel button
        addAndMakeVisible(cancelButton_);
        cancelButton_.setButtonText("Cancel");
        cancelButton_.addListener(this);
        cancelButton_.setColour(juce::TextButton::buttonColourId, juce::Colour(CustomLookAndFeel::kPanelBg));
        cancelButton_.setColour(juce::TextButton::textColourOffId, juce::Colour(CustomLookAndFeel::kTextDim));

        // Title label
        addAndMakeVisible(titleLabel_);
        titleLabel_.setText("Preset Browser", juce::dontSendNotification);
        titleLabel_.setColour(juce::Label::textColourId, juce::Colour(CustomLookAndFeel::kAccent));
        titleLabel_.setFont(juce::FontOptions(16.0f, juce::Font::bold));
        titleLabel_.setJustificationType(juce::Justification::centredLeft);

        refreshCategories();
    }

    void resized() override
    {
        auto bounds = getLocalBounds().reduced(8);

        // Title row
        titleLabel_.setBounds(bounds.removeFromTop(28));
        bounds.removeFromTop(4);

        // Bottom buttons
        auto btnRow = bounds.removeFromBottom(32);
        deleteButton_.setBounds(btnRow.removeFromLeft(70));
        cancelButton_.setBounds(btnRow.removeFromRight(70));
        loadButton_.setBounds(btnRow.removeFromRight(70));
        bounds.removeFromBottom(8);

        // Split: category list (left, ~25% width) + preset list (right)
        auto leftCol = bounds.removeFromLeft(juce::jmax(100, bounds.getWidth() / 4));
        categoryBox_.setBounds(leftCol.reduced(2));
        presetBox_.setBounds(bounds.reduced(2));
    }

    // --- Category ListBoxModel ---
    struct CategoryEntry
    {
        juce::String name;
        int count = 0;
        bool isHeader = false;  // true = section header, false = clickable category
    };

    struct CategoryModel : public juce::ListBoxModel
    {
        juce::Array<CategoryEntry> entries;

        int getNumRows() override { return entries.size(); }

        void paintListBoxItem(int row, juce::Graphics& g, int width, int height,
                              bool rowIsSelected) override
        {
            if (row < 0 || row >= entries.size())
                return;

            const auto& entry = entries.getReference(row);

            if (entry.isHeader)
            {
                // Section header: bold text, dim colour, separator line above
                if (row > 0)
                {
                    g.setColour(juce::Colour(CustomLookAndFeel::kTextDim).withAlpha(0.3f));
                    g.drawHorizontalLine(0, 4.0f, static_cast<float>(width - 4));
                }
                g.setColour(juce::Colour(CustomLookAndFeel::kAccent));
                g.setFont(juce::FontOptions(11.0f, juce::Font::bold));
                g.drawText(entry.name.toUpperCase(), 8, 0, width - 16, height,
                           juce::Justification::centredLeft, true);
            }
            else
            {
                if (rowIsSelected)
                    g.fillAll(juce::Colour(CustomLookAndFeel::kAccent).withAlpha(0.3f));

                // Category name
                g.setColour(juce::Colour(CustomLookAndFeel::kText));
                g.setFont(13.0f);
                g.drawText(entry.name, 10, 0, width - 50, height,
                           juce::Justification::centredLeft, true);

                // Count badge
                g.setColour(juce::Colour(CustomLookAndFeel::kTextDim));
                g.setFont(11.0f);
                g.drawText(juce::String(entry.count), width - 30, 0, 24, height,
                           juce::Justification::centredRight, true);
            }
        }

        void selectedRowsChanged(int lastRowSelected) override
        {
            if (onSelectionChanged)
                onSelectionChanged(lastRowSelected);
        }

        /// Map a row index to the category name it represents (skipping headers).
        juce::String getCategoryForRow(int row) const
        {
            if (row >= 0 && row < entries.size() && !entries.getReference(row).isHeader)
                return entries.getReference(row).name;
            return {};
        }

        /// Find the row index for a given category name (skipping headers).
        int getRowForCategory(const juce::String& category) const
        {
            for (int i = 0; i < entries.size(); ++i)
            {
                if (!entries.getReference(i).isHeader && entries.getReference(i).name == category)
                    return i;
            }
            // Fall back to first non-header row ("All")
            for (int i = 0; i < entries.size(); ++i)
            {
                if (!entries.getReference(i).isHeader)
                    return i;
            }
            return 0;
        }

        std::function<void(int)> onSelectionChanged;
    };

    // --- Preset ListBoxModel ---
    int getNumRows() override
    {
        return static_cast<int>(filteredPresets_.size());
    }

    void paintListBoxItem(int row, juce::Graphics& g, int width, int height,
                          bool rowIsSelected) override
    {
        if (row < 0 || row >= static_cast<int>(filteredPresets_.size()))
            return;

        if (rowIsSelected)
            g.fillAll(juce::Colour(CustomLookAndFeel::kAccent).withAlpha(0.3f));

        auto& preset = filteredPresets_[static_cast<size_t>(row)];

        // Preset name
        g.setColour(juce::Colour(CustomLookAndFeel::kText));
        g.setFont(13.0f);
        g.drawText(preset.name, 8, 0, width - 100, height,
                   juce::Justification::centredLeft, true);

        // Category badge
        g.setColour(juce::Colour(CustomLookAndFeel::kTextDim));
        g.setFont(11.0f);
        g.drawText(preset.category, width - 90, 0, 82, height,
                   juce::Justification::centredRight, true);
    }

    void listBoxItemDoubleClicked(int row, const juce::MouseEvent&) override
    {
        if (row >= 0 && row < static_cast<int>(filteredPresets_.size()))
            loadSelectedPreset(row);
    }

    void selectedRowsChanged(int /*lastRowSelected*/) override
    {
        int row = presetBox_.getSelectedRow();
        if (row >= 0 && row < static_cast<int>(filteredPresets_.size()))
        {
            bool isFactory = filteredPresets_[static_cast<size_t>(row)].isFactory;
            deleteButton_.setEnabled(!isFactory);
        }
    }

    // --- Button::Listener ---
    void buttonClicked(juce::Button* button) override
    {
        if (button == &loadButton_)
        {
            int row = presetBox_.getSelectedRow();
            if (row >= 0)
                loadSelectedPreset(row);
        }
        else if (button == &deleteButton_)
        {
            int row = presetBox_.getSelectedRow();
            if (row >= 0 && row < static_cast<int>(filteredPresets_.size()))
            {
                auto& preset = filteredPresets_[static_cast<size_t>(row)];
                // Find the global index in allPresets
                auto& all = presetManager_.getAllPresets();
                for (int i = 0; i < all.size(); ++i)
                {
                    if (all.getReference(i).name == preset.name &&
                        all.getReference(i).category == preset.category)
                    {
                        presetManager_.deletePreset(i);
                        break;
                    }
                }
                refreshPresets();
            }
        }
        else if (button == &cancelButton_)
        {
            owner_.dismiss();
        }
    }

    void refreshCategories()
    {
        categoryModel_.entries.clear();

        // Build a map of category → count
        juce::StringArray categories = presetManager_.getCategories();
        auto& allPresets = presetManager_.getAllPresets();

        // Count presets per category
        juce::HashMap<juce::String, int> catCounts;
        int totalCount = 0;
        for (const auto& preset : allPresets)
        {
            catCounts.set(preset.category, catCounts[preset.category] + 1);
            ++totalCount;
        }

        // "All" entry
        {
            CategoryEntry e;
            e.name = "All";
            e.count = totalCount;
            e.isHeader = false;
            categoryModel_.entries.add(e);
        }

        // Group categories by type
        static const char* catGroups[] = {
            "Factory", "Pads", "Bass", "Lead", "Keys", "Organ", "FX", "Ambient"
        };
        static const char* groupHeaders[] = {
            "Factory", "Pads", "Bass", "Leads", "Keys", "Organ", "FX", "Ambient"
        };

        for (int g = 0; g < 8; ++g)
        {
            if (!categories.contains(catGroups[g]))
                continue;

            // Section header
            CategoryEntry header;
            header.name = groupHeaders[g];
            header.isHeader = true;
            categoryModel_.entries.add(header);

            // The category itself
            CategoryEntry cat;
            cat.name = catGroups[g];
            cat.count = catCounts[catGroups[g]];
            cat.isHeader = false;
            categoryModel_.entries.add(cat);
        }

        // Add any remaining categories not in the predefined groups
        for (const auto& cat : categories)
        {
            if (cat == "All")
                continue;

            bool alreadyAdded = false;
            for (int g = 0; g < 8; ++g)
            {
                if (cat == catGroups[g])
                {
                    alreadyAdded = true;
                    break;
                }
            }
            if (alreadyAdded)
                continue;

            // Section header for user category
            CategoryEntry header;
            header.name = cat;
            header.isHeader = true;
            categoryModel_.entries.add(header);

            CategoryEntry entry;
            entry.name = cat;
            entry.count = catCounts[cat];
            entry.isHeader = false;
            categoryModel_.entries.add(entry);
        }

        categoryModel_.onSelectionChanged = [this](int row) { categorySelected(row); };
        categoryBox_.updateContent();
        categoryBox_.selectRow(0);
    }

    void refreshPresets()
    {
        juce::String currentCat = categoryModel_.getCategoryForRow(categoryBox_.getSelectedRow());
        refreshCategories();

        // Re-select the previously selected category
        int row = categoryModel_.getRowForCategory(currentCat);
        categoryBox_.selectRow(row);
        filterPresetsByCategory(row);
        presetBox_.updateContent();
        presetBox_.deselectAllRows();
    }

private:
    void categorySelected(int categoryRow)
    {
        // If a header was selected, don't change the filter
        if (categoryRow >= 0 && categoryRow < categoryModel_.entries.size()
            && categoryModel_.entries.getReference(categoryRow).isHeader)
            return;

        filterPresetsByCategory(categoryRow);
        presetBox_.updateContent();
        presetBox_.deselectAllRows();
    }

    void filterPresetsByCategory(int categoryRow)
    {
        filteredPresets_.clear();

        juce::String selectedCategory = categoryModel_.getCategoryForRow(categoryRow);

        auto& all = presetManager_.getAllPresets();
        for (int i = 0; i < all.size(); ++i)
        {
            if (selectedCategory == "All")
            {
                filteredPresets_.push_back(all.getReference(i));
            }
            else if (all.getReference(i).category == selectedCategory)
            {
                filteredPresets_.push_back(all.getReference(i));
            }
        }
    }

    void loadSelectedPreset(int filteredRow)
    {
        if (filteredRow < 0 || filteredRow >= static_cast<int>(filteredPresets_.size()))
            return;

        auto& preset = filteredPresets_[static_cast<size_t>(filteredRow)];

        // Find global index
        auto& all = presetManager_.getAllPresets();
        for (int i = 0; i < all.size(); ++i)
        {
            if (all.getReference(i).name == preset.name &&
                all.getReference(i).category == preset.category)
            {
                presetManager_.selectPreset(i);
                break;
            }
        }

        owner_.dismiss();
    }

    PresetManager& presetManager_;
    PresetBrowser& owner_;

    juce::Label titleLabel_;
    juce::ListBox categoryBox_;
    CategoryModel categoryModel_;
    juce::ListBox presetBox_;
    juce::TextButton loadButton_;
    juce::TextButton deleteButton_;
    juce::TextButton cancelButton_;

    std::vector<PresetInfo> filteredPresets_;
};

// ---------------------------------------------------------------
// PresetBrowser (Component overlay)
// ---------------------------------------------------------------

PresetBrowser::PresetBrowser(PresetManager& presetManager)
    : presetManager_(presetManager)
{
    content_ = new ContentComponent(presetManager, *this);
    addAndMakeVisible(content_);
    content_->setBounds(getLocalBounds());

    setAlwaysOnTop(true);
    setVisible(false);
}

PresetBrowser::~PresetBrowser() = default;

void PresetBrowser::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(CustomLookAndFeel::kBackground));

    // Draw a border around the entire browser
    g.setColour(juce::Colour(CustomLookAndFeel::kAccent));
    g.drawRoundedRectangle(getLocalBounds().toFloat(), 6.0f, 2.0f);
}

void PresetBrowser::resized()
{
    content_->setBounds(getLocalBounds());
}

void PresetBrowser::dismiss()
{
    setVisible(false);
}
