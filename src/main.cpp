#include <QApplication>
#include <QStyleFactory>
#include <QDir>
#include <QStandardPaths>
#include <QLoggingCategory>
#include <QSurfaceFormat>
#include "CADApplication.h"
#include "MainWindow.h"

Q_LOGGING_CATEGORY(cadMain, "cad.main")

void setupOpenGL()
{
    // Configure OpenGL format for high-performance rendering
    QSurfaceFormat format;
    format.setDepthBufferSize(24);
    format.setStencilBufferSize(8);
    format.setVersion(4, 5);
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setSamples(4); // 4x MSAA
    format.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
    format.setSwapInterval(1); // VSync
    QSurfaceFormat::setDefaultFormat(format);
}

void setupLogging()
{
    // Enable comprehensive debugging output
    QLoggingCategory::setFilterRules("cad.*=true");
    qCDebug(cadMain) << "CAD Application starting with comprehensive debugging enabled";
}

void setupApplicationProperties(QApplication& app)
{
    app.setApplicationName("AutoCAD Clone");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("Darkspace Software and Security");
    app.setOrganizationDomain("darkspacesoftwareandsecurity.com");
    
    // Set application icon
    app.setWindowIcon(QIcon(":/icons/app_icon.png"));
    
    // Apply dark theme styling
    app.setStyle(QStyleFactory::create("Fusion"));
    
    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::WindowText, Qt::white);
    darkPalette.setColor(QPalette::Base, QColor(25, 25, 25));
    darkPalette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
    darkPalette.setColor(QPalette::ToolTipText, Qt::white);
    darkPalette.setColor(QPalette::Text, Qt::white);
    darkPalette.setColor(QPalette::Button, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::ButtonText, Qt::white);
    darkPalette.setColor(QPalette::BrightText, Qt::red);
    darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::HighlightedText, Qt::black);
    app.setPalette(darkPalette);
}

void createApplicationDirectories()
{
    // Create necessary application directories
    QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(appDataPath + "/templates");
    QDir().mkpath(appDataPath + "/blocks");
    QDir().mkpath(appDataPath + "/hatches");
    QDir().mkpath(appDataPath + "/materials");
    QDir().mkpath(appDataPath + "/plugins");
    QDir().mkpath(appDataPath + "/workspaces");
    
    qCDebug(cadMain) << "Application directories created at:" << appDataPath;
}

int main(int argc, char *argv[])
{
    // Enable high DPI scaling
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    
    // Setup OpenGL before creating QApplication
    setupOpenGL();
    
    // Create application
    CADApplication app(argc, argv);
    
    // Setup application properties and styling
    setupApplicationProperties(app);
    setupLogging();
    createApplicationDirectories();
    
    // Initialize OpenCASCADE and other systems
    if (!app.initialize()) {
        qCCritical(cadMain) << "Failed to initialize CAD application";
        return -1;
    }
    
    // Create and show main window
    MainWindow window;
    window.show();
    
    qCDebug(cadMain) << "CAD Application started successfully";
    qCDebug(cadMain) << "OpenGL Version:" << QOpenGLContext::currentContext()->format().version();
    qCDebug(cadMain) << "Qt Version:" << QT_VERSION_STR;
    
    return app.exec();
}
