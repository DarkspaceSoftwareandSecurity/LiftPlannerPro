#include "RibbonInterface.h"
#include <QTabBar>
#include <QStackedWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QToolButton>
#include <QButtonGroup>
#include <QSplitter>
#include <QLabel>
#include <QMenu>
#include <QScrollArea>
#include <QApplication>
#include <QStyle>

Q_LOGGING_CATEGORY(cadRibbon, "cad.ribbon")

// RibbonPanel implementation
RibbonPanel::RibbonPanel(const QString& title, QWidget* parent)
    : QWidget(parent)
    , m_title(title)
    , m_buttonCount(0)
{
    setupUI();
    setTitle(title);
}

RibbonPanel::~RibbonPanel() = default;

void RibbonPanel::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(2, 2, 2, 2);
    m_mainLayout->setSpacing(0);
    
    // Content widget
    m_contentWidget = new QWidget();
    m_contentLayout = new QHBoxLayout(m_contentWidget);
    m_contentLayout->setContentsMargins(4, 4, 4, 4);
    m_contentLayout->setSpacing(2);
    
    // Title label
    m_titleLabel = new QLabel();
    m_titleLabel->setAlignment(Qt::AlignCenter);
    m_titleLabel->setStyleSheet("QLabel { font-size: 8pt; color: #ffffff; background: transparent; }");
    m_titleLabel->setMaximumHeight(16);
    
    m_mainLayout->addWidget(m_contentWidget, 1);
    m_mainLayout->addWidget(m_titleLabel, 0);
    
    // Panel styling
    setStyleSheet(
        "RibbonPanel {"
        "    border: 1px solid #555555;"
        "    border-radius: 3px;"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #4a4a4a, stop:1 #3a3a3a);"
        "    margin: 1px;"
        "}"
    );
}

void RibbonPanel::setTitle(const QString& title)
{
    m_title = title;
    m_titleLabel->setText(title);
}

QToolButton* RibbonPanel::createButton(const QString& text, const QIcon& icon, const QString& tooltip, int size)
{
    QToolButton* button = new QToolButton();
    button->setText(text);
    button->setIcon(icon);
    button->setToolTip(tooltip.isEmpty() ? text : tooltip);
    button->setAutoRaise(true);
    button->setFocusPolicy(Qt::NoFocus);
    
    // Set button properties based on size
    switch (size) {
    case 0: // Large button
        button->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        button->setIconSize(QSize(32, 32));
        button->setMinimumSize(64, 64);
        button->setMaximumSize(80, 80);
        break;
    case 1: // Medium button
        button->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        button->setIconSize(QSize(16, 16));
        button->setMinimumSize(48, 32);
        button->setMaximumSize(80, 32);
        break;
    case 2: // Small button
        button->setToolButtonStyle(Qt::ToolButtonIconOnly);
        button->setIconSize(QSize(16, 16));
        button->setMinimumSize(24, 24);
        button->setMaximumSize(24, 24);
        break;
    }
    
    // Button styling
    button->setStyleSheet(
        "QToolButton {"
        "    border: 1px solid transparent;"
        "    border-radius: 2px;"
        "    padding: 2px;"
        "    background: transparent;"
        "    color: #ffffff;"
        "}"
        "QToolButton:hover {"
        "    border: 1px solid #0078d4;"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #4a90e2, stop:1 #357abd);"
        "}"
        "QToolButton:pressed {"
        "    border: 1px solid #005a9e;"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #357abd, stop:1 #2e6da4);"
        "}"
    );
    
    connect(button, &QToolButton::clicked, this, &RibbonPanel::onButtonClicked);
    return button;
}

void RibbonPanel::addLargeButton(const QString& text, const QIcon& icon, const QString& tooltip)
{
    QToolButton* button = createButton(text, icon, tooltip, 0);
    m_contentLayout->addWidget(button);
    m_buttonCount++;
}

void RibbonPanel::addMediumButton(const QString& text, const QIcon& icon, const QString& tooltip)
{
    QToolButton* button = createButton(text, icon, tooltip, 1);
    m_contentLayout->addWidget(button);
    m_buttonCount++;
}

void RibbonPanel::addSmallButton(const QString& text, const QIcon& icon, const QString& tooltip)
{
    QToolButton* button = createButton(text, icon, tooltip, 2);
    m_contentLayout->addWidget(button);
    m_buttonCount++;
}

void RibbonPanel::addSeparator()
{
    QFrame* separator = new QFrame();
    separator->setFrameShape(QFrame::VLine);
    separator->setFrameShadow(QFrame::Sunken);
    separator->setStyleSheet("QFrame { color: #666666; }");
    m_contentLayout->addWidget(separator);
}

void RibbonPanel::onButtonClicked()
{
    QToolButton* button = qobject_cast<QToolButton*>(sender());
    if (button) {
        emit buttonClicked(button->text().toLower().replace(" ", ""));
    }
}

// RibbonTab implementation
RibbonTab::RibbonTab(const QString& name, QWidget* parent)
    : QWidget(parent)
    , m_name(name)
{
    setupUI();
}

RibbonTab::~RibbonTab() = default;

void RibbonTab::setupUI()
{
    m_layout = new QHBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);
    
    // Create scroll area for panels
    m_scrollArea = new QScrollArea();
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setFrameShape(QFrame::NoFrame);
    
    m_scrollWidget = new QWidget();
    m_scrollLayout = new QHBoxLayout(m_scrollWidget);
    m_scrollLayout->setContentsMargins(4, 4, 4, 4);
    m_scrollLayout->setSpacing(2);
    m_scrollLayout->addStretch();
    
    m_scrollArea->setWidget(m_scrollWidget);
    m_layout->addWidget(m_scrollArea);
    
    // Tab styling
    setStyleSheet(
        "RibbonTab {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #505050, stop:1 #404040);"
        "    border-bottom: 1px solid #333333;"
        "}"
    );
}

RibbonPanel* RibbonTab::addPanel(const QString& title)
{
    auto panel = std::make_unique<RibbonPanel>(title, this);
    RibbonPanel* panelPtr = panel.get();
    
    connect(panelPtr, &RibbonPanel::buttonClicked, this, &RibbonTab::buttonClicked);
    
    // Insert before stretch
    m_scrollLayout->insertWidget(m_scrollLayout->count() - 1, panelPtr);
    m_panels.push_back(std::move(panel));
    
    return panelPtr;
}

RibbonPanel* RibbonTab::getPanel(const QString& title) const
{
    for (const auto& panel : m_panels) {
        if (panel->title() == title) {
            return panel.get();
        }
    }
    return nullptr;
}

// RibbonInterface implementation
RibbonInterface::RibbonInterface(QWidget* parent)
    : QToolBar(parent)
    , m_minimized(false)
    , m_autoHide(false)
{
    qCDebug(cadRibbon) << "Creating ribbon interface...";
    
    setupUI();
    createDefaultTabs();
    applyRibbonStyling();
    
    qCDebug(cadRibbon) << "Ribbon interface created";
}

RibbonInterface::~RibbonInterface()
{
    qCDebug(cadRibbon) << "Ribbon interface destroyed";
}

void RibbonInterface::setupUI()
{
    setObjectName("RibbonInterface");
    setMovable(false);
    setFloatable(false);
    setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    
    // Main widget
    m_mainWidget = new QWidget();
    addWidget(m_mainWidget);
    
    m_mainLayout = new QVBoxLayout(m_mainWidget);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);
    
    // Top widget (Application button + Quick Access + Tab bar)
    m_topWidget = new QWidget();
    m_topLayout = new QHBoxLayout(m_topWidget);
    m_topLayout->setContentsMargins(2, 2, 2, 2);
    m_topLayout->setSpacing(2);
    
    setupApplicationButton();
    setupQuickAccessToolbar();
    
    // Tab bar
    m_tabBar = new QTabBar();
    m_tabBar->setExpanding(false);
    m_tabBar->setUsesScrollButtons(true);
    m_tabBar->setElideMode(Qt::ElideNone);
    
    // Minimize and help buttons
    m_minimizeButton = new QToolButton();
    m_minimizeButton->setIcon(QIcon(":/icons/ribbon_minimize.png"));
    m_minimizeButton->setToolTip("Minimize Ribbon");
    m_minimizeButton->setAutoRaise(true);
    
    m_helpButton = new QToolButton();
    m_helpButton->setIcon(QIcon(":/icons/help.png"));
    m_helpButton->setToolTip("Help");
    m_helpButton->setAutoRaise(true);
    
    // Layout top widget
    m_topLayout->addWidget(m_applicationButton);
    m_topLayout->addWidget(m_quickAccessToolbar);
    m_topLayout->addWidget(m_tabBar, 1);
    m_topLayout->addWidget(m_minimizeButton);
    m_topLayout->addWidget(m_helpButton);
    
    // Content stack
    m_contentStack = new QStackedWidget();
    
    // Layout main widget
    m_mainLayout->addWidget(m_topWidget);
    m_mainLayout->addWidget(m_contentStack);
    
    // Connect signals
    connect(m_tabBar, &QTabBar::currentChanged, this, &RibbonInterface::onTabChanged);
    connect(m_tabBar, &QTabBar::tabBarDoubleClicked, this, &RibbonInterface::onTabBarDoubleClicked);
    connect(m_minimizeButton, &QToolButton::clicked, [this]() {
        setMinimized(!m_minimized);
    });
}

void RibbonInterface::setupApplicationButton()
{
    m_applicationButton = new QToolButton();
    m_applicationButton->setText("File");
    m_applicationButton->setIcon(QIcon(":/icons/app_button.png"));
    m_applicationButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    m_applicationButton->setPopupMode(QToolButton::InstantPopup);
    m_applicationButton->setMinimumSize(60, 32);
    m_applicationButton->setStyleSheet(
        "QToolButton {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #0078d4, stop:1 #005a9e);"
        "    border: 1px solid #004578;"
        "    border-radius: 3px;"
        "    color: white;"
        "    font-weight: bold;"
        "    padding: 4px 8px;"
        "}"
        "QToolButton:hover {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #106ebe, stop:1 #0078d4);"
        "}"
        "QToolButton:pressed {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #005a9e, stop:1 #004578);"
        "}"
    );
    
    connect(m_applicationButton, &QToolButton::clicked, this, &RibbonInterface::onApplicationButtonClicked);
}

void RibbonInterface::setupQuickAccessToolbar()
{
    m_quickAccessToolbar = new QToolBar();
    m_quickAccessToolbar->setIconSize(QSize(16, 16));
    m_quickAccessToolbar->setToolButtonStyle(Qt::ToolButtonIconOnly);
    m_quickAccessToolbar->setStyleSheet(
        "QToolBar {"
        "    border: none;"
        "    background: transparent;"
        "    spacing: 2px;"
        "}"
        "QToolButton {"
        "    border: 1px solid transparent;"
        "    border-radius: 2px;"
        "    padding: 2px;"
        "    background: transparent;"
        "}"
        "QToolButton:hover {"
        "    border: 1px solid #0078d4;"
        "    background: #4a90e2;"
        "}"
    );
    
    // Add default quick access buttons
    addQuickAccessButton("New", QIcon(":/icons/new.png"), "new");
    addQuickAccessButton("Open", QIcon(":/icons/open.png"), "open");
    addQuickAccessButton("Save", QIcon(":/icons/save.png"), "save");
    addQuickAccessButton("Undo", QIcon(":/icons/undo.png"), "undo");
    addQuickAccessButton("Redo", QIcon(":/icons/redo.png"), "redo");
}

void RibbonInterface::createDefaultTabs()
{
    qCDebug(cadRibbon) << "Creating default ribbon tabs...";

    createHomeTab();
    createInsertTab();
    createAnnotateTab();
    createParametricTab();
    createViewTab();
    createManageTab();
    createOutputTab();
    createAddInsTab();

    // Set Home tab as default
    setCurrentTab("Home");
}

void RibbonInterface::createHomeTab()
{
    RibbonTab* homeTab = addTab("Home");

    // Draw panel
    RibbonPanel* drawPanel = homeTab->addPanel("Draw");
    drawPanel->addLargeButton("Line", QIcon(":/icons/line.png"), "Draw a line");
    drawPanel->addLargeButton("Polyline", QIcon(":/icons/polyline.png"), "Draw a polyline");
    drawPanel->addLargeButton("Circle", QIcon(":/icons/circle.png"), "Draw a circle");
    drawPanel->addLargeButton("Arc", QIcon(":/icons/arc.png"), "Draw an arc");
    drawPanel->addSeparator();
    drawPanel->addMediumButton("Rectangle", QIcon(":/icons/rectangle.png"), "Draw a rectangle");
    drawPanel->addMediumButton("Polygon", QIcon(":/icons/polygon.png"), "Draw a polygon");
    drawPanel->addMediumButton("Ellipse", QIcon(":/icons/ellipse.png"), "Draw an ellipse");
    drawPanel->addMediumButton("Spline", QIcon(":/icons/spline.png"), "Draw a spline");

    // Modify panel
    RibbonPanel* modifyPanel = homeTab->addPanel("Modify");
    modifyPanel->addLargeButton("Move", QIcon(":/icons/move.png"), "Move objects");
    modifyPanel->addLargeButton("Copy", QIcon(":/icons/copy.png"), "Copy objects");
    modifyPanel->addLargeButton("Rotate", QIcon(":/icons/rotate.png"), "Rotate objects");
    modifyPanel->addLargeButton("Scale", QIcon(":/icons/scale.png"), "Scale objects");
    modifyPanel->addSeparator();
    modifyPanel->addMediumButton("Mirror", QIcon(":/icons/mirror.png"), "Mirror objects");
    modifyPanel->addMediumButton("Offset", QIcon(":/icons/offset.png"), "Offset objects");
    modifyPanel->addMediumButton("Trim", QIcon(":/icons/trim.png"), "Trim objects");
    modifyPanel->addMediumButton("Extend", QIcon(":/icons/extend.png"), "Extend objects");
    modifyPanel->addMediumButton("Fillet", QIcon(":/icons/fillet.png"), "Fillet objects");
    modifyPanel->addMediumButton("Chamfer", QIcon(":/icons/chamfer.png"), "Chamfer objects");

    // Layers panel
    RibbonPanel* layersPanel = homeTab->addPanel("Layers");
    layersPanel->addLargeButton("Layer\nProperties", QIcon(":/icons/layer_properties.png"), "Layer Properties Manager");
    layersPanel->addMediumButton("Make Current", QIcon(":/icons/layer_current.png"), "Make layer current");
    layersPanel->addMediumButton("Layer Previous", QIcon(":/icons/layer_previous.png"), "Previous layer");
    layersPanel->addSmallButton("On/Off", QIcon(":/icons/layer_on.png"), "Turn layer on/off");
    layersPanel->addSmallButton("Freeze", QIcon(":/icons/layer_freeze.png"), "Freeze/thaw layer");
    layersPanel->addSmallButton("Lock", QIcon(":/icons/layer_lock.png"), "Lock/unlock layer");

    // Properties panel
    RibbonPanel* propertiesPanel = homeTab->addPanel("Properties");
    propertiesPanel->addLargeButton("Properties", QIcon(":/icons/properties.png"), "Properties palette");
    propertiesPanel->addMediumButton("Match Properties", QIcon(":/icons/match_properties.png"), "Match properties");
    propertiesPanel->addSmallButton("Color", QIcon(":/icons/color.png"), "Object color");
    propertiesPanel->addSmallButton("Linetype", QIcon(":/icons/linetype.png"), "Object linetype");
    propertiesPanel->addSmallButton("Lineweight", QIcon(":/icons/lineweight.png"), "Object lineweight");

    // Clipboard panel
    RibbonPanel* clipboardPanel = homeTab->addPanel("Clipboard");
    clipboardPanel->addLargeButton("Paste", QIcon(":/icons/paste.png"), "Paste from clipboard");
    clipboardPanel->addMediumButton("Cut", QIcon(":/icons/cut.png"), "Cut to clipboard");
    clipboardPanel->addMediumButton("Copy", QIcon(":/icons/copy_clipboard.png"), "Copy to clipboard");
    clipboardPanel->addMediumButton("Copy with Base Point", QIcon(":/icons/copy_base.png"), "Copy with base point");
}

void RibbonInterface::createInsertTab()
{
    RibbonTab* insertTab = addTab("Insert");

    // Block panel
    RibbonPanel* blockPanel = insertTab->addPanel("Block");
    blockPanel->addLargeButton("Insert", QIcon(":/icons/block_insert.png"), "Insert block");
    blockPanel->addMediumButton("Create", QIcon(":/icons/block_create.png"), "Create block");
    blockPanel->addMediumButton("Edit", QIcon(":/icons/block_edit.png"), "Edit block");
    blockPanel->addMediumButton("Write", QIcon(":/icons/block_write.png"), "Write block");

    // Reference panel
    RibbonPanel* referencePanel = insertTab->addPanel("Reference");
    referencePanel->addLargeButton("Attach", QIcon(":/icons/xref_attach.png"), "Attach external reference");
    referencePanel->addMediumButton("Clip", QIcon(":/icons/xref_clip.png"), "Clip external reference");
    referencePanel->addMediumButton("Adjust", QIcon(":/icons/xref_adjust.png"), "Adjust external reference");
    referencePanel->addMediumButton("Manager", QIcon(":/icons/xref_manager.png"), "External reference manager");

    // Content panel
    RibbonPanel* contentPanel = insertTab->addPanel("Content");
    contentPanel->addLargeButton("Design\nCenter", QIcon(":/icons/design_center.png"), "Design Center");
    contentPanel->addMediumButton("Tool Palettes", QIcon(":/icons/tool_palettes.png"), "Tool Palettes");
    contentPanel->addMediumButton("Content Browser", QIcon(":/icons/content_browser.png"), "Content Browser");

    // Data panel
    RibbonPanel* dataPanel = insertTab->addPanel("Data");
    dataPanel->addLargeButton("Table", QIcon(":/icons/table.png"), "Insert table");
    dataPanel->addMediumButton("Field", QIcon(":/icons/field.png"), "Insert field");
    dataPanel->addMediumButton("Object", QIcon(":/icons/ole_object.png"), "Insert OLE object");
}

void RibbonInterface::createAnnotateTab()
{
    RibbonTab* annotateTab = addTab("Annotate");

    // Text panel
    RibbonPanel* textPanel = annotateTab->addPanel("Text");
    textPanel->addLargeButton("Multiline\nText", QIcon(":/icons/mtext.png"), "Multiline text");
    textPanel->addLargeButton("Single Line\nText", QIcon(":/icons/text.png"), "Single line text");
    textPanel->addMediumButton("Text Style", QIcon(":/icons/text_style.png"), "Text style manager");
    textPanel->addMediumButton("Edit Text", QIcon(":/icons/text_edit.png"), "Edit text");

    // Dimensions panel
    RibbonPanel* dimensionsPanel = annotateTab->addPanel("Dimensions");
    dimensionsPanel->addLargeButton("Linear", QIcon(":/icons/dim_linear.png"), "Linear dimension");
    dimensionsPanel->addLargeButton("Aligned", QIcon(":/icons/dim_aligned.png"), "Aligned dimension");
    dimensionsPanel->addLargeButton("Angular", QIcon(":/icons/dim_angular.png"), "Angular dimension");
    dimensionsPanel->addSeparator();
    dimensionsPanel->addMediumButton("Radius", QIcon(":/icons/dim_radius.png"), "Radius dimension");
    dimensionsPanel->addMediumButton("Diameter", QIcon(":/icons/dim_diameter.png"), "Diameter dimension");
    dimensionsPanel->addMediumButton("Arc Length", QIcon(":/icons/dim_arc.png"), "Arc length dimension");
    dimensionsPanel->addMediumButton("Ordinate", QIcon(":/icons/dim_ordinate.png"), "Ordinate dimension");

    // Leaders panel
    RibbonPanel* leadersPanel = annotateTab->addPanel("Leaders");
    leadersPanel->addLargeButton("Multileader", QIcon(":/icons/multileader.png"), "Multileader");
    leadersPanel->addMediumButton("Quick Leader", QIcon(":/icons/qleader.png"), "Quick leader");
    leadersPanel->addMediumButton("Leader Style", QIcon(":/icons/leader_style.png"), "Multileader style");

    // Markup panel
    RibbonPanel* markupPanel = annotateTab->addPanel("Markup");
    markupPanel->addLargeButton("Revision\nCloud", QIcon(":/icons/revcloud.png"), "Revision cloud");
    markupPanel->addMediumButton("Wipeout", QIcon(":/icons/wipeout.png"), "Wipeout");
    markupPanel->addMediumButton("Markup", QIcon(":/icons/markup.png"), "Markup set");
}

void RibbonInterface::createViewTab()
{
    RibbonTab* viewTab = addTab("View");

    // Views panel
    RibbonPanel* viewsPanel = viewTab->addPanel("Views");
    viewsPanel->addLargeButton("Top", QIcon(":/icons/view_top.png"), "Top view");
    viewsPanel->addLargeButton("Front", QIcon(":/icons/view_front.png"), "Front view");
    viewsPanel->addLargeButton("Right", QIcon(":/icons/view_right.png"), "Right view");
    viewsPanel->addLargeButton("SW Isometric", QIcon(":/icons/view_sw_iso.png"), "SW Isometric view");

    // Navigate panel
    RibbonPanel* navigatePanel = viewTab->addPanel("Navigate");
    navigatePanel->addLargeButton("Zoom\nExtents", QIcon(":/icons/zoom_extents.png"), "Zoom to extents");
    navigatePanel->addLargeButton("Zoom\nWindow", QIcon(":/icons/zoom_window.png"), "Zoom window");
    navigatePanel->addMediumButton("Pan", QIcon(":/icons/pan.png"), "Pan");
    navigatePanel->addMediumButton("Orbit", QIcon(":/icons/orbit.png"), "3D orbit");
    navigatePanel->addMediumButton("Zoom Previous", QIcon(":/icons/zoom_previous.png"), "Zoom previous");

    // Coordinates panel
    RibbonPanel* coordinatesPanel = viewTab->addPanel("Coordinates");
    coordinatesPanel->addLargeButton("UCS", QIcon(":/icons/ucs.png"), "User coordinate system");
    coordinatesPanel->addMediumButton("World UCS", QIcon(":/icons/ucs_world.png"), "World UCS");
    coordinatesPanel->addMediumButton("Object UCS", QIcon(":/icons/ucs_object.png"), "Object UCS");
    coordinatesPanel->addMediumButton("Face UCS", QIcon(":/icons/ucs_face.png"), "Face UCS");

    // Palettes panel
    RibbonPanel* palettesPanel = viewTab->addPanel("Palettes");
    palettesPanel->addLargeButton("Properties", QIcon(":/icons/properties.png"), "Properties palette");
    palettesPanel->addMediumButton("Layers", QIcon(":/icons/layer_properties.png"), "Layer Properties Manager");
    palettesPanel->addMediumButton("Tool Palettes", QIcon(":/icons/tool_palettes.png"), "Tool Palettes");
    palettesPanel->addMediumButton("Design Center", QIcon(":/icons/design_center.png"), "Design Center");

    // Visual Styles panel
    RibbonPanel* visualPanel = viewTab->addPanel("Visual Styles");
    visualPanel->addLargeButton("Wireframe", QIcon(":/icons/visual_wireframe.png"), "Wireframe visual style");
    visualPanel->addLargeButton("Hidden", QIcon(":/icons/visual_hidden.png"), "Hidden visual style");
    visualPanel->addLargeButton("Realistic", QIcon(":/icons/visual_realistic.png"), "Realistic visual style");
    visualPanel->addLargeButton("Conceptual", QIcon(":/icons/visual_conceptual.png"), "Conceptual visual style");
}

void RibbonInterface::createOutputTab()
{
    RibbonTab* outputTab = addTab("Output");

    // Plot panel
    RibbonPanel* plotPanel = outputTab->addPanel("Plot");
    plotPanel->addLargeButton("Plot", QIcon(":/icons/plot.png"), "Plot drawing");
    plotPanel->addMediumButton("Page Setup", QIcon(":/icons/page_setup.png"), "Page setup manager");
    plotPanel->addMediumButton("Plot Preview", QIcon(":/icons/plot_preview.png"), "Plot preview");
    plotPanel->addMediumButton("Plotter Manager", QIcon(":/icons/plotter_manager.png"), "Plotter manager");

    // Publish panel
    RibbonPanel* publishPanel = outputTab->addPanel("Publish");
    publishPanel->addLargeButton("Batch Plot", QIcon(":/icons/batch_plot.png"), "Batch plot");
    publishPanel->addMediumButton("Publish to Web", QIcon(":/icons/publish_web.png"), "Publish to web");
    publishPanel->addMediumButton("3D Print", QIcon(":/icons/3d_print.png"), "3D print");

    // Export panel
    RibbonPanel* exportPanel = outputTab->addPanel("Export");
    exportPanel->addLargeButton("Export", QIcon(":/icons/export.png"), "Export drawing");
    exportPanel->addMediumButton("PDF", QIcon(":/icons/export_pdf.png"), "Export to PDF");
    exportPanel->addMediumButton("DWF", QIcon(":/icons/export_dwf.png"), "Export to DWF");
    exportPanel->addMediumButton("Image", QIcon(":/icons/export_image.png"), "Export to image");
}

// Tab management methods
RibbonTab* RibbonInterface::addTab(const QString& name)
{
    auto tab = std::make_unique<RibbonTab>(name, this);
    RibbonTab* tabPtr = tab.get();

    connect(tabPtr, &RibbonTab::buttonClicked, this, &RibbonInterface::commandTriggered);

    int index = m_tabBar->addTab(name);
    m_tabIndexMap[name] = index;

    m_contentStack->addWidget(tabPtr);
    m_tabs.push_back(std::move(tab));

    return tabPtr;
}

void RibbonInterface::setCurrentTab(const QString& name)
{
    auto it = m_tabIndexMap.find(name);
    if (it != m_tabIndexMap.end()) {
        m_tabBar->setCurrentIndex(it->second);
        m_contentStack->setCurrentIndex(it->second);
        m_currentTab = name;
        emit tabChanged(name);
    }
}

void RibbonInterface::addQuickAccessButton(const QString& text, const QIcon& icon, const QString& command)
{
    QAction* action = m_quickAccessToolbar->addAction(icon, text);
    action->setData(command);
    action->setToolTip(text);
    connect(action, &QAction::triggered, this, &RibbonInterface::onQuickAccessButtonClicked);
}

void RibbonInterface::setMinimized(bool minimized)
{
    if (m_minimized != minimized) {
        m_minimized = minimized;
        m_contentStack->setVisible(!minimized);
        m_minimizeButton->setIcon(QIcon(minimized ? ":/icons/ribbon_expand.png" : ":/icons/ribbon_minimize.png"));
        m_minimizeButton->setToolTip(minimized ? "Expand Ribbon" : "Minimize Ribbon");
        emit minimizedChanged(minimized);
    }
}

void RibbonInterface::applyRibbonStyling()
{
    m_ribbonStyleSheet =
        "RibbonInterface {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #606060, stop:1 #505050);"
        "    border: none;"
        "}"
        "QTabBar::tab {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #505050, stop:1 #404040);"
        "    border: 1px solid #333333;"
        "    border-bottom: none;"
        "    padding: 6px 12px;"
        "    margin-right: 2px;"
        "    color: #ffffff;"
        "    min-width: 60px;"
        "}"
        "QTabBar::tab:selected {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #606060, stop:1 #505050);"
        "    border-bottom: 2px solid #0078d4;"
        "}"
        "QTabBar::tab:hover {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #555555, stop:1 #454545);"
        "}";

    setStyleSheet(m_ribbonStyleSheet);
}

// Slot implementations
void RibbonInterface::onTabChanged(int index)
{
    if (index >= 0 && index < static_cast<int>(m_tabs.size())) {
        m_contentStack->setCurrentIndex(index);
        m_currentTab = m_tabs[index]->name();
        emit tabChanged(m_currentTab);
    }
}

void RibbonInterface::onTabBarDoubleClicked(int index)
{
    Q_UNUSED(index)
    setMinimized(!m_minimized);
}

void RibbonInterface::onQuickAccessButtonClicked()
{
    QAction* action = qobject_cast<QAction*>(sender());
    if (action) {
        emit commandTriggered(action->data().toString());
    }
}

void RibbonInterface::onApplicationButtonClicked()
{
    // Application button clicked - show file menu
    emit commandTriggered("file_menu");
}

// Placeholder methods for remaining tabs
void RibbonInterface::createParametricTab()
{
    RibbonTab* parametricTab = addTab("Parametric");
    // TODO: Add parametric tools
}

void RibbonInterface::createManageTab()
{
    RibbonTab* manageTab = addTab("Manage");
    // TODO: Add management tools
}

void RibbonInterface::createAddInsTab()
{
    RibbonTab* addInsTab = addTab("Add-ins");
    // TODO: Add plugin/add-in tools
}
