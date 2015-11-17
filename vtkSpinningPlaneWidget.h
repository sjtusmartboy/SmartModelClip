#ifndef __vtkSpinningPlaneWidget_h
#define __vtkSpinningPlaneWidget_h

#include "vtkQuadPlaneWidget.h"

class vtkActor;
class vtkCellPicker;
class vtkConeSource;
class vtkLineSource;
class vtkPlaneSource;
class vtkPoints;
class vtkPolyData;
class vtkPolyDataMapper;
class vtkProp;
class vtkProperty;
class vtkSphereSource;
class vtkTransform;
class vtkPlane;

class vtkSpinningPlaneWidget:public vtkQuadPlaneWidget
{
public:
	// Description:Instantiate the object.
	static vtkSpinningPlaneWidget *New();
	vtkTypeMacro(vtkSpinningPlaneWidget, vtkQuadPlaneWidget);

	// Description:
	// Get the plane properties. The properties of the plane when selected 
	// and unselected can be manipulated.
	virtual void SetPlaneProperty(vtkProperty*);
	vtkGetObjectMacro(PlaneProperty,vtkProperty);
	vtkGetObjectMacro(SelectedPlaneProperty,vtkProperty);

	// Description:
	// Methods that satisfy the superclass' API.
	static void ProcessEvents(vtkObject* object, 
		unsigned long event,
		void* clientdata, 
		void* calldata);

	virtual void OnLeftButtonDown();
	virtual void OnLeftButtonUp();
	virtual void OnRightButtonDown();
	virtual void OnRightButtonUp();
	virtual void OnMouseMove();
	void Spin(double *p1, double *p2);

	int HighlightHandle(vtkProp *prop);
	int HighlightBoundary(vtkProp *prop);

protected:
	vtkSpinningPlaneWidget();

	virtual ~vtkSpinningPlaneWidget();
};
















#endif