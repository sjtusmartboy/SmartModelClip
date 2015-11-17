#ifndef __vtkQuadPlaneWidgetPlus_h
#define __vtkQuadPlaneWidgetPlus_h


#include"vtkQuadPlaneWidget.h"

class vtkActor;
class vtkCellPicker;
class vtkConeSource;
class vtkLineSource;
class vtkQuadPlaneSource;
class vtkPoints;
class vtkPolyData;
class vtkPolyDataMapper;
class vtkProp;
class vtkProperty;
class vtkSphereSource;
class vtkTransform;
class vtkPlane;


class vtkQuadPlaneWidgetPlus:public vtkQuadPlaneWidget
{
public:
	static vtkQuadPlaneWidgetPlus *New();
	vtkTypeMacro(vtkQuadPlaneWidgetPlus,vtkQuadPlaneWidget); //cause memory leak if comment this word

    virtual void SetPlaneProperty(vtkProperty*);

	/*vtkSetClampMacro(HandleSize,double,0.001,1000);
    vtkGetMacro(HandleSize,double);*/

	void planeFixing(int Fixing);

protected:
	vtkQuadPlaneWidgetPlus();
	virtual ~vtkQuadPlaneWidgetPlus();

};














#endif
