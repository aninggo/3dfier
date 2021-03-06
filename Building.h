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

#ifndef Building_h
#define Building_h

#include "TopoFeature.h"


class Building : public Flat
{
public:
  Building (char *wkt, std::string pid, float heightref_top, float heightref_base);
  bool        lift();
  bool        add_elevation_point(double x, double y, double z, float radius, LAS14Class lasclass, bool lastreturn);
  std::string get_citygml();
  std::string get_csv();
  std::string get_mtl();
  std::string get_obj_f_floor(std::unordered_map<std::string, std::vector<Point3>::size_type> &vertices_map);
  std::string get_obj_v_building_volume(std::vector<Point3>::size_type &idx, std::unordered_map<std::string, std::vector<Point3>::size_type> &vertices_map, int z_exaggeration);
  std::string get_obj_f_building_volume(std::unordered_map<std::string, std::vector<Point3>::size_type> &vertices_map, bool usemtl);
  bool        get_shape(OGRLayer * layer);
  TopoClass   get_class();
  bool        is_hard();
  int         get_height_base();
private: 
  std::vector<int>    _zvaluesground;
  static float          _heightref_top;
  static float          _heightref_base;
  int                 _height_base;
};


#endif /* Building_h */

