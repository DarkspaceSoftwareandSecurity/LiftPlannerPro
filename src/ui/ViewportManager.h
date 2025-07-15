#pragma once

#include <QWidget>
#include <QLoggingCategory>
#include <memory>

class QSplitter;
class QTabWidget;
class QOpenGLWidget;
class QVBoxLayout;
class QHBoxLayout;
class QLabel;
class QToolButton;

Q_DECLARE_LOGGING_CATEGORY(cadViewport)

/**
 * @brief Individual viewport widget for 2D/3D display
 */
class CADViewport : public QOpenGLWidget
{
    Q_OBJECT

public:
    enum ViewType {
        Top,
        Front,
        Right,
        Bottom,
        Back,
        Left,
        SWIsometric,
        SEIsometric,
        NEIsometric,
        NWIsometric,
        Current,
        Custom
    };

    enum ViewMode {
        Wireframe,
        Hidden,
        Shaded,
        Realistic,
        Conceptual
    };

    explicit CADViewport(QWidget* parent = nullptr);
    ~CADViewport();

    // View management
    void setViewType(ViewType type);
    ViewType getViewType() const { return m_viewType; }
    
    void setViewMode(ViewMode mode);
    ViewMode getViewMode() const { return m_viewMode; }
    
    void setViewName(const QString& name) { m_viewName = name; }
    QString getViewName() const { return m_viewName; }

    // Navigation
    void zoomExtents();
    void zoomWindow();
    void zoomPrevious();
    void zoomIn();
    void zoomOut();
    void pan(const QPoint& delta);
    void orbit(const QPoint& delta);
    void resetView();

    // Display properties
    void setGridVisible(bool visible);
    bool isGridVisible() const { return m_gridVisible; }
    
    void setAxisVisible(bool visible);
    bool isAxisVisible() const { return m_axisVisible; }
    
    void setBackgroundColor(const QColor& color);
    QColor getBackgroundColor() const { return m_backgroundColor; }

    // Selection
    void setSelectionMode(bool enabled) { m_selectionMode = enabled; }
    bool isSelectionMode() const { return m_selectionMode; }

    // Active state
    void setActive(bool active);
    bool isActive() const { return m_active; }

signals:
    void viewChanged();
    void selectionChanged();
    void contextMenuRequested(const QPoint& position);

protected:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int width, int height) override;
    
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void contextMenuEvent(QContextMenuEvent* event) override;

private:
    void setupOpenGL();
    void drawGrid();
    void drawAxis();
    void drawViewCube();
    void updateProjection();
    void updateView();

    ViewType m_viewType;
    ViewMode m_viewMode;
    QString m_viewName;
    
    bool m_gridVisible;
    bool m_axisVisible;
    bool m_selectionMode;
    bool m_active;
    
    QColor m_backgroundColor;
    
    // Navigation state
    QPoint m_lastMousePos;
    bool m_panning;
    bool m_orbiting;
    bool m_zooming;
    
    // View parameters
    float m_zoom;
    float m_panX, m_panY;
    float m_rotationX, m_rotationY;
    
    // OpenGL resources
    unsigned int m_gridVBO;
    unsigned int m_axisVBO;
    unsigned int m_shaderProgram;
};

/**
 * @brief Manages multiple viewports with layout options
 */
class ViewportManager : public QWidget
{
    Q_OBJECT

public:
    enum LayoutType {
        Single,
        TwoHorizontal,
        TwoVertical,
        ThreeLeft,
        ThreeRight,
        ThreeTop,
        ThreeBottom,
        Four
    };

    explicit ViewportManager(QWidget* parent = nullptr);
    ~ViewportManager();

    // Viewport management
    CADViewport* addViewport(const QString& name = QString());
    void removeViewport(int index);
    CADViewport* getViewport(int index) const;
    CADViewport* getCurrentViewport() const;
    int getViewportCount() const;
    
    void setCurrentViewport(int index);
    int getCurrentViewportIndex() const { return m_currentViewportIndex; }

    // Layout management
    void setLayout(LayoutType layout);
    LayoutType getLayout() const { return m_layoutType; }
    
    void maximizeCurrentViewport();
    void restoreViewports();
    void tileViewports();
    void cascadeViewports();

    // View operations (applied to current viewport)
    void zoomExtents();
    void zoomWindow();
    void zoomPrevious();
    void setPanMode(bool enabled);
    void setOrbitMode(bool enabled);

    // Display settings
    void setGridVisible(bool visible);
    void setAxisVisible(bool visible);
    void setViewMode(CADViewport::ViewMode mode);

    // Named views
    void saveView(const QString& name);
    void restoreView(const QString& name);
    QStringList getSavedViews() const;

signals:
    void viewportChanged(int index);
    void viewportAdded(int index);
    void viewportRemoved(int index);
    void layoutChanged(LayoutType layout);

private slots:
    void onViewportClicked();
    void onViewportContextMenu(const QPoint& position);

private:
    void setupUI();
    void updateLayout();
    void createDefaultViewports();
    void connectViewportSignals(CADViewport* viewport);
    void updateViewportBorders();

    QVBoxLayout* m_mainLayout;
    QSplitter* m_mainSplitter;
    QSplitter* m_horizontalSplitter;
    QSplitter* m_verticalSplitter;
    
    std::vector<std::unique_ptr<CADViewport>> m_viewports;
    int m_currentViewportIndex;
    LayoutType m_layoutType;
    bool m_maximized;
    
    // Saved views
    struct SavedView {
        QString name;
        CADViewport::ViewType type;
        CADViewport::ViewMode mode;
        float zoom;
        float panX, panY;
        float rotationX, rotationY;
    };
    
    std::vector<SavedView> m_savedViews;
    
    // UI state
    bool m_panMode;
    bool m_orbitMode;
};
