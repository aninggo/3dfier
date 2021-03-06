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

 
#include "Forest.h"


Forest::Forest (char *wkt, std::string pid, int simplification, float innerbuffer) : TIN(wkt, pid, simplification, innerbuffer)
{}


bool Forest::lift() {
  TopoFeature::lift_each_boundary_vertices(0.5);
  return true;
}


bool Forest::add_elevation_point(double x, double y, double z, float radius, LAS14Class lasclass, bool lastreturn) {
  bool toadd = false;
  if (_simplification <= 1)
    toadd = true;
  else {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dis(1, _simplification);
    if (dis(gen) == 1)
      toadd = true;
  }
  if (toadd && lastreturn && lasclass != LAS_BUILDING) {
    Point2 p(x, y);
    if (bg::within(p, *(_p2)) && (this->get_distance_to_boundaries(p) > _innerbuffer)) {
      _lidarpts.push_back(Point3(x, y, z));
    }
    assign_elevation_to_vertex(x, y, z, radius);
  }
  return toadd;
}


TopoClass Forest::get_class() {
  return FOREST;
}


bool Forest::is_hard() {
  return false;
}


std::string Forest::get_citygml() {
  return "<EMPTY/>";
}


std::string Forest::get_mtl() {
  return "usemtl Forest\n";
}


bool Forest::get_shape(OGRLayer* layer) {
  return TopoFeature::get_shape_features(layer, "Forest");
}
