#pragma once

#include <QObject>
#include <QColor>
#include <QLoggingCategory>
#include <memory>

Q_DECLARE_LOGGING_CATEGORY(cadLayers)

/**
 * @brief Layer properties and settings
 */
struct LayerProperties
{
    QString name;
    QColor color;
    QString lineType;
    double lineWeight;
    bool visible;
    bool frozen;
    bool locked;
    bool plottable;
    QString description;
    
    LayerProperties()
        : color(Qt::white)
        , lineType("Continuous")
        , lineWeight(0.25)
        , visible(true)
        , frozen(false)
        , locked(false)
        , plottable(true)
    {}
    
    LayerProperties(const QString& layerName)
        : name(layerName)
        , color(Qt::white)
        , lineType("Continuous")
        , lineWeight(0.25)
        , visible(true)
        , frozen(false)
        , locked(false)
        , plottable(true)
    {}
};

/**
 * @brief Comprehensive layer management system
 * 
 * Provides complete layer functionality including:
 * - Layer creation, deletion, and renaming
 * - Layer properties (color, linetype, lineweight, visibility)
 * - Layer states (frozen, locked, current)
 * - Layer filters and groups
 * - Layer standards and templates
 * - Import/export of layer configurations
 * - Undo/redo support for layer operations
 */
class LayerManager : public QObject
{
    Q_OBJECT

public:
    explicit LayerManager(QObject *parent = nullptr);
    ~LayerManager();

    // Layer creation and management
    bool createLayer(const QString& name, const LayerProperties& properties = LayerProperties());
    bool deleteLayer(const QString& name);
    bool renameLayer(const QString& oldName, const QString& newName);
    bool duplicateLayer(const QString& sourceName, const QString& newName);
    
    // Layer existence and validation
    bool layerExists(const QString& name) const;
    bool isValidLayerName(const QString& name) const;
    QStringList getLayerNames() const;
    int getLayerCount() const;

    // Current layer management
    void setCurrentLayer(const QString& name);
    QString getCurrentLayer() const { return m_currentLayer; }
    LayerProperties getCurrentLayerProperties() const;

    // Layer properties
    bool setLayerProperties(const QString& name, const LayerProperties& properties);
    LayerProperties getLayerProperties(const QString& name) const;
    
    bool setLayerColor(const QString& name, const QColor& color);
    QColor getLayerColor(const QString& name) const;
    
    bool setLayerLineType(const QString& name, const QString& lineType);
    QString getLayerLineType(const QString& name) const;
    
    bool setLayerLineWeight(const QString& name, double weight);
    double getLayerLineWeight(const QString& name) const;
    
    bool setLayerDescription(const QString& name, const QString& description);
    QString getLayerDescription(const QString& name) const;

    // Layer states
    bool setLayerVisible(const QString& name, bool visible);
    bool isLayerVisible(const QString& name) const;
    
    bool setLayerFrozen(const QString& name, bool frozen);
    bool isLayerFrozen(const QString& name) const;
    
    bool setLayerLocked(const QString& name, bool locked);
    bool isLayerLocked(const QString& name) const;
    
    bool setLayerPlottable(const QString& name, bool plottable);
    bool isLayerPlottable(const QString& name) const;

    // Bulk operations
    void setAllLayersVisible(bool visible);
    void setAllLayersFrozen(bool frozen);
    void setAllLayersLocked(bool locked);
    void freezeAllLayersExcept(const QString& layerName);
    void lockAllLayersExcept(const QString& layerName);

    // Layer filters
    void createFilter(const QString& filterName, const QString& criteria);
    void deleteFilter(const QString& filterName);
    QStringList getFilterNames() const;
    QStringList getLayersInFilter(const QString& filterName) const;
    void applyFilter(const QString& filterName);
    void clearFilter();

    // Layer groups
    void createGroup(const QString& groupName, const QStringList& layerNames);
    void deleteGroup(const QString& groupName);
    void addLayerToGroup(const QString& groupName, const QString& layerName);
    void removeLayerFromGroup(const QString& groupName, const QString& layerName);
    QStringList getGroupNames() const;
    QStringList getLayersInGroup(const QString& groupName) const;

    // Layer standards and templates
    void saveLayerStandard(const QString& standardName);
    void loadLayerStandard(const QString& standardName);
    QStringList getLayerStandards() const;
    void createLayerTemplate(const QString& templateName, const QStringList& layerNames);
    void applyLayerTemplate(const QString& templateName);
    QStringList getLayerTemplates() const;

    // Import/Export
    bool exportLayers(const QString& filePath, const QStringList& layerNames = QStringList());
    bool importLayers(const QString& filePath, bool replaceExisting = false);
    bool exportLayerStandard(const QString& filePath, const QString& standardName);
    bool importLayerStandard(const QString& filePath);

    // Utility functions
    void purgeUnusedLayers();
    QStringList getUnusedLayers() const;
    void resetToDefaults();
    void clear();
    
    // Statistics
    int getObjectCountInLayer(const QString& name) const;
    QStringList getLayersWithObjects() const;
    QStringList getEmptyLayers() const;

    // Layer history and undo
    void saveState();
    void restoreState();
    bool canUndo() const;
    bool canRedo() const;

signals:
    void layerCreated(const QString& name);
    void layerDeleted(const QString& name);
    void layerRenamed(const QString& oldName, const QString& newName);
    void layerPropertiesChanged(const QString& name, const LayerProperties& properties);
    void currentLayerChanged(const QString& name);
    void layerVisibilityChanged(const QString& name, bool visible);
    void layerFrozenChanged(const QString& name, bool frozen);
    void layerLockedChanged(const QString& name, bool locked);
    void filterApplied(const QString& filterName);
    void filterCleared();

public slots:
    void onObjectCreated(int objectId, const QString& layerName);
    void onObjectDeleted(int objectId);
    void onObjectLayerChanged(int objectId, const QString& oldLayer, const QString& newLayer);

private:
    struct LayerFilter {
        QString name;
        QString criteria;
        QStringList matchingLayers;
    };
    
    struct LayerGroup {
        QString name;
        QStringList layerNames;
    };
    
    struct LayerState {
        std::map<QString, LayerProperties> layers;
        QString currentLayer;
    };

    void initializeDefaultLayers();
    void validateLayerName(const QString& name) const;
    void updateLayerCounts();
    void applyFilterCriteria(const QString& criteria, QStringList& matchingLayers);
    bool matchesFilter(const QString& layerName, const QString& criteria) const;
    
    // Layer storage
    std::map<QString, LayerProperties> m_layers;
    QString m_currentLayer;
    
    // Layer usage tracking
    std::map<QString, QList<int>> m_layerObjects;
    
    // Filters and groups
    std::vector<LayerFilter> m_filters;
    std::vector<LayerGroup> m_groups;
    QString m_activeFilter;
    
    // Standards and templates
    std::map<QString, std::vector<LayerProperties>> m_layerStandards;
    std::map<QString, QStringList> m_layerTemplates;
    
    // Undo/Redo
    std::vector<LayerState> m_undoStack;
    std::vector<LayerState> m_redoStack;
    int m_maxUndoLevels;
    
    // Default layer properties
    LayerProperties m_defaultProperties;
};
