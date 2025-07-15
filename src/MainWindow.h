#pragma once

#include <QMainWindow>
#include <QLoggingCategory>
#include <memory>

class QMenuBar;
class QToolBar;
class QStatusBar;
class QDockWidget;
class QSplitter;
class QTabWidget;
class QCommandLineEdit;

class RibbonInterface;
class DockablePalettes;
class ViewportManager;
class CADStatusBar;
class ContextMenus;
class NavigationControls;
class ViewCube;

Q_DECLARE_LOGGING_CATEGORY(cadMainWindow)

/**
 * @brief Main application window with comprehensive AutoCAD-style interface
 * 
 * Provides complete UI layout including:
 * - Ribbon interface with all tool tabs
 * - Dockable palettes (Properties, Layers, Tool Palettes, etc.)
 * - Multiple viewports with 2D/3D capabilities
 * - Command line interface
 * - Status bar with mode toggles
 * - Navigation controls and ViewCube
 * - Context menus and dynamic UI elements
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    // UI component access
    RibbonInterface* ribbonInterface() const { return m_ribbonInterface.get(); }
    DockablePalettes* dockablePalettes() const { return m_dockablePalettes.get(); }
    ViewportManager* viewportManager() const { return m_viewportManager.get(); }
    CADStatusBar* cadStatusBar() const { return m_cadStatusBar.get(); }
    NavigationControls* navigationControls() const { return m_navigationControls.get(); }

    // Workspace management
    void saveWorkspace(const QString& name);
    void loadWorkspace(const QString& name);
    void resetWorkspace();

    // UI state management
    void setRibbonVisible(bool visible);
    void setCommandLineVisible(bool visible);
    void setStatusBarVisible(bool visible);
    
    bool isRibbonVisible() const;
    bool isCommandLineVisible() const;
    bool isStatusBarVisible() const;

    // View management
    void maximizeViewport();
    void restoreViewports();
    void tileViewports();
    void cascadeViewports();

protected:
    void closeEvent(QCloseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;

private slots:
    void onNewDocument();
    void onOpenDocument();
    void onSaveDocument();
    void onSaveDocumentAs();
    void onCloseDocument();
    void onExit();
    
    void onUndo();
    void onRedo();
    void onCut();
    void onCopy();
    void onPaste();
    void onSelectAll();
    
    void onZoomExtents();
    void onZoomWindow();
    void onZoomPrevious();
    void onPan();
    void onOrbit();
    
    void onLayerManager();
    void onBlockManager();
    void onXrefManager();
    void onLayoutManager();
    
    void onOptions();
    void onAbout();
    
    void onCommandEntered(const QString& command);
    void onViewportChanged(int index);
    void onWorkspaceChanged(const QString& workspace);

private:
    void setupUI();
    void setupMenuBar();
    void setupRibbonInterface();
    void setupCentralWidget();
    void setupDockWidgets();
    void setupStatusBar();
    void setupToolBars();
    void setupShortcuts();
    void setupConnections();
    
    void createFileMenu();
    void createEditMenu();
    void createViewMenu();
    void createDrawMenu();
    void createModifyMenu();
    void createInsertMenu();
    void createFormatMenu();
    void createToolsMenu();
    void createWindowMenu();
    void createHelpMenu();
    
    void createQuickAccessToolbar();
    void createNavigationToolbar();
    
    void updateWindowTitle();
    void updateRecentFiles();
    void addRecentFile(const QString& filePath);
    
    bool confirmClose();
    void saveWindowState();
    void restoreWindowState();

    // UI Components
    std::unique_ptr<RibbonInterface> m_ribbonInterface;
    std::unique_ptr<DockablePalettes> m_dockablePalettes;
    std::unique_ptr<ViewportManager> m_viewportManager;
    std::unique_ptr<CADStatusBar> m_cadStatusBar;
    std::unique_ptr<ContextMenus> m_contextMenus;
    std::unique_ptr<NavigationControls> m_navigationControls;

    // Central widget and layout
    QWidget* m_centralWidget;
    QSplitter* m_mainSplitter;
    QSplitter* m_viewportSplitter;
    
    // Command line interface
    QDockWidget* m_commandDock;
    QWidget* m_commandWidget;
    QCommandLineEdit* m_commandLine;
    QTextEdit* m_commandHistory;
    
    // Menu and toolbar
    QMenuBar* m_menuBar;
    QToolBar* m_quickAccessToolbar;
    QToolBar* m_navigationToolbar;
    
    // Recent files
    QStringList m_recentFiles;
    QList<QAction*> m_recentFileActions;
    static const int MaxRecentFiles = 10;
    
    // UI state
    bool m_ribbonVisible;
    bool m_commandLineVisible;
    bool m_statusBarVisible;
    bool m_viewportsMaximized;
    
    // Window state
    QByteArray m_normalGeometry;
    QByteArray m_normalState;
};
