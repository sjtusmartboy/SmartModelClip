/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkQuadPlaneWidget.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkQuadPlaneWidget.h"

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
#include "vtkLineWidget.h"


vtkStandardNewMacro(vtkQuadPlaneWidget);

vtkCxxSetObjectMacro(vtkQuadPlaneWidget,PlaneProperty,vtkProperty);

vtkQuadPlaneWidget::vtkQuadPlaneWidget() : vtkPolyDataSourceWidget()
{
  this->State = vtkQuadPlaneWidget::Start;
  this->EventCallbackCommand->SetCallback(vtkQuadPlaneWidget::ProcessEvents);
  
  this->NormalToXAxis = 0;
  this->NormalToYAxis = 0;
  this->NormalToZAxis = 0;
  //this->Representation = VTK_PLANE_WIREFRAME;
  this->Representation = VTK_PLANE_SURFACE;


  //Build the representation of the widget
  int i;
  // Represent the plane
  this->PlaneSource = vtkQuadPlaneSource::New();
  this->PlaneSource->SetXResolution(4);
  this->PlaneSource->SetYResolution(4);
  this->PlaneOutline = vtkPolyData::New();
  vtkPoints *pts = vtkPoints::New();
  pts->SetNumberOfPoints(4);
  vtkCellArray *outline = vtkCellArray::New();
  outline->InsertNextCell(4);
  outline->InsertCellPoint(0);
  outline->InsertCellPoint(1);
  outline->InsertCellPoint(2);
  outline->InsertCellPoint(3);
  this->PlaneOutline->SetPoints(pts);
  pts->Delete();
  this->PlaneOutline->SetPolys(outline);
  outline->Delete();
  this->PlaneMapper = vtkPolyDataMapper::New();
  this->PlaneMapper->SetInput(this->PlaneSource->GetOutput());
  this->PlaneActor = vtkActor::New();
  this->PlaneActor->SetMapper(this->PlaneMapper);

  // Create the handles
  this->Handle = new vtkActor* [4];
  this->HandleMapper = new vtkPolyDataMapper* [4];
  this->HandleGeometry = new vtkSphereSource* [4];
  for (i=0; i<4; i++)
    {
    this->HandleGeometry[i] = vtkSphereSource::New();
    this->HandleGeometry[i]->SetThetaResolution(16);
    this->HandleGeometry[i]->SetPhiResolution(8);
    this->HandleMapper[i] = vtkPolyDataMapper::New();
    this->HandleMapper[i]->SetInput(this->HandleGeometry[i]->GetOutput());
    this->Handle[i] = vtkActor::New();
    this->Handle[i]->SetMapper(this->HandleMapper[i]);
    }
  
  // Create the + plane normal
  this->LineSource = vtkLineSource::New();
  this->LineSource->SetResolution(1);
  this->LineMapper = vtkPolyDataMapper::New();
  this->LineMapper->SetInput(this->LineSource->GetOutput());
  this->LineActor = vtkActor::New();
  this->LineActor->SetMapper(this->LineMapper);

  this->ConeSource = vtkConeSource::New();
  this->ConeSource->SetResolution(12);
  this->ConeSource->SetAngle(25.0);
  this->ConeMapper = vtkPolyDataMapper::New();
  this->ConeMapper->SetInput(this->ConeSource->GetOutput());
  this->ConeActor = vtkActor::New();
  this->ConeActor->SetMapper(this->ConeMapper);

  // Create the - plane normal
  this->LineSource2 = vtkLineSource::New();
  this->LineSource2->SetResolution(1);
  this->LineMapper2 = vtkPolyDataMapper::New();
  this->LineMapper2->SetInput(this->LineSource2->GetOutput());
  this->LineActor2 = vtkActor::New();
  this->LineActor2->SetMapper(this->LineMapper2);

  this->ConeSource2 = vtkConeSource::New();
  this->ConeSource2->SetResolution(12);
  this->ConeSource2->SetAngle(25.0);
  //this->ConeSource2->SetRadius(50);
  this->ConeMapper2 = vtkPolyDataMapper::New();
  this->ConeMapper2->SetInput(this->ConeSource2->GetOutput());
  this->ConeActor2 = vtkActor::New();
  this->ConeActor2->SetMapper(this->ConeMapper2);

  //Create the Boundary Line
  BoundarySource = new vtkLineSource* [4];
  BoundaryMapper = new vtkPolyDataMapper* [4];
  BoundaryActor = new vtkActor* [4];
  for(int i=0;i<4;i++)
  {
	  this->BoundarySource[i]=vtkLineSource::New();
	  this->BoundarySource[i]->SetResolution(1);
	  this->BoundaryMapper[i] = vtkPolyDataMapper::New();
	  this->BoundaryMapper[i]->SetInput(this->BoundarySource[i]->GetOutput());
	  this->BoundaryActor[i] = vtkActor::New();
	  this->BoundaryActor[i]->SetMapper(this->BoundaryMapper[i]);
  }
  this->UpdateBoundary();

  this->Transform = vtkTransform::New();

  // Define the point coordinates
  double bounds[6];
  bounds[0] = -0.5;
  bounds[1] = 0.5;
  bounds[2] = -0.5;
  bounds[3] = 0.5;
  bounds[4] = -0.5;
  bounds[5] = 0.5;

  //Manage the picking stuff
  this->HandlePicker = vtkCellPicker::New();
  this->HandlePicker->SetTolerance(0.001);
  for (i=0; i<4; i++)
    {
    this->HandlePicker->AddPickList(this->Handle[i]);
    }
  this->HandlePicker->PickFromListOn();

  this->PlanePicker = vtkCellPicker::New();
  this->PlanePicker->SetTolerance(0.005); //need some fluff
  this->PlanePicker->AddPickList(this->PlaneActor);
  this->PlanePicker->AddPickList(this->ConeActor);
  this->PlanePicker->AddPickList(this->LineActor);
  this->PlanePicker->AddPickList(this->ConeActor2);
  this->PlanePicker->AddPickList(this->LineActor2);
  this->PlanePicker->PickFromListOn();

  this->BoundaryPicker = vtkCellPicker::New();
  this->BoundaryPicker->SetTolerance(0.005);
  for (i=0; i<4; i++)
  {
	  this->BoundaryPicker->AddPickList(this->BoundaryActor[i]);
  }
  this->BoundaryPicker->PickFromListOn();
  
  this->CurrentHandle = NULL;
  
  this->LastPickValid = 0;
  this->HandleSizeFactor = 1;
  this->SetHandleSize( 0.05 );
  
  // Set up the initial properties
  this->CreateDefaultProperties();
  
  this->SelectRepresentation();
  
  // Initial creation of the widget, serves to initialize it
  // Call PlaceWidget() LAST in the constructor as it depends on ivar
  // values.
  this->PlaceWidget(bounds);
  
  plane = vtkSmartPointer<vtkPlane>::New();
  
}

vtkQuadPlaneWidget::~vtkQuadPlaneWidget()
{
  this->PlaneActor->Delete();
  this->PlaneMapper->Delete();
  this->PlaneSource->Delete();
  this->PlaneOutline->Delete();

  for (int i=0; i<4; i++)
    {
    this->HandleGeometry[i]->Delete();
    this->HandleMapper[i]->Delete();
    this->Handle[i]->Delete();
    }
  for(int i=0;i<4;i++)
  {
	  this->BoundarySource[i]->Delete();
	  this->BoundaryMapper[i]->Delete();
	  this->BoundaryActor[i]->Delete();
  }


  delete [] this->Handle;
  delete [] this->HandleMapper;
  delete [] this->HandleGeometry;
  delete [] this->BoundarySource;
  delete [] this->BoundaryMapper;
  delete [] this->BoundaryActor;

  this->ConeActor->Delete();
  this->ConeMapper->Delete();
  this->ConeSource->Delete();

  this->LineActor->Delete();
  this->LineMapper->Delete();
  this->LineSource->Delete();

  this->ConeActor2->Delete();
  this->ConeMapper2->Delete();
  this->ConeSource2->Delete();

  this->LineActor2->Delete();
  this->LineMapper2->Delete();
  this->LineSource2->Delete();

  this->HandlePicker->Delete();
  this->PlanePicker->Delete();
  this->BoundaryPicker->Delete();

  if (this->HandleProperty)
    {
    this->HandleProperty->Delete();
    this->HandleProperty = 0;
    }
  if (this->BoundaryProperty)
  {
	  this->BoundaryProperty->Delete();
	  this->BoundaryProperty = 0;
  }

  if (this->SelectedHandleProperty)
    {
    this->SelectedHandleProperty->Delete();
    this->SelectedHandleProperty = 0;
    }

  if (this->SelectedBoundaryProperty)
    {
    this->SelectedBoundaryProperty->Delete();
    this->SelectedBoundaryProperty = 0;
    }

  if (this->PlaneProperty)
    {
    this->PlaneProperty->Delete();
    this->PlaneProperty = 0;
    }

  if (this->SelectedPlaneProperty)
    {
    this->SelectedPlaneProperty->Delete();
    this->SelectedPlaneProperty = 0;
    }

  this->Transform->Delete();

}

void vtkQuadPlaneWidget::SetEnabled(int enabling)
{
  if ( ! this->Interactor )
    {
    vtkErrorMacro(<<"The interactor must be set prior to enabling/disabling widget");
    return;
    }

  if ( enabling ) //-----------------------------------------------------------
    {
    vtkDebugMacro(<<"Enabling plane widget");

    if ( this->Enabled ) //already enabled, just return
      {
      return;
      }
    
    if ( ! this->CurrentRenderer )
      {
      this->SetCurrentRenderer(this->Interactor->FindPokedRenderer(
        this->Interactor->GetLastEventPosition()[0],
        this->Interactor->GetLastEventPosition()[1]));
      if (this->CurrentRenderer == NULL)
        {
        return;
        }
      }

    this->Enabled = 1;

    // listen for the following events
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

    // Add the plane
    this->CurrentRenderer->AddActor(this->PlaneActor);
    this->PlaneActor->SetProperty(this->PlaneProperty);

    // turn on the handles
    for (int j=0; j<4; j++)
      {
      this->CurrentRenderer->AddActor(this->Handle[j]);
      this->Handle[j]->SetProperty(this->HandleProperty);
	  this->CurrentRenderer->AddActor(this->BoundaryActor[j]);
	  this->BoundaryActor[j]->SetProperty(this->BoundaryProperty);
      }

    // add the normal vector
    this->CurrentRenderer->AddActor(this->LineActor);
    this->LineActor->SetProperty(this->HandleProperty);
    this->CurrentRenderer->AddActor(this->ConeActor);
    this->ConeActor->SetProperty(this->HandleProperty);
    this->CurrentRenderer->AddActor(this->LineActor2);
    this->LineActor2->SetProperty(this->HandleProperty);
    this->CurrentRenderer->AddActor(this->ConeActor2);
    this->ConeActor2->SetProperty(this->HandleProperty);

    this->SelectRepresentation();
    this->InvokeEvent(vtkCommand::EnableEvent,NULL);
    }
  
  else //disabling----------------------------------------------------------
    {
    vtkDebugMacro(<<"Disabling plane widget");

    if ( ! this->Enabled ) //already disabled, just return
      {
      return;
      }

	
    
    this->Enabled = 0;

    // don't listen for events any more
    this->Interactor->RemoveObserver(this->EventCallbackCommand);

    // turn off the plane
    this->CurrentRenderer->RemoveActor(this->PlaneActor);

    // turn off the handles
    for (int i=0; i<4; i++)
      {
      this->CurrentRenderer->RemoveActor(this->Handle[i]);
	  this->CurrentRenderer->RemoveActor(this->BoundaryActor[i]);
      }

    // turn off the normal vector
    this->CurrentRenderer->RemoveActor(this->LineActor);
    this->CurrentRenderer->RemoveActor(this->ConeActor);
    this->CurrentRenderer->RemoveActor(this->LineActor2);
    this->CurrentRenderer->RemoveActor(this->ConeActor2);

    this->CurrentHandle = NULL;
    this->InvokeEvent(vtkCommand::DisableEvent,NULL);
    this->SetCurrentRenderer(NULL);
    }

  this->Interactor->Render();
}

void vtkQuadPlaneWidget::ProcessEvents(vtkObject* vtkNotUsed(object), 
                                   unsigned long event,
                                   void* clientdata, 
                                   void* vtkNotUsed(calldata))
{
  vtkQuadPlaneWidget* self = reinterpret_cast<vtkQuadPlaneWidget *>( clientdata );

  //okay, let's do the right thing
  switch(event)
    {
    case vtkCommand::LeftButtonPressEvent:
      self->OnLeftButtonDown();
      break;
    case vtkCommand::LeftButtonReleaseEvent:
      self->OnLeftButtonUp();
      break;
    case vtkCommand::MiddleButtonPressEvent:
      self->OnMiddleButtonDown();
      break;
    case vtkCommand::MiddleButtonReleaseEvent:
      self->OnMiddleButtonUp();
      break;
    case vtkCommand::RightButtonPressEvent:
      self->OnRightButtonDown();
      break;
    case vtkCommand::RightButtonReleaseEvent:
      self->OnRightButtonUp();
      break;
    case vtkCommand::MouseMoveEvent:
      self->OnMouseMove();
      break;
    }
}

void vtkQuadPlaneWidget::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  if ( this->HandleProperty )
    {
    os << indent << "Handle Property: " << this->HandleProperty << "\n";
    }
  else
    {
    os << indent << "Handle Property: (none)\n";
    }
  if ( this->SelectedHandleProperty )
    {
    os << indent << "Selected Handle Property: " 
       << this->SelectedHandleProperty << "\n";
    }
  else
    {
    os << indent << "SelectedHandle Property: (none)\n";
    }

  if ( this->BoundaryProperty )
    {
    os << indent << "Boundary Property: " << this->BoundaryProperty << "\n";
    }
  else
    {
    os << indent << "Boundary Property: (none)\n";
    }
  if ( this->SelectedBoundaryProperty )
    {
    os << indent << "Selected Boundary Property: " 
       << this->SelectedBoundaryProperty << "\n";
    }
  else
    {
    os << indent << "SelectedBoundary Property: (none)\n";
    }

  if ( this->PlaneProperty )
    {
    os << indent << "Plane Property: " << this->PlaneProperty << "\n";
    }
  else
    {
    os << indent << "Plane Property: (none)\n";
    }
  if ( this->SelectedPlaneProperty )
    {
    os << indent << "Selected Plane Property: " 
       << this->SelectedPlaneProperty << "\n";
    }
  else
    {
    os << indent << "Selected Plane Property: (none)\n";
    }

  os << indent << "Plane Representation: ";
  if ( this->Representation == VTK_PLANE_WIREFRAME )
    {
    os << "Wireframe\n";
    }
  else if ( this->Representation == VTK_PLANE_SURFACE )
    {
    os << "Surface\n";
    }
  else //( this->Representation == VTK_PLANE_OUTLINE )
    {
    os << "Outline\n";
    }

  os << indent << "Normal To X Axis: " 
     << (this->NormalToXAxis ? "On" : "Off") << "\n";
  os << indent << "Normal To Y Axis: "
     << (this->NormalToYAxis ? "On" : "Off") << "\n";
  os << indent << "Normal To Z Axis: " 
     << (this->NormalToZAxis ? "On" : "Off") << "\n";

  int res = this->PlaneSource->GetXResolution();
  double *o = this->PlaneSource->GetOrigin();
  double *pt1 = this->PlaneSource->GetPoint1();
  double *pt2 = this->PlaneSource->GetPoint2();
  double *pt3 = this->PlaneSource->GetPoint3();

  os << indent << "Resolution: " << res << "\n";
  os << indent << "Origin: (" << o[0] << ", "
     << o[1] << ", "
     << o[2] << ")\n";
  os << indent << "Point 1: (" << pt1[0] << ", "
     << pt1[1] << ", "
     << pt1[2] << ")\n";
  os << indent << "Point 2: (" << pt2[0] << ", "
     << pt2[1] << ", "
     << pt2[2] << ")\n";
  os << indent << "Point 3: (" << pt3[0] << ", "
     << pt3[1] << ", "
     << pt3[2] << ")\n";
}

void vtkQuadPlaneWidget::PositionHandles()
{
	
  double *o = this->PlaneSource->GetOrigin();
  double *pt1 = this->PlaneSource->GetPoint1();
  double *pt2 = this->PlaneSource->GetPoint2();
  double *pt3 = this->PlaneSource->GetPoint3();

  this->HandleGeometry[0]->SetCenter(o);
  this->HandleGeometry[1]->SetCenter(pt1);
  this->HandleGeometry[2]->SetCenter(pt2);
  this->HandleGeometry[3]->SetCenter(pt3);
  
  // set up the outline
  if ( this->Representation == VTK_PLANE_OUTLINE )
    {
    this->PlaneOutline->GetPoints()->SetPoint(0,o);
    this->PlaneOutline->GetPoints()->SetPoint(1,pt1);
    //this->PlaneOutline->GetPoints()->SetPoint(2,x);
	this->PlaneOutline->GetPoints()->SetPoint(2,pt2);
    /*this->PlaneOutline->GetPoints()->SetPoint(3,pt2);*/
	this->PlaneOutline->GetPoints()->SetPoint(3,pt3);
    this->PlaneOutline->Modified();
    }
  this->SelectRepresentation();

  // Create the normal vector
  double center[3];
  this->PlaneSource->GetCenter(center);
  this->LineSource->SetPoint1(center);
  this->LineSource2->SetPoint1(center);
  double p2[3];
  this->PlaneSource->GetNormal(this->Normal);
  vtkMath::Normalize(this->Normal);
  double d = sqrt( 
    vtkMath::Distance2BetweenPoints(
      this->PlaneSource->GetPoint1(),this->PlaneSource->GetPoint2()) );

  p2[0] = center[0] + 0.35 * d * this->Normal[0];
  p2[1] = center[1] + 0.35 * d * this->Normal[1];
  p2[2] = center[2] + 0.35 * d * this->Normal[2];
  this->LineSource->SetPoint2(p2);
  this->ConeSource->SetCenter(p2);
  this->ConeSource->SetDirection(this->Normal);

  p2[0] = center[0] - 0.35 * d * this->Normal[0];
  p2[1] = center[1] - 0.35 * d * this->Normal[1];
  p2[2] = center[2] - 0.35 * d * this->Normal[2];
  this->LineSource2->SetPoint2(p2);
  this->ConeSource2->SetCenter(p2);
  this->ConeSource2->SetDirection(this->Normal);

  this->UpdateBoundary();
}

int vtkQuadPlaneWidget::HighlightHandle(vtkProp *prop)
{
  // first unhighlight anything picked
  if ( this->CurrentHandle )
    {
    this->CurrentHandle->SetProperty(this->HandleProperty);
    }

  this->CurrentHandle = static_cast<vtkActor *>(prop);

  if ( this->CurrentHandle )
    {
    this->ValidPick = 1;
    this->HandlePicker->GetPickPosition(this->LastPickPosition);
    this->CurrentHandle->SetProperty(this->SelectedHandleProperty);
    for (int i=0; i<4; i++) //find handle
      {
      if ( this->CurrentHandle == this->Handle[i] )
        {
        return i;
        }
      }
    }
  
  return -1;
}

int vtkQuadPlaneWidget::HighlightBoundary(vtkProp *prop)
{
  // first unhighlight anything picked
  if ( this->CurrentHandle )
    {
    this->CurrentHandle->SetProperty(this->BoundaryProperty);
    }

  this->CurrentHandle = static_cast<vtkActor *>(prop);

  if ( this->CurrentHandle )
    {
    this->ValidPick = 1;
    this->BoundaryPicker->GetPickPosition(this->LastPickPosition);
    this->CurrentHandle->SetProperty(this->SelectedBoundaryProperty);
    for (int i=0; i<4; i++) //find handle
      {
      if ( this->CurrentHandle == this->BoundaryActor[i] )
        {
        return i;
        }
      }
    }
  
  return -1;
}

void vtkQuadPlaneWidget::HighlightNormal(int highlight)
{
  if ( highlight )
    {
    this->ValidPick = 1;
    this->PlanePicker->GetPickPosition(this->LastPickPosition);
    this->LineActor->SetProperty(this->SelectedHandleProperty);
    this->ConeActor->SetProperty(this->SelectedHandleProperty);
    this->LineActor2->SetProperty(this->SelectedHandleProperty);
    this->ConeActor2->SetProperty(this->SelectedHandleProperty);
    }
  else
    {
    this->LineActor->SetProperty(this->HandleProperty);
    this->ConeActor->SetProperty(this->HandleProperty);
    this->LineActor2->SetProperty(this->HandleProperty);
    this->ConeActor2->SetProperty(this->HandleProperty);
    }
}

void vtkQuadPlaneWidget::HighlightPlane(int highlight)
{
  if ( highlight )
    {
    this->ValidPick = 1;
    this->PlanePicker->GetPickPosition(this->LastPickPosition);
    this->PlaneActor->SetProperty(this->SelectedPlaneProperty);
    }
  else
    {
    this->PlaneActor->SetProperty(this->PlaneProperty);
    }
}

void vtkQuadPlaneWidget::OnLeftButtonDown()
{
	int X = this->Interactor->GetEventPosition()[0];
	int Y = this->Interactor->GetEventPosition()[1];

	// Okay, make sure that the pick is in the current renderer
	if (!this->CurrentRenderer || !this->CurrentRenderer->IsInViewport(X, Y))
	{
		this->State = vtkQuadPlaneWidget::Outside;
		return;
	}

	// Okay, we can process this. Try to pick handles first;
	// if no handles picked, then try to pick the plane.
	vtkAssemblyPath *path;
	this->HandlePicker->Pick(X,Y,0.0,this->CurrentRenderer);
	path = this->HandlePicker->GetPath();

	if ( path != NULL )
	{
		this->State = vtkQuadPlaneWidget::Moving;
		this->HighlightHandle(path->GetFirstNode()->GetViewProp());
	}
	else
	{
		this->BoundaryPicker->Pick(X,Y,0.0,this->CurrentRenderer);
		path = this->BoundaryPicker->GetPath();
		if ( path != NULL )
		{
			vtkProp *prop = path->GetFirstNode()->GetViewProp();
			if(prop == this->BoundaryActor[0] || prop == this->BoundaryActor[1] ||
				prop == this->BoundaryActor[2] || prop == this->BoundaryActor[3])
			{
				this->State = vtkQuadPlaneWidget::BoundaryDragging;
				this->HighlightBoundary(path->GetFirstNode()->GetViewProp());
			}
		}
		else
		{
			this->PlanePicker->Pick(X,Y,0.0,this->CurrentRenderer);
			path = this->PlanePicker->GetPath();

			if ( path != NULL )
			{
				vtkProp *prop = path->GetFirstNode()->GetViewProp();
				if ( prop == this->ConeActor || prop == this->LineActor ||
					prop == this->ConeActor2 || prop == this->LineActor2 )
				{
					this->State = vtkQuadPlaneWidget::Rotating;
					this->HighlightNormal(1);
				}
				else if (this->Interactor->GetControlKey())
				{
					this->State = vtkQuadPlaneWidget::Spinning;
					this->HighlightNormal(1);
				}
				else if(prop == this->PlaneActor)
				{
					this->State = vtkQuadPlaneWidget::Moving;
					this->HighlightPlane(1);
				}
			}
			else
			{
				this->State = vtkQuadPlaneWidget::Outside;
				this->HighlightHandle(NULL);
				return;
			}
		}
	}

	this->EventCallbackCommand->SetAbortFlag(1);
	this->StartInteraction();
	this->InvokeEvent(vtkCommand::StartInteractionEvent,NULL);
	this->Interactor->Render();
}

void vtkQuadPlaneWidget::OnLeftButtonUp()
{
  if ( this->State == vtkQuadPlaneWidget::Outside ||
       this->State == vtkQuadPlaneWidget::Start )
    {
    return;
    }

  this->State = vtkQuadPlaneWidget::Start;
  if(this->CurrentHandle == this->Handle[0] || this->CurrentHandle == this->Handle[1] ||
  this->CurrentHandle == this->Handle[2] || this->CurrentHandle == this->Handle[3] )
		this->HighlightHandle(NULL);

  if(this->CurrentHandle == this->BoundaryActor[0] || this->CurrentHandle == this->BoundaryActor[1] ||
  this->CurrentHandle == this->BoundaryActor[2] || this->CurrentHandle == this->BoundaryActor[3] )
		this->HighlightBoundary(NULL);
  this->HighlightPlane(0);
  this->HighlightNormal(0);
  this->SizeHandles();

  this->EventCallbackCommand->SetAbortFlag(1);
  this->EndInteraction();
  this->InvokeEvent(vtkCommand::EndInteractionEvent,NULL);
  this->Interactor->Render();
}

void vtkQuadPlaneWidget::OnMiddleButtonDown()
{
  int X = this->Interactor->GetEventPosition()[0];
  int Y = this->Interactor->GetEventPosition()[1];

  // Okay, make sure that the pick is in the current renderer
  if (!this->CurrentRenderer || !this->CurrentRenderer->IsInViewport(X, Y))
    {
    this->State = vtkQuadPlaneWidget::Outside;
    return;
    }
  
  // Okay, we can process this. If anything is picked, then we
  // can start pushing the plane.
  vtkAssemblyPath *path;
  this->HandlePicker->Pick(X,Y,0.0,this->CurrentRenderer);
  path = this->HandlePicker->GetPath();
  if ( path != NULL )
    {
    this->State = vtkQuadPlaneWidget::Pushing;
    this->HighlightPlane(1);
    this->HighlightNormal(1);
    this->HighlightHandle(path->GetFirstNode()->GetViewProp());
    }
  else
    {
    this->PlanePicker->Pick(X,Y,0.0,this->CurrentRenderer);
    path = this->PlanePicker->GetPath();
    if ( path == NULL ) //nothing picked
      {
      this->State = vtkQuadPlaneWidget::Outside;
      return;
      }
    else
      {
      this->State = vtkQuadPlaneWidget::Pushing;
      this->HighlightNormal(1);
      this->HighlightPlane(1);
      }
    }
  
  this->EventCallbackCommand->SetAbortFlag(1);
  this->StartInteraction();
  this->InvokeEvent(vtkCommand::StartInteractionEvent,NULL);
  this->Interactor->Render();
}

void vtkQuadPlaneWidget::OnMiddleButtonUp()
{
  if ( this->State == vtkQuadPlaneWidget::Outside ||
       this->State == vtkQuadPlaneWidget::Start )
    {
    return;
    }

  this->State = vtkQuadPlaneWidget::Start;
  this->HighlightPlane(0);
  this->HighlightNormal(0);
  this->HighlightHandle(NULL);
  this->SizeHandles();
  
  this->EventCallbackCommand->SetAbortFlag(1);
  this->EndInteraction();
  this->InvokeEvent(vtkCommand::EndInteractionEvent,NULL);
  this->Interactor->Render();
}

void vtkQuadPlaneWidget::OnRightButtonDown()
{
  int X = this->Interactor->GetEventPosition()[0];
  int Y = this->Interactor->GetEventPosition()[1];

  // Okay, make sure that the pick is in the current renderer
  if (!this->CurrentRenderer || !this->CurrentRenderer->IsInViewport(X, Y))
    {
    this->State = vtkQuadPlaneWidget::Outside;
    return;
    }
  
  // Okay, we can process this. Try to pick handles first;
  // if no handles picked, then pick the bounding box.
  vtkAssemblyPath *path;
  this->HandlePicker->Pick(X,Y,0.0,this->CurrentRenderer);
  path = this->HandlePicker->GetPath();
  if ( path != NULL )
    {
    this->State = vtkQuadPlaneWidget::Scaling;
    this->HighlightPlane(1);
    this->HighlightHandle(path->GetFirstNode()->GetViewProp());
    }
  else //see if we picked the plane or a normal
    {
    this->PlanePicker->Pick(X,Y,0.0,this->CurrentRenderer);
    path = this->PlanePicker->GetPath();
    if ( path == NULL )
      {
      this->State = vtkQuadPlaneWidget::Outside;
      return;
      }
    else
      {
      this->State = vtkQuadPlaneWidget::Scaling;
	  this->HighlightNormal(1);
      this->HighlightPlane(1);
      }
    }
  
  this->EventCallbackCommand->SetAbortFlag(1);
  this->StartInteraction();
  this->InvokeEvent(vtkCommand::StartInteractionEvent,NULL);
  this->Interactor->Render();
}

void vtkQuadPlaneWidget::OnRightButtonUp()
{
  if ( this->State == vtkQuadPlaneWidget::Outside ||
       this->State == vtkQuadPlaneWidget::Start )
    {
    return;
    }

  this->State = vtkQuadPlaneWidget::Start;
  if(this->CurrentHandle == this->Handle[0] || this->CurrentHandle == this->Handle[1] ||
  this->CurrentHandle == this->Handle[2] || this->CurrentHandle == this->Handle[3] )
		this->HighlightHandle(NULL);

  if(this->CurrentHandle == this->BoundaryActor[0] || this->CurrentHandle == this->BoundaryActor[1] ||
  this->CurrentHandle == this->BoundaryActor[2] || this->CurrentHandle == this->BoundaryActor[3] )
		this->HighlightBoundary(NULL);
  this->HighlightPlane(0);
  this->HighlightNormal(0);
  this->SizeHandles();
  
  this->EventCallbackCommand->SetAbortFlag(1);
  this->EndInteraction();
  this->InvokeEvent(vtkCommand::EndInteractionEvent,NULL);
  this->Interactor->Render();
}

void vtkQuadPlaneWidget::OnMouseMove()
{
  // See whether we're active
  if ( this->State == vtkQuadPlaneWidget::Outside || 
       this->State == vtkQuadPlaneWidget::Start )
    {
    return;
    }
  
  int X = this->Interactor->GetEventPosition()[0];
  int Y = this->Interactor->GetEventPosition()[1];

  // Do different things depending on state
  // Calculations everybody does
  double focalPoint[4], pickPoint[4], prevPickPoint[4];
  double z, vpn[3];

  vtkCamera *camera = this->CurrentRenderer->GetActiveCamera();
  if ( !camera )
    {
    return;
    }

  // Compute the two points defining the motion vector
  this->ComputeWorldToDisplay(this->LastPickPosition[0], 
                              this->LastPickPosition[1],
                              this->LastPickPosition[2], focalPoint);
  z = focalPoint[2];
  this->ComputeDisplayToWorld(
    double(this->Interactor->GetLastEventPosition()[0]),
    double(this->Interactor->GetLastEventPosition()[1]),
    z, prevPickPoint);
  this->ComputeDisplayToWorld(double(X), double(Y), z, pickPoint);
  
  // Process the motion
  if ( this->State == vtkQuadPlaneWidget::Moving )
  {
	  // Okay to process
	  if ( this->CurrentHandle )
	  {
		  if ( this->CurrentHandle == this->Handle[0] )
		  {
			  this->MoveOrigin(prevPickPoint, pickPoint);
		  }
		  else if ( this->CurrentHandle == this->Handle[1] )
		  {
			  this->MovePoint1(prevPickPoint, pickPoint);
		  }
		  else if ( this->CurrentHandle == this->Handle[2] )
		  {
			  this->MovePoint2(prevPickPoint, pickPoint);
		  }
		  else if ( this->CurrentHandle == this->Handle[3] )
		  {
			  this->MovePoint3(prevPickPoint, pickPoint);
		  }
	  }
	  else //must be moving the plane
	  {
		  this->Translate(prevPickPoint, pickPoint);
	  }
  }
  else if ( this->State == vtkQuadPlaneWidget::Scaling )
  {
	  this->Scale(prevPickPoint, pickPoint, X, Y);
  }
  else if ( this->State == vtkQuadPlaneWidget::Pushing )
  {
	  this->Push(prevPickPoint, pickPoint);
  }
  else if ( this->State == vtkQuadPlaneWidget::Rotating )
  {
	  camera->GetViewPlaneNormal(vpn);
	  this->Rotate(X, Y, prevPickPoint, pickPoint, vpn);
  }
  else if ( this->State == vtkQuadPlaneWidget::Spinning )
  {
	  this->Spin(prevPickPoint, pickPoint);
  }
  else if (this->State == vtkQuadPlaneWidget::BoundaryDragging)
  {
	  this->BoundaryDrag(prevPickPoint, pickPoint);
  }

  // Interact, if desired
  this->EventCallbackCommand->SetAbortFlag(1);
  this->InvokeEvent(vtkCommand::InteractionEvent,NULL);
  
  this->Interactor->Render();
}

void vtkQuadPlaneWidget::MoveOrigin(double *p1, double *p2)
{
  //Get the plane definition
  double *o = this->PlaneSource->GetOrigin();   //返回一个数组的指针
  double *norm = PlaneSource->GetNormal();

  //define the vector of motion
  double v[3];
  v[0] = p2[0] - p1[0];
  v[1] = p2[1] - p1[1];
  v[2] = p2[2] - p1[2];

  double n[3];
  for (int i=0; i<3; i++)
  {
     n[i] = norm[i];
  }

  double vN = vtkMath::Norm(v);
  vtkMath::Normalize(n);

  double origin[3];
  for (int i=0; i<3; i++)
  {
	  origin[i] = o[i] + v[i] - vtkMath::Dot(v,n)*n[i];   //notice *n[i]
  }
  
  this->PlaneSource->SetOrigin(origin);  
  this->PlaneSource->Update();
  this->PositionHandles();
}

void vtkQuadPlaneWidget::MovePoint1(double *p1, double *p2)
{
 //Get the plane definition
  double *pt1 = this->PlaneSource->GetPoint1();
  double *norm = PlaneSource->GetNormal();
  
  double v[3];
  double n[3];

  for (int i=0; i<3; i++)
  {
	  v[i] = p2[i] - p1[i];
	  n[i] = norm[i];
  }

  double point1[3];
  for (int i=0; i<3; i++)
  {
	  point1[i] = pt1[i] + v[i] - vtkMath::Dot(v,n)*n[i];   //notice *n[i]
  }
  
  this->PlaneSource->SetPoint1(point1);
  this->PlaneSource->Update();
  this->PositionHandles();
}

void vtkQuadPlaneWidget::MovePoint2(double *p1, double *p2)
{
  //Get the plane definition
  double *pt2 = this->PlaneSource->GetPoint2();
  double *norm = PlaneSource->GetNormal();

  double v[3];
  double n[3];

  for (int i=0; i<3; i++)
  {
	  v[i] = p2[i] - p1[i];
	  n[i] = norm[i];
  }

  double point2[3];
  for (int i=0; i<3; i++)
  {
	  point2[i] = pt2[i] + v[i] - vtkMath::Dot(v,n)*n[i];   //notice *n[i]
  }

  this->PlaneSource->SetPoint2(point2);
  this->PlaneSource->Update();
  this->PositionHandles();
}

void vtkQuadPlaneWidget::MovePoint3(double *p1, double *p2)
{
	//Get the plane definition
  double *pt3 = this->PlaneSource->GetPoint3();
  double *norm = PlaneSource->GetNormal();

  double v[3];
  double n[3];
  //Get the vector of motion
  for (int i=0; i<3; i++)
  {
	  v[i] = p2[i] - p1[i];
	  n[i] = norm[i];
  }

  double point3[3];
  for (int i=0; i<3; i++)
  {
	  point3[i] = pt3[i] + v[i] - vtkMath::Dot(v,n)*n[i];   //notice *n[i]
  }

  this->PlaneSource->SetPoint3(point3);
  this->PlaneSource->Update();
  this->PositionHandles();
}

void vtkQuadPlaneWidget::BoundaryDrag(double *p1, double *p2)
{
	if (CurrentHandle == this->BoundaryActor[0])
	{
		this->MoveOrigin(p1, p2);
		this->MovePoint1(p1, p2);
	}
	else if (CurrentHandle == this->BoundaryActor[1])
	{
		this->MovePoint1(p1, p2);
		this->MovePoint3(p1, p2);
	}
	else if (CurrentHandle == this->BoundaryActor[2])
	{
		this->MovePoint2(p1, p2);
		this->MovePoint3(p1, p2);
	}
	else if (CurrentHandle == this->BoundaryActor[3])
	{
		this->MovePoint2(p1, p2);
		this->MoveOrigin(p1, p2);
	}

	for(int i=0;i<4;i++)
		this->BoundarySource[i]->Update();
	this->PositionHandles();
}

//work while button down on the axes and rotate
void vtkQuadPlaneWidget::Rotate(int X, int Y, double *p1, double *p2, double *vpn)
{
  double *o = this->PlaneSource->GetOrigin();
  double *pt1 = this->PlaneSource->GetPoint1();
  double *pt2 = this->PlaneSource->GetPoint2();
  double *pt3 = this->PlaneSource->GetPoint3();
  double *center = this->PlaneSource->GetCenter();

  double v[3]; //vector of motion
  double axis[3]; //axis of rotation
  double theta; //rotation angle

  // mouse motion vector in world space
  v[0] = p2[0] - p1[0];
  v[1] = p2[1] - p1[1];
  v[2] = p2[2] - p1[2];

  // Create axis of rotation and angle of rotation
  vtkMath::Cross(vpn,v,axis);
  if ( vtkMath::Normalize(axis) == 0.0 )
    {
    return;
    }
  int *size = this->CurrentRenderer->GetSize();
  double l2 =
    (X-this->Interactor->GetLastEventPosition()[0])*
    (X-this->Interactor->GetLastEventPosition()[0]) + 
    (Y-this->Interactor->GetLastEventPosition()[1])*
    (Y-this->Interactor->GetLastEventPosition()[1]);
  theta = 360.0 * sqrt(l2/(size[0]*size[0]+size[1]*size[1]));

  //Manipulate the transform to reflect the rotation
  this->Transform->Identity();
  this->Transform->Translate(center[0],center[1],center[2]);
  this->Transform->RotateWXYZ(theta,axis);
  this->Transform->Translate(-center[0],-center[1],-center[2]);

  //Set the corners
  double oNew[3], pt1New[3], pt2New[3],pt3New[3];
  this->Transform->TransformPoint(o,oNew);
  this->Transform->TransformPoint(pt1,pt1New);
  this->Transform->TransformPoint(pt2,pt2New);
  this->Transform->TransformPoint(pt3,pt3New);

  this->PlaneSource->SetOrigin(oNew);
  this->PlaneSource->SetPoint1(pt1New);
  this->PlaneSource->SetPoint2(pt2New);
  this->PlaneSource->SetPoint3(pt3New);
  this->PlaneSource->Update();

  this->PositionHandles();
}

//work while left button down on the plane with the 'Ctrl' and then spin around the Normal axes 
void vtkQuadPlaneWidget::Spin(double *p1, double *p2)
{
  // Mouse motion vector in world space
  double v[3];
  v[0] = p2[0] - p1[0];
  v[1] = p2[1] - p1[1];
  v[2] = p2[2] - p1[2];

  double* normal = this->PlaneSource->GetNormal();
  // Axis of rotation
  double axis[3] = { normal[0], normal[1], normal[2] };
  vtkMath::Normalize(axis);

  double *o = this->PlaneSource->GetOrigin();
  double *pt1 = this->PlaneSource->GetPoint1();
  double *pt2 = this->PlaneSource->GetPoint2();
  double *pt3 = this->PlaneSource->GetPoint3();
  double *center = this->PlaneSource->GetCenter();

  // Radius vector (from center to cursor position)
  double rv[3] = {p2[0] - center[0],
                  p2[1] - center[1],
                  p2[2] - center[2]};

  // Distance between center and cursor location
  double rs = vtkMath::Normalize(rv);

  // Spin direction
  double ax_cross_rv[3];
  vtkMath::Cross(axis,rv,ax_cross_rv);

  // Spin angle
  double theta = vtkMath::DegreesFromRadians( vtkMath::Dot( v, ax_cross_rv ) / rs );

  // Manipulate the transform to reflect the rotation
  this->Transform->Identity();
  this->Transform->Translate(center[0],center[1],center[2]);
  this->Transform->RotateWXYZ(theta,axis);
  this->Transform->Translate(-center[0],-center[1],-center[2]);

  //Set the corners
  double oNew[3], pt1New[3], pt2New[3],pt3New[3];
  this->Transform->TransformPoint(o,oNew);
  this->Transform->TransformPoint(pt1,pt1New);
  this->Transform->TransformPoint(pt2,pt2New);
  this->Transform->TransformPoint(pt3,pt3New);

  this->PlaneSource->SetOrigin(oNew);
  this->PlaneSource->SetPoint1(pt1New);
  this->PlaneSource->SetPoint2(pt2New);
  this->PlaneSource->SetPoint3(pt3New);
  this->PlaneSource->Update();

  this->PositionHandles();
}

//work while left button down on the plane and drag
// Loop through all points and translate them
void vtkQuadPlaneWidget::Translate(double *p1, double *p2)
{
  //Get the motion vector
  double v[3];
  v[0] = p2[0] - p1[0];
  v[1] = p2[1] - p1[1];
  v[2] = p2[2] - p1[2];
  
  //int res = this->PlaneSource->GetXResolution();
  double *o = this->PlaneSource->GetOrigin();
  double *pt1 = this->PlaneSource->GetPoint1();
  double *pt2 = this->PlaneSource->GetPoint2();
  double *pt3 = this->PlaneSource->GetPoint3();

  double origin[3], point1[3], point2[3],point3[3];
  for (int i=0; i<3; i++)
    {
    origin[i] = o[i] + v[i];
    point1[i] = pt1[i] + v[i];
    point2[i] = pt2[i] + v[i];
	point3[i] = pt3[i] + v[i];
    }
  
  this->PlaneSource->SetOrigin(origin);
  this->PlaneSource->SetPoint1(point1);
  this->PlaneSource->SetPoint2(point2);
  this->PlaneSource->SetPoint3(point3);
  this->PlaneSource->Update();

  this->PositionHandles();
}

//work while right button down on the plane and drag
void vtkQuadPlaneWidget::Scale(double *p1, double *p2, int vtkNotUsed(X), int Y)
{
  //Get the motion vector
  double v[3];
  v[0] = p2[0] - p1[0];
  v[1] = p2[1] - p1[1];
  v[2] = p2[2] - p1[2];

  //int res = this->PlaneSource->GetXResolution();
  double *o = this->PlaneSource->GetOrigin();
  double *pt1 = this->PlaneSource->GetPoint1();
  double *pt2 = this->PlaneSource->GetPoint2();
  double *pt3 = this->PlaneSource->GetPoint3();

  double center[3];
  for(int i=0;i<3;i++)
  {
	  center[i]=o[i]+pt1[i]+pt2[i]+pt3[i];
  }

  // Compute the scale factor
  double sf = 
    vtkMath::Norm(v) / sqrt(vtkMath::Distance2BetweenPoints(pt1,pt2));
  if ( Y > this->Interactor->GetLastEventPosition()[1] )
    {
    sf = 1.0 + sf;
    }
  else
    {
    sf = 1.0 - sf;
    }
  
  // Move the corner points
  double origin[3], point1[3], point2[3],point3[3];
  for (int i=0; i<3; i++)
    {
    origin[i] = sf * (o[i] - center[i]) + center[i];
    point1[i] = sf * (pt1[i] - center[i]) + center[i];
    point2[i] = sf * (pt2[i] - center[i]) + center[i];
	point3[i] = sf * (pt3[i] - center[i]) + center[i];
    }

  this->PlaneSource->SetOrigin(origin);
  this->PlaneSource->SetPoint1(point1);
  this->PlaneSource->SetPoint2(point2);
  this->PlaneSource->SetPoint3(point3);
  this->PlaneSource->Update();

  this->PositionHandles();
}

//work while middle button down on the plane and drag
void vtkQuadPlaneWidget::Push(double *p1, double *p2)
{
  //Get the motion vector
  double v[3];
  v[0] = p2[0] - p1[0];
  v[1] = p2[1] - p1[1];
  v[2] = p2[2] - p1[2];
  
  this->PlaneSource->Push( vtkMath::Dot(v,this->Normal) );
  this->PlaneSource->Update();
  this->PositionHandles();
}

void vtkQuadPlaneWidget::CreateDefaultProperties()
{
  // Handle properties
  this->HandleProperty = vtkProperty::New();
  this->HandleProperty->SetColor(1,1,1);
  this->BoundaryProperty = vtkProperty::New();
  /*this->BoundaryProperty->SetColor(0.6,0.6,0.4);*/
  this->BoundaryProperty->SetColor(0,0,1);
  this->BoundaryProperty->SetLineWidth(3);

  this->SelectedHandleProperty = vtkProperty::New();
  this->SelectedHandleProperty->SetColor(1,0,0);
  this->SelectedBoundaryProperty = vtkProperty::New();
  this->SelectedBoundaryProperty->SetColor(1,0,0);
  this->SelectedBoundaryProperty->SetLineWidth(3);

  // Plane properties
  this->PlaneProperty = vtkProperty::New();
  this->PlaneProperty->SetAmbient(1.0);
  this->PlaneProperty->SetAmbientColor(1.0,1.0,1.0);

  this->SelectedPlaneProperty = vtkProperty::New();
  this->SelectRepresentation();
  this->SelectedPlaneProperty->SetAmbient(1.0);
  this->SelectedPlaneProperty->SetAmbientColor(0.0,1.0,0.0);
}

void vtkQuadPlaneWidget::PlaceWidget(double bds[6])
{
  int i;
  double bounds[6], center[3];

  this->AdjustBounds(bds, bounds, center);

  if (this->Input || this->Prop3D)
    {
    if ( this->NormalToYAxis )
      {
      this->PlaneSource->SetOrigin(bounds[0],center[1],bounds[4]);
      this->PlaneSource->SetPoint1(bounds[1],center[1],bounds[4]);
      this->PlaneSource->SetPoint2(bounds[0],center[1],bounds[5]);
      }
    else if ( this->NormalToZAxis )
      {
      this->PlaneSource->SetOrigin(bounds[0],bounds[2],center[2]);
      this->PlaneSource->SetPoint1(bounds[1],bounds[2],center[2]);
      this->PlaneSource->SetPoint2(bounds[0],bounds[3],center[2]);
      }
    else //default or x-normal
      {
      this->PlaneSource->SetOrigin(center[0],bounds[2],bounds[4]);
      this->PlaneSource->SetPoint1(center[0],bounds[3],bounds[4]);
      this->PlaneSource->SetPoint2(center[0],bounds[2],bounds[5]);
      }
    }

  this->PlaneSource->Update();

  // Position the handles at the end of the planes
  this->PositionHandles();

  for (i=0; i<6; i++)
    {
    this->InitialBounds[i] = bounds[i];
    }
  

  if (this->Input || this->Prop3D)
    {
    this->InitialLength = sqrt((bounds[1]-bounds[0])*(bounds[1]-bounds[0]) +
                               (bounds[3]-bounds[2])*(bounds[3]-bounds[2]) +
                               (bounds[5]-bounds[4])*(bounds[5]-bounds[4]));
    }
  else
    {
    // this means we have to make use of the PolyDataSource, so
    // we just calculate the magnitude of the longest diagonal on
    // the plane and use that as InitialLength
    double origin[3], point1[3], point2[3];
    this->PlaneSource->GetOrigin(origin);
    this->PlaneSource->GetPoint1(point1);
    this->PlaneSource->GetPoint2(point2);
    double sqr1 = 0, sqr2 = 0;
    for (i = 0; i < 3; i++)
      {
      sqr1 += (point1[i] - origin[i]) * (point1[i] - origin[i]);
      sqr2 += (point2[i] - origin[i]) * (point2[i] - origin[i]);
      }

    this->InitialLength = sqrt(sqr1 + sqr2);
    }
  
  // Set the radius on the sphere handles
  this->SizeHandles();
}

void vtkQuadPlaneWidget::SizeHandles()
{
	double radius = this->vtk3DWidget::SizeHandles(this->HandleSizeFactor);

	if (this->ValidPick && !this->LastPickValid)
	{
		// Adjust factor to preserve old radius.
		double oldradius = this->HandleGeometry[0]->GetRadius();
		if (oldradius != 0 && radius != 0)
		{
			this->HandleSizeFactor = oldradius / radius;
			radius = oldradius;
		}
	}
	

	this->LastPickValid = this->ValidPick;

	for(int i=0; i<4; i++)
	{
		this->HandleGeometry[i]->SetRadius(radius);
	}

	// Set the height and radius of the cone
	this->ConeSource->SetHeight(2.0*radius);
	this->ConeSource->SetRadius(radius);
	this->ConeSource2->SetHeight(2.0*radius);
	this->ConeSource2->SetRadius(radius);
}

void vtkQuadPlaneWidget::SelectRepresentation()
{
  if ( ! this->CurrentRenderer )
    {
    return;
    }

  if ( this->Representation == VTK_PLANE_OFF )
    {
    this->CurrentRenderer->RemoveActor(this->PlaneActor);
    }
  else if ( this->Representation == VTK_PLANE_OUTLINE )
    {
    this->CurrentRenderer->RemoveActor(this->PlaneActor);
    this->CurrentRenderer->AddActor(this->PlaneActor);
    this->PlaneMapper->SetInput( this->PlaneOutline );
    this->PlaneActor->GetProperty()->SetRepresentationToWireframe();
	
    }
  else if ( this->Representation == VTK_PLANE_SURFACE )
    {
    this->CurrentRenderer->RemoveActor(this->PlaneActor);
    this->CurrentRenderer->AddActor(this->PlaneActor);
    this->PlaneMapper->SetInput( this->PlaneSource->GetOutput() );
    this->PlaneActor->GetProperty()->SetRepresentationToSurface();
    }
  else //( this->Representation == VTK_PLANE_WIREFRAME )
    {
    this->CurrentRenderer->RemoveActor(this->PlaneActor);
    this->CurrentRenderer->AddActor(this->PlaneActor);
    this->PlaneMapper->SetInput( this->PlaneSource->GetOutput() );
    this->PlaneActor->GetProperty()->SetRepresentationToWireframe();
    }
}

// Description:
// Set/Get the resolution (number of subdivisions) of the plane.
void vtkQuadPlaneWidget::SetResolution(int r)
{
  this->PlaneSource->SetXResolution(r); 
  this->PlaneSource->SetYResolution(r); 
}

int vtkQuadPlaneWidget::GetResolution()
{ 
  return this->PlaneSource->GetXResolution(); 
}

// Description:
// Set/Get the origin of the plane.
void vtkQuadPlaneWidget::SetOrigin(double x, double y, double z) 
{
  this->PlaneSource->SetOrigin(x,y,z);
  this->PositionHandles();
}

void vtkQuadPlaneWidget::SetOrigin(double x[3]) 
{
  this->SetOrigin(x[0], x[1], x[2]);
}

double* vtkQuadPlaneWidget::GetOrigin() 
{
  return this->PlaneSource->GetOrigin();
}

void vtkQuadPlaneWidget::GetOrigin(double xyz[3]) 
{
  this->PlaneSource->GetOrigin(xyz);
}

// Description:
// Set/Get the position of the point defining the first axis of the plane.
void vtkQuadPlaneWidget::SetPoint1(double x, double y, double z) 
{
  this->PlaneSource->SetPoint1(x,y,z);
  this->PositionHandles();
}

void vtkQuadPlaneWidget::SetPoint1(double x[3]) 
{
  this->SetPoint1(x[0], x[1], x[2]);
}

double* vtkQuadPlaneWidget::GetPoint1() 
{
  return this->PlaneSource->GetPoint1();
}

void vtkQuadPlaneWidget::GetPoint1(double xyz[3]) 
{
  this->PlaneSource->GetPoint1(xyz);
}

// Description:
// Set/Get the position of the point defining the second axis of the plane.
void vtkQuadPlaneWidget::SetPoint2(double x, double y, double z) 
{
  this->PlaneSource->SetPoint2(x,y,z);
  this->PositionHandles();
}

void vtkQuadPlaneWidget::SetPoint2(double x[3]) 
{
  this->SetPoint2(x[0], x[1], x[2]);
}

double* vtkQuadPlaneWidget::GetPoint2() 
{
  return this->PlaneSource->GetPoint2();
}

void vtkQuadPlaneWidget::GetPoint2(double xyz[3]) 
{
  this->PlaneSource->GetPoint2(xyz);
}

// Description:
// Set/Get the position of the point defining the third vertex of the plane
void vtkQuadPlaneWidget::SetPoint3(double x, double y, double z) 
{
  this->PlaneSource->SetPoint3(x,y,z);
  this->PositionHandles();
}

void vtkQuadPlaneWidget::SetPoint3(double x[3]) 
{
  this->SetPoint3(x[0], x[1], x[2]);
}

double* vtkQuadPlaneWidget::GetPoint3() 
{
  return this->PlaneSource->GetPoint3();
}

void vtkQuadPlaneWidget::GetPoint3(double xyz[3]) 
{
  this->PlaneSource->GetPoint3(xyz);
}

// Description:
// The four points of the Quadrilateral plane should be on the same plane.
// So if you just know the three point of the Quadrilateral plane,then use 
// one of the following four methods to generate the last point. 
void vtkQuadPlaneWidget::GenerateOrigin()
{
	this->PlaneSource->GenerateOrigin();
	this->PositionHandles();
}

void vtkQuadPlaneWidget::GeneratePoint1()
{
	this->PlaneSource->GeneratePoint1();
	this->PositionHandles();
}

void vtkQuadPlaneWidget::GeneratePoint2()
{
  this->PlaneSource->GeneratePoint2();
  this->PositionHandles();
}

void vtkQuadPlaneWidget::GeneratePoint3()
{
  this->PlaneSource->GeneratePoint3();
  this->PositionHandles();
}

// Description:
// Set the center of the plane.
void vtkQuadPlaneWidget::SetCenter(double x, double y, double z) 
{
  this->PlaneSource->SetCenter(x, y, z);
  this->PositionHandles();
}

// Description:
// Set the center of the plane.
void vtkQuadPlaneWidget::SetCenter(double c[3]) 
{
  this->SetCenter(c[0], c[1], c[2]);
}

// Description:
// Get the center of the plane.
double* vtkQuadPlaneWidget::GetCenter() 
{
  return this->PlaneSource->GetCenter();
}

void vtkQuadPlaneWidget::GetCenter(double xyz[3]) 
{
  this->PlaneSource->GetCenter(xyz);
}

// Description:
// Set the normal to the plane.
void vtkQuadPlaneWidget::SetNormal(double x, double y, double z) 
{
  this->PlaneSource->SetNormal(x, y, z);
  this->PositionHandles();
}

// Description:
// Set the normal to the plane.
void vtkQuadPlaneWidget::SetNormal(double n[3]) 
{
  this->SetNormal(n[0], n[1], n[2]);
}

// Description:
// Get the normal to the plane.
double* vtkQuadPlaneWidget::GetNormal() 
{
  return this->PlaneSource->GetNormal();
}

void vtkQuadPlaneWidget::GetNormal(double xyz[3]) 
{
  this->PlaneSource->GetNormal(xyz);
}

// Description:
//Set the handle size 
void vtkQuadPlaneWidget::SetHandleSize(double size)
{
	if(size>0)
	{
		this->HandleSize = size;
		this->Modified();
		this->SizeHandles();
	}	
}

// Description:
// Set the Visibility of the plane or its handles.The input 
// argument visibility '1'represents as on and '0'as off.
void vtkQuadPlaneWidget::SetPlaneVisibility(int visibility)
{
	if(visibility)
	{
		this->SetHandlesVisibility(1);
		PlaneActor->VisibilityOn();
	}
	else
	{
		this->SetHandlesVisibility(0);
		PlaneActor->VisibilityOff();
	}
}

void vtkQuadPlaneWidget::SetHandlesVisibility(int visibility)
{
	if(visibility)
	{
		for(int i=0;i<4;i++)
		{
			this->Handle[i]->VisibilityOn();
			this->BoundaryActor[i]->VisibilityOn();
		}
		LineActor->VisibilityOn();
		ConeActor->VisibilityOn();
		LineActor2->VisibilityOn();
		ConeActor2->VisibilityOn();
	}
	else
	{
		for(int i=0;i<4;i++)
		{
			this->Handle[i]->VisibilityOff();
			this->BoundaryActor[i]->VisibilityOff();
		}
		LineActor->VisibilityOff();
		ConeActor->VisibilityOff();
		LineActor2->VisibilityOff();
		ConeActor2->VisibilityOff();
	}
}

// Description:
// Set the Color of the plane.Default color is white
void vtkQuadPlaneWidget::SetPlaneColor(double r,double g,double b)
{
	this->PlaneProperty->SetColor(r,g,b);
}

void vtkQuadPlaneWidget::GetPolyData(vtkPolyData *pd)
{ 
  pd->ShallowCopy(this->PlaneSource->GetOutput()); 
}

vtkPolyDataAlgorithm *vtkQuadPlaneWidget::GetPolyDataAlgorithm()
{
  return this->PlaneSource;
}

//Description:
//Get the Implicit function of the Quadrilateral plane.
void vtkQuadPlaneWidget::GetPlane(vtkPlane *plane)
{
  if ( plane == NULL )
    {
    return;
    }
  
  plane->SetNormal(this->GetNormal());
  plane->SetOrigin(this->GetCenter());
}

//Description:
//get the implicit function of class vtkQuadPlaneWidget and return;
vtkPlane* vtkQuadPlaneWidget::GetPlane()
{
	GetPlane(plane);
	return this->plane;
}

void vtkQuadPlaneWidget::UpdatePlacement(void)
{
  this->PlaneSource->Update();
  this->PositionHandles();
}

void vtkQuadPlaneWidget::UpdateBoundary()
{
	this->BoundarySource[0]->SetPoint1(this->GetOrigin());
	this->BoundarySource[0]->SetPoint2(this->GetPoint1());
	this->BoundarySource[1]->SetPoint1(this->GetPoint1());
	this->BoundarySource[1]->SetPoint2(this->GetPoint3());
	this->BoundarySource[2]->SetPoint1(this->GetPoint3());
	this->BoundarySource[2]->SetPoint2(this->GetPoint2());
	this->BoundarySource[3]->SetPoint1(this->GetPoint2());
	this->BoundarySource[3]->SetPoint2(this->GetOrigin());
}