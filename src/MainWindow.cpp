#include "MainWindow.h"
#include "CADApplication.h"
#include "ui/RibbonInterface.h"
#include "ui/DockablePalettes.h"
#include "ui/ViewportManager.h"
#include "ui/StatusBar.h"
#include "ui/ContextMenus.h"
#include "ui/NavigationControls.h"

#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QDockWidget>
#include <QSplitter>
#include <QTabWidget>
#include <QTextEdit>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QAction>
#include <QActionGroup>
#include <QKeySequence>
#include <QCloseEvent>
#include <QResizeEvent>
#include <QShowEvent>
#include <QKeyEvent>
#include <QContextMenuEvent>
#include <QMessageBox>
#include <QFileDialog>
#include <QSettings>
#include <QApplication>

Q_LOGGING_CATEGORY(cadMainWindow, "cad.mainwindow")

class QCommandLineEdit : public QLineEdit
{
public:
    explicit QCommandLineEdit(QWidget* parent = nullptr) : QLineEdit(parent) {}

protected:
    void keyPressEvent(QKeyEvent* event) override
    {
        if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
            emit commandEntered(text());
            clear();
            return;
        }
        QLineEdit::keyPressEvent(event);
    }

signals:
    void commandEntered(const QString& command);
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_centralWidget(nullptr)
    , m_mainSplitter(nullptr)
    , m_viewportSplitter(nullptr)
    , m_commandDock(nullptr)
    , m_commandWidget(nullptr)
    , m_commandLine(nullptr)
    , m_commandHistory(nullptr)
    , m_menuBar(nullptr)
    , m_quickAccessToolbar(nullptr)
    , m_navigationToolbar(nullptr)
    , m_ribbonVisible(true)
    , m_commandLineVisible(true)
    , m_statusBarVisible(true)
    , m_viewportsMaximized(false)
{
    qCDebug(cadMainWindow) << "Creating main window...";
    
    setupUI();
    setupShortcuts();
    setupConnections();
    restoreWindowState();
    updateWindowTitle();
    
    qCDebug(cadMainWindow) << "Main window created successfully";
}

MainWindow::~MainWindow()
{
    qCDebug(cadMainWindow) << "Destroying main window...";
    saveWindowState();
}

void MainWindow::setupUI()
{
    qCDebug(cadMainWindow) << "Setting up UI components...";
    
    // Set window properties
    setWindowTitle("AutoCAD Clone");
    setMinimumSize(1024, 768);
    resize(1400, 900);
    
    // Setup menu bar
    setupMenuBar();
    
    // Setup ribbon interface
    setupRibbonInterface();
    
    // Setup central widget with viewports
    setupCentralWidget();
    
    // Setup dockable widgets
    setupDockWidgets();
    
    // Setup status bar
    setupStatusBar();
    
    // Setup toolbars
    setupToolBars();
    
    qCDebug(cadMainWindow) << "UI setup complete";
}

void MainWindow::setupMenuBar()
{
    qCDebug(cadMainWindow) << "Setting up menu bar...";
    
    m_menuBar = menuBar();
    
    createFileMenu();
    createEditMenu();
    createViewMenu();
    createDrawMenu();
    createModifyMenu();
    createInsertMenu();
    createFormatMenu();
    createToolsMenu();
    createWindowMenu();
    createHelpMenu();
}

void MainWindow::setupRibbonInterface()
{
    qCDebug(cadMainWindow) << "Setting up ribbon interface...";
    
    m_ribbonInterface = std::make_unique<RibbonInterface>(this);
    
    // Add ribbon as a toolbar (will be positioned at top)
    addToolBar(Qt::TopToolBarArea, m_ribbonInterface.get());
    
    // Make ribbon non-movable and set as main toolbar
    m_ribbonInterface->setMovable(false);
    m_ribbonInterface->setFloatable(false);
}

void MainWindow::setupCentralWidget()
{
    qCDebug(cadMainWindow) << "Setting up central widget...";
    
    // Create central widget
    m_centralWidget = new QWidget(this);
    setCentralWidget(m_centralWidget);
    
    // Create main splitter (horizontal)
    m_mainSplitter = new QSplitter(Qt::Horizontal, m_centralWidget);
    
    // Create viewport manager
    m_viewportManager = std::make_unique<ViewportManager>(m_mainSplitter);
    m_mainSplitter->addWidget(m_viewportManager.get());
    
    // Create navigation controls
    m_navigationControls = std::make_unique<NavigationControls>(m_mainSplitter);
    m_mainSplitter->addWidget(m_navigationControls.get());
    
    // Set splitter proportions (viewport takes most space)
    m_mainSplitter->setStretchFactor(0, 1);
    m_mainSplitter->setStretchFactor(1, 0);
    m_mainSplitter->setSizes({1000, 200});
    
    // Layout central widget
    QHBoxLayout* centralLayout = new QHBoxLayout(m_centralWidget);
    centralLayout->setContentsMargins(0, 0, 0, 0);
    centralLayout->addWidget(m_mainSplitter);
}

void MainWindow::setupDockWidgets()
{
    qCDebug(cadMainWindow) << "Setting up dock widgets...";
    
    // Create dockable palettes
    m_dockablePalettes = std::make_unique<DockablePalettes>(this);
    
    // Setup command line dock
    m_commandDock = new QDockWidget("Command Line", this);
    m_commandDock->setObjectName("CommandLineDock");
    m_commandDock->setAllowedAreas(Qt::BottomDockWidgetArea | Qt::TopDockWidgetArea);
    
    m_commandWidget = new QWidget();
    QVBoxLayout* commandLayout = new QVBoxLayout(m_commandWidget);
    commandLayout->setContentsMargins(5, 5, 5, 5);
    
    // Command history
    m_commandHistory = new QTextEdit();
    m_commandHistory->setMaximumHeight(100);
    m_commandHistory->setReadOnly(true);
    m_commandHistory->setStyleSheet("background-color: #1e1e1e; color: #ffffff; font-family: 'Consolas', monospace;");
    commandLayout->addWidget(m_commandHistory);
    
    // Command input
    QHBoxLayout* inputLayout = new QHBoxLayout();
    QLabel* promptLabel = new QLabel("Command:");
    promptLabel->setStyleSheet("color: #ffffff; font-weight: bold;");
    m_commandLine = new QCommandLineEdit();
    m_commandLine->setStyleSheet("background-color: #2d2d2d; color: #ffffff; border: 1px solid #555; padding: 5px; font-family: 'Consolas', monospace;");
    m_commandLine->setPlaceholderText("Enter command...");
    
    inputLayout->addWidget(promptLabel);
    inputLayout->addWidget(m_commandLine);
    commandLayout->addLayout(inputLayout);
    
    m_commandDock->setWidget(m_commandWidget);
    addDockWidget(Qt::BottomDockWidgetArea, m_commandDock);
    
    // Set command line to have larger size
    m_commandDock->setMinimumHeight(150);
    resizeDocks({m_commandDock}, {150}, Qt::Vertical);
}

void MainWindow::setupStatusBar()
{
    qCDebug(cadMainWindow) << "Setting up status bar...";
    
    m_cadStatusBar = std::make_unique<CADStatusBar>(this);
    setStatusBar(m_cadStatusBar.get());
}

void MainWindow::setupToolBars()
{
    qCDebug(cadMainWindow) << "Setting up toolbars...";
    
    createQuickAccessToolbar();
    createNavigationToolbar();
}

void MainWindow::createFileMenu()
{
    QMenu* fileMenu = m_menuBar->addMenu("&File");
    
    // New
    QAction* newAction = fileMenu->addAction("&New");
    newAction->setShortcut(QKeySequence::New);
    newAction->setIcon(QIcon(":/icons/new.png"));
    connect(newAction, &QAction::triggered, this, &MainWindow::onNewDocument);
    
    // Open
    QAction* openAction = fileMenu->addAction("&Open...");
    openAction->setShortcut(QKeySequence::Open);
    openAction->setIcon(QIcon(":/icons/open.png"));
    connect(openAction, &QAction::triggered, this, &MainWindow::onOpenDocument);
    
    fileMenu->addSeparator();
    
    // Save
    QAction* saveAction = fileMenu->addAction("&Save");
    saveAction->setShortcut(QKeySequence::Save);
    saveAction->setIcon(QIcon(":/icons/save.png"));
    connect(saveAction, &QAction::triggered, this, &MainWindow::onSaveDocument);
    
    // Save As
    QAction* saveAsAction = fileMenu->addAction("Save &As...");
    saveAsAction->setShortcut(QKeySequence::SaveAs);
    saveAsAction->setIcon(QIcon(":/icons/save_as.png"));
    connect(saveAsAction, &QAction::triggered, this, &MainWindow::onSaveDocumentAs);
    
    fileMenu->addSeparator();
    
    // Recent Files
    QMenu* recentMenu = fileMenu->addMenu("Recent Files");
    for (int i = 0; i < MaxRecentFiles; ++i) {
        QAction* action = new QAction(this);
        action->setVisible(false);
        connect(action, &QAction::triggered, [this, action]() {
            CADApplication::instance()->openDocument(action->data().toString());
        });
        m_recentFileActions.append(action);
        recentMenu->addAction(action);
    }
    updateRecentFiles();
    
    fileMenu->addSeparator();
    
    // Close
    QAction* closeAction = fileMenu->addAction("&Close");
    closeAction->setShortcut(QKeySequence::Close);
    connect(closeAction, &QAction::triggered, this, &MainWindow::onCloseDocument);
    
    // Exit
    QAction* exitAction = fileMenu->addAction("E&xit");
    exitAction->setShortcut(QKeySequence::Quit);
    connect(exitAction, &QAction::triggered, this, &MainWindow::onExit);
}

void MainWindow::createEditMenu()
{
    QMenu* editMenu = m_menuBar->addMenu("&Edit");

    // Undo
    QAction* undoAction = editMenu->addAction("&Undo");
    undoAction->setShortcut(QKeySequence::Undo);
    undoAction->setIcon(QIcon(":/icons/undo.png"));
    connect(undoAction, &QAction::triggered, this, &MainWindow::onUndo);

    // Redo
    QAction* redoAction = editMenu->addAction("&Redo");
    redoAction->setShortcut(QKeySequence::Redo);
    redoAction->setIcon(QIcon(":/icons/redo.png"));
    connect(redoAction, &QAction::triggered, this, &MainWindow::onRedo);

    editMenu->addSeparator();

    // Cut
    QAction* cutAction = editMenu->addAction("Cu&t");
    cutAction->setShortcut(QKeySequence::Cut);
    cutAction->setIcon(QIcon(":/icons/cut.png"));
    connect(cutAction, &QAction::triggered, this, &MainWindow::onCut);

    // Copy
    QAction* copyAction = editMenu->addAction("&Copy");
    copyAction->setShortcut(QKeySequence::Copy);
    copyAction->setIcon(QIcon(":/icons/copy.png"));
    connect(copyAction, &QAction::triggered, this, &MainWindow::onCopy);

    // Paste
    QAction* pasteAction = editMenu->addAction("&Paste");
    pasteAction->setShortcut(QKeySequence::Paste);
    pasteAction->setIcon(QIcon(":/icons/paste.png"));
    connect(pasteAction, &QAction::triggered, this, &MainWindow::onPaste);

    editMenu->addSeparator();

    // Select All
    QAction* selectAllAction = editMenu->addAction("Select &All");
    selectAllAction->setShortcut(QKeySequence::SelectAll);
    connect(selectAllAction, &QAction::triggered, this, &MainWindow::onSelectAll);
}

void MainWindow::createViewMenu()
{
    QMenu* viewMenu = m_menuBar->addMenu("&View");

    // Zoom submenu
    QMenu* zoomMenu = viewMenu->addMenu("&Zoom");

    QAction* zoomExtentsAction = zoomMenu->addAction("Zoom &Extents");
    zoomExtentsAction->setShortcut(QKeySequence("Ctrl+E"));
    connect(zoomExtentsAction, &QAction::triggered, this, &MainWindow::onZoomExtents);

    QAction* zoomWindowAction = zoomMenu->addAction("Zoom &Window");
    zoomWindowAction->setShortcut(QKeySequence("Ctrl+W"));
    connect(zoomWindowAction, &QAction::triggered, this, &MainWindow::onZoomWindow);

    QAction* zoomPreviousAction = zoomMenu->addAction("Zoom &Previous");
    zoomPreviousAction->setShortcut(QKeySequence("Ctrl+P"));
    connect(zoomPreviousAction, &QAction::triggered, this, &MainWindow::onZoomPrevious);

    // Pan and Orbit
    QAction* panAction = viewMenu->addAction("&Pan");
    panAction->setShortcut(QKeySequence("Ctrl+Shift+P"));
    connect(panAction, &QAction::triggered, this, &MainWindow::onPan);

    QAction* orbitAction = viewMenu->addAction("&Orbit");
    orbitAction->setShortcut(QKeySequence("Ctrl+Shift+O"));
    connect(orbitAction, &QAction::triggered, this, &MainWindow::onOrbit);

    viewMenu->addSeparator();

    // UI toggles
    QAction* ribbonAction = viewMenu->addAction("&Ribbon");
    ribbonAction->setCheckable(true);
    ribbonAction->setChecked(m_ribbonVisible);
    connect(ribbonAction, &QAction::toggled, this, &MainWindow::setRibbonVisible);

    QAction* commandLineAction = viewMenu->addAction("&Command Line");
    commandLineAction->setCheckable(true);
    commandLineAction->setChecked(m_commandLineVisible);
    connect(commandLineAction, &QAction::toggled, this, &MainWindow::setCommandLineVisible);

    QAction* statusBarAction = viewMenu->addAction("&Status Bar");
    statusBarAction->setCheckable(true);
    statusBarAction->setChecked(m_statusBarVisible);
    connect(statusBarAction, &QAction::toggled, this, &MainWindow::setStatusBarVisible);
}

void MainWindow::createDrawMenu()
{
    QMenu* drawMenu = m_menuBar->addMenu("&Draw");

    // Line tools
    QMenu* lineMenu = drawMenu->addMenu("&Line");
    lineMenu->addAction("&Line");
    lineMenu->addAction("&Polyline");
    lineMenu->addAction("&Spline");

    // Circle tools
    QMenu* circleMenu = drawMenu->addMenu("&Circle");
    circleMenu->addAction("Center, &Radius");
    circleMenu->addAction("Center, &Diameter");
    circleMenu->addAction("&2 Point");
    circleMenu->addAction("&3 Point");

    // Arc tools
    QMenu* arcMenu = drawMenu->addMenu("&Arc");
    arcMenu->addAction("&3 Point");
    arcMenu->addAction("&Start, Center, End");
    arcMenu->addAction("Start, Center, &Angle");

    // Shape tools
    QMenu* shapeMenu = drawMenu->addMenu("&Shapes");
    shapeMenu->addAction("&Rectangle");
    shapeMenu->addAction("&Polygon");
    shapeMenu->addAction("&Ellipse");

    drawMenu->addSeparator();

    // Text and annotation
    QMenu* textMenu = drawMenu->addMenu("&Text");
    textMenu->addAction("&Single Line Text");
    textMenu->addAction("&Multiline Text");

    QMenu* dimensionMenu = drawMenu->addMenu("&Dimensions");
    dimensionMenu->addAction("&Linear");
    dimensionMenu->addAction("&Aligned");
    dimensionMenu->addAction("&Angular");
    dimensionMenu->addAction("&Radius");
    dimensionMenu->addAction("&Diameter");

    // Hatch and fill
    drawMenu->addAction("&Hatch...");
    drawMenu->addAction("&Gradient...");
}

void MainWindow::createModifyMenu()
{
    QMenu* modifyMenu = m_menuBar->addMenu("&Modify");

    // Transform tools
    QMenu* transformMenu = modifyMenu->addMenu("&Transform");
    transformMenu->addAction("&Move");
    transformMenu->addAction("&Copy");
    transformMenu->addAction("&Rotate");
    transformMenu->addAction("&Scale");
    transformMenu->addAction("&Mirror");

    // Editing tools
    QMenu* editMenu = modifyMenu->addMenu("&Edit");
    editMenu->addAction("&Trim");
    editMenu->addAction("&Extend");
    editMenu->addAction("&Fillet");
    editMenu->addAction("&Chamfer");
    editMenu->addAction("&Offset");

    // Array tools
    QMenu* arrayMenu = modifyMenu->addMenu("&Array");
    arrayMenu->addAction("&Rectangular Array");
    arrayMenu->addAction("&Polar Array");
    arrayMenu->addAction("&Path Array");

    modifyMenu->addSeparator();

    // Other modification tools
    modifyMenu->addAction("&Break");
    modifyMenu->addAction("&Join");
    modifyMenu->addAction("&Explode");
    modifyMenu->addAction("&Stretch");
    modifyMenu->addAction("&Align");
    modifyMenu->addAction("Match &Properties");
}

void MainWindow::createInsertMenu()
{
    QMenu* insertMenu = m_menuBar->addMenu("&Insert");

    insertMenu->addAction("&Block...");
    insertMenu->addAction("&External Reference...");
    insertMenu->addAction("&Image...");
    insertMenu->addAction("&Table...");

    insertMenu->addSeparator();

    // 3D objects
    QMenu* objectsMenu = insertMenu->addMenu("3D &Objects");
    objectsMenu->addAction("&Box");
    objectsMenu->addAction("&Sphere");
    objectsMenu->addAction("&Cylinder");
    objectsMenu->addAction("&Cone");
    objectsMenu->addAction("&Wedge");
    objectsMenu->addAction("&Torus");
}

void MainWindow::createFormatMenu()
{
    QMenu* formatMenu = m_menuBar->addMenu("&Format");

    formatMenu->addAction("&Layer Manager...");
    formatMenu->addAction("&Text Style...");
    formatMenu->addAction("&Dimension Style...");
    formatMenu->addAction("&Table Style...");
    formatMenu->addAction("&Multileader Style...");

    formatMenu->addSeparator();

    formatMenu->addAction("&Units...");
    formatMenu->addAction("&Drawing Limits...");
    formatMenu->addAction("&Lineweight...");
    formatMenu->addAction("&Color...");
}

void MainWindow::createToolsMenu()
{
    QMenu* toolsMenu = m_menuBar->addMenu("&Tools");

    // Inquiry tools
    QMenu* inquiryMenu = toolsMenu->addMenu("&Inquiry");
    inquiryMenu->addAction("&Distance");
    inquiryMenu->addAction("&Radius");
    inquiryMenu->addAction("&Angle");
    inquiryMenu->addAction("&Area");
    inquiryMenu->addAction("&Volume");
    inquiryMenu->addAction("&List");
    inquiryMenu->addAction("&Properties");

    // Utilities
    QMenu* utilitiesMenu = toolsMenu->addMenu("&Utilities");
    utilitiesMenu->addAction("&Purge...");
    utilitiesMenu->addAction("&Audit");
    utilitiesMenu->addAction("&Recover...");
    utilitiesMenu->addAction("&QuickCalc");

    toolsMenu->addSeparator();

    // Customization
    toolsMenu->addAction("&Customize Interface...");
    toolsMenu->addAction("&Load Application...");
    toolsMenu->addAction("&Options...");
}

void MainWindow::createWindowMenu()
{
    QMenu* windowMenu = m_menuBar->addMenu("&Window");

    windowMenu->addAction("&Cascade");
    windowMenu->addAction("&Tile Horizontally");
    windowMenu->addAction("&Tile Vertically");
    windowMenu->addAction("&Arrange Icons");

    windowMenu->addSeparator();

    windowMenu->addAction("&Close All");

    // Workspace management
    QMenu* workspaceMenu = windowMenu->addMenu("&Workspaces");
    workspaceMenu->addAction("&Save Current As...");
    workspaceMenu->addAction("&Workspace Settings...");
    workspaceMenu->addSeparator();
    workspaceMenu->addAction("&2D Drafting && Annotation");
    workspaceMenu->addAction("&3D Modeling");
    workspaceMenu->addAction("&3D Basics");
}

void MainWindow::createHelpMenu()
{
    QMenu* helpMenu = m_menuBar->addMenu("&Help");

    helpMenu->addAction("&Help Topics");
    helpMenu->addAction("&What's New");
    helpMenu->addAction("&Learning Resources");

    helpMenu->addSeparator();

    QAction* aboutAction = helpMenu->addAction("&About...");
    connect(aboutAction, &QAction::triggered, this, &MainWindow::onAbout);

    helpMenu->addAction("About &Qt");
}

void MainWindow::createQuickAccessToolbar()
{
    m_quickAccessToolbar = addToolBar("Quick Access");
    m_quickAccessToolbar->setObjectName("QuickAccessToolbar");
    m_quickAccessToolbar->setMovable(false);
    m_quickAccessToolbar->setFloatable(false);
    m_quickAccessToolbar->setToolButtonStyle(Qt::ToolButtonIconOnly);

    // Add commonly used actions
    m_quickAccessToolbar->addAction(QIcon(":/icons/new.png"), "New", this, &MainWindow::onNewDocument);
    m_quickAccessToolbar->addAction(QIcon(":/icons/open.png"), "Open", this, &MainWindow::onOpenDocument);
    m_quickAccessToolbar->addAction(QIcon(":/icons/save.png"), "Save", this, &MainWindow::onSaveDocument);
    m_quickAccessToolbar->addSeparator();
    m_quickAccessToolbar->addAction(QIcon(":/icons/undo.png"), "Undo", this, &MainWindow::onUndo);
    m_quickAccessToolbar->addAction(QIcon(":/icons/redo.png"), "Redo", this, &MainWindow::onRedo);
}

void MainWindow::createNavigationToolbar()
{
    m_navigationToolbar = addToolBar("Navigation");
    m_navigationToolbar->setObjectName("NavigationToolbar");
    m_navigationToolbar->setMovable(true);
    m_navigationToolbar->setFloatable(true);
    m_navigationToolbar->setToolButtonStyle(Qt::ToolButtonIconOnly);

    // Add navigation actions
    m_navigationToolbar->addAction(QIcon(":/icons/zoom_extents.png"), "Zoom Extents", this, &MainWindow::onZoomExtents);
    m_navigationToolbar->addAction(QIcon(":/icons/zoom_window.png"), "Zoom Window", this, &MainWindow::onZoomWindow);
    m_navigationToolbar->addAction(QIcon(":/icons/zoom_previous.png"), "Zoom Previous", this, &MainWindow::onZoomPrevious);
    m_navigationToolbar->addSeparator();
    m_navigationToolbar->addAction(QIcon(":/icons/pan.png"), "Pan", this, &MainWindow::onPan);
    m_navigationToolbar->addAction(QIcon(":/icons/orbit.png"), "Orbit", this, &MainWindow::onOrbit);
}

void MainWindow::setupShortcuts()
{
    // Additional shortcuts not covered by menu actions
    QAction* escapeAction = new QAction(this);
    escapeAction->setShortcut(QKeySequence(Qt::Key_Escape));
    connect(escapeAction, &QAction::triggered, [this]() {
        // Cancel current command
        m_commandLine->clear();
        qCDebug(cadMainWindow) << "Command cancelled";
    });
    addAction(escapeAction);

    // Function keys
    QAction* f1Action = new QAction(this);
    f1Action->setShortcut(QKeySequence(Qt::Key_F1));
    connect(f1Action, &QAction::triggered, [this]() {
        // Toggle help
        qCDebug(cadMainWindow) << "F1 - Help";
    });
    addAction(f1Action);

    QAction* f2Action = new QAction(this);
    f2Action->setShortcut(QKeySequence(Qt::Key_F2));
    connect(f2Action, &QAction::triggered, [this]() {
        // Toggle text window
        setCommandLineVisible(!isCommandLineVisible());
    });
    addAction(f2Action);

    QAction* f3Action = new QAction(this);
    f3Action->setShortcut(QKeySequence(Qt::Key_F3));
    connect(f3Action, &QAction::triggered, [this]() {
        // Toggle object snap
        CADApplication::instance()->setSnapEnabled(!CADApplication::instance()->isSnapEnabled());
    });
    addAction(f3Action);

    QAction* f8Action = new QAction(this);
    f8Action->setShortcut(QKeySequence(Qt::Key_F8));
    connect(f8Action, &QAction::triggered, [this]() {
        // Toggle ortho mode
        CADApplication::instance()->setOrthoMode(!CADApplication::instance()->isOrthoMode());
    });
    addAction(f8Action);

    QAction* f9Action = new QAction(this);
    f9Action->setShortcut(QKeySequence(Qt::Key_F9));
    connect(f9Action, &QAction::triggered, [this]() {
        // Toggle grid
        CADApplication::instance()->setGridVisible(!CADApplication::instance()->isGridVisible());
    });
    addAction(f9Action);

    QAction* f10Action = new QAction(this);
    f10Action->setShortcut(QKeySequence(Qt::Key_F10));
    connect(f10Action, &QAction::triggered, [this]() {
        // Toggle polar tracking
        CADApplication::instance()->setPolarTracking(!CADApplication::instance()->isPolarTracking());
    });
    addAction(f10Action);

    QAction* f11Action = new QAction(this);
    f11Action->setShortcut(QKeySequence(Qt::Key_F11));
    connect(f11Action, &QAction::triggered, [this]() {
        // Toggle object snap tracking
        qCDebug(cadMainWindow) << "F11 - Object snap tracking";
    });
    addAction(f11Action);

    QAction* f12Action = new QAction(this);
    f12Action->setShortcut(QKeySequence(Qt::Key_F12));
    connect(f12Action, &QAction::triggered, [this]() {
        // Toggle dynamic input
        CADApplication::instance()->setDynamicInput(!CADApplication::instance()->isDynamicInput());
    });
    addAction(f12Action);
}

void MainWindow::setupConnections()
{
    // Connect command line
    connect(static_cast<QCommandLineEdit*>(m_commandLine), &QCommandLineEdit::commandEntered,
            this, &MainWindow::onCommandEntered);

    // Connect application signals
    CADApplication* app = CADApplication::instance();
    connect(app, &CADApplication::modifiedChanged, this, &MainWindow::updateWindowTitle);
    connect(app, &CADApplication::currentDocumentChanged, this, &MainWindow::updateWindowTitle);

    // Connect viewport manager
    if (m_viewportManager) {
        connect(m_viewportManager.get(), &ViewportManager::viewportChanged,
                this, &MainWindow::onViewportChanged);
    }
}

// Slot implementations
void MainWindow::onNewDocument()
{
    qCDebug(cadMainWindow) << "New document requested";
    CADApplication::instance()->newDocument();
}

void MainWindow::onOpenDocument()
{
    qCDebug(cadMainWindow) << "Open document requested";
    CADApplication::instance()->openDocument();
}

void MainWindow::onSaveDocument()
{
    qCDebug(cadMainWindow) << "Save document requested";
    CADApplication::instance()->saveDocument();
}

void MainWindow::onSaveDocumentAs()
{
    qCDebug(cadMainWindow) << "Save document as requested";
    CADApplication::instance()->saveDocumentAs();
}

void MainWindow::onCloseDocument()
{
    qCDebug(cadMainWindow) << "Close document requested";
    CADApplication::instance()->closeDocument();
}

void MainWindow::onExit()
{
    qCDebug(cadMainWindow) << "Exit requested";
    close();
}

void MainWindow::onUndo()
{
    qCDebug(cadMainWindow) << "Undo requested";
    if (CADApplication::instance()->commandManager()) {
        CADApplication::instance()->commandManager()->undo();
    }
}

void MainWindow::onRedo()
{
    qCDebug(cadMainWindow) << "Redo requested";
    if (CADApplication::instance()->commandManager()) {
        CADApplication::instance()->commandManager()->redo();
    }
}

void MainWindow::onCut()
{
    qCDebug(cadMainWindow) << "Cut requested";
    // TODO: Implement cut functionality
}

void MainWindow::onCopy()
{
    qCDebug(cadMainWindow) << "Copy requested";
    // TODO: Implement copy functionality
}

void MainWindow::onPaste()
{
    qCDebug(cadMainWindow) << "Paste requested";
    // TODO: Implement paste functionality
}

void MainWindow::onSelectAll()
{
    qCDebug(cadMainWindow) << "Select all requested";
    // TODO: Implement select all functionality
}

void MainWindow::onZoomExtents()
{
    qCDebug(cadMainWindow) << "Zoom extents requested";
    if (m_viewportManager) {
        m_viewportManager->zoomExtents();
    }
}

void MainWindow::onZoomWindow()
{
    qCDebug(cadMainWindow) << "Zoom window requested";
    if (m_viewportManager) {
        m_viewportManager->zoomWindow();
    }
}

void MainWindow::onZoomPrevious()
{
    qCDebug(cadMainWindow) << "Zoom previous requested";
    if (m_viewportManager) {
        m_viewportManager->zoomPrevious();
    }
}

void MainWindow::onPan()
{
    qCDebug(cadMainWindow) << "Pan requested";
    if (m_viewportManager) {
        m_viewportManager->setPanMode(true);
    }
}

void MainWindow::onOrbit()
{
    qCDebug(cadMainWindow) << "Orbit requested";
    if (m_viewportManager) {
        m_viewportManager->setOrbitMode(true);
    }
}

void MainWindow::onLayerManager()
{
    qCDebug(cadMainWindow) << "Layer manager requested";
    if (m_dockablePalettes) {
        m_dockablePalettes->showLayerManager();
    }
}

void MainWindow::onBlockManager()
{
    qCDebug(cadMainWindow) << "Block manager requested";
    if (m_dockablePalettes) {
        m_dockablePalettes->showBlockManager();
    }
}

void MainWindow::onXrefManager()
{
    qCDebug(cadMainWindow) << "Xref manager requested";
    if (m_dockablePalettes) {
        m_dockablePalettes->showXrefManager();
    }
}

void MainWindow::onLayoutManager()
{
    qCDebug(cadMainWindow) << "Layout manager requested";
    if (m_dockablePalettes) {
        m_dockablePalettes->showLayoutManager();
    }
}

void MainWindow::onOptions()
{
    qCDebug(cadMainWindow) << "Options requested";
    // TODO: Implement options dialog
}

void MainWindow::onAbout()
{
    QMessageBox::about(this, "About AutoCAD Clone",
                      "AutoCAD Clone v1.0.0\n\n"
                      "A comprehensive CAD application built with Qt 6 and OpenCASCADE.\n\n"
                      "© 2024 Darkspace Software and Security");
}

void MainWindow::onCommandEntered(const QString& command)
{
    qCDebug(cadMainWindow) << "Command entered:" << command;

    // Add to command history
    m_commandHistory->append(QString("> %1").arg(command));
    m_commandHistory->moveCursor(QTextCursor::End);

    // Process command through command manager
    if (CADApplication::instance()->commandManager()) {
        CADApplication::instance()->commandManager()->executeCommand(command);
    }
}

void MainWindow::onViewportChanged(int index)
{
    qCDebug(cadMainWindow) << "Viewport changed to:" << index;
    // Update UI based on active viewport
}

void MainWindow::onWorkspaceChanged(const QString& workspace)
{
    qCDebug(cadMainWindow) << "Workspace changed to:" << workspace;
    loadWorkspace(workspace);
}

// UI state management
void MainWindow::setRibbonVisible(bool visible)
{
    if (m_ribbonVisible != visible) {
        m_ribbonVisible = visible;
        if (m_ribbonInterface) {
            m_ribbonInterface->setVisible(visible);
        }
        qCDebug(cadMainWindow) << "Ribbon visibility changed to:" << visible;
    }
}

void MainWindow::setCommandLineVisible(bool visible)
{
    if (m_commandLineVisible != visible) {
        m_commandLineVisible = visible;
        if (m_commandDock) {
            m_commandDock->setVisible(visible);
        }
        qCDebug(cadMainWindow) << "Command line visibility changed to:" << visible;
    }
}

void MainWindow::setStatusBarVisible(bool visible)
{
    if (m_statusBarVisible != visible) {
        m_statusBarVisible = visible;
        if (m_cadStatusBar) {
            m_cadStatusBar->setVisible(visible);
        }
        qCDebug(cadMainWindow) << "Status bar visibility changed to:" << visible;
    }
}

bool MainWindow::isRibbonVisible() const
{
    return m_ribbonVisible;
}

bool MainWindow::isCommandLineVisible() const
{
    return m_commandLineVisible;
}

bool MainWindow::isStatusBarVisible() const
{
    return m_statusBarVisible;
}

// Workspace management
void MainWindow::saveWorkspace(const QString& name)
{
    qCDebug(cadMainWindow) << "Saving workspace:" << name;

    QSettings settings;
    settings.beginGroup("Workspaces");
    settings.beginGroup(name);

    settings.setValue("geometry", saveGeometry());
    settings.setValue("state", saveState());
    settings.setValue("ribbonVisible", m_ribbonVisible);
    settings.setValue("commandLineVisible", m_commandLineVisible);
    settings.setValue("statusBarVisible", m_statusBarVisible);

    settings.endGroup();
    settings.endGroup();
}

void MainWindow::loadWorkspace(const QString& name)
{
    qCDebug(cadMainWindow) << "Loading workspace:" << name;

    QSettings settings;
    settings.beginGroup("Workspaces");
    settings.beginGroup(name);

    if (settings.contains("geometry")) {
        restoreGeometry(settings.value("geometry").toByteArray());
        restoreState(settings.value("state").toByteArray());

        setRibbonVisible(settings.value("ribbonVisible", true).toBool());
        setCommandLineVisible(settings.value("commandLineVisible", true).toBool());
        setStatusBarVisible(settings.value("statusBarVisible", true).toBool());
    }

    settings.endGroup();
    settings.endGroup();
}

void MainWindow::resetWorkspace()
{
    qCDebug(cadMainWindow) << "Resetting workspace to default";

    // Reset to default layout
    setRibbonVisible(true);
    setCommandLineVisible(true);
    setStatusBarVisible(true);

    // Reset dock widgets to default positions
    if (m_dockablePalettes) {
        m_dockablePalettes->resetToDefault();
    }

    // Reset window size
    resize(1400, 900);
}

// View management
void MainWindow::maximizeViewport()
{
    if (m_viewportManager && !m_viewportsMaximized) {
        m_viewportsMaximized = true;
        m_viewportManager->maximizeCurrentViewport();
        qCDebug(cadMainWindow) << "Viewport maximized";
    }
}

void MainWindow::restoreViewports()
{
    if (m_viewportManager && m_viewportsMaximized) {
        m_viewportsMaximized = false;
        m_viewportManager->restoreViewports();
        qCDebug(cadMainWindow) << "Viewports restored";
    }
}

void MainWindow::tileViewports()
{
    if (m_viewportManager) {
        m_viewportManager->tileViewports();
        qCDebug(cadMainWindow) << "Viewports tiled";
    }
}

void MainWindow::cascadeViewports()
{
    if (m_viewportManager) {
        m_viewportManager->cascadeViewports();
        qCDebug(cadMainWindow) << "Viewports cascaded";
    }
}

// Event handlers
void MainWindow::closeEvent(QCloseEvent *event)
{
    qCDebug(cadMainWindow) << "Close event received";

    if (confirmClose()) {
        saveWindowState();
        event->accept();
    } else {
        event->ignore();
    }
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    qCDebug(cadMainWindow) << "Window resized to:" << event->size();
}

void MainWindow::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent(event);
    qCDebug(cadMainWindow) << "Window shown";
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    // Handle global key events
    if (event->key() == Qt::Key_Space && m_viewportManager) {
        // Space bar for pan mode
        m_viewportManager->setPanMode(true);
        event->accept();
        return;
    }

    QMainWindow::keyPressEvent(event);
}

void MainWindow::contextMenuEvent(QContextMenuEvent *event)
{
    if (m_contextMenus) {
        m_contextMenus->showContextMenu(event->globalPos());
        event->accept();
    } else {
        QMainWindow::contextMenuEvent(event);
    }
}

// Utility methods
void MainWindow::updateWindowTitle()
{
    QString title = "AutoCAD Clone";

    CADApplication* app = CADApplication::instance();
    if (!app->currentDocument().isEmpty()) {
        QFileInfo fileInfo(app->currentDocument());
        title += " - " + fileInfo.fileName();

        if (app->isModified()) {
            title += "*";
        }
    } else if (app->isModified()) {
        title += " - Untitled*";
    }

    setWindowTitle(title);
}

void MainWindow::updateRecentFiles()
{
    QSettings settings;
    m_recentFiles = settings.value("recentFiles").toStringList();

    int numRecentFiles = qMin(m_recentFiles.size(), MaxRecentFiles);

    for (int i = 0; i < numRecentFiles; ++i) {
        QString text = QString("&%1 %2").arg(i + 1).arg(QFileInfo(m_recentFiles[i]).fileName());
        m_recentFileActions[i]->setText(text);
        m_recentFileActions[i]->setData(m_recentFiles[i]);
        m_recentFileActions[i]->setVisible(true);
    }

    for (int i = numRecentFiles; i < MaxRecentFiles; ++i) {
        m_recentFileActions[i]->setVisible(false);
    }
}

void MainWindow::addRecentFile(const QString& filePath)
{
    m_recentFiles.removeAll(filePath);
    m_recentFiles.prepend(filePath);

    while (m_recentFiles.size() > MaxRecentFiles) {
        m_recentFiles.removeLast();
    }

    QSettings settings;
    settings.setValue("recentFiles", m_recentFiles);

    updateRecentFiles();
}

bool MainWindow::confirmClose()
{
    CADApplication* app = CADApplication::instance();
    if (app->isModified()) {
        int ret = QMessageBox::question(this, "Save Changes",
                                       "The document has been modified.\n"
                                       "Do you want to save your changes?",
                                       QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
                                       QMessageBox::Save);

        if (ret == QMessageBox::Save) {
            app->saveDocument();
            return true;
        } else if (ret == QMessageBox::Cancel) {
            return false;
        }
    }

    return true;
}

void MainWindow::saveWindowState()
{
    QSettings settings;
    settings.setValue("geometry", saveGeometry());
    settings.setValue("windowState", saveState());
    settings.setValue("ribbonVisible", m_ribbonVisible);
    settings.setValue("commandLineVisible", m_commandLineVisible);
    settings.setValue("statusBarVisible", m_statusBarVisible);
}

void MainWindow::restoreWindowState()
{
    QSettings settings;

    if (settings.contains("geometry")) {
        restoreGeometry(settings.value("geometry").toByteArray());
        restoreState(settings.value("windowState").toByteArray());

        m_ribbonVisible = settings.value("ribbonVisible", true).toBool();
        m_commandLineVisible = settings.value("commandLineVisible", true).toBool();
        m_statusBarVisible = settings.value("statusBarVisible", true).toBool();

        setRibbonVisible(m_ribbonVisible);
        setCommandLineVisible(m_commandLineVisible);
        setStatusBarVisible(m_statusBarVisible);
    }
}
