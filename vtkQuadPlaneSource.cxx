/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkQuadPlaneSource.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkQuadPlaneSource.h"

#include "vtkCellArray.h"
#include "vtkFloatArray.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPoints.h"
#include "vtkPolyData.h"
#include "vtkTransform.h"

vtkStandardNewMacro(vtkQuadPlaneSource);

// Construct plane perpendicular to z-axis, resolution 1x1, width and height
// 1.0, and centered at the origin.
vtkQuadPlaneSource::vtkQuadPlaneSource()
{
  this->XResolution = 1;
  this->YResolution = 1;

  this->Origin[0] = this->Origin[1] = -0.5;
  this->Origin[2] = 0.0;

  this->Point1[0] = 0.5;
  this->Point1[1] = -0.5;
  this->Point1[2] = 0.0;

  this->Point2[0] = -0.5;
  this->Point2[1] = 0.5;
  this->Point2[2] = 0.0;

  this->Point3[0] = 0.5;
  this->Point3[1] = 0.5;
  this->Point3[2] = 0.0;

  this->Normal[2] = 1.0;
  this->Normal[0] = this->Normal[1] = 0.0;

  this->Center[0] = this->Center[1] = this->Center[2] = 0.0;

  this->SetNumberOfInputPorts(0);
}

// Set the number of x-y subdivisions in the plane.
void vtkQuadPlaneSource::SetResolution(const int xR, const int yR)
{
  if ( xR != this->XResolution || yR != this->YResolution )
    {
    this->XResolution = xR;
    this->YResolution = yR;

    this->XResolution = (this->XResolution > 0 ? this->XResolution : 1);
    this->YResolution = (this->YResolution > 0 ? this->YResolution : 1);

    this->Modified();
    }
}

int vtkQuadPlaneSource::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **vtkNotUsed(inputVector),
  vtkInformationVector *outputVector)
{
  // get the info object
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  // get the ouptut
  vtkPolyData *output = vtkPolyData::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

  double x[3], tc[2], v1[3], v2[3];
  vtkIdType pts[4];
  int i, j, ii;
  int numPts;
  int numPolys;
  vtkPoints *newPoints; 
  vtkFloatArray *newNormals;
  vtkFloatArray *newTCoords;
  vtkCellArray *newPolys;
  
  // Check input
  for ( i=0; i < 3; i++ )
    {
    v1[i] = this->Point1[i] - this->Origin[i];
    v2[i] = this->Point2[i] - this->Origin[i];
    }

  if ( !this->UpdatePlane(v1,v2) )
    {
    vtkErrorMacro(<<"Bad plane coordinate system");
    return 0;
    }

  // Set things up; allocate memory
  //
  numPts = (this->XResolution+1) * (this->YResolution+1);
  numPolys = this->XResolution * this->YResolution;

  newPoints = vtkPoints::New();
  newPoints->Allocate(numPts);
  newNormals = vtkFloatArray::New();
  newNormals->SetNumberOfComponents(3);
  newNormals->Allocate(3*numPts);
  newTCoords = vtkFloatArray::New();
  newTCoords->SetNumberOfComponents(2);
  newTCoords->Allocate(2*numPts);

  newPolys = vtkCellArray::New();
  newPolys->Allocate(newPolys->EstimateSize(numPolys,4));

  double* p1 = this->GetPoint1();
  double* p2 = this->GetPoint2();
  double* p3 = this->GetPoint3();

  for (numPts=0, i=0; i<(this->YResolution+1); i++)
    {
    tc[1] = static_cast<double>(i)/ this->YResolution;
    for (j=0; j<(this->XResolution+1); j++)
      {
      tc[0] = static_cast<double>(j) / this->XResolution;

      for ( ii=0; ii < 3; ii++)
        {
			x[ii] = this->Origin[ii] + i*(j*(p3[ii]-p1[ii]+Origin[ii]-p2[ii])/XResolution+(p2[ii]-Origin[ii]))/YResolution + j*(p1[ii]-Origin[ii])/XResolution;
        }

      newPoints->InsertPoint(numPts,x);
      newTCoords->InsertTuple(numPts,tc);
      newNormals->InsertTuple(numPts++,this->Normal);
      }
    }

  // Generate polygon connectivity
  //
  for (i=0; i<this->YResolution; i++)
    {
    for (j=0; j<this->XResolution; j++)
      {
      pts[0] = j + i*(this->XResolution+1);
      pts[1] = pts[0] + 1;
      pts[2] = pts[0] + this->XResolution + 2;
      pts[3] = pts[0] + this->XResolution + 1;
      newPolys->InsertNextCell(4,pts);
      }
    }

  // Update ourselves and release memory
  //
  output->SetPoints(newPoints);
  newPoints->Delete();

  newNormals->SetName("Normals");
  output->GetPointData()->SetNormals(newNormals);
  newNormals->Delete();

  newTCoords->SetName("TextureCoordinates");
  output->GetPointData()->SetTCoords(newTCoords);
  newTCoords->Delete();

  output->SetPolys(newPolys);
  newPolys->Delete();

  return 1;
}

// Set the normal to the plane. Will modify the Origin, Point1, and Point2
// instance variables as necessary (i.e., rotate the plane around its center).
void vtkQuadPlaneSource::SetNormal(double N[3])
{
  double n[3], rotVector[3], theta;

  //make sure input is decent
  n[0] = N[0];
  n[1] = N[1];
  n[2] = N[2];
  if ( vtkMath::Normalize(n) == 0.0 )
    {
    vtkErrorMacro(<<"Specified zero normal");
    return;
    }
  
  // Compute rotation vector using a transformation matrix.
  // Note that if normals are parallel then the rotation is either
  // 0 or 180 degrees.
  double dp = vtkMath::Dot(this->Normal,n);
  if ( dp >= 1.0 )
    {
    return; //zero rotation
    }
  else if ( dp <= -1.0 )
    {
    theta = 180.0;
    rotVector[0] = this->Point1[0] - this->Origin[0];
    rotVector[1] = this->Point1[1] - this->Origin[1];
    rotVector[2] = this->Point1[2] - this->Origin[2];
    }
  else
    {
    vtkMath::Cross(this->Normal,n,rotVector);
    theta = vtkMath::DegreesFromRadians( acos( dp ) );
    }

  // create rotation matrix
  vtkTransform *transform = vtkTransform::New();
  transform->PostMultiply();

  transform->Translate(-this->Center[0],-this->Center[1],-this->Center[2]);
  transform->RotateWXYZ(theta,rotVector[0],rotVector[1],rotVector[2]);
  transform->Translate(this->Center[0],this->Center[1],this->Center[2]);

  // transform the three defining points
  transform->TransformPoint(this->Origin,this->Origin);
  transform->TransformPoint(this->Point1,this->Point1);
  transform->TransformPoint(this->Point2,this->Point2);
  transform->TransformPoint(this->Point3,this->Point3);
    
  this->Normal[0] = n[0]; this->Normal[1] = n[1]; this->Normal[2] = n[2];

  this->Modified();
  transform->Delete();
}

// Set the normal to the plane. Will modify the Origin, Point1, and Point2
// instance variables as necessary (i.e., rotate the plane around its center).
void vtkQuadPlaneSource::SetNormal(double nx, double ny, double nz)
{
  double n[3];

  n[0] = nx; n[1] = ny; n[2] = nz;
  this->SetNormal(n);
}

// Set the center of the plane. Will modify the Origin, Point1, and Point2
// instance variables as necessary (i.e., translate the plane).
void vtkQuadPlaneSource::SetCenter(double center[3])
{
  if ( this->Center[0] == center[0] &&
       this->Center[1] == center[1] &&
       this->Center[2] == center[2] )
    {
    return; //no change
    }
  else
    {
    int i;
	double rco[3];double rc1[3];double rc2[3];double rc3[3];
	for ( i=0; i < 3; i++ )
      {
		  rco[i] = 3/4.0*Origin[i] - 1/4.0*(Point1[i]+Point2[i]+Point3[i]);
		  rc1[i] = 3/4.0*Point1[i] - 1/4.0*(Origin[i]+Point2[i]+Point3[i]);
		  rc2[i] = 3/4.0*Point2[i] - 1/4.0*(Point1[i]+Origin[i]+Point3[i]);
		  rc3[i] = 3/4.0*Point3[i] - 1/4.0*(Point1[i]+Point2[i]+Origin[i]);
      }
	for ( i=0; i < 3; i++ )
      {
      this->Center[i] = center[i];
      this->Origin[i] = this->Center[i] + rco[i];
      this->Point1[i] = this->Origin[i] + rc1[i];
      this->Point2[i] = this->Origin[i] + rc2[i];
	  this->Point3[i] = this->Origin[i] + rc3[i];
      }

    this->Modified();
    }
}

// Set the center of the plane. Will modify the Origin, Point1, and Point2
// instance variables as necessary (i.e., translate the plane).
void vtkQuadPlaneSource::SetCenter(double x, double y, double z)
{
  double center[3];

  center[0] = x; center[1] = y; center[2] = z;
  this->SetCenter(center);
}

// modifies the normal and origin
void vtkQuadPlaneSource::SetPoint1(double pnt[3])
{
  if ( this->Point1[0] == pnt[0] && this->Point1[1] == pnt[1] &&
       this->Point1[2] == pnt[2] )
    {
    return; //no change
    }
  else
    {
    int i;
    double v1[3], v2[3];

    for ( i=0; i < 3; i++ )
      {
      this->Point1[i] = pnt[i];
      v1[i] = this->Point1[i] - this->Origin[i];
      v2[i] = this->Point2[i] - this->Origin[i];
      }

    // set plane normal
    this->UpdatePlane(v1,v2);


    this->Modified();
    }
}

// modifies the normal and origin
void vtkQuadPlaneSource::SetPoint2(double pnt[3])
{
  if ( this->Point2[0] == pnt[0] &&
       this->Point2[1] == pnt[1] &&
       this->Point2[2] == pnt[2] )
    {
    return; //no change
    }
  else
    {
    int i;
    double v1[3], v2[3];

    for ( i=0; i < 3; i++ )
      {
      this->Point2[i] = pnt[i];
      v1[i] = this->Point1[i] - this->Origin[i];
      v2[i] = this->Point2[i] - this->Origin[i];
      }
    // set plane normal
    this->UpdatePlane(v1,v2);
    this->Modified();
    }
}

// modifies the normal and origin
void vtkQuadPlaneSource::SetPoint3(double pnt[3])
{
  if ( this->Point3[0] == pnt[0] &&
       this->Point3[1] == pnt[1] &&
       this->Point3[2] == pnt[2] )
    {
    return; //no change
    }
  else
    {
    int i;
    double v1[3], v2[3];

    for ( i=0; i < 3; i++ )
      {
      this->Point3[i] = pnt[i];
      v1[i] = this->Point1[i] - this->Origin[i];
      v2[i] = this->Point2[i] - this->Origin[i];
      }
    // set plane normal
    this->UpdatePlane(v1,v2);
    this->Modified();
    }
}

void vtkQuadPlaneSource::SetPoint1(double x, double y, double z)
{
  double pnt[3];

  pnt[0] = x; pnt[1] = y; pnt[2] = z;
  this->SetPoint1(pnt);
}

void vtkQuadPlaneSource::SetPoint2(double x, double y, double z)
{
  double pnt[3];

  pnt[0] = x; pnt[1] = y; pnt[2] = z;
  this->SetPoint2(pnt);
}

void vtkQuadPlaneSource::SetPoint3(double x, double y, double z)
{
  double pnt[3];

  pnt[0] = x; pnt[1] = y; pnt[2] = z;
  this->SetPoint3(pnt);
}

// Description:
// The four points of the Quadrilateral plane should be on the same plane.
// So if you just know the three point of the Quadrilateral plane,then use 
// one of the following four methods to generate the last point. 
void vtkQuadPlaneSource::GenerateOrigin()
{
  double o[3];
  double *pt1 = this->GetPoint1();
  double *pt2 = this->GetPoint2();
  double *pt3 = this->GetPoint3();

  double v13[3];
  double v23[3];
  double vo3[3];
  vtkMath::Subtract(pt1,pt3,v13);
  vtkMath::Subtract(pt2,pt3,v23);
  vtkMath::Add(v13,v23,vo3);
  vtkMath::Add(vo3,pt3,o);

  this->SetOrigin(o);
}

void vtkQuadPlaneSource::GeneratePoint1()
{
  double *o = this->GetOrigin();
  double pt1[3];
  double *pt2 = this->GetPoint2();
  double *pt3 = this->GetPoint3();

  double vo2[3];
  double v32[3];
  double v12[2];
  vtkMath::Subtract(o,pt2,vo2);
  vtkMath::Subtract(pt3,pt2,v32);
  vtkMath::Add(vo2,v32,v12);
  vtkMath::Add(v12,pt2,pt1);

  this->SetPoint1(pt1);
}

void vtkQuadPlaneSource::GeneratePoint2()
{
  double *o = this->GetOrigin();
  double *pt1 = this->GetPoint1();
  double pt2[3];
  double *pt3 = this->GetPoint3();

  double vo1[3];
  double v31[3];
  double v21[3];
  vtkMath::Subtract(o,pt1,vo1);
  vtkMath::Subtract(pt3,pt1,v31);
  vtkMath::Add(vo1,v31,v21);
  vtkMath::Add(v21,pt1,pt2);

  this->SetPoint2(pt2);
}

void vtkQuadPlaneSource::GeneratePoint3()
{
  double *o = this->GetOrigin();
  double *pt1 = this->GetPoint1();
  double *pt2 = this->GetPoint2();
  double pt3[3];

  double v1o[3];
  double v2o[3];
  double v3o[3];
  vtkMath::Subtract(pt1,o,v1o);
  vtkMath::Subtract(pt2,o,v2o);
  vtkMath::Add(v1o,v2o,v3o);
  vtkMath::Add(v3o,o,pt3);

  this->SetPoint3(pt3);
}

// Translate the plane in the direction of the normal by the distance specified.
// Negative values move the plane in the opposite direction.
void vtkQuadPlaneSource::Push(double distance)
{
  int i;

  if ( distance == 0.0 )
    {
    return;
    }
  for (i=0; i < 3; i++ )
    {
    this->Origin[i] += distance * this->Normal[i];
    this->Point1[i] += distance * this->Normal[i];
    this->Point2[i] += distance * this->Normal[i];
	this->Point3[i] += distance * this->Normal[i];
    }
  // set the new center
  for ( i=0; i < 3; i++ )
    {
		this->Center[i] = 1/4.0*(this->Origin[i]+this->Point1[i]+this->Point2[i]+this->Point3[i]);
    }

  this->Modified();
}

// Protected method updates normals and plane center from the intersection point of 
// two pairs of planes's oppsite sides's midpoint.
int vtkQuadPlaneSource::UpdatePlane(double v1[3], double v2[3])
{
  // set plane center
  for ( int i=0; i < 3; i++ )
    {
		this->Center[i] = 1/4.0*(this->Origin[i]+this->Point1[i]+this->Point2[i]+this->Point3[i]);
    }

  // set plane normal
  vtkMath::Cross(v1,v2,this->Normal);
  if ( vtkMath::Normalize(this->Normal) == 0.0 )
    {
    return 0;
    }
  else
    {
    return 1;
    }
}

void vtkQuadPlaneSource::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "X Resolution: " << this->XResolution << "\n";
  os << indent << "Y Resolution: " << this->YResolution << "\n";

  os << indent << "Origin: ("
     << this->Origin[0] << ", "
     << this->Origin[1] << ", "
     << this->Origin[2] << ")\n";

  os << indent << "Point 1: ("
     << this->Point1[0] << ", "
     << this->Point1[1] << ", "
     << this->Point1[2] << ")\n";

  os << indent << "Point 2: ("
     << this->Point2[0] << ", "
     << this->Point2[1] << ", "
     << this->Point2[2] << ")\n";

  os << indent << "Point 3: ("
     << this->Point3[0] << ", "
     << this->Point3[1] << ", "
     << this->Point3[2] << ")\n";

  os << indent << "Normal: ("
     << this->Normal[0] << ", "
     << this->Normal[1] << ", "
     << this->Normal[2] << ")\n";

  os << indent << "Center: ("
     << this->Center[0] << ", "
     << this->Center[1] << ", "
     << this->Center[2] << ")\n";

}
