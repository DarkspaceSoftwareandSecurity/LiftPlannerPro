#include "GeometryEngine.h"

// OpenCASCADE includes
#include <V3d_Viewer.hxx>
#include <V3d_View.hxx>
#include <AIS_InteractiveContext.hxx>
#include <AIS_Shape.hxx>
#include <AIS_Point.hxx>
#include <OpenGl_GraphicDriver.hxx>
#include <Aspect_Handle.hxx>
#include <Aspect_DisplayConnection.hxx>

// Geometry creation
#include <BRepBuilderAPI_MakeVertex.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakePolygon.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepPrimAPI_MakeSphere.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <BRepPrimAPI_MakeCone.hxx>
#include <BRepPrimAPI_MakeTorus.hxx>
#include <BRepPrimAPI_MakeWedge.hxx>
#include <BRepPrimAPI_MakePrism.hxx>
#include <BRepPrimAPI_MakeRevol.hxx>

// Boolean operations
#include <BRepAlgoAPI_Fuse.hxx>
#include <BRepAlgoAPI_Cut.hxx>
#include <BRepAlgoAPI_Common.hxx>
#include <BRepAlgoAPI_Section.hxx>

// Geometric primitives
#include <GC_MakeCircle.hxx>
#include <GC_MakeArcOfCircle.hxx>
#include <GC_MakeEllipse.hxx>
#include <GC_MakeSegment.hxx>

// Geometric analysis
#include <GProp_GProps.hxx>
#include <BRepGProp.hxx>
#include <Bnd_Box.hxx>
#include <BRepBndLib.hxx>

// Import/Export
#include <STEPCAFControl_Reader.hxx>
#include <STEPCAFControl_Writer.hxx>
#include <IGESControl_Reader.hxx>
#include <IGESControl_Writer.hxx>
#include <BRepTools.hxx>

#include <QDebug>

Q_LOGGING_CATEGORY(cadGeometry, "cad.geometry")

GeometryEngine::GeometryEngine(QObject *parent)
    : QObject(parent)
    , m_nextEntityId(1)
    , m_initialized(false)
{
    qCDebug(cadGeometry) << "Geometry engine created";
}

GeometryEngine::~GeometryEngine()
{
    qCDebug(cadGeometry) << "Geometry engine destroyed";
    shutdown();
}

bool GeometryEngine::initialize()
{
    qCDebug(cadGeometry) << "Initializing geometry engine...";
    
    try {
        initializeOpenCASCADE();
        setupViewer();
        setupContext();
        
        m_initialized = true;
        qCDebug(cadGeometry) << "Geometry engine initialized successfully";
        return true;
    }
    catch (const std::exception& e) {
        qCCritical(cadGeometry) << "Failed to initialize geometry engine:" << e.what();
        return false;
    }
}

void GeometryEngine::shutdown()
{
    if (!m_initialized) {
        return;
    }
    
    qCDebug(cadGeometry) << "Shutting down geometry engine...";
    
    clearAllEntities();
    
    if (!m_context.IsNull()) {
        m_context.Nullify();
    }
    
    if (!m_viewer.IsNull()) {
        m_viewer.Nullify();
    }
    
    m_initialized = false;
    qCDebug(cadGeometry) << "Geometry engine shutdown complete";
}

void GeometryEngine::initializeOpenCASCADE()
{
    qCDebug(cadGeometry) << "Initializing OpenCASCADE...";
    
    // Initialize display connection
    Handle(Aspect_DisplayConnection) displayConnection = new Aspect_DisplayConnection();
    
    // Create graphics driver
    Handle(OpenGl_GraphicDriver) graphicDriver = new OpenGl_GraphicDriver(displayConnection);
    
    qCDebug(cadGeometry) << "OpenCASCADE initialized";
}

void GeometryEngine::setupViewer()
{
    qCDebug(cadGeometry) << "Setting up 3D viewer...";
    
    // Create viewer
    m_viewer = new V3d_Viewer(new OpenGl_GraphicDriver(new Aspect_DisplayConnection()));
    
    // Configure viewer
    m_viewer->SetDefaultLights();
    m_viewer->SetLightOn();
    
    qCDebug(cadGeometry) << "3D viewer setup complete";
}

void GeometryEngine::setupContext()
{
    qCDebug(cadGeometry) << "Setting up interactive context...";
    
    // Create interactive context
    m_context = new AIS_InteractiveContext(m_viewer);
    
    // Configure context
    m_context->SetDisplayMode(AIS_Shaded, Standard_True);
    m_context->SetHilightColor(Quantity_NOC_CYAN1);
    m_context->SetSelectionColor(Quantity_NOC_YELLOW);
    
    qCDebug(cadGeometry) << "Interactive context setup complete";
}

int GeometryEngine::getNextEntityId()
{
    return m_nextEntityId++;
}

int GeometryEngine::addEntity(const CADEntity& entity)
{
    int id = getNextEntityId();
    m_entities[id] = entity;
    
    // Create AIS object if shape is valid
    if (!entity.shape.IsNull()) {
        Handle(AIS_InteractiveObject) aisObject = createAISObject(entity);
        if (!aisObject.IsNull()) {
            m_entities[id].aisObject = aisObject;
            m_context->Display(aisObject, Standard_False);
        }
    }
    
    // Add to layer
    if (!entity.layer.isEmpty()) {
        m_layers[entity.layer].push_back(id);
    }
    
    qCDebug(cadGeometry) << "Entity added with ID:" << id;
    emit entityAdded(id);
    
    return id;
}

bool GeometryEngine::removeEntity(int id)
{
    auto it = m_entities.find(id);
    if (it == m_entities.end()) {
        qCWarning(cadGeometry) << "Entity not found:" << id;
        return false;
    }
    
    // Remove from display
    if (!it->second.aisObject.IsNull()) {
        m_context->Remove(it->second.aisObject, Standard_False);
    }
    
    // Remove from layer
    if (!it->second.layer.isEmpty()) {
        auto& layerEntities = m_layers[it->second.layer];
        layerEntities.erase(std::remove(layerEntities.begin(), layerEntities.end(), id), layerEntities.end());
    }
    
    m_entities.erase(it);
    
    qCDebug(cadGeometry) << "Entity removed:" << id;
    emit entityRemoved(id);
    
    return true;
}

bool GeometryEngine::updateEntity(int id, const CADEntity& entity)
{
    auto it = m_entities.find(id);
    if (it == m_entities.end()) {
        qCWarning(cadGeometry) << "Entity not found:" << id;
        return false;
    }
    
    // Remove old AIS object
    if (!it->second.aisObject.IsNull()) {
        m_context->Remove(it->second.aisObject, Standard_False);
    }
    
    // Update entity
    it->second = entity;
    
    // Create new AIS object
    if (!entity.shape.IsNull()) {
        Handle(AIS_InteractiveObject) aisObject = createAISObject(entity);
        if (!aisObject.IsNull()) {
            it->second.aisObject = aisObject;
            m_context->Display(aisObject, Standard_False);
        }
    }
    
    qCDebug(cadGeometry) << "Entity updated:" << id;
    emit entityModified(id);
    
    return true;
}

CADEntity GeometryEngine::getEntity(int id) const
{
    auto it = m_entities.find(id);
    if (it != m_entities.end()) {
        return it->second;
    }
    return CADEntity();
}

std::vector<int> GeometryEngine::getAllEntityIds() const
{
    std::vector<int> ids;
    for (const auto& pair : m_entities) {
        ids.push_back(pair.first);
    }
    return ids;
}

void GeometryEngine::clearAllEntities()
{
    qCDebug(cadGeometry) << "Clearing all entities...";
    
    // Remove all from display
    for (const auto& pair : m_entities) {
        if (!pair.second.aisObject.IsNull()) {
            m_context->Remove(pair.second.aisObject, Standard_False);
        }
    }
    
    m_entities.clear();
    m_layers.clear();
    m_nextEntityId = 1;
    
    qCDebug(cadGeometry) << "All entities cleared";
}

// 2D Primitive creation
int GeometryEngine::createPoint(const gp_Pnt& point)
{
    qCDebug(cadGeometry) << "Creating point at:" << point.X() << point.Y() << point.Z();
    
    CADEntity entity;
    entity.type = CADEntity::Point;
    entity.shape = BRepBuilderAPI_MakeVertex(point).Vertex();
    
    return addEntity(entity);
}

int GeometryEngine::createLine(const gp_Pnt& start, const gp_Pnt& end)
{
    qCDebug(cadGeometry) << "Creating line from:" << start.X() << start.Y() << start.Z() 
                        << "to:" << end.X() << end.Y() << end.Z();
    
    CADEntity entity;
    entity.type = CADEntity::Line;
    entity.shape = BRepBuilderAPI_MakeEdge(start, end).Edge();
    
    return addEntity(entity);
}

// 3D Primitive creation
int GeometryEngine::createBox(const gp_Pnt& corner, double dx, double dy, double dz)
{
    qCDebug(cadGeometry) << "Creating box at:" << corner.X() << corner.Y() << corner.Z()
                        << "size:" << dx << dy << dz;

    CADEntity entity;
    entity.type = CADEntity::Box;
    entity.shape = BRepPrimAPI_MakeBox(corner, dx, dy, dz).Solid();

    return addEntity(entity);
}

int GeometryEngine::createSphere(const gp_Pnt& center, double radius)
{
    qCDebug(cadGeometry) << "Creating sphere at:" << center.X() << center.Y() << center.Z()
                        << "radius:" << radius;

    CADEntity entity;
    entity.type = CADEntity::Sphere;
    entity.shape = BRepPrimAPI_MakeSphere(center, radius).Solid();

    return addEntity(entity);
}

int GeometryEngine::createCylinder(const gp_Pnt& center, const gp_Dir& axis, double radius, double height)
{
    qCDebug(cadGeometry) << "Creating cylinder at:" << center.X() << center.Y() << center.Z()
                        << "radius:" << radius << "height:" << height;

    CADEntity entity;
    entity.type = CADEntity::Cylinder;

    gp_Ax2 ax2(center, axis);
    entity.shape = BRepPrimAPI_MakeCylinder(ax2, radius, height).Solid();

    return addEntity(entity);
}

int GeometryEngine::createCone(const gp_Pnt& center, const gp_Dir& axis, double radius1, double radius2, double height)
{
    qCDebug(cadGeometry) << "Creating cone at:" << center.X() << center.Y() << center.Z()
                        << "radii:" << radius1 << radius2 << "height:" << height;

    CADEntity entity;
    entity.type = CADEntity::Cone;

    gp_Ax2 ax2(center, axis);
    entity.shape = BRepPrimAPI_MakeCone(ax2, radius1, radius2, height).Solid();

    return addEntity(entity);
}

int GeometryEngine::createTorus(const gp_Pnt& center, const gp_Dir& axis, double majorRadius, double minorRadius)
{
    qCDebug(cadGeometry) << "Creating torus at:" << center.X() << center.Y() << center.Z()
                        << "radii:" << majorRadius << minorRadius;

    CADEntity entity;
    entity.type = CADEntity::Torus;

    gp_Ax2 ax2(center, axis);
    entity.shape = BRepPrimAPI_MakeTorus(ax2, majorRadius, minorRadius).Solid();

    return addEntity(entity);
}

// 3D Operations
int GeometryEngine::extrudeProfile(int profileId, const gp_Vec& direction, double distance)
{
    auto it = m_entities.find(profileId);
    if (it == m_entities.end()) {
        qCWarning(cadGeometry) << "Profile entity not found:" << profileId;
        return -1;
    }

    qCDebug(cadGeometry) << "Extruding profile:" << profileId << "distance:" << distance;

    CADEntity entity;
    entity.type = CADEntity::Solid;

    gp_Vec extrudeVec = direction.Normalized() * distance;
    entity.shape = BRepPrimAPI_MakePrism(it->second.shape, extrudeVec).Shape();

    return addEntity(entity);
}

int GeometryEngine::revolveProfile(int profileId, const gp_Ax1& axis, double angle)
{
    auto it = m_entities.find(profileId);
    if (it == m_entities.end()) {
        qCWarning(cadGeometry) << "Profile entity not found:" << profileId;
        return -1;
    }

    qCDebug(cadGeometry) << "Revolving profile:" << profileId << "angle:" << angle;

    CADEntity entity;
    entity.type = CADEntity::Solid;
    entity.shape = BRepPrimAPI_MakeRevol(it->second.shape, axis, angle).Shape();

    return addEntity(entity);
}

// Boolean operations
int GeometryEngine::booleanUnion(int entity1Id, int entity2Id)
{
    auto it1 = m_entities.find(entity1Id);
    auto it2 = m_entities.find(entity2Id);

    if (it1 == m_entities.end() || it2 == m_entities.end()) {
        qCWarning(cadGeometry) << "Entity not found for boolean union";
        return -1;
    }

    qCDebug(cadGeometry) << "Boolean union of entities:" << entity1Id << entity2Id;

    CADEntity entity;
    entity.type = CADEntity::Solid;

    BRepAlgoAPI_Fuse fuse(it1->second.shape, it2->second.shape);
    if (fuse.IsDone()) {
        entity.shape = fuse.Shape();
        return addEntity(entity);
    }

    qCWarning(cadGeometry) << "Boolean union failed";
    return -1;
}

int GeometryEngine::booleanSubtract(int entity1Id, int entity2Id)
{
    auto it1 = m_entities.find(entity1Id);
    auto it2 = m_entities.find(entity2Id);

    if (it1 == m_entities.end() || it2 == m_entities.end()) {
        qCWarning(cadGeometry) << "Entity not found for boolean subtract";
        return -1;
    }

    qCDebug(cadGeometry) << "Boolean subtract of entities:" << entity1Id << entity2Id;

    CADEntity entity;
    entity.type = CADEntity::Solid;

    BRepAlgoAPI_Cut cut(it1->second.shape, it2->second.shape);
    if (cut.IsDone()) {
        entity.shape = cut.Shape();
        return addEntity(entity);
    }

    qCWarning(cadGeometry) << "Boolean subtract failed";
    return -1;
}

int GeometryEngine::booleanIntersect(int entity1Id, int entity2Id)
{
    auto it1 = m_entities.find(entity1Id);
    auto it2 = m_entities.find(entity2Id);

    if (it1 == m_entities.end() || it2 == m_entities.end()) {
        qCWarning(cadGeometry) << "Entity not found for boolean intersect";
        return -1;
    }

    qCDebug(cadGeometry) << "Boolean intersect of entities:" << entity1Id << entity2Id;

    CADEntity entity;
    entity.type = CADEntity::Solid;

    BRepAlgoAPI_Common common(it1->second.shape, it2->second.shape);
    if (common.IsDone()) {
        entity.shape = common.Shape();
        return addEntity(entity);
    }

    qCWarning(cadGeometry) << "Boolean intersect failed";
    return -1;
}

// Analysis and measurement
double GeometryEngine::getDistance(const gp_Pnt& point1, const gp_Pnt& point2)
{
    return point1.Distance(point2);
}

double GeometryEngine::getArea(int entityId)
{
    auto it = m_entities.find(entityId);
    if (it == m_entities.end()) {
        qCWarning(cadGeometry) << "Entity not found:" << entityId;
        return 0.0;
    }

    GProp_GProps props;
    BRepGProp::SurfaceProperties(it->second.shape, props);
    return props.Mass();
}

double GeometryEngine::getVolume(int entityId)
{
    auto it = m_entities.find(entityId);
    if (it == m_entities.end()) {
        qCWarning(cadGeometry) << "Entity not found:" << entityId;
        return 0.0;
    }

    GProp_GProps props;
    BRepGProp::VolumeProperties(it->second.shape, props);
    return props.Mass();
}

gp_Pnt GeometryEngine::getCentroid(int entityId)
{
    auto it = m_entities.find(entityId);
    if (it == m_entities.end()) {
        qCWarning(cadGeometry) << "Entity not found:" << entityId;
        return gp_Pnt();
    }

    GProp_GProps props;
    BRepGProp::VolumeProperties(it->second.shape, props);
    return props.CentreOfMass();
}

std::pair<gp_Pnt, gp_Pnt> GeometryEngine::getBoundingBox(int entityId)
{
    auto it = m_entities.find(entityId);
    if (it == m_entities.end()) {
        qCWarning(cadGeometry) << "Entity not found:" << entityId;
        return std::make_pair(gp_Pnt(), gp_Pnt());
    }

    Bnd_Box box;
    BRepBndLib::Add(it->second.shape, box);

    double xmin, ymin, zmin, xmax, ymax, zmax;
    box.Get(xmin, ymin, zmin, xmax, ymax, zmax);

    return std::make_pair(gp_Pnt(xmin, ymin, zmin), gp_Pnt(xmax, ymax, zmax));
}

// Display properties
void GeometryEngine::setEntityColor(int entityId, int color)
{
    auto it = m_entities.find(entityId);
    if (it == m_entities.end()) {
        qCWarning(cadGeometry) << "Entity not found:" << entityId;
        return;
    }

    it->second.color = color;
    updateAISObject(entityId);
    emit entityModified(entityId);
}

void GeometryEngine::setEntityVisible(int entityId, bool visible)
{
    auto it = m_entities.find(entityId);
    if (it == m_entities.end()) {
        qCWarning(cadGeometry) << "Entity not found:" << entityId;
        return;
    }

    it->second.visible = visible;

    if (!it->second.aisObject.IsNull()) {
        if (visible) {
            m_context->Display(it->second.aisObject, Standard_False);
        } else {
            m_context->Erase(it->second.aisObject, Standard_False);
        }
    }

    emit entityModified(entityId);
}

void GeometryEngine::setEntitySelected(int entityId, bool selected)
{
    auto it = m_entities.find(entityId);
    if (it == m_entities.end()) {
        qCWarning(cadGeometry) << "Entity not found:" << entityId;
        return;
    }

    it->second.selected = selected;

    if (!it->second.aisObject.IsNull()) {
        if (selected) {
            m_context->SetSelected(it->second.aisObject, Standard_False);
        } else {
            m_context->ClearSelected(Standard_False);
        }
    }

    emit entityModified(entityId);
}

// Layer management
void GeometryEngine::setEntityLayer(int entityId, const QString& layer)
{
    auto it = m_entities.find(entityId);
    if (it == m_entities.end()) {
        qCWarning(cadGeometry) << "Entity not found:" << entityId;
        return;
    }

    // Remove from old layer
    if (!it->second.layer.isEmpty()) {
        auto& oldLayerEntities = m_layers[it->second.layer];
        oldLayerEntities.erase(std::remove(oldLayerEntities.begin(), oldLayerEntities.end(), entityId),
                              oldLayerEntities.end());
    }

    // Add to new layer
    it->second.layer = layer;
    if (!layer.isEmpty()) {
        m_layers[layer].push_back(entityId);
    }

    emit entityModified(entityId);
}

QString GeometryEngine::getEntityLayer(int entityId) const
{
    auto it = m_entities.find(entityId);
    if (it != m_entities.end()) {
        return it->second.layer;
    }
    return QString();
}

void GeometryEngine::setLayerVisible(const QString& layer, bool visible)
{
    m_layerVisibility[layer] = visible;

    auto layerIt = m_layers.find(layer);
    if (layerIt != m_layers.end()) {
        for (int entityId : layerIt->second) {
            setEntityVisible(entityId, visible);
        }
    }
}

void GeometryEngine::setLayerColor(const QString& layer, int color)
{
    m_layerColors[layer] = color;

    auto layerIt = m_layers.find(layer);
    if (layerIt != m_layers.end()) {
        for (int entityId : layerIt->second) {
            setEntityColor(entityId, color);
        }
    }
}

// Import/Export
bool GeometryEngine::importSTEP(const QString& filename)
{
    qCDebug(cadGeometry) << "Importing STEP file:" << filename;

    STEPCAFControl_Reader reader;
    IFSelect_ReturnStatus status = reader.ReadFile(filename.toStdString().c_str());

    if (status != IFSelect_RetDone) {
        qCWarning(cadGeometry) << "Failed to read STEP file:" << filename;
        return false;
    }

    reader.TransferRoots();
    TopoDS_Shape shape = reader.OneShape();

    if (!shape.IsNull()) {
        CADEntity entity;
        entity.type = CADEntity::Solid;
        entity.shape = shape;
        addEntity(entity);

        qCDebug(cadGeometry) << "STEP file imported successfully";
        return true;
    }

    return false;
}

bool GeometryEngine::exportSTEP(const QString& filename, const std::vector<int>& entityIds)
{
    qCDebug(cadGeometry) << "Exporting STEP file:" << filename;

    STEPCAFControl_Writer writer;

    if (entityIds.empty()) {
        // Export all entities
        for (const auto& pair : m_entities) {
            if (!pair.second.shape.IsNull()) {
                writer.Transfer(pair.second.shape, STEPControl_AsIs);
            }
        }
    } else {
        // Export specified entities
        for (int id : entityIds) {
            auto it = m_entities.find(id);
            if (it != m_entities.end() && !it->second.shape.IsNull()) {
                writer.Transfer(it->second.shape, STEPControl_AsIs);
            }
        }
    }

    IFSelect_ReturnStatus status = writer.Write(filename.toStdString().c_str());

    if (status == IFSelect_RetDone) {
        qCDebug(cadGeometry) << "STEP file exported successfully";
        return true;
    }

    qCWarning(cadGeometry) << "Failed to export STEP file:" << filename;
    return false;
}

// Utility functions
Handle(AIS_InteractiveObject) GeometryEngine::createAISObject(const CADEntity& entity)
{
    if (entity.shape.IsNull()) {
        return Handle(AIS_InteractiveObject)();
    }

    Handle(AIS_Shape) aisShape = new AIS_Shape(entity.shape);

    // Set display properties
    aisShape->SetColor(Quantity_Color(entity.color / 255.0, entity.color / 255.0, entity.color / 255.0, Quantity_TOC_RGB));
    aisShape->SetTransparency(entity.visible ? 0.0 : 0.8);

    return aisShape;
}

void GeometryEngine::updateAISObject(int entityId)
{
    auto it = m_entities.find(entityId);
    if (it == m_entities.end() || it->second.aisObject.IsNull()) {
        return;
    }

    // Update display properties
    Handle(AIS_Shape) aisShape = Handle(AIS_Shape)::DownCast(it->second.aisObject);
    if (!aisShape.IsNull()) {
        aisShape->SetColor(Quantity_Color(it->second.color / 255.0, it->second.color / 255.0, it->second.color / 255.0, Quantity_TOC_RGB));
        aisShape->SetTransparency(it->second.visible ? 0.0 : 0.8);
        m_context->Redisplay(aisShape, Standard_False);
    }
}

void GeometryEngine::updateDisplay()
{
    if (!m_context.IsNull()) {
        m_context->UpdateCurrentViewer();
    }
}

int GeometryEngine::createCircle(const gp_Pnt& center, double radius, const gp_Dir& normal)
{
    qCDebug(cadGeometry) << "Creating circle at:" << center.X() << center.Y() << center.Z() 
                        << "radius:" << radius;
    
    CADEntity entity;
    entity.type = CADEntity::Circle;
    
    gp_Ax2 axis(center, normal);
    gp_Circ circle(axis, radius);
    entity.shape = BRepBuilderAPI_MakeEdge(circle).Edge();
    
    return addEntity(entity);
}

int GeometryEngine::createRectangle(const gp_Pnt& corner1, const gp_Pnt& corner2)
{
    qCDebug(cadGeometry) << "Creating rectangle from:" << corner1.X() << corner1.Y() << corner1.Z()
                        << "to:" << corner2.X() << corner2.Y() << corner2.Z();
    
    CADEntity entity;
    entity.type = CADEntity::Rectangle;
    
    // Create rectangle as a wire
    gp_Pnt p1 = corner1;
    gp_Pnt p2(corner2.X(), corner1.Y(), corner1.Z());
    gp_Pnt p3 = corner2;
    gp_Pnt p4(corner1.X(), corner2.Y(), corner1.Z());
    
    BRepBuilderAPI_MakePolygon polygon;
    polygon.Add(p1);
    polygon.Add(p2);
    polygon.Add(p3);
    polygon.Add(p4);
    polygon.Close();
    
    entity.shape = polygon.Wire();
    
    return addEntity(entity);
}
