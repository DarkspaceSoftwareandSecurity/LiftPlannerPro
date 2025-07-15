#pragma once

#include <QObject>
#include <QLoggingCategory>
#include <memory>

class QMainWindow;
class QDockWidget;
class QTreeWidget;
class QListWidget;
class QTableWidget;
class QTextEdit;
class QVBoxLayout;
class QHBoxLayout;
class QToolButton;
class QLineEdit;
class QComboBox;
class QSpinBox;
class QCheckBox;
class QGroupBox;
class QTabWidget;

Q_DECLARE_LOGGING_CATEGORY(cadPalettes)

/**
 * @brief Properties palette for object inspection and editing
 */
class PropertiesPalette : public QWidget
{
    Q_OBJECT

public:
    explicit PropertiesPalette(QWidget* parent = nullptr);
    ~PropertiesPalette();

    void setSelectedObjects(const QList<int>& objectIds);
    void clearSelection();
    void refreshProperties();

signals:
    void propertyChanged(const QString& property, const QVariant& value);

private slots:
    void onPropertyValueChanged();
    void onCategorizeToggled(bool categorized);

private:
    void setupUI();
    void populateProperties();
    void addProperty(const QString& category, const QString& name, const QVariant& value, const QString& type = "string");

    QVBoxLayout* m_layout;
    QHBoxLayout* m_headerLayout;
    QToolButton* m_categorizeButton;
    QLineEdit* m_searchEdit;
    QTreeWidget* m_propertyTree;
    
    QList<int> m_selectedObjects;
    bool m_categorized;
};

/**
 * @brief Layer manager palette
 */
class LayerPalette : public QWidget
{
    Q_OBJECT

public:
    explicit LayerPalette(QWidget* parent = nullptr);
    ~LayerPalette();

    void refreshLayers();
    void setCurrentLayer(const QString& layerName);
    QString getCurrentLayer() const;

signals:
    void layerChanged(const QString& layerName);
    void layerCreated(const QString& layerName);
    void layerDeleted(const QString& layerName);
    void layerPropertyChanged(const QString& layerName, const QString& property, const QVariant& value);

private slots:
    void onNewLayer();
    void onDeleteLayer();
    void onLayerDoubleClicked();
    void onLayerContextMenu(const QPoint& position);

private:
    void setupUI();
    void populateLayers();
    void addLayer(const QString& name, bool visible, bool frozen, bool locked, const QColor& color, const QString& lineType);

    QVBoxLayout* m_layout;
    QHBoxLayout* m_toolbarLayout;
    QToolButton* m_newLayerButton;
    QToolButton* m_deleteLayerButton;
    QToolButton* m_setCurrentButton;
    QTreeWidget* m_layerTree;
    
    QString m_currentLayer;
};

/**
 * @brief Tool palettes for frequently used tools and blocks
 */
class ToolPalette : public QWidget
{
    Q_OBJECT

public:
    explicit ToolPalette(QWidget* parent = nullptr);
    ~ToolPalette();

    void addTool(const QString& name, const QIcon& icon, const QString& command, const QString& category = "General");
    void removeTool(const QString& name);
    void addCategory(const QString& category);
    void removeCategory(const QString& category);

signals:
    void toolActivated(const QString& command);

private slots:
    void onToolClicked();
    void onCategoryChanged(int index);

private:
    void setupUI();
    void populateTools();

    QVBoxLayout* m_layout;
    QComboBox* m_categoryCombo;
    QListWidget* m_toolList;
    
    struct Tool {
        QString name;
        QIcon icon;
        QString command;
        QString category;
    };
    
    std::vector<Tool> m_tools;
    QStringList m_categories;
};

/**
 * @brief Design Center for browsing and inserting content
 */
class DesignCenterPalette : public QWidget
{
    Q_OBJECT

public:
    explicit DesignCenterPalette(QWidget* parent = nullptr);
    ~DesignCenterPalette();

    void setRootPath(const QString& path);
    void refreshContent();

signals:
    void contentSelected(const QString& path, const QString& type);
    void contentInserted(const QString& path, const QString& type);

private slots:
    void onTreeItemClicked();
    void onListItemDoubleClicked();
    void onInsertClicked();

private:
    void setupUI();
    void populateTree();
    void populateList(const QString& path);

    QVBoxLayout* m_layout;
    QHBoxLayout* m_splitterLayout;
    QTreeWidget* m_treeWidget;
    QListWidget* m_listWidget;
    QHBoxLayout* m_buttonLayout;
    QToolButton* m_insertButton;
    QToolButton* m_previewButton;
    
    QString m_rootPath;
    QString m_currentPath;
};

/**
 * @brief External References Manager
 */
class XrefPalette : public QWidget
{
    Q_OBJECT

public:
    explicit XrefPalette(QWidget* parent = nullptr);
    ~XrefPalette();

    void refreshXrefs();
    void addXref(const QString& name, const QString& path, const QString& status);

signals:
    void xrefAttached(const QString& path);
    void xrefDetached(const QString& name);
    void xrefReloaded(const QString& name);

private slots:
    void onAttachXref();
    void onDetachXref();
    void onReloadXref();
    void onXrefContextMenu(const QPoint& position);

private:
    void setupUI();
    void populateXrefs();

    QVBoxLayout* m_layout;
    QHBoxLayout* m_toolbarLayout;
    QToolButton* m_attachButton;
    QToolButton* m_detachButton;
    QToolButton* m_reloadButton;
    QTreeWidget* m_xrefTree;
};

/**
 * @brief Sheet Set Manager
 */
class SheetSetPalette : public QWidget
{
    Q_OBJECT

public:
    explicit SheetSetPalette(QWidget* parent = nullptr);
    ~SheetSetPalette();

    void openSheetSet(const QString& path);
    void closeSheetSet();
    void refreshSheets();

signals:
    void sheetOpened(const QString& path);
    void sheetCreated(const QString& name);

private slots:
    void onNewSheet();
    void onOpenSheet();
    void onSheetDoubleClicked();

private:
    void setupUI();
    void populateSheets();

    QVBoxLayout* m_layout;
    QHBoxLayout* m_toolbarLayout;
    QToolButton* m_newSheetButton;
    QToolButton* m_openSheetButton;
    QTreeWidget* m_sheetTree;
    
    QString m_currentSheetSet;
};

/**
 * @brief Manages all dockable palettes
 */
class DockablePalettes : public QObject
{
    Q_OBJECT

public:
    explicit DockablePalettes(QMainWindow* mainWindow);
    ~DockablePalettes();

    // Palette access
    PropertiesPalette* propertiesPalette() const { return m_propertiesPalette.get(); }
    LayerPalette* layerPalette() const { return m_layerPalette.get(); }
    ToolPalette* toolPalette() const { return m_toolPalette.get(); }
    DesignCenterPalette* designCenterPalette() const { return m_designCenterPalette.get(); }
    XrefPalette* xrefPalette() const { return m_xrefPalette.get(); }
    SheetSetPalette* sheetSetPalette() const { return m_sheetSetPalette.get(); }

    // Palette visibility
    void showPropertiesPalette();
    void hidePropertiesPalette();
    void togglePropertiesPalette();
    
    void showLayerManager();
    void hideLayerManager();
    void toggleLayerManager();
    
    void showToolPalettes();
    void hideToolPalettes();
    void toggleToolPalettes();
    
    void showDesignCenter();
    void hideDesignCenter();
    void toggleDesignCenter();
    
    void showXrefManager();
    void hideXrefManager();
    void toggleXrefManager();
    
    void showSheetSetManager();
    void hideSheetSetManager();
    void toggleSheetSetManager();

    // Layout management
    void resetToDefault();
    void saveLayout(const QString& name);
    void loadLayout(const QString& name);

signals:
    void paletteVisibilityChanged(const QString& palette, bool visible);

private:
    void setupPalettes();
    void createPropertiesPalette();
    void createLayerPalette();
    void createToolPalette();
    void createDesignCenterPalette();
    void createXrefPalette();
    void createSheetSetPalette();
    
    void setupDefaultLayout();
    QDockWidget* createDockWidget(const QString& title, QWidget* widget, Qt::DockWidgetArea area);

    QMainWindow* m_mainWindow;
    
    // Dock widgets
    QDockWidget* m_propertiesDock;
    QDockWidget* m_layerDock;
    QDockWidget* m_toolPaletteDock;
    QDockWidget* m_designCenterDock;
    QDockWidget* m_xrefDock;
    QDockWidget* m_sheetSetDock;
    
    // Palette widgets
    std::unique_ptr<PropertiesPalette> m_propertiesPalette;
    std::unique_ptr<LayerPalette> m_layerPalette;
    std::unique_ptr<ToolPalette> m_toolPalette;
    std::unique_ptr<DesignCenterPalette> m_designCenterPalette;
    std::unique_ptr<XrefPalette> m_xrefPalette;
    std::unique_ptr<SheetSetPalette> m_sheetSetPalette;
};
