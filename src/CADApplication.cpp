#include "CADApplication.h"
#include "CommandManager.h"
#include "GeometryEngine.h"
#include "LayerManager.h"
#include "BlockManager.h"
#include "XrefManager.h"
#include "LayoutManager.h"
#include "MaterialSystem.h"
#include "ObjectSnaps.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QStandardPaths>
#include <QDir>

Q_LOGGING_CATEGORY(cadApp, "cad.application")

CADApplication* CADApplication::s_instance = nullptr;

CADApplication::CADApplication(int &argc, char **argv)
    : QApplication(argc, argv)
    , m_isModified(false)
    , m_currentUnits(Units::Millimeters)
    , m_precision(2)
    , m_gridVisible(true)
    , m_snapEnabled(true)
    , m_gridSpacing(10.0)
    , m_orthoMode(false)
    , m_polarTracking(false)
    , m_dynamicInput(true)
{
    s_instance = this;
    
    // Initialize settings
    QString configPath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QDir().mkpath(configPath);
    m_settings = std::make_unique<QSettings>(configPath + "/settings.ini", QSettings::IniFormat);
    
    qCDebug(cadApp) << "CAD Application created";
    
    connect(this, &QApplication::aboutToQuit, this, &CADApplication::onAboutToQuit);
}

CADApplication::~CADApplication()
{
    qCDebug(cadApp) << "CAD Application destroyed";
    s_instance = nullptr;
}

CADApplication* CADApplication::instance()
{
    return s_instance;
}

bool CADApplication::initialize()
{
    qCDebug(cadApp) << "Initializing CAD Application...";
    
    try {
        initializeCore();
        initializeManagers();
        setupDefaultSettings();
        loadSettings();
        connectSignals();
        
        qCDebug(cadApp) << "CAD Application initialized successfully";
        return true;
    }
    catch (const std::exception& e) {
        qCCritical(cadApp) << "Failed to initialize CAD Application:" << e.what();
        return false;
    }
}

void CADApplication::shutdown()
{
    qCDebug(cadApp) << "Shutting down CAD Application...";
    
    saveSettings();
    
    // Shutdown in reverse order
    m_objectSnaps.reset();
    m_materialSystem.reset();
    m_layoutManager.reset();
    m_xrefManager.reset();
    m_blockManager.reset();
    m_layerManager.reset();
    m_geometryEngine.reset();
    m_commandManager.reset();
    
    qCDebug(cadApp) << "CAD Application shutdown complete";
}

void CADApplication::initializeCore()
{
    qCDebug(cadApp) << "Initializing core systems...";
    
    // Initialize geometry engine first (OpenCASCADE)
    m_geometryEngine = std::make_unique<GeometryEngine>();
    if (!m_geometryEngine->initialize()) {
        throw std::runtime_error("Failed to initialize geometry engine");
    }
    
    // Initialize command manager
    m_commandManager = std::make_unique<CommandManager>();
    
    qCDebug(cadApp) << "Core systems initialized";
}

void CADApplication::initializeManagers()
{
    qCDebug(cadApp) << "Initializing managers...";
    
    // Initialize all managers
    m_layerManager = std::make_unique<LayerManager>();
    m_blockManager = std::make_unique<BlockManager>();
    m_xrefManager = std::make_unique<XrefManager>();
    m_layoutManager = std::make_unique<LayoutManager>();
    m_materialSystem = std::make_unique<MaterialSystem>();
    m_objectSnaps = std::make_unique<ObjectSnaps>();
    
    qCDebug(cadApp) << "Managers initialized";
}

void CADApplication::setupDefaultSettings()
{
    qCDebug(cadApp) << "Setting up default settings...";
    
    // Set default values if not already set
    if (!m_settings->contains("units")) {
        m_settings->setValue("units", static_cast<int>(Units::Millimeters));
    }
    if (!m_settings->contains("precision")) {
        m_settings->setValue("precision", 2);
    }
    if (!m_settings->contains("gridVisible")) {
        m_settings->setValue("gridVisible", true);
    }
    if (!m_settings->contains("snapEnabled")) {
        m_settings->setValue("snapEnabled", true);
    }
    if (!m_settings->contains("gridSpacing")) {
        m_settings->setValue("gridSpacing", 10.0);
    }
    if (!m_settings->contains("orthoMode")) {
        m_settings->setValue("orthoMode", false);
    }
    if (!m_settings->contains("polarTracking")) {
        m_settings->setValue("polarTracking", false);
    }
    if (!m_settings->contains("dynamicInput")) {
        m_settings->setValue("dynamicInput", true);
    }
}

void CADApplication::connectSignals()
{
    // Connect internal signals for state management
    connect(this, &CADApplication::modifiedChanged, [this](bool modified) {
        qCDebug(cadApp) << "Document modified state changed:" << modified;
    });
    
    connect(this, &CADApplication::currentDocumentChanged, [this](const QString& path) {
        qCDebug(cadApp) << "Current document changed:" << path;
    });
}

void CADApplication::saveSettings()
{
    qCDebug(cadApp) << "Saving application settings...";
    
    m_settings->setValue("units", static_cast<int>(m_currentUnits));
    m_settings->setValue("precision", m_precision);
    m_settings->setValue("gridVisible", m_gridVisible);
    m_settings->setValue("snapEnabled", m_snapEnabled);
    m_settings->setValue("gridSpacing", m_gridSpacing);
    m_settings->setValue("orthoMode", m_orthoMode);
    m_settings->setValue("polarTracking", m_polarTracking);
    m_settings->setValue("dynamicInput", m_dynamicInput);
    
    m_settings->sync();
    qCDebug(cadApp) << "Settings saved";
}

void CADApplication::loadSettings()
{
    qCDebug(cadApp) << "Loading application settings...";
    
    m_currentUnits = static_cast<Units>(m_settings->value("units", static_cast<int>(Units::Millimeters)).toInt());
    m_precision = m_settings->value("precision", 2).toInt();
    m_gridVisible = m_settings->value("gridVisible", true).toBool();
    m_snapEnabled = m_settings->value("snapEnabled", true).toBool();
    m_gridSpacing = m_settings->value("gridSpacing", 10.0).toDouble();
    m_orthoMode = m_settings->value("orthoMode", false).toBool();
    m_polarTracking = m_settings->value("polarTracking", false).toBool();
    m_dynamicInput = m_settings->value("dynamicInput", true).toBool();
    
    qCDebug(cadApp) << "Settings loaded";
}

void CADApplication::resetToDefaults()
{
    qCDebug(cadApp) << "Resetting to default settings...";
    
    m_settings->clear();
    setupDefaultSettings();
    loadSettings();
    
    // Emit signals for UI updates
    emit unitsChanged(m_currentUnits);
    emit precisionChanged(m_precision);
    emit gridVisibilityChanged(m_gridVisible);
    emit snapEnabledChanged(m_snapEnabled);
    emit gridSpacingChanged(m_gridSpacing);
    emit orthoModeChanged(m_orthoMode);
    emit polarTrackingChanged(m_polarTracking);
    emit dynamicInputChanged(m_dynamicInput);
}

void CADApplication::setModified(bool modified)
{
    if (m_isModified != modified) {
        m_isModified = modified;
        emit modifiedChanged(modified);
    }
}

void CADApplication::setCurrentDocument(const QString& path)
{
    if (m_currentDocument != path) {
        m_currentDocument = path;
        emit currentDocumentChanged(path);
    }
}

void CADApplication::setCurrentUnits(Units units)
{
    if (m_currentUnits != units) {
        m_currentUnits = units;
        emit unitsChanged(units);
        setModified(true);
    }
}

void CADApplication::setPrecision(int precision)
{
    if (m_precision != precision) {
        m_precision = precision;
        emit precisionChanged(precision);
        setModified(true);
    }
}

void CADApplication::setGridVisible(bool visible)
{
    if (m_gridVisible != visible) {
        m_gridVisible = visible;
        emit gridVisibilityChanged(visible);
    }
}

void CADApplication::setSnapEnabled(bool enabled)
{
    if (m_snapEnabled != enabled) {
        m_snapEnabled = enabled;
        emit snapEnabledChanged(enabled);
    }
}

void CADApplication::setGridSpacing(double spacing)
{
    if (m_gridSpacing != spacing) {
        m_gridSpacing = spacing;
        emit gridSpacingChanged(spacing);
        setModified(true);
    }
}

void CADApplication::setOrthoMode(bool enabled)
{
    if (m_orthoMode != enabled) {
        m_orthoMode = enabled;
        emit orthoModeChanged(enabled);
    }
}

void CADApplication::setPolarTracking(bool enabled)
{
    if (m_polarTracking != enabled) {
        m_polarTracking = enabled;
        emit polarTrackingChanged(enabled);
    }
}

void CADApplication::setDynamicInput(bool enabled)
{
    if (m_dynamicInput != enabled) {
        m_dynamicInput = enabled;
        emit dynamicInputChanged(enabled);
    }
}

void CADApplication::newDocument()
{
    qCDebug(cadApp) << "Creating new document...";
    
    if (m_isModified) {
        // Ask user to save current document
        int ret = QMessageBox::question(nullptr, "Save Changes", 
                                       "Do you want to save changes to the current document?",
                                       QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        if (ret == QMessageBox::Save) {
            saveDocument();
        } else if (ret == QMessageBox::Cancel) {
            return;
        }
    }
    
    // Clear all managers
    m_layerManager->clear();
    m_blockManager->clear();
    m_xrefManager->clear();
    m_layoutManager->clear();
    
    setCurrentDocument("");
    setModified(false);
    
    qCDebug(cadApp) << "New document created";
}

void CADApplication::openDocument(const QString& path)
{
    QString filePath = path;
    if (filePath.isEmpty()) {
        filePath = QFileDialog::getOpenFileName(nullptr, "Open Document", 
                                               QString(), "CAD Files (*.dwg *.dxf)");
        if (filePath.isEmpty()) {
            return;
        }
    }
    
    qCDebug(cadApp) << "Opening document:" << filePath;
    
    // TODO: Implement document loading
    setCurrentDocument(filePath);
    setModified(false);
}

void CADApplication::saveDocument()
{
    if (m_currentDocument.isEmpty()) {
        saveDocumentAs();
        return;
    }
    
    qCDebug(cadApp) << "Saving document:" << m_currentDocument;
    
    // TODO: Implement document saving
    setModified(false);
}

void CADApplication::saveDocumentAs(const QString& path)
{
    QString filePath = path;
    if (filePath.isEmpty()) {
        filePath = QFileDialog::getSaveFileName(nullptr, "Save Document As", 
                                               QString(), "CAD Files (*.dwg *.dxf)");
        if (filePath.isEmpty()) {
            return;
        }
    }
    
    qCDebug(cadApp) << "Saving document as:" << filePath;
    
    // TODO: Implement document saving
    setCurrentDocument(filePath);
    setModified(false);
}

void CADApplication::closeDocument()
{
    qCDebug(cadApp) << "Closing document...";
    
    if (m_isModified) {
        int ret = QMessageBox::question(nullptr, "Save Changes", 
                                       "Do you want to save changes before closing?",
                                       QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        if (ret == QMessageBox::Save) {
            saveDocument();
        } else if (ret == QMessageBox::Cancel) {
            return;
        }
    }
    
    setCurrentDocument("");
    setModified(false);
}

void CADApplication::onAboutToQuit()
{
    qCDebug(cadApp) << "Application about to quit";
    shutdown();
}
