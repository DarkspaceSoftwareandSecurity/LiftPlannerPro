#pragma once

#include <QToolBar>
#include <QTabWidget>
#include <QLoggingCategory>
#include <memory>

class QTabBar;
class QStackedWidget;
class QHBoxLayout;
class QVBoxLayout;
class QToolButton;
class QButtonGroup;
class QSplitter;

Q_DECLARE_LOGGING_CATEGORY(cadRibbon)

/**
 * @brief Ribbon panel containing related tools
 */
class RibbonPanel : public QWidget
{
    Q_OBJECT

public:
    explicit RibbonPanel(const QString& title, QWidget* parent = nullptr);
    ~RibbonPanel();

    void addLargeButton(const QString& text, const QIcon& icon, const QString& tooltip = QString());
    void addMediumButton(const QString& text, const QIcon& icon, const QString& tooltip = QString());
    void addSmallButton(const QString& text, const QIcon& icon, const QString& tooltip = QString());
    void addSeparator();
    void addDropDownButton(const QString& text, const QIcon& icon, QMenu* menu, const QString& tooltip = QString());
    void addSplitButton(const QString& text, const QIcon& icon, QMenu* menu, const QString& tooltip = QString());
    
    QString title() const { return m_title; }
    void setTitle(const QString& title);

signals:
    void buttonClicked(const QString& command);

private slots:
    void onButtonClicked();

private:
    void setupUI();
    QToolButton* createButton(const QString& text, const QIcon& icon, const QString& tooltip, int size);

    QString m_title;
    QVBoxLayout* m_mainLayout;
    QHBoxLayout* m_contentLayout;
    QLabel* m_titleLabel;
    QWidget* m_contentWidget;
    
    int m_buttonCount;
};

/**
 * @brief Ribbon tab containing multiple panels
 */
class RibbonTab : public QWidget
{
    Q_OBJECT

public:
    explicit RibbonTab(const QString& name, QWidget* parent = nullptr);
    ~RibbonTab();

    RibbonPanel* addPanel(const QString& title);
    void removePanel(const QString& title);
    RibbonPanel* getPanel(const QString& title) const;
    QStringList getPanelTitles() const;
    
    QString name() const { return m_name; }
    void setName(const QString& name) { m_name = name; }

signals:
    void buttonClicked(const QString& command);

private:
    void setupUI();

    QString m_name;
    QHBoxLayout* m_layout;
    QScrollArea* m_scrollArea;
    QWidget* m_scrollWidget;
    QHBoxLayout* m_scrollLayout;
    
    std::vector<std::unique_ptr<RibbonPanel>> m_panels;
};

/**
 * @brief Complete AutoCAD-style ribbon interface
 * 
 * Provides comprehensive ribbon interface with:
 * - Home tab: Draw, Modify, Layers, Properties, Clipboard
 * - Insert tab: Block, Xref, Objects, Text, Tables
 * - Annotate tab: Dimensions, Text, Leaders, Tables, Markup
 * - Parametric tab: Geometric, Dimensional, Manage
 * - View tab: Views, Coordinates, Navigate, Palettes
 * - Manage tab: CAD Standards, Reference, Applications
 * - Output tab: Plot, Publish, Export, Send
 * - Add-ins tab: Custom tools and plugins
 */
class RibbonInterface : public QToolBar
{
    Q_OBJECT

public:
    explicit RibbonInterface(QWidget* parent = nullptr);
    ~RibbonInterface();

    // Tab management
    RibbonTab* addTab(const QString& name);
    void removeTab(const QString& name);
    RibbonTab* getTab(const QString& name) const;
    void setCurrentTab(const QString& name);
    QString getCurrentTab() const;
    QStringList getTabNames() const;

    // Quick access toolbar
    void addQuickAccessButton(const QString& text, const QIcon& icon, const QString& command);
    void removeQuickAccessButton(const QString& command);
    void setQuickAccessVisible(bool visible);

    // Application button
    void setApplicationButton(const QIcon& icon, QMenu* menu);
    void setApplicationButtonVisible(bool visible);

    // Ribbon state
    void setMinimized(bool minimized);
    bool isMinimized() const { return m_minimized; }
    
    void setAutoHide(bool autoHide);
    bool isAutoHide() const { return m_autoHide; }

signals:
    void commandTriggered(const QString& command);
    void tabChanged(const QString& tabName);
    void minimizedChanged(bool minimized);

public slots:
    void showContextualTab(const QString& tabName);
    void hideContextualTab(const QString& tabName);

private slots:
    void onTabChanged(int index);
    void onTabBarDoubleClicked(int index);
    void onQuickAccessButtonClicked();
    void onApplicationButtonClicked();

private:
    void setupUI();
    void createDefaultTabs();
    void createHomeTab();
    void createInsertTab();
    void createAnnotateTab();
    void createParametricTab();
    void createViewTab();
    void createManageTab();
    void createOutputTab();
    void createAddInsTab();
    
    void setupQuickAccessToolbar();
    void setupApplicationButton();
    void updateLayout();
    void applyRibbonStyling();

    // Main layout components
    QWidget* m_mainWidget;
    QVBoxLayout* m_mainLayout;
    
    // Top row (Application button + Quick Access + Tab bar)
    QWidget* m_topWidget;
    QHBoxLayout* m_topLayout;
    QToolButton* m_applicationButton;
    QToolBar* m_quickAccessToolbar;
    QTabBar* m_tabBar;
    QToolButton* m_minimizeButton;
    QToolButton* m_helpButton;
    
    // Content area
    QStackedWidget* m_contentStack;
    
    // Tabs
    std::vector<std::unique_ptr<RibbonTab>> m_tabs;
    std::map<QString, int> m_tabIndexMap;
    
    // State
    bool m_minimized;
    bool m_autoHide;
    QString m_currentTab;
    
    // Contextual tabs
    QStringList m_contextualTabs;
    
    // Styling
    QString m_ribbonStyleSheet;
};

/**
 * @brief Ribbon gallery widget for showing collections of items
 */
class RibbonGallery : public QWidget
{
    Q_OBJECT

public:
    explicit RibbonGallery(QWidget* parent = nullptr);
    ~RibbonGallery();

    void addItem(const QString& text, const QIcon& icon, const QVariant& data = QVariant());
    void removeItem(int index);
    void clear();
    
    void setItemSize(const QSize& size);
    void setColumns(int columns);
    void setPreviewEnabled(bool enabled);

signals:
    void itemClicked(int index, const QVariant& data);
    void itemHovered(int index, const QVariant& data);

private:
    void setupUI();
    void updateLayout();

    QGridLayout* m_layout;
    QSize m_itemSize;
    int m_columns;
    bool m_previewEnabled;
    
    struct GalleryItem {
        QString text;
        QIcon icon;
        QVariant data;
        QToolButton* button;
    };
    
    std::vector<GalleryItem> m_items;
};
