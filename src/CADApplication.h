#pragma once

#include <QApplication>
#include <QSettings>
#include <QLoggingCategory>
#include <memory>

class CommandManager;
class GeometryEngine;
class LayerManager;
class BlockManager;
class XrefManager;
class LayoutManager;
class MaterialSystem;
class ObjectSnaps;

Q_DECLARE_LOGGING_CATEGORY(cadApp)

/**
 * @brief Main CAD Application class
 * 
 * Manages the overall application state, settings, and core systems.
 * Implements singleton pattern for global access to application services.
 */
class CADApplication : public QApplication
{
    Q_OBJECT

public:
    explicit CADApplication(int &argc, char **argv);
    ~CADApplication();

    // Singleton access
    static CADApplication* instance();

    // Initialization
    bool initialize();
    void shutdown();

    // Core system access
    CommandManager* commandManager() const { return m_commandManager.get(); }
    GeometryEngine* geometryEngine() const { return m_geometryEngine.get(); }
    LayerManager* layerManager() const { return m_layerManager.get(); }
    BlockManager* blockManager() const { return m_blockManager.get(); }
    XrefManager* xrefManager() const { return m_xrefManager.get(); }
    LayoutManager* layoutManager() const { return m_layoutManager.get(); }
    MaterialSystem* materialSystem() const { return m_materialSystem.get(); }
    ObjectSnaps* objectSnaps() const { return m_objectSnaps.get(); }

    // Settings management
    QSettings* settings() const { return m_settings.get(); }
    void saveSettings();
    void loadSettings();
    void resetToDefaults();

    // Application state
    bool isModified() const { return m_isModified; }
    void setModified(bool modified);
    
    QString currentDocument() const { return m_currentDocument; }
    void setCurrentDocument(const QString& path);

    // Plugin system
    bool loadPlugin(const QString& pluginPath);
    void unloadPlugin(const QString& pluginName);
    QStringList loadedPlugins() const;

    // Workspace management
    void saveWorkspace(const QString& name);
    void loadWorkspace(const QString& name);
    QStringList availableWorkspaces() const;

    // Units and precision
    enum class Units {
        Millimeters,
        Centimeters,
        Meters,
        Inches,
        Feet
    };

    Units currentUnits() const { return m_currentUnits; }
    void setCurrentUnits(Units units);
    
    int precision() const { return m_precision; }
    void setPrecision(int precision);

    // Grid and snap settings
    bool isGridVisible() const { return m_gridVisible; }
    void setGridVisible(bool visible);
    
    bool isSnapEnabled() const { return m_snapEnabled; }
    void setSnapEnabled(bool enabled);
    
    double gridSpacing() const { return m_gridSpacing; }
    void setGridSpacing(double spacing);

    // Drawing aids
    bool isOrthoMode() const { return m_orthoMode; }
    void setOrthoMode(bool enabled);
    
    bool isPolarTracking() const { return m_polarTracking; }
    void setPolarTracking(bool enabled);
    
    bool isDynamicInput() const { return m_dynamicInput; }
    void setDynamicInput(bool enabled);

signals:
    void modifiedChanged(bool modified);
    void currentDocumentChanged(const QString& path);
    void unitsChanged(Units units);
    void precisionChanged(int precision);
    void gridVisibilityChanged(bool visible);
    void snapEnabledChanged(bool enabled);
    void gridSpacingChanged(double spacing);
    void orthoModeChanged(bool enabled);
    void polarTrackingChanged(bool enabled);
    void dynamicInputChanged(bool enabled);

public slots:
    void newDocument();
    void openDocument(const QString& path = QString());
    void saveDocument();
    void saveDocumentAs(const QString& path = QString());
    void closeDocument();

private slots:
    void onAboutToQuit();

private:
    void initializeCore();
    void initializeManagers();
    void setupDefaultSettings();
    void connectSignals();

    // Core systems
    std::unique_ptr<CommandManager> m_commandManager;
    std::unique_ptr<GeometryEngine> m_geometryEngine;
    std::unique_ptr<LayerManager> m_layerManager;
    std::unique_ptr<BlockManager> m_blockManager;
    std::unique_ptr<XrefManager> m_xrefManager;
    std::unique_ptr<LayoutManager> m_layoutManager;
    std::unique_ptr<MaterialSystem> m_materialSystem;
    std::unique_ptr<ObjectSnaps> m_objectSnaps;

    // Settings and state
    std::unique_ptr<QSettings> m_settings;
    bool m_isModified;
    QString m_currentDocument;

    // Application preferences
    Units m_currentUnits;
    int m_precision;
    bool m_gridVisible;
    bool m_snapEnabled;
    double m_gridSpacing;
    bool m_orthoMode;
    bool m_polarTracking;
    bool m_dynamicInput;

    // Plugin management
    QStringList m_loadedPlugins;

    static CADApplication* s_instance;
};
