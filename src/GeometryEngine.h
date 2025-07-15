#pragma once

#include <QObject>
#include <QLoggingCategory>
#include <memory>
#include <vector>

// OpenCASCADE includes
#include <TopoDS_Shape.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Vertex.hxx>
#include <TopoDS_Solid.hxx>
#include <gp_Pnt.hxx>
#include <gp_Vec.hxx>
#include <gp_Dir.hxx>
#include <gp_Ax1.hxx>
#include <gp_Ax2.hxx>
#include <gp_Pln.hxx>
#include <gp_Circ.hxx>
#include <gp_Elips.hxx>
#include <Handle_AIS_InteractiveObject.hxx>

Q_DECLARE_LOGGING_CATEGORY(cadGeometry)

class AIS_InteractiveContext;
class V3d_Viewer;
class V3d_View;

/**
 * @brief Geometry data structure for CAD entities
 */
struct CADEntity
{
    enum Type {
        Point,
        Line,
        Circle,
        Arc,
        Ellipse,
        Polyline,
        Spline,
        Rectangle,
        Polygon,
        Text,
        Dimension,
        Hatch,
        Block,
        // 3D entities
        Box,
        Sphere,
        Cylinder,
        Cone,
        Torus,
        Wedge,
        Surface,
        Solid
    };
    
    Type type;
    TopoDS_Shape shape;
    Handle(AIS_InteractiveObject) aisObject;
    QString layer;
    int color;
    int lineType;
    double lineWeight;
    bool visible;
    bool selected;
    
    // Entity-specific data
    QVariantMap properties;
    
    CADEntity() : type(Point), color(7), lineType(0), lineWeight(0.25), visible(true), selected(false) {}
};

/**
 * @brief OpenCASCADE-based geometry engine
 * 
 * Provides comprehensive 3D geometry operations including:
 * - 2D primitive creation (lines, circles, arcs, etc.)
 * - 3D solid modeling (extrude, revolve, boolean operations)
 * - Surface modeling and mesh operations
 * - Geometric analysis and measurement
 * - Import/export of standard CAD formats
 * - Visualization and rendering support
 */
class GeometryEngine : public QObject
{
    Q_OBJECT

public:
    explicit GeometryEngine(QObject *parent = nullptr);
    ~GeometryEngine();

    // Initialization
    bool initialize();
    void shutdown();

    // Viewer management
    Handle(V3d_Viewer) getViewer() const { return m_viewer; }
    Handle(AIS_InteractiveContext) getContext() const { return m_context; }

    // Entity management
    int addEntity(const CADEntity& entity);
    bool removeEntity(int id);
    bool updateEntity(int id, const CADEntity& entity);
    CADEntity getEntity(int id) const;
    std::vector<int> getAllEntityIds() const;
    void clearAllEntities();

    // 2D Primitive creation
    int createPoint(const gp_Pnt& point);
    int createLine(const gp_Pnt& start, const gp_Pnt& end);
    int createPolyline(const std::vector<gp_Pnt>& points, bool closed = false);
    int createCircle(const gp_Pnt& center, double radius, const gp_Dir& normal = gp_Dir(0, 0, 1));
    int createCircle(const gp_Pnt& center, double radius, const gp_Pnt& point);
    int createCircle(const gp_Pnt& p1, const gp_Pnt& p2, const gp_Pnt& p3);
    int createArc(const gp_Pnt& center, double radius, double startAngle, double endAngle);
    int createArc(const gp_Pnt& start, const gp_Pnt& end, const gp_Pnt& point);
    int createEllipse(const gp_Pnt& center, double majorRadius, double minorRadius, const gp_Dir& normal = gp_Dir(0, 0, 1));
    int createRectangle(const gp_Pnt& corner1, const gp_Pnt& corner2);
    int createPolygon(const std::vector<gp_Pnt>& vertices);
    int createSpline(const std::vector<gp_Pnt>& points, bool closed = false);

    // 3D Primitive creation
    int createBox(const gp_Pnt& corner, double dx, double dy, double dz);
    int createSphere(const gp_Pnt& center, double radius);
    int createCylinder(const gp_Pnt& center, const gp_Dir& axis, double radius, double height);
    int createCone(const gp_Pnt& center, const gp_Dir& axis, double radius1, double radius2, double height);
    int createTorus(const gp_Pnt& center, const gp_Dir& axis, double majorRadius, double minorRadius);
    int createWedge(const gp_Pnt& corner, double dx, double dy, double dz, double ltx = 0.0);

    // 3D Operations
    int extrudeProfile(int profileId, const gp_Vec& direction, double distance);
    int extrudeProfile(int profileId, const gp_Pnt& point1, const gp_Pnt& point2);
    int revolveProfile(int profileId, const gp_Ax1& axis, double angle);
    int sweepProfile(int profileId, int pathId);
    int loftProfiles(const std::vector<int>& profileIds, bool solid = true);

    // Boolean operations
    int booleanUnion(int entity1Id, int entity2Id);
    int booleanSubtract(int entity1Id, int entity2Id);
    int booleanIntersect(int entity1Id, int entity2Id);
    int booleanCut(int entityId, const gp_Pln& plane);

    // Modification operations
    bool moveEntity(int entityId, const gp_Vec& translation);
    bool rotateEntity(int entityId, const gp_Ax1& axis, double angle);
    bool scaleEntity(int entityId, const gp_Pnt& center, double factor);
    bool mirrorEntity(int entityId, const gp_Ax2& plane);
    bool copyEntity(int entityId, const gp_Vec& translation);
    bool arrayEntity(int entityId, int countX, int countY, double spacingX, double spacingY);
    bool polarArrayEntity(int entityId, const gp_Pnt& center, int count, double angle);

    // Editing operations
    bool trimEntity(int entityId, const gp_Pnt& point1, const gp_Pnt& point2);
    bool extendEntity(int entityId, int boundaryId);
    bool filletEdges(int entityId, const std::vector<TopoDS_Edge>& edges, double radius);
    bool chamferEdges(int entityId, const std::vector<TopoDS_Edge>& edges, double distance);
    bool offsetEntity(int entityId, double distance, bool bothSides = false);

    // Analysis and measurement
    double getDistance(const gp_Pnt& point1, const gp_Pnt& point2);
    double getAngle(const gp_Vec& vec1, const gp_Vec& vec2);
    double getArea(int entityId);
    double getVolume(int entityId);
    double getLength(int entityId);
    gp_Pnt getCentroid(int entityId);
    std::pair<gp_Pnt, gp_Pnt> getBoundingBox(int entityId);

    // Selection and snapping
    std::vector<int> selectEntitiesInWindow(const gp_Pnt& corner1, const gp_Pnt& corner2);
    std::vector<int> selectEntitiesAtPoint(const gp_Pnt& point, double tolerance = 1.0);
    gp_Pnt snapToEntity(int entityId, const gp_Pnt& point, const QString& snapMode);
    std::vector<gp_Pnt> getSnapPoints(int entityId, const QString& snapMode);

    // Layer management
    void setEntityLayer(int entityId, const QString& layer);
    QString getEntityLayer(int entityId) const;
    void setLayerVisible(const QString& layer, bool visible);
    void setLayerColor(const QString& layer, int color);

    // Display properties
    void setEntityColor(int entityId, int color);
    void setEntityLineType(int entityId, int lineType);
    void setEntityLineWeight(int entityId, double weight);
    void setEntityVisible(int entityId, bool visible);
    void setEntitySelected(int entityId, bool selected);

    // Import/Export
    bool importSTEP(const QString& filename);
    bool exportSTEP(const QString& filename, const std::vector<int>& entityIds = {});
    bool importIGES(const QString& filename);
    bool exportIGES(const QString& filename, const std::vector<int>& entityIds = {});
    bool importBREP(const QString& filename);
    bool exportBREP(const QString& filename, const std::vector<int>& entityIds = {});

    // Utility functions
    TopoDS_Shape createShapeFromPoints(const std::vector<gp_Pnt>& points, bool closed = false);
    std::vector<gp_Pnt> getPointsFromShape(const TopoDS_Shape& shape);
    bool isValidShape(const TopoDS_Shape& shape);
    void updateDisplay();

signals:
    void entityAdded(int entityId);
    void entityRemoved(int entityId);
    void entityModified(int entityId);
    void selectionChanged(const std::vector<int>& selectedIds);

private:
    void initializeOpenCASCADE();
    void setupViewer();
    void setupContext();
    
    int getNextEntityId();
    Handle(AIS_InteractiveObject) createAISObject(const CADEntity& entity);
    void updateAISObject(int entityId);

    // OpenCASCADE objects
    Handle(V3d_Viewer) m_viewer;
    Handle(AIS_InteractiveContext) m_context;

    // Entity storage
    std::map<int, CADEntity> m_entities;
    int m_nextEntityId;

    // Layer management
    std::map<QString, std::vector<int>> m_layers;
    std::map<QString, bool> m_layerVisibility;
    std::map<QString, int> m_layerColors;

    bool m_initialized;
};
