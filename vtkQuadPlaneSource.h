/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkQuadPlaneSource.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkQuadPlaneSource - create an array of quadrilaterals located in a plane
// .SECTION Description
// vtkQuadPlaneSource creates an m x n array of quadrilaterals arranged as
// a regular tiling in a quadrilateral plane. The quadrilateral plane is defined 
// by specifying an origin point, and then three other points.Seeing from the 
// counterclockwise,these Points are Origin Point,Point1,Point3 and Point2.
// The quadrilateral plane do not have to be parallelogram. (The axes must not
// be parallel.) The resolution of the plane (i.e., number of subdivisions) is
// controlled by the ivars XResolution and YResolution.
//
// By default, the plane is centered at the origin and perpendicular to the
// z-axis, with width and height of length 1 and resolutions set to 1.
//
// There are three convenience methods that allow you to easily move the
// plane.  The first, SetNormal(), allows you to specify the plane
// normal. The effect of this method is to rotate the plane around the center
// of the plane, aligning the plane normal with the specified normal. The
// rotation is about the axis defined by the cross product of the current
// normal with the new normal. The second, SetCenter(), translates the center
// of the plane to the specified center point. The third method, Push(),
// allows you to translate the plane along the plane normal by the distance
// specified. (Negative Push values translate the plane in the negative
// normal direction.)  Note that the SetNormal(), SetCenter() and Push()
// methods modify the Origin, Point1, Point3 and/or Point2 instance variables.

// .SECTION Caveats
// The normal to the plane will point in the direction of the cross product
// of the first axis (Origin->Point1) with the second (Origin->Point2). This
// also affects the normals to the generated polygons.

#ifndef __vtkQuadPlaneSource_h
#define __vtkQuadPlaneSource_h

#include "vtkPolyDataAlgorithm.h"

class vtkQuadPlaneSource : public vtkPolyDataAlgorithm 
{
public:
  void PrintSelf(ostream& os, vtkIndent indent);
  vtkTypeMacro(vtkQuadPlaneSource,vtkPolyDataAlgorithm);

  // Description:
  // Construct plane perpendicular to z-axis, resolution 1x1, width
  // and height 1.0, and centered at the origin.
  static vtkQuadPlaneSource *New();

  // Description:
  // Specify the resolution of the plane along the first axes.
  vtkSetMacro(XResolution,int);
  vtkGetMacro(XResolution,int);

  // Description:
  // Specify the resolution of the plane along the second axes.
  vtkSetMacro(YResolution,int);
  vtkGetMacro(YResolution,int);

  // Description:
  // Set the number of x-y subdivisions in the plane.
  void SetResolution(const int xR, const int yR);
  void GetResolution(int& xR,int& yR) {
    xR=this->XResolution; yR=this->YResolution;};

  // Description:
  // Specify a point defining the origin of the Quadrilateral plane.
  // Origin's diagonal point is the Point3.
  // The Quadrilateral plane is made up of four vertex is Origin,Point1,
  // Point3 and Point2 seeing from the counterclockwise.
  vtkSetVector3Macro(Origin,double);
  vtkGetVectorMacro(Origin,double,3);

  // Description:
  // Specify a point defining the first axis of the Quadrilateral plane.
  // Point1's diagonal point is Point2.
  // The Quadrilateral plane is made up of four vertex is Origin,Point1,
  // Point3 and Point2 seeing from the counterclockwise.
  void SetPoint1(double x, double y, double z);
  void SetPoint1(double pnt[3]);
  vtkGetVectorMacro(Point1,double,3);

  // Description:
  // Specify a point defining the second axis of the Quadrilateral plane.
  // Point2's diagonal point is Point1.
  // The Quadrilateral plane is made up of four vertex is Origin,Point1,
  // Point3 and Point2 seeing from the counterclockwise.
  void SetPoint2(double x, double y, double z);
  void SetPoint2(double pnt[3]);
  vtkGetVectorMacro(Point2,double,3);

  // Description:
  // Specify a point defining the third point of a Quadrilateral plane.
  // Point3's diagonal point is the Origin.
  // The Quadrilateral plane is made up of four vertex is Origin,Point1,
  // Point3 and Point2 seeing from the counterclockwise.
  void SetPoint3(double x, double y, double z);
  void SetPoint3(double pnt[3]);
  vtkGetVectorMacro(Point3,double,3);

  // Description:
  // The four points of the Quadrilateral plane should be on the same plane.
  // So if you just know the three point of the Quadrilateral plane,then use 
  // one of the following four methods to generate the last point. 
  void GenerateOrigin();
  void GeneratePoint1();
  void GeneratePoint2();
  void GeneratePoint3();

  // Description:
  // Set/Get the center of the plane. Works in conjunction with the plane 
  // normal to position the plane. Don't use this method to define the plane.
  // Instead, use it to move the plane to a new center point.
  void SetCenter(double x, double y, double z);
  void SetCenter(double center[3]);
  vtkGetVectorMacro(Center,double,3);

  // Description:
  // Set/Get the plane normal. Works in conjunction with the plane center to
  // orient the plane. Don't use this method to define the plane. Instead, use
  // it to rotate the plane around the current center point.
  void SetNormal(double nx, double ny, double nz);
  void SetNormal(double n[3]);
  vtkGetVectorMacro(Normal,double,3);

  // Description:
  // Translate the plane in the direction of the normal by the
  // distance specified.  Negative values move the plane in the
  // opposite direction.
  void Push(double distance);

protected:
  vtkQuadPlaneSource();
  ~vtkQuadPlaneSource() {};

  int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);

  int XResolution;
  int YResolution;
  double Origin[3];
  double Point1[3];
  double Point2[3];
  double Point3[3];
  double Normal[3];
  double Center[3];


  int UpdatePlane(double v1[3], double v2[3]);
private:
  vtkQuadPlaneSource(const vtkQuadPlaneSource&);  // Not implemented.
  void operator=(const vtkQuadPlaneSource&);  // Not implemented.
};

#endif
