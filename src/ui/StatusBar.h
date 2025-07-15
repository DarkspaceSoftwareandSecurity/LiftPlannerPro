#pragma once

#include <QStatusBar>
#include <QLoggingCategory>
#include <memory>

class QLabel;
class QToolButton;
class QProgressBar;
class QComboBox;
class QSpinBox;

Q_DECLARE_LOGGING_CATEGORY(cadStatusBar)

/**
 * @brief Comprehensive AutoCAD-style status bar
 * 
 * Provides complete status information including:
 * - Coordinate display (X, Y, Z)
 * - Drawing mode toggles (Snap, Grid, Ortho, Polar, OSnap, OTrack, DUcs, Dyn, LWT, Transparency)
 * - Annotation scale
 * - Visual style indicator
 * - Layout/Model space indicator
 * - Progress indication for long operations
 * - Command status and prompts
 */
class CADStatusBar : public QStatusBar
{
    Q_OBJECT

public:
    explicit CADStatusBar(QWidget* parent = nullptr);
    ~CADStatusBar();

    // Coordinate display
    void setCoordinates(double x, double y, double z = 0.0);
    void setCoordinateFormat(const QString& format);
    void setCoordinatePrecision(int precision);

    // Mode toggles
    void setSnapMode(bool enabled);
    bool isSnapMode() const;
    
    void setGridMode(bool enabled);
    bool isGridMode() const;
    
    void setOrthoMode(bool enabled);
    bool isOrthoMode() const;
    
    void setPolarMode(bool enabled);
    bool isPolarMode() const;
    
    void setOSnapMode(bool enabled);
    bool isOSnapMode() const;
    
    void setOTrackMode(bool enabled);
    bool isOTrackMode() const;
    
    void setDUcsMode(bool enabled);
    bool isDUcsMode() const;
    
    void setDynMode(bool enabled);
    bool isDynMode() const;
    
    void setLwtMode(bool enabled);
    bool isLwtMode() const;
    
    void setTransparencyMode(bool enabled);
    bool isTransparencyMode() const;

    // Scale and units
    void setAnnotationScale(const QString& scale);
    QString getAnnotationScale() const;
    
    void setUnits(const QString& units);
    QString getUnits() const;

    // Visual style
    void setVisualStyle(const QString& style);
    QString getVisualStyle() const;

    // Layout/Model space
    void setLayoutMode(bool layout);
    bool isLayoutMode() const;

    // Status messages
    void showMessage(const QString& message, int timeout = 0);
    void showPermanentMessage(const QString& message);
    void clearMessage();

    // Progress indication
    void showProgress(const QString& operation, int maximum = 100);
    void updateProgress(int value);
    void hideProgress();

    // Command status
    void setCommandPrompt(const QString& prompt);
    void clearCommandPrompt();

signals:
    void snapModeToggled(bool enabled);
    void gridModeToggled(bool enabled);
    void orthoModeToggled(bool enabled);
    void polarModeToggled(bool enabled);
    void oSnapModeToggled(bool enabled);
    void oTrackModeToggled(bool enabled);
    void dUcsModeToggled(bool enabled);
    void dynModeToggled(bool enabled);
    void lwtModeToggled(bool enabled);
    void transparencyModeToggled(bool enabled);
    
    void annotationScaleChanged(const QString& scale);
    void visualStyleChanged(const QString& style);
    void layoutModeChanged(bool layout);
    
    void coordinateClicked();

private slots:
    void onModeButtonClicked();
    void onAnnotationScaleChanged();
    void onVisualStyleChanged();
    void onLayoutButtonClicked();
    void onCoordinateClicked();

private:
    void setupUI();
    void createCoordinateDisplay();
    void createModeToggles();
    void createScaleControls();
    void createLayoutControls();
    void createProgressIndicator();
    void updateModeButtonStyles();
    QToolButton* createModeButton(const QString& text, const QString& tooltip, const QString& icon);

    // Coordinate display
    QLabel* m_coordinateLabel;
    QString m_coordinateFormat;
    int m_coordinatePrecision;
    double m_currentX, m_currentY, m_currentZ;

    // Mode toggle buttons
    QToolButton* m_snapButton;
    QToolButton* m_gridButton;
    QToolButton* m_orthoButton;
    QToolButton* m_polarButton;
    QToolButton* m_oSnapButton;
    QToolButton* m_oTrackButton;
    QToolButton* m_dUcsButton;
    QToolButton* m_dynButton;
    QToolButton* m_lwtButton;
    QToolButton* m_transparencyButton;

    // Scale and style controls
    QComboBox* m_annotationScaleCombo;
    QComboBox* m_visualStyleCombo;
    QLabel* m_unitsLabel;

    // Layout controls
    QToolButton* m_layoutButton;
    QLabel* m_layoutLabel;

    // Progress indicator
    QProgressBar* m_progressBar;
    QLabel* m_progressLabel;

    // Status message
    QLabel* m_statusLabel;
    QLabel* m_commandPromptLabel;

    // State
    bool m_snapMode;
    bool m_gridMode;
    bool m_orthoMode;
    bool m_polarMode;
    bool m_oSnapMode;
    bool m_oTrackMode;
    bool m_dUcsMode;
    bool m_dynMode;
    bool m_lwtMode;
    bool m_transparencyMode;
    bool m_layoutMode;
    
    QString m_currentUnits;
    QString m_currentVisualStyle;
    QString m_currentAnnotationScale;
};

/**
 * @brief Custom coordinate display widget
 */
class CoordinateDisplay : public QWidget
{
    Q_OBJECT

public:
    explicit CoordinateDisplay(QWidget* parent = nullptr);
    
    void setCoordinates(double x, double y, double z = 0.0);
    void setFormat(const QString& format);
    void setPrecision(int precision);
    void setUnits(const QString& units);

signals:
    void clicked();

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;

private:
    void updateDisplay();

    double m_x, m_y, m_z;
    QString m_format;
    QString m_units;
    int m_precision;
    QString m_displayText;
};

/**
 * @brief Mode toggle button with enhanced styling
 */
class ModeToggleButton : public QToolButton
{
    Q_OBJECT

public:
    explicit ModeToggleButton(const QString& text, const QString& tooltip, QWidget* parent = nullptr);
    
    void setModeEnabled(bool enabled);
    bool isModeEnabled() const { return m_modeEnabled; }

protected:
    void paintEvent(QPaintEvent* event) override;
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;

private:
    void updateStyle();

    bool m_modeEnabled;
    bool m_hovered;
};
