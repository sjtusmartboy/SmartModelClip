#include"vtkQuadPlaneWidgetPlus.h"

#include "vtkActor.h"
#include "vtkAssemblyNode.h"
#include "vtkAssemblyPath.h"
#include "vtkCallbackCommand.h"
#include "vtkCamera.h"
#include "vtkCellArray.h"
#include "vtkCellPicker.h"
#include "vtkConeSource.h"
#include "vtkDoubleArray.h"
#include "vtkFloatArray.h"
#include "vtkLineSource.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"
#include "vtkPlane.h"
#include "vtkQuadPlaneSource.h"
#include "vtkPlanes.h"
#include "vtkPolyData.h"
#include "vtkPolyDataMapper.h"
#include "vtkProperty.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderer.h"
#include "vtkSphereSource.h"
#include "vtkTransform.h"

vtkStandardNewMacro(vtkQuadPlaneWidgetPlus);
vtkCxxSetObjectMacro(vtkQuadPlaneWidgetPlus,PlaneProperty,vtkProperty);

vtkQuadPlaneWidgetPlus::vtkQuadPlaneWidgetPlus():vtkQuadPlaneWidget()
{
	/*this->SetOrigin(-100, -100, 0);
	this->SetPoint1(-50, -100, 0);
	this->SetPoint2(-100, 100, 0);*/
	////this->SetPoint3(-50, 100, 0);
	//this->GeneratePoint3();

	this->SetRepresentationToSurface();
	//SetRepresentationToOff();
	//SetRepresentationToOutline();
	//this->PlaneSource->SetResolution(5,6);
    this->SetHandleSize( 3 );
	
};

vtkQuadPlaneWidgetPlus::~vtkQuadPlaneWidgetPlus(){};

void vtkQuadPlaneWidgetPlus::planeFixing(int Fixing)
{
	if(Fixing)
		this->Interactor->RemoveObserver(this->EventCallbackCommand);
	else
	{
		vtkRenderWindowInteractor *i = this->Interactor;
		i->AddObserver(vtkCommand::MouseMoveEvent, this->EventCallbackCommand, 
					   this->Priority);
		i->AddObserver(vtkCommand::LeftButtonPressEvent, 
					   this->EventCallbackCommand, this->Priority);
		i->AddObserver(vtkCommand::LeftButtonReleaseEvent, 
					   this->EventCallbackCommand, this->Priority);
		i->AddObserver(vtkCommand::MiddleButtonPressEvent, 
					   this->EventCallbackCommand, this->Priority);
		i->AddObserver(vtkCommand::MiddleButtonReleaseEvent, 
					   this->EventCallbackCommand, this->Priority);
		i->AddObserver(vtkCommand::RightButtonPressEvent, 
					   this->EventCallbackCommand, this->Priority);
		i->AddObserver(vtkCommand::RightButtonReleaseEvent, 
					   this->EventCallbackCommand, this->Priority);
	}
}

