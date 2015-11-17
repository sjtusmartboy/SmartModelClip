/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

#ifndef __qSlicerSmartModelClipModuleWidget_h
#define __qSlicerSmartModelClipModuleWidget_h

#include <Qt/qlist.h>

// SlicerQt includes
#include "qSlicerAbstractModuleWidget.h"
#include "qSlicerSmartModelClipModuleExport.h"
#include <qSlicerApplication.h>

#include <vtkRenderWindow.h>
#include <vtkClipPolyData.h>
#include <vtkPlane.h>
#include <vtkPlaneWidget.h>
#include <vtkPlaneCollection.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>
#include <vtkImplicitBoolean.h>
#include <vtkAppendPolyData.h>
//#include <vtkPlaneWidget.h>
#include "vtkQuadPlaneWidget.h"
#include "vtkQuadPlaneWidgetPlus.h"
#include "vtkSpinningPlaneWidget.h"

class qSlicerSmartModelClipModuleWidgetPrivate;
class vtkMRMLNode;

/// \ingroup Slicer_QtModules_ExtensionTemplate
class Q_SLICER_QTMODULES_SMARTMODELCLIP_EXPORT qSlicerSmartModelClipModuleWidget :
	public qSlicerAbstractModuleWidget
{
	Q_OBJECT

public:
	typedef qSlicerAbstractModuleWidget Superclass;
	qSlicerSmartModelClipModuleWidget(QWidget *parent=0);
	virtual ~qSlicerSmartModelClipModuleWidget();

	vtkRenderer* renderer;
	vtkRenderWindow* renderWindow;
	vtkRenderWindowInteractor* renderWindowInteractor;

	vtkQuadPlaneWidget* planeWidget;
	vtkQuadPlaneWidgetPlus* DepthPlaneWidget;
	QList<vtkQuadPlaneWidget*> planeList;

	int numOfPlanes;

public slots:
	void createPlane();
	void deletePlane();
	void clearPlanes();
	void SetPlaneVisibility();
	void setDepthPlane();
	void clip();
	void reverseClippingPlane();
	void reverseDepthPlane();

protected:
	QScopedPointer<qSlicerSmartModelClipModuleWidgetPrivate> d_ptr;

	virtual void setup();

	QList<vtkSmartPointer<vtkPolyData>> reservedList;
	QList<vtkSmartPointer<vtkPolyData>> clippedList;

	int timesOfClip;
	int numOfFiducials;

private:
	//The algorithm of model clipping is based on recursion
	vtkSmartPointer<vtkImplicitBoolean> makeBody(int m,int n);

	//Specify the depth plane to clip the model 
	vtkSmartPointer<vtkImplicitBoolean> SetClipDepth(vtkSmartPointer<vtkImplicitFunction>);

    // get the position of a fiducial and return its coordinates.The function returns 0 if no fiducial is on the scenery.
	double* getPositionOfFiducials();

    // If the last plane's line segment is intersected with its previous planes' line segments twice,we define the 
    // first plane Of clipping model is the larger sequence number.Or we define the first plane Of clipping model 
    // is the plane 0
	int determineFirstPlaneOfClipping();

	// calculate the intersection point of the plane(which pass through Origin of plane m-2,
    // Point2 of plane m-2 and Point2 of plane m-1)and the line (which pass through Point3 and Point2 
    // of plane m).We define this intersection point as the plane m 's new coordinates of point2's.
    // We do this because it's convenient to judge whether the plane m is intersected with the previous plane
	double* CalIntersectionPointOfPlaneAndLine(int m);

	// judge whether the plane i is inside plane i-1(i>=1)
	int isPlaneInside(int i);

	// extend a line segment on plane i,the line of which pass through Point2,
    // from the Origin on the plane to infinitive
	double* extendLineSegment(int);

	// extend a line segment on plane i reversely,which pass through Origin,
    // from the Point2 on the plane to infinitive
	double* reverseExtendLineSegment(int i);//the first plane is m

	// judge whether the plane i will intersect with the previous plane from plane m to plane i-2
    // the line segment of plane i is extended infinitely on both of the line segment direction
	bool isIntersect1(int,int);

	bool isIntersect2(int,int);

	// If the last plane's line segment is intersected with its previous planes' line segments twice,we define the 
    // first plane Of clipping model is the larger sequence number.This function checks whteher the final plane's extended
    // line segment will intersected with the line segment of plane m.
	bool isInterSectWithTheSingleLineSegment(int m);

	// Point2 coordinates of first two planes satisfy such requirements that the line segment of Point2 and Point3 is
    // perpendicular with the line segment of Origin and Point1.
	double* CalculatePoint2CoordinatesOfFirstTwoPlanes(
		double* newPlaneOrigin,double* newPlanePoint1,double* newPlanePoint2);

	bool isReversedClippingPlane;
	bool isReversedDepthPlane;

	void setButtonState();

private:
	Q_DECLARE_PRIVATE(qSlicerSmartModelClipModuleWidget);
	Q_DISABLE_COPY(qSlicerSmartModelClipModuleWidget);
};

#endif
