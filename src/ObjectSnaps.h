#pragma once

#include <QObject>
#include <QPoint>
#include <QLoggingCategory>
#include <memory>

// OpenCASCADE includes
#include <gp_Pnt.hxx>
#include <gp_Vec.hxx>
#include <gp_Dir.hxx>

Q_DECLARE_LOGGING_CATEGORY(cadSnaps)

/**
 * @brief Object snap types
 */
enum class SnapType {
    None,
    Endpoint,
    Midpoint,
    Center,
    Node,
    Quadrant,
    Intersection,
    Extension,
    Tangent,
    Perpendicular,
    Parallel,
    Insert,
    Nearest,
    Apparent,
    // Advanced snaps
    GeometricCenter,
    BoundingBoxCenter,
    DivisionPoint,
    // Temporary snaps
    From,
    MidBetween,
    // Tracking snaps
    PolarTracking,
    ObjectTracking
};

/**
 * @brief Snap result information
 */
struct SnapResult
{
    bool valid;
    SnapType type;
    gp_Pnt point;
    gp_Vec direction;
    int entityId;
    QString description;
    double distance;
    
    SnapResult() : valid(false), type(SnapType::None), entityId(-1), distance(0.0) {}
    
    SnapResult(SnapType snapType, const gp_Pnt& snapPoint, int id = -1)
        : valid(true), type(snapType), point(snapPoint), entityId(id), distance(0.0) {}
};

/**
 * @brief Snap settings and configuration
 */
struct SnapSettings
{
    bool enabled;
    double aperture;           // Snap aperture in pixels
    double tolerance;          // Snap tolerance in drawing units
    bool autoSnap;            // Automatic snap mode
    bool snapTip;             // Show snap tooltips
    bool snapMarker;          // Show snap markers
    bool magneticSnap;        // Magnetic cursor behavior
    
    // Individual snap enables
    bool endpointSnap;
    bool midpointSnap;
    bool centerSnap;
    bool nodeSnap;
    bool quadrantSnap;
    bool intersectionSnap;
    bool extensionSnap;
    bool tangentSnap;
    bool perpendicularSnap;
    bool parallelSnap;
    bool insertSnap;
    bool nearestSnap;
    bool apparentSnap;
    
    // Tracking settings
    bool polarTracking;
    bool objectTracking;
    double polarAngleIncrement;
    QList<double> polarAngles;
    
    SnapSettings()
        : enabled(true)
        , aperture(10.0)
        , tolerance(1.0)
        , autoSnap(true)
        , snapTip(true)
        , snapMarker(true)
        , magneticSnap(true)
        , endpointSnap(true)
        , midpointSnap(true)
        , centerSnap(true)
        , nodeSnap(true)
        , quadrantSnap(true)
        , intersectionSnap(true)
        , extensionSnap(true)
        , tangentSnap(true)
        , perpendicularSnap(true)
        , parallelSnap(true)
        , insertSnap(true)
        , nearestSnap(true)
        , apparentSnap(true)
        , polarTracking(false)
        , objectTracking(false)
        , polarAngleIncrement(90.0)
    {
        // Default polar angles: 0, 90, 180, 270 degrees
        polarAngles << 0.0 << 90.0 << 180.0 << 270.0;
    }
};

/**
 * @brief Comprehensive object snap system
 * 
 * Provides complete AutoCAD-style object snapping including:
 * - All standard snap modes (endpoint, midpoint, center, etc.)
 * - Intersection detection with preview
 * - Extension and parallel snap modes
 * - Polar tracking with custom angles
 * - Object tracking for alignment
 * - Snap overrides and temporary snaps
 * - Visual feedback with markers and tooltips
 * - Magnetic cursor behavior
 * - Snap filtering and priority
 */
class ObjectSnaps : public QObject
{
    Q_OBJECT

public:
    explicit ObjectSnaps(QObject *parent = nullptr);
    ~ObjectSnaps();

    // Settings management
    void setSettings(const SnapSettings& settings);
    SnapSettings getSettings() const { return m_settings; }
    
    void setEnabled(bool enabled);
    bool isEnabled() const { return m_settings.enabled; }
    
    void setAperture(double aperture);
    double getAperture() const { return m_settings.aperture; }
    
    void setTolerance(double tolerance);
    double getTolerance() const { return m_settings.tolerance; }

    // Individual snap mode control
    void setSnapMode(SnapType type, bool enabled);
    bool isSnapModeEnabled(SnapType type) const;
    void enableAllSnaps();
    void disableAllSnaps();
    void resetToDefaults();

    // Snap detection
    SnapResult findSnap(const gp_Pnt& cursorPoint, const QList<int>& entityIds = QList<int>());
    SnapResult findSnapWithOverride(const gp_Pnt& cursorPoint, SnapType override, const QList<int>& entityIds = QList<int>());
    QList<SnapResult> findAllSnaps(const gp_Pnt& cursorPoint, const QList<int>& entityIds = QList<int>());
    
    // Specific snap detection methods
    SnapResult findEndpointSnap(const gp_Pnt& cursorPoint, int entityId);
    SnapResult findMidpointSnap(const gp_Pnt& cursorPoint, int entityId);
    SnapResult findCenterSnap(const gp_Pnt& cursorPoint, int entityId);
    SnapResult findQuadrantSnap(const gp_Pnt& cursorPoint, int entityId);
    SnapResult findIntersectionSnap(const gp_Pnt& cursorPoint, const QList<int>& entityIds);
    SnapResult findTangentSnap(const gp_Pnt& cursorPoint, int entityId, const gp_Pnt& referencePoint);
    SnapResult findPerpendicularSnap(const gp_Pnt& cursorPoint, int entityId, const gp_Pnt& referencePoint);
    SnapResult findNearestSnap(const gp_Pnt& cursorPoint, int entityId);

    // Tracking and alignment
    SnapResult findPolarTracking(const gp_Pnt& cursorPoint, const gp_Pnt& basePoint);
    SnapResult findObjectTracking(const gp_Pnt& cursorPoint, const QList<gp_Pnt>& trackingPoints);
    void addTrackingPoint(const gp_Pnt& point);
    void clearTrackingPoints();
    QList<gp_Pnt> getTrackingPoints() const { return m_trackingPoints; }

    // Polar tracking settings
    void setPolarTracking(bool enabled);
    bool isPolarTrackingEnabled() const { return m_settings.polarTracking; }
    
    void setPolarAngleIncrement(double increment);
    double getPolarAngleIncrement() const { return m_settings.polarAngleIncrement; }
    
    void setPolarAngles(const QList<double>& angles);
    QList<double> getPolarAngles() const { return m_settings.polarAngles; }
    
    void addPolarAngle(double angle);
    void removePolarAngle(double angle);

    // Object tracking settings
    void setObjectTracking(bool enabled);
    bool isObjectTrackingEnabled() const { return m_settings.objectTracking; }

    // Temporary snap overrides
    void setTemporarySnapOverride(SnapType type);
    void clearTemporarySnapOverride();
    SnapType getTemporarySnapOverride() const { return m_temporaryOverride; }

    // Snap filtering
    void setSnapFilter(const QList<SnapType>& allowedTypes);
    void clearSnapFilter();
    QList<SnapType> getSnapFilter() const { return m_snapFilter; }

    // Visual feedback
    void setSnapMarkerVisible(bool visible);
    bool isSnapMarkerVisible() const { return m_settings.snapMarker; }
    
    void setSnapTooltipVisible(bool visible);
    bool isSnapTooltipVisible() const { return m_settings.snapTip; }

    // Utility functions
    QString getSnapTypeName(SnapType type) const;
    QString getSnapTypeDescription(SnapType type) const;
    QIcon getSnapTypeIcon(SnapType type) const;
    
    double distanceToPoint(const gp_Pnt& point1, const gp_Pnt& point2) const;
    bool isPointInAperture(const gp_Pnt& point, const gp_Pnt& cursor) const;
    
    // Snap priority (lower number = higher priority)
    int getSnapPriority(SnapType type) const;
    void setSnapPriority(SnapType type, int priority);

signals:
    void snapFound(const SnapResult& result);
    void snapLost();
    void settingsChanged();
    void trackingPointAdded(const gp_Pnt& point);
    void trackingPointsCleared();

public slots:
    void onEntityAdded(int entityId);
    void onEntityRemoved(int entityId);
    void onEntityModified(int entityId);

private:
    struct SnapPriority {
        SnapType type;
        int priority;
    };

    void initializeSnapPriorities();
    void updateSnapCache();
    bool isSnapTypeEnabled(SnapType type) const;
    SnapResult getBestSnap(const QList<SnapResult>& snaps, const gp_Pnt& cursorPoint) const;
    
    // Geometric calculations
    gp_Pnt projectPointOnLine(const gp_Pnt& point, const gp_Pnt& lineStart, const gp_Pnt& lineEnd) const;
    gp_Pnt findLineIntersection(const gp_Pnt& line1Start, const gp_Pnt& line1End,
                               const gp_Pnt& line2Start, const gp_Pnt& line2End) const;
    QList<gp_Pnt> findCircleLineIntersections(const gp_Pnt& center, double radius,
                                             const gp_Pnt& lineStart, const gp_Pnt& lineEnd) const;
    QList<gp_Pnt> findCircleCircleIntersections(const gp_Pnt& center1, double radius1,
                                               const gp_Pnt& center2, double radius2) const;
    
    // Angle calculations
    double normalizeAngle(double angle) const;
    double getAngleBetweenPoints(const gp_Pnt& point1, const gp_Pnt& point2) const;
    bool isAngleInPolarSet(double angle) const;
    double getClosestPolarAngle(double angle) const;

    SnapSettings m_settings;
    SnapType m_temporaryOverride;
    QList<SnapType> m_snapFilter;
    QList<gp_Pnt> m_trackingPoints;
    QList<SnapPriority> m_snapPriorities;
    
    // Cache for performance
    QList<int> m_cachedEntityIds;
    bool m_cacheValid;
    
    // Current snap state
    SnapResult m_lastSnapResult;
    bool m_snapActive;
};
