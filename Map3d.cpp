/*
 Copyright (c) 2015 Hugo Ledoux

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
*/

#include "Map3d.h"
#include "io.h"
#include "boost/locale.hpp"


Map3d::Map3d() {
  OGRRegisterAll();
  _building_include_floor = false;
  _building_heightref_roof = 0.9;
  _building_heightref_floor = 0.1;
  _building_triangulate = true;
  _terrain_simplification = 0;
  _forest_simplification = 0;
  _terrain_innerbuffer = 0.0;
  _forest_innerbuffer = 0.0;
  _radius_vertex_elevation = 1.0;
  _building_radius_vertex_elevation = 1.0;
  _threshold_jump_edges = 50;
  _minx = 1e8;
  _miny = 1e8;
}


Map3d::~Map3d() {
  // TODO : destructor Map3d
  _lsFeatures.clear();
}

void Map3d::set_building_heightref_roof(float h) {
  _building_heightref_roof = h;
}

void Map3d::set_building_heightref_floor(float h) {
  _building_heightref_floor = h;
}

void Map3d::set_radius_vertex_elevation(float radius) {
  _radius_vertex_elevation = radius;
}

void Map3d::set_building_radius_vertex_elevation(float radius) {
  _building_radius_vertex_elevation = radius;
}

void Map3d::set_threshold_jump_edges(float threshold) {
  _threshold_jump_edges = int(threshold * 100);
}

void Map3d::set_building_include_floor(bool include) {
  _building_include_floor = include;
}

void Map3d::set_building_triangulate(bool triangulate) {
  _building_triangulate = triangulate;
}

void Map3d::set_terrain_simplification(int simplification) {
  _terrain_simplification = simplification;
}

void Map3d::set_forest_simplification(int simplification) {
  _forest_simplification = simplification;
} 

void Map3d::set_terrain_innerbuffer(float innerbuffer) {
  _terrain_innerbuffer = innerbuffer;
}

void Map3d::set_forest_innerbuffer(float innerbuffer) {
  _forest_innerbuffer = innerbuffer;
}  

void Map3d::set_water_heightref(float h) {
  _water_heightref = h;
}

void Map3d::set_road_heightref(float h) {
  _road_heightref = h;
}

void Map3d::set_separation_heightref(float h) {
  _separation_heightref = h;
}

void Map3d::set_bridge_heightref(float h) {
  _bridge_heightref = h;
}

std::string Map3d::get_citygml() {
  std::stringstream ss;
  ss << get_xml_header();
  ss << get_citygml_namespaces();
  ss << "<gml:name>my3dmap</gml:name>";
  for (auto& p3 : _lsFeatures) {
    ss << p3->get_citygml();
  }
  ss << "</CityModel>";
  return ss.str();
}

std::string Map3d::get_csv_buildings() {
  std::stringstream ss;
  ss << "id;roof;floor" << std::endl;
  for (auto& p : _lsFeatures) {
    if (p->get_class() == BUILDING) {
      Building* b = dynamic_cast<Building*>(p);
      // if (b != nullptr)
      ss << b->get_csv();
    }
  }
  return ss.str();
}

std::string Map3d::get_obj_per_feature(int z_exaggeration) {
  std::unordered_map<std::string, std::vector<Point3>::size_type> vertices_map;
  std::vector<Point3>::size_type idx = 1;
  std::stringstream ss;
  ss << "mtllib ./3dfier.mtl" << std::endl;
  for (auto& p3 : _lsFeatures) {
    ss << p3->get_obj_v(idx, vertices_map, z_exaggeration);
  }

  int offset = 0;
  for (auto& p3 : _lsFeatures) {
    ss << "o " << p3->get_id() << std::endl;
    ss << p3->get_mtl();
    ss << p3->get_obj_f(vertices_map, true);
    //-- TODO: floor for buildings
//    if (_building_include_floor == true) {  
//      Building* b = dynamic_cast<Building*>(p3);
//      if (b != nullptr)
//        ss << b->get_obj_f_floor(offset);
//    }
  }
  return ss.str();
}

std::string Map3d::get_obj_building_volume(int z_exaggeration) {
  std::unordered_map<std::string, std::vector<Point3>::size_type> vertices_map;
  std::vector<Point3>::size_type idx = 1;
  std::stringstream ss;
  ss << "mtllib ./3dfier.mtl" << std::endl;
  for (auto& p3 : _lsFeatures) {
    Building* b = dynamic_cast<Building*>(p3);
    if (b != nullptr) {
      ss << b->get_obj_v_building_volume(idx, vertices_map, z_exaggeration);
    }
  }
  for (auto& p3 : _lsFeatures) {
    Building* b = dynamic_cast<Building*>(p3);
    if (b != nullptr) {
      ss << "o " << p3->get_id() << std::endl;
      ss << b->get_obj_f_building_volume(vertices_map, true);
    }
  }
  return ss.str();
}


std::string Map3d::get_obj_per_class(int z_exaggeration) {
  std::unordered_map<std::string, std::vector<Point3>::size_type> vertices_map;
  std::vector<Point3>::size_type idx = 1;
  std::stringstream ss;
  ss << "mtllib ./3dfier.mtl" << std::endl;
  //-- go class by class sequentially
  for (int c = 0; c < 6; c++) {
    for (auto& p3 : _lsFeatures) {
      if (p3->get_class() == c) {
        ss << p3->get_obj_v(idx, vertices_map, z_exaggeration);
      }
    }
  }
  for (int c = 0; c < 6; c++) {
    ss << "o " << c << std::endl;
    for (auto& p3 : _lsFeatures) {
      if (p3->get_class() == c) {
        ss << p3->get_mtl();
        ss << p3->get_obj_f(vertices_map, false);
        if (_building_include_floor == true) {
          Building* b = dynamic_cast<Building*>(p3);
          if (b != nullptr)
            ss << b->get_obj_f_floor(vertices_map);
        }
      }
    }
  }
  return ss.str();
}

bool Map3d::get_shapefile(std::string filename) {
#if GDAL_VERSION_MAJOR < 2
  std::cerr << "Exporting to a 3D Shapefile requires GDAL/OGC 2.x or higher." << std::endl;
  return false;
#else
  if (GDALGetDriverCount() == 0)
    GDALAllRegister();
  GDALDriver *driver = GetGDALDriverManager()->GetDriverByName("ESRI Shapefile");
  GDALDataset *dataSource = driver->Create(filename.c_str(), 0, 0, 0, GDT_Unknown, NULL);

  if (dataSource == NULL) {
    std::cerr << "\tERROR: could not open file, skipping it." << std::endl;
    return false;
  }
  OGRLayer *layer = dataSource->CreateLayer("my3dmap", NULL, OGR_GT_SetZ(wkbMultiPolygon), NULL);

  OGRFieldDefn oField("Id", OFTString);
  if (layer->CreateField(&oField) != OGRERR_NONE)
  {
    std::cerr << "Creating Id field failed." << std::endl;
    return false;
  }
  OGRFieldDefn oField2("Class", OFTString);
  if (layer->CreateField(&oField2) != OGRERR_NONE)
  {
    std::cerr << "Creating Class field failed." << std::endl;
    return false;
  }
  for (auto& p3 : _lsFeatures) {
    p3->get_shape(layer);
  }
  GDALClose(dataSource);
  return true;
#endif
}

unsigned long Map3d::get_num_polygons() {
  return _lsFeatures.size();
}


const std::vector<TopoFeature*>& Map3d::get_polygons3d() {
  return _lsFeatures;
}


// void Map3d::add_elevation_point(double x, double y, double z, int returnno, liblas::Classification lasclass) {
void Map3d::add_elevation_point(liblas::Point const& laspt) {
  //-- filter out vegetation TODO: shouldn't be here me thinks, but in each specific classes
  // if ( (laspt.GetClassification() == liblas::Classification(1)) && (laspt.GetReturnNumber() != 1) )
    // return;

  // p.GetX(), p.GetY(), p.GetZ(), p.GetReturnNumber(), p.GetClassification()

  Point2 p(laspt.GetX(), laspt.GetY());
  std::vector<PairIndexed> re;
  float radius = _radius_vertex_elevation;
  if (_building_radius_vertex_elevation > _radius_vertex_elevation) {
    radius = _building_radius_vertex_elevation;
  }
  Point2 minp(laspt.GetX() - radius, laspt.GetY() - radius);
  Point2 maxp(laspt.GetX() + radius, laspt.GetY() + radius);
  Box2 querybox(minp, maxp);
  _rtree.query(bgi::intersects(querybox), std::back_inserter(re));
  LAS14Class lasclass = LAS_UNKNOWN;
  for (auto& v : re) {
    TopoFeature* f = v.second;
    if (f->get_class() == BUILDING)
    {
      radius = _building_radius_vertex_elevation;
    }
    else {
      radius = _radius_vertex_elevation;
    }

    if (bg::distance(p, *(f->get_Polygon2())) < radius) {
      //-- get LAS class
      if (laspt.GetClassification() == liblas::Classification(LAS_UNCLASSIFIED))
        lasclass = LAS_UNCLASSIFIED;
      else if (laspt.GetClassification() == liblas::Classification(LAS_GROUND))
        lasclass = LAS_GROUND;
      else if (laspt.GetClassification() == liblas::Classification(LAS_BUILDING))
        lasclass = LAS_BUILDING;
      else if (laspt.GetClassification() == liblas::Classification(LAS_WATER))
        lasclass = LAS_WATER;
      else if (laspt.GetClassification() == liblas::Classification(LAS_BRIDGE))
        lasclass = LAS_BRIDGE;
      else
        lasclass = LAS_UNKNOWN;

      f->add_elevation_point(laspt.GetX(),
        laspt.GetY(),
        laspt.GetZ(),
        radius,
        lasclass,
        (laspt.GetReturnNumber() == laspt.GetNumberOfReturns()));
    }
  }
}


bool Map3d::threeDfy(bool triangulate) {
  /*
    1. lift
    2. stitch
    3. process vertical walls
    4. CDT
  */
  std::clog << "===== /LIFTING =====" << std::endl;
  for (auto& p : _lsFeatures) {
    p->lift();
    //if (p->get_top_level() == true) {
    //  //std::clog << p->get_id() << std::endl;
    //  p->lift();
    //}
    //else
    //{
    //  std::clog << "non-top-niveau " << p->get_id() << std::endl;
    //  // Lift to height for bridges/overpass
    //  //if (p->get_class() == BRIDGE)
    //  //{
    //  //  p->lift();
    //  //}
    //}
  }
  std::clog << "===== LIFTING/ =====" << std::endl;
  if (triangulate == true) {
    std::clog << "=====  /ADJACENT FEATURES =====" << std::endl;
    for (auto& p : _lsFeatures) {
      get_adjacent_features(p);
    }
    std::clog << "=====  ADJACENT FEATURES/ =====" << std::endl;

    std::clog << "=====  /STITCHING =====" << std::endl;
    this->stitch_lifted_features();
    std::clog << "=====  STITCHING/ =====" << std::endl;

    // std::clog << "SIZE FEATURES: " << _lsFeatures.size() << std::endl;
    // std::clog << "SIZE NC: " << _nc.size() << std::endl;

    //-- Sort all node column vectors
    for (auto& nc : _nc) {
      std::sort(nc.second.begin(), nc.second.end());
    }

    std::clog << "=====  /BOWTIES =====" << std::endl;
    for (auto& p : _lsFeatures) {
      if (p->has_vertical_walls() == true) {
        p->fix_bowtie();
      }
    }
    std::clog << "=====  BOWTIES/ =====" << std::endl;

    std::clog << "=====  /VERTICAL WALLS =====" << std::endl;
    for (auto& p : _lsFeatures) {
      if (p->has_vertical_walls() == true) {
        p->construct_vertical_walls(_nc);
      }
    }
    std::clog << "=====  VERTICAL WALLS/ =====" << std::endl;

    std::clog << "=====  /CDT =====" << std::endl;
    for (auto& p : _lsFeatures) {
      p->buildCDT();
    }
    std::clog << "=====  CDT/ =====" << std::endl;
  }
  return true;
}

bool Map3d::threeDfy_building_volume() {
  /*
    1. lift
    2. CDT
  */
  std::clog << "===== /LIFTING =====" << std::endl;
  for (auto& p : _lsFeatures) {
    std::clog << p->get_id() << std::endl;
    p->lift();
  }
  std::clog << "===== LIFTING/ =====" << std::endl;

  std::clog << "=====  /CDT =====" << std::endl;
  for (auto& p : _lsFeatures) {
    // std::clog << p->get_id() << " (" << p->get_class() << ")" << std::endl;
    p->buildCDT();
  }
  std::clog << "=====  CDT/ =====" << std::endl;
  return true;
}



bool Map3d::construct_rtree() {
  std::clog << "Constructing the R-tree...";
  for (auto p : _lsFeatures)
    _rtree.insert(std::make_pair(p->get_bbox2d(), p));
  std::clog << " done." << std::endl;
  return true;
}


bool Map3d::add_polygons_files(std::vector<PolygonFile> &files) {
#if GDAL_VERSION_MAJOR < 2
  if (OGRSFDriverRegistrar::GetRegistrar()->GetDriverCount() == 0)
    OGRRegisterAll();
#else
  if (GDALGetDriverCount() == 0)
    GDALAllRegister();
#endif
  for (auto file = files.begin(); file != files.end(); ++file) {
    std::clog << "Reading input dataset: " << file->filename << std::endl;
#if GDAL_VERSION_MAJOR < 2
    OGRDataSource *dataSource = OGRSFDriverRegistrar::Open(file->filename.c_str(), false);
#else
    GDALDataset *dataSource = (GDALDataset*)GDALOpenEx(file->filename.c_str(), GDAL_OF_READONLY, NULL, NULL, NULL);
#endif

    if (dataSource == NULL) {
      std::cerr << "\tERROR: could not open file, skipping it." << std::endl;
      return false;
    }

    // if the file doesn't have layers specified, add all
    if (file->layers[0].first.empty())
    {
      std::string lifting = file->layers[0].second;
      file->layers.clear();
      int numberOfLayers = dataSource->GetLayerCount();
      for (int i = 0; i < numberOfLayers; i++) {
        OGRLayer *dataLayer = dataSource->GetLayer(i);
        file->layers.emplace_back(dataLayer->GetName(), lifting);
      }
    }

    bool wentgood = this->extract_and_add_polygon(dataSource, &(*file));
#if GDAL_VERSION_MAJOR < 2
    OGRDataSource::DestroyDataSource(dataSource);
#else
    GDALClose(dataSource);
#endif

    if (wentgood == false) {
      std::cerr << "ERROR: Something went bad while reading input polygons. Aborting." << std::endl;
      return false;
    }
  }
  return true;
}


#if GDAL_VERSION_MAJOR < 2
bool Map3d::extract_and_add_polygon(OGRDataSource* dataSource, PolygonFile* file)
#else
bool Map3d::extract_and_add_polygon(GDALDataset* dataSource, PolygonFile* file)
#endif
{
  const char *idfield = file->idfield.c_str();
  const char *heightfield = file->heightfield.c_str();
  bool multiple_heights = file->handle_multiple_heights;
  bool wentgood = false;
  for (auto l : file->layers) {
    OGRLayer *dataLayer = dataSource->GetLayerByName((l.first).c_str());
    if (dataLayer == NULL) {
      continue;
    }
    if (dataLayer->FindFieldIndex(idfield, false) == -1) {
      std::cerr << "ERROR: field '" << idfield << "' not found in layer '" << l.first << "'." << std::endl;
      continue;
    }
    if (dataLayer->FindFieldIndex(heightfield, false) == -1) {
      std::cerr << "ERROR: field '" << heightfield << "' not found in layer '" << l.first << "', using all polygons." << std::endl;
    }
    dataLayer->ResetReading();
    unsigned int numberOfPolygons = dataLayer->GetFeatureCount(true);
    std::clog << "\tLayer: " << dataLayer->GetName() << std::endl;
    std::clog << "\t(" << boost::locale::as::number << numberOfPolygons << " features --> " << l.second << ")" << std::endl;
    OGRFeature *f;

    while ((f = dataLayer->GetNextFeature()) != NULL) {
      switch (f->GetGeometryRef()->getGeometryType()) {
      case wkbPolygon:
      case wkbPolygon25D: {
        extract_feature(f, idfield, heightfield, l.second, multiple_heights);
        break;
      }
      case wkbMultiPolygon:
      case wkbMultiPolygon25D: {
        OGRMultiPolygon* multipolygon = (OGRMultiPolygon*)f->GetGeometryRef();
        int numGeom = multipolygon->getNumGeometries();
        if (numGeom >= 1) {
          for (int i = 0; i < numGeom; i++) {
            OGRFeature* cf = f->Clone();
            if (numGeom > 1) {
              std::stringstream ss;
              ss << f->GetFieldAsString(idfield) << "-" << std::to_string(i);
              cf->SetField(idfield, ss.str().c_str());
            }
            cf->SetGeometry((OGRPolygon*)multipolygon->getGeometryRef(i));
            extract_feature(cf, idfield, heightfield, l.second, multiple_heights);
          }
          std::clog << "MultiPolygon with " << numGeom << " geometries processed" << std::endl;
        }
        break;
      }
      default: {
        continue;
      }
      }
    }
    wentgood = true;
  }
  return wentgood;
}

void Map3d::extract_feature(OGRFeature *f, const char *idfield, const char *heightfield, std::string layertype, bool multiple_heights) {
  Polygon2* p2 = new Polygon2();
  char *wkt;
  f->GetGeometryRef()->flattenTo2D();
  f->GetGeometryRef()->exportToWkt(&wkt);
  bg::unique(*p2); //-- remove duplicate vertices
  if (layertype == "Building") {
    Building* p3 = new Building(wkt, f->GetFieldAsString(idfield), _building_heightref_roof, _building_heightref_floor);
    _lsFeatures.push_back(p3);
  }
  else if (layertype == "Terrain") {
    Terrain* p3 = new Terrain(wkt, f->GetFieldAsString(idfield), this->_terrain_simplification, this->_terrain_innerbuffer);
    _lsFeatures.push_back(p3);
  }
  else if (layertype == "Forest") {
    Forest* p3 = new Forest(wkt, f->GetFieldAsString(idfield), this->_forest_simplification, this->_forest_innerbuffer);
    _lsFeatures.push_back(p3);
  }
  else if (layertype == "Water") {
    Water* p3 = new Water(wkt, f->GetFieldAsString(idfield), this->_water_heightref);
    _lsFeatures.push_back(p3);
  }
  else if (layertype == "Road") {
    Road* p3 = new Road(wkt, f->GetFieldAsString(idfield), this->_road_heightref);
    _lsFeatures.push_back(p3);
  }
  else if (layertype == "Separation") {
    Separation* p3 = new Separation(wkt, f->GetFieldAsString(idfield), this->_separation_heightref);
    _lsFeatures.push_back(p3);
  }
  else if (layertype == "Bridge/Overpass") {
    Bridge* p3 = new Bridge(wkt, f->GetFieldAsString(idfield), this->_bridge_heightref);
    _lsFeatures.push_back(p3);
  }
  //-- flag all polygons at (niveau != 0) or remove if not handling multiple height levels
  if ((f->GetFieldIndex(heightfield) != -1) && (f->GetFieldAsInteger(heightfield) != 0)) {
    if (multiple_heights) {
      // std::clog << "niveau=" << f->GetFieldAsInteger(heightfield) << ": " << f->GetFieldAsString(idfield) << std::endl;
      _lsFeatures.back()->set_top_level(false);
    }
    else {
      _lsFeatures.pop_back();
    }
  }
}


//-- http://www.liblas.org/tutorial/cpp.html#applying-filters-to-a-reader-to-extract-specified-classes
bool Map3d::add_las_file(std::string ifile, std::vector<int> lasomits, int skip) {
  std::clog << "Reading LAS/LAZ file: " << ifile << std::endl;
  std::ifstream ifs;
  ifs.open(ifile.c_str(), std::ios::in | std::ios::binary);
  if (ifs.is_open() == false) {
    std::cerr << "\tERROR: could not open file, skipping it." << std::endl;
    return false;
  }
  //-- LAS classes to omit
  std::vector<liblas::Classification> liblasomits;
  for (int i : lasomits)
    liblasomits.push_back(liblas::Classification(i));
  //-- read each point 1-by-1
  liblas::ReaderFactory f;
  liblas::Reader reader = f.CreateWithStream(ifs);
  liblas::Header const& header = reader.GetHeader();

  std::clog << "\t(" << boost::locale::as::number << header.GetPointRecordsCount() << " points in the file)" << std::endl;
  if ((skip != 0) && (skip != 1)) {
    std::clog << "\t(skipping every " << skip << "th points, thus ";
    std::clog << boost::locale::as::number << (header.GetPointRecordsCount() / skip) << " are used)" << std::endl;
  }
  else
    std::clog << "\t(all points used, no skipping)" << std::endl;

  if (lasomits.empty() == false) {
    std::clog << "\t(omitting LAS classes: ";
    for (int i : lasomits)
      std::clog << i << " ";
    std::clog << ")" << std::endl;
  }
  int i = 0;
  while (reader.ReadNextPoint()) {
    liblas::Point const& p = reader.GetPoint();
    if ((skip != 0) && (skip != 1)) {
      if (i % skip == 0) {
        if (std::find(liblasomits.begin(), liblasomits.end(), p.GetClassification()) == liblasomits.end())
          this->add_elevation_point(p);
      }
    }
    else {
      if (std::find(liblasomits.begin(), liblasomits.end(), p.GetClassification()) == liblasomits.end())
        this->add_elevation_point(p);
    }
    if (i % 100000 == 0)
      printProgressBar(100 * (i / double(header.GetPointRecordsCount())));
    i++;
  }
  printProgressBar(100);
  std::clog << "done" << std::endl;
  ifs.close();
  return true;
}


std::vector<TopoFeature*> Map3d::get_adjacent_features(TopoFeature* f) {
  std::vector<PairIndexed> re;
  _rtree.query(bgi::intersects(f->get_bbox2d()), std::back_inserter(re));
  std::vector<TopoFeature*> lsAdjacent;
  for (auto& each : re) {
    TopoFeature* fadj = each.second;
    if (f != fadj && (bg::touches(*(f->get_Polygon2()), *(fadj->get_Polygon2())) || !bg::disjoint(*(f->get_Polygon2()), *(fadj->get_Polygon2())))) {
      f->add_adjacent_feature(fadj);
    }
  }
  return lsAdjacent;
}


void Map3d::stitch_lifted_features() {
  std::vector<int> ringis, pis;
  for (auto& f : _lsFeatures) {
    std::vector<PairIndexed> re;
    _rtree.query(bgi::intersects(f->get_bbox2d()), std::back_inserter(re));
    std::vector<TopoFeature*> lstouching = f->get_adjacent_features();

    ////-- 1. store all touching top level (adjacent + incident)
    //for (auto& each : re) {
    //  TopoFeature* fadj = each.second;

    //  // if (bg::intersects(*(f->get_Polygon2()), *(fadj->get_Polygon2())))
    //  // {
    //  //   std::clog << f->get_id() << " intersects " << fadj->get_id() << std::endl;
    //  // }
    //  // if (bg::touches(*(f->get_Polygon2()), *(fadj->get_Polygon2())))
    //  // {
    //  //   std::clog << f->get_id() << " touches " << fadj->get_id() << std::endl;
    //  // }

    //  //if (fadj->get_top_level() == true && bg::touches(*(f->get_Polygon2()), *(fadj->get_Polygon2())) == true) {
    //  if (f != fadj && (bg::touches(*(f->get_Polygon2()), *(fadj->get_Polygon2())) || !bg::disjoint(*(f->get_Polygon2()), *(fadj->get_Polygon2())))) {
    //    lstouching.push_back(fadj);
    //  }
    //}

    //-- 2. build the node-column for each vertex
    // oring
    Ring2 oring = bg::exterior_ring(*(f->get_Polygon2()));
    for (int i = 0; i < oring.size(); i++) {
      // std::cout << std::setprecision(3) << std::fixed << bg::get<0>(oring[i]) << " : " << bg::get<1>(oring[i]) << std::endl;
      std::vector< std::tuple<TopoFeature*, int, int> > star;
      bool toprocess = false;
      for (auto& fadj : lstouching) {
        ringis.clear();
        pis.clear();
        if (fadj->has_point2_(oring[i], ringis, pis) == true) {
          if (f->get_counter() < fadj->get_counter()) {  //-- here that only lowID-->highID are processed
            for (int k = 0; k < ringis.size(); k++) {
              toprocess = true;
              star.push_back(std::make_tuple(fadj, ringis[k], pis[k]));
            }
          }
          else {
            break;
          }
        }
      }
      if (toprocess == true) {
        this->stitch_one_vertex(f, 0, i, star);
      }
    }
    // irings
    int noiring = 0;
    for (Ring2& iring : bg::interior_rings(*(f->get_Polygon2()))) {
      noiring++;
      //std::clog << f->get_id() << " irings " << std::endl;
      for (int i = 0; i < iring.size(); i++) {
        std::vector< std::tuple<TopoFeature*, int, int> > star;
        bool toprocess = false;
        for (auto& fadj : lstouching) {
          ringis.clear();
          pis.clear();
          if (fadj->has_point2_(iring[i], ringis, pis) == true) {
            if (f->get_counter() < fadj->get_counter()) {  //-- here that only lowID-->highID are processed
              for (int k = 0; k < ringis.size(); k++) {
                toprocess = true;
                star.push_back(std::make_tuple(fadj, ringis[k], pis[k]));
              }
            }
            else {
              break;
            }
          }
        }
        if (toprocess == true) {
          this->stitch_one_vertex(f, noiring, i, star);
        }
      }
    }
  }
}

void Map3d::stitch_one_vertex(TopoFeature* f, int ringi, int pi, std::vector< std::tuple<TopoFeature*, int, int> >& star) {
  //-- degree of vertex == 2
  if (star.size() == 1) {
    TopoFeature* fadj = std::get<0>(star[0]);
    //-- if not building and same class or both soft, then average.
    if (f->get_class() != BUILDING && (f->get_class() == fadj->get_class() || (f->is_hard() == false && fadj->is_hard() == false))) {
      stitch_average(f, ringi, pi, fadj, std::get<1>(star[0]), std::get<2>(star[0]));
    }
    else {
      stitch_jumpedge(f, ringi, pi, fadj, std::get<1>(star[0]), std::get<2>(star[0]));
    }
  }
  //-- degree of vertex >= 3: more complex cases
  else if (star.size() > 1) {
    //-- collect all elevations
    std::vector< std::tuple< int, TopoFeature*, int, int > > zstar;
    zstar.push_back(std::make_tuple(
      f->get_vertex_elevation(ringi, pi),
      f,
      ringi,
      pi));
    for (auto& fadj : star) {
      zstar.push_back(std::make_tuple(
        std::get<0>(fadj)->get_vertex_elevation(std::get<1>(fadj), std::get<2>(fadj)),
        std::get<0>(fadj),
        std::get<1>(fadj),
        std::get<2>(fadj)));
    }

    //-- sort low-high based on heights (get<0>)
    std::sort(zstar.begin(), zstar.end(),
      [](std::tuple<int, TopoFeature*, int, int> const &t1, std::tuple<int, TopoFeature*, int, int> const &t2) {
      return std::get<0>(t1) < std::get<0>(t2);
    });

    //-- Calculate height of every class first, except buildings, also used for identifying buildings and water
    std::vector<int> heightperclass(7, 0);
    std::vector<int> classcount(7, 0);
    int building = -1;
    int water = -1;
    for (int i = 0; i < zstar.size(); i++) {
      if (std::get<1>(zstar[i])->get_class() != BUILDING) {
        if (std::get<1>(zstar[i])->get_class() == WATER) {
          water = i;
        }
        heightperclass[std::get<1>(zstar[i])->get_class()] += std::get<1>(zstar[i])->get_vertex_elevation(std::get<2>(zstar[i]), std::get<3>(zstar[i]));
        classcount[std::get<1>(zstar[i])->get_class()]++;
      }
      else {
        //-- set building to the one with the lowest base
        if (building == -1 || dynamic_cast<Building*>(std::get<1>(zstar[building]))->get_height_base() > dynamic_cast<Building*>(std::get<1>(zstar[i]))->get_height_base()) {
          building = i;
        }
      }
    }

    //-- Deal with buildings. If there's a building and a soft class incident, then this soft class
    //-- get allocated the height value of the floor of the building. Any building will do if >1.
    //-- Also ignore water so it doesn't get snapped to the floor of a building
    if (building != -1) {
      int baseheight = dynamic_cast<Building*>(std::get<1>(zstar[building]))->get_height_base();
      for (auto& each : zstar) {
        if (std::get<1>(each)->get_class() != BUILDING && std::get<1>(each)->get_class() != WATER) {
          std::get<0>(each) = baseheight;
          if (water != -1) {
            //- add a vertical wall between the feature and the water
            std::get<1>(each)->add_vertical_wall();
          }
        }
        else if (std::get<1>(each)->get_class() == BUILDING) {
          std::get<1>(each)->add_vertical_wall();
        }
        else if (std::get<1>(each)->get_class() == WATER) {
          std::get<0>(each) = heightperclass[WATER] / classcount[WATER];
        }
      }
    }
    else {
      for (auto& each : zstar) {
        std::get<0>(each) = heightperclass[std::get<1>(each)->get_class()] / classcount[std::get<1>(each)->get_class()];
      }
      for (std::vector< std::tuple< int, TopoFeature*, int, int > >::iterator it = zstar.begin(); it != zstar.end(); ++it) {
        std::vector< std::tuple< int, TopoFeature*, int, int > >::iterator fnext = it;
        for (std::vector< std::tuple< int, TopoFeature*, int, int > >::iterator it2 = it + 1; it2 != zstar.end(); ++it2) {
          int deltaz = std::abs(std::get<0>(*it) - std::get<0>(*it2));
          if (deltaz < this->_threshold_jump_edges) {
            fnext = it2;
            if (std::get<1>(*it)->is_hard()) {
              if (std::get<1>(*it2)->is_hard()) {
                std::get<1>(*it2)->add_vertical_wall();
                break;
              }
              else {
                std::get<0>(*it2) = std::get<0>(*it);
              }
            }
            else {
              if (std::get<1>(*it2)->is_hard()) {
                std::get<0>(*it) = std::get<0>(*it2);
                break;
              }
            }
          }
          else {
            // add a vertical wall to the highest feature
            std::get<1>(*it2)->add_vertical_wall();
            break;
          }
        }
        //-- Average heights of soft features within the jumpedge threshold counted from the lowest feature or skip to the next hard feature
        if (it != fnext) {
          if (std::get<1>(*it)->is_hard() == false && std::get<1>(*fnext)->is_hard() == false) {
            int totalz = 0;
            int count = 0;
            for (std::vector< std::tuple< int, TopoFeature*, int, int > >::iterator it2 = it; it2 != fnext + 1; ++it2) {
              totalz += std::get<0>(*it2);
              count++;
            }
            totalz = totalz / count;
            for (std::vector< std::tuple< int, TopoFeature*, int, int > >::iterator it2 = it; it2 != fnext + 1; ++it2) {
              std::get<0>(*it2) = totalz;
            }
          }
          else if (std::get<1>(*it)->is_hard() == false) {
            // Adjust all intermediate soft features
            for (std::vector< std::tuple< int, TopoFeature*, int, int > >::iterator it2 = it; it2 != fnext; ++it2) {
              std::get<0>(*it2) = std::get<0>(*fnext);
            }
          }
          it = fnext;
        }
      }
    }

    //-- assign the adjusted heights and build the nc
    int tmph = -99999;
    for (auto& each : zstar) {
      std::get<1>(each)->set_vertex_elevation(std::get<2>(each), std::get<3>(each), std::get<0>(each));
      if (std::get<0>(each) != tmph) { //-- not to repeat the same height
        Point2 p = std::get<1>(each)->get_point2(std::get<2>(each), std::get<3>(each));
        _nc[gen_key_bucket(&p)].push_back(std::get<0>(each));
        tmph = std::get<0>(each);
      }
    }
  }
}


void Map3d::stitch_jumpedge(TopoFeature* f1, int ringi1, int pi1, TopoFeature* f2, int ringi2, int pi2) {
  Point2 p = f1->get_point2(ringi1, pi1);
  std::string key_bucket = gen_key_bucket(&p);
  int f1z = f1->get_vertex_elevation(ringi1, pi1);
  int f2z = f2->get_vertex_elevation(ringi2, pi2);

  //-- Buildings involved
  if ((f1->get_class() == BUILDING) || (f2->get_class() == BUILDING)) {
    if ((f1->get_class() == BUILDING) && (f2->get_class() == BUILDING)) {
      //- add a wall to the heighest feature
      if (f1z > f2z) {
        f1->add_vertical_wall();
      }
      else if (f2z > f1z) {
        f2->add_vertical_wall();
      }
      _nc[key_bucket].push_back(f1z);
      _nc[key_bucket].push_back(f2z);
    }
    else if (f1->get_class() == BUILDING) {
      if (f2->get_class() != WATER) {
        f2->set_vertex_elevation(ringi2, pi2, dynamic_cast<Building*>(f1)->get_height_base());
      }
      else
      {
        //- keep water flat, add the water height to the nc
        _nc[key_bucket].push_back(f2z);
      }
      //- expect a building to always be heighest adjacent feature
      f1->add_vertical_wall();
      _nc[key_bucket].push_back(f1z);
      _nc[key_bucket].push_back(dynamic_cast<Building*>(f1)->get_height_base());
    }
    else { //-- f2 is Building
      if (f1->get_class() != WATER) {
        f1->set_vertex_elevation(ringi1, pi1, dynamic_cast<Building*>(f2)->get_height_base());
      }
      else
      {
        //- keep water flat, add the water height to the nc
        _nc[key_bucket].push_back(f1z);
      }
      //- expect a building to always be heighest adjacent feature
      f2->add_vertical_wall();
      _nc[key_bucket].push_back(f2z);
      _nc[key_bucket].push_back(dynamic_cast<Building*>(f2)->get_height_base());
    }
  }
  //-- no Buildings involved
  else {
    bool bStitched = false;
    int deltaz = std::abs(f1z - f2z);
    if (deltaz < this->_threshold_jump_edges) {
      if (f1->is_hard() == false) {
        f1->set_vertex_elevation(ringi1, pi1, f2z);
        bStitched = true;
      }
      else if (f2->is_hard() == false) {
        f2->set_vertex_elevation(ringi2, pi2, f1z);
        bStitched = true;
      }
    }
    //-- then vertical walls must be added: nc to highest
    if (bStitched == false) {
      //- add a wall to the heighest feature
      if (f1z > f2z) {
        f1->add_vertical_wall();
      }
      else if (f2z > f1z) {
        f2->add_vertical_wall();
      }
      _nc[key_bucket].push_back(f1z);
      _nc[key_bucket].push_back(f2z);
    }
  }
}

void Map3d::stitch_average(TopoFeature* f1, int ringi1, int pi1, TopoFeature* f2, int ringi2, int pi2) {
  float avgz = (f1->get_vertex_elevation(ringi1, pi1) + f2->get_vertex_elevation(ringi2, pi2)) / 2;
  f1->set_vertex_elevation(ringi1, pi1, avgz);
  f2->set_vertex_elevation(ringi2, pi2, avgz);
  Point2 p = f1->get_point2(ringi1, pi1);
  _nc[gen_key_bucket(&p)].push_back(avgz);
}