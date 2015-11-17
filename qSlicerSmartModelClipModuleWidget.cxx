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
//	MRML includes
#include <vtkMRMLModelDisplayNode.h>
#include <vtkMRMLModelNode.h>
#include <vtkMRMLModelStorageNode.h>
#include <vtkMRMLNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLViewNode.h>

#include<windows.h>
// Qt includes
#include <QDebug>
#include <Qt/qlist.h>
#include <QString>
#include <QMessageBox>

// SlicerQt includes
#include <qMRMLThreeDView.h>
#include <qMRMLThreeDWidget.h>
#include <qMRMLThreeDViewControllerWidget.h>
#include <qSlicerApplication.h>
#include <qSlicerIOManager.h>
#include <qSlicerLayoutManager.h>
#include "qSlicerSmartModelClipModuleWidget.h"
#include "ui_qSlicerSmartModelClipModuleWidget.h"


//VTK includes
#include<vtkRenderWindowInteractor.h>
#include<vtkSmartPointer.h>
#include<vtkDataSetMapper.h>
#include<vtkLineSource.h>
//#include<vtkInteractorStyleTrackballCamera.h>
#include<vtkOrientationMarkerWidget.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkPolyData.h>
#include <vtkAppendPolyData.h>
#include <vtkSmartPointer.h>
#include <vtkSphereSource.h>
#include <vtkCommand.h>
#include <vtkMath.h>
#include <vtkClipPolyData.h>
#include <vtkImplicitBoolean.h>
#include <vtkPlane.h>
#include <vtkProperty.h>
//#include <vtkPlaneWidget.h>
#include <vtkRendererCollection.h>
#include <vtkImplicitWindowFunction.h>


#include <vtkMRMLFiducial.h>
#include <vtkMRMLTransformableNode.h>
#include <vtkMRMLStorableNode.h>
#include <vtkMRMLDisplayableNode.h>
#include <vtkMRMLAnnotationNode.h>
#include <vtkMRMLAnnotationControlPointsNode.h>
#include <vtkMRMLAnnotationFiducialNode.h>
#include <vtkMRMLAnnotationHierarchyNode.h>

#include <vtkPolyData.h>
#include <vtkPolyLine.h>
#include "vtkQuadPlaneWidget.h"
#include "vtkQuadPlaneWidgetPlus.h"
#include "vtkSpinningPlaneWidget.h"

#include <cstring>
#include <sstream>
#include <iostream>
#include <TCHAR.h>
//#include "time.h"  
#include <windows.h>
#include <fstream>

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qSlicerSmartModelClipModuleWidgetPrivate: public Ui_qSlicerSmartModelClipModuleWidget
{
public:
  qSlicerSmartModelClipModuleWidgetPrivate();
};

//-----------------------------------------------------------------------------
// qSlicerSmartModelClipModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerSmartModelClipModuleWidgetPrivate::qSlicerSmartModelClipModuleWidgetPrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerSmartModelClipModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerSmartModelClipModuleWidget::qSlicerSmartModelClipModuleWidget(QWidget* _parent)
	: Superclass( _parent )
	, d_ptr( new qSlicerSmartModelClipModuleWidgetPrivate )
{
	qSlicerApplication *app = qSlicerApplication::application();
	qSlicerLayoutManager *layoutManager = app->layoutManager();
	qMRMLThreeDWidget *threeDWidget = layoutManager->threeDWidget(0);
	qMRMLThreeDView *threeDView = threeDWidget->threeDView();
	renderWindow = threeDView->renderWindow();
	renderer = renderWindow->GetRenderers()->GetFirstRenderer();
	renderWindowInteractor = this->renderWindow->GetInteractor();

	numOfPlanes = 0;
	timesOfClip = 0;
	numOfFiducials=0;
	isReversedClippingPlane=0;
    isReversedDepthPlane=0;
	
}

//-----------------------------------------------------------------------------
qSlicerSmartModelClipModuleWidget::~qSlicerSmartModelClipModuleWidget()
{
	clearPlanes();
}

// ---------------------------------THE SLOTS ---------------------------------------------

void qSlicerSmartModelClipModuleWidget::setup()
{
  Q_D(qSlicerSmartModelClipModuleWidget);
  d->setupUi(this);
  this->Superclass::setup();

  QObject::connect(d->createButton, SIGNAL(clicked()), this, SLOT(createPlane()));
  QObject::connect(d->deleteButton, SIGNAL(clicked()), this, SLOT(deletePlane()));
  QObject::connect(d->clearButton, SIGNAL(clicked()), this, SLOT(clearPlanes()));
  QObject::connect(d->hidePlaneBox, SIGNAL(stateChanged(int)), this, SLOT(SetPlaneVisibility()));
  QObject::connect(d->reverseClippingPlaneButton, SIGNAL(clicked()), this, SLOT(reverseClippingPlane()));
  QObject::connect(d->reverseDepthPlaneButton, SIGNAL(clicked()), this, SLOT(reverseDepthPlane()));
  QObject::connect(d->depthButton, SIGNAL(clicked()), this, SLOT(setDepthPlane()));
  QObject::connect(d->clipButton, SIGNAL(clicked()), this, SLOT(clip()));

}

void qSlicerSmartModelClipModuleWidget::createPlane()
{
	if(numOfPlanes == 0)
	{
		double* newPlaneOrigin;
		double* newPlanePoint1;
		double* newPlanePoint2;
		//specify three points of the first plane
    	newPlaneOrigin=getPositionOfFiducials();
		newPlanePoint1=getPositionOfFiducials();
		newPlanePoint2=getPositionOfFiducials();

	    if(newPlaneOrigin==0||newPlanePoint1==0||newPlanePoint2==0)
	    {
			MessageBox(NULL,"Cannot find enough fiducials to create a plane!\n At least three fiducials should be specified to create the first plane","Error Message",MB_ICONHAND);
			numOfFiducials=0;
			return;
		}

		planeWidget = vtkQuadPlaneWidgetPlus::New();
		newPlanePoint2 = CalculatePoint2CoordinatesOfFirstTwoPlanes(newPlaneOrigin,newPlanePoint1,newPlanePoint2);

		planeWidget->SetOrigin(newPlaneOrigin);
		planeWidget->SetPoint1(newPlanePoint1);
		planeWidget->SetPoint2(newPlanePoint2);
		planeWidget->GeneratePoint3();
		delete []newPlanePoint2;
		planeList.append(planeWidget);
		++numOfPlanes;
	}
    else
	{
		if(numOfPlanes == 1)
		{
			vtkQuadPlaneWidgetPlus* pFirstPlane = dynamic_cast<vtkQuadPlaneWidgetPlus*>(planeList.at(0));
			pFirstPlane->planeFixing(1);
		}
		else if(numOfPlanes == 2)//(if numOfPlanes >= 2)before we create the new plane, we should adjust the last plane's point2 
		{
			double* newPlanePoint2 = CalculatePoint2CoordinatesOfFirstTwoPlanes(
			planeList.at(0)->GetPoint2(),planeList.at(0)->GetPoint3(),planeList.at(1)->GetPoint2());
			planeList.last()->SetPoint2(newPlanePoint2);
		}
		else
		{
			double *lastPlanePoint2=CalIntersectionPointOfPlaneAndLine(numOfPlanes-1);
			planeList.last()->SetPoint2(lastPlanePoint2);
			delete []lastPlanePoint2;
		}
		

		//the following is the process of creating new plane
		double* lastPlanePoint1 = planeList.last()->GetPoint1();   
		double* lastPlanePoint2 = planeList.last()->GetPoint2();   
		double* lastPlanePoint3 = planeList.last()->GetPoint3();
		double lastPlaneVetor23[3];
		double newPlanePoint3[3];
		double* coordinateOfNewPlaneFiducial;

		coordinateOfNewPlaneFiducial=getPositionOfFiducials();
	
		if(coordinateOfNewPlaneFiducial==0)
		{
			/*MessageBox(NULL,"Cannot find enough fiducials to create a plane!Please specify one more fiducial",NULL,MB_OK);*/
			MessageBox(NULL,"Cannot find enough fiducials to create a plane! \n Please specify one more fiducial","Error Message",MB_ICONHAND);
			return;
		}

		vtkMath::Subtract(lastPlanePoint3,lastPlanePoint2,lastPlaneVetor23);
		vtkMath::Add(coordinateOfNewPlaneFiducial,lastPlaneVetor23,newPlanePoint3);

		planeList.last()->SetHandlesVisibility(0);
		planeWidget = vtkSpinningPlaneWidget::New();
		++numOfPlanes;
		planeList.append(planeWidget);

		//positioning the newly created plane
		if(numOfPlanes == 2)
		{
			double* newPlanePoint2 = CalculatePoint2CoordinatesOfFirstTwoPlanes(
				lastPlanePoint2,lastPlanePoint3,coordinateOfNewPlaneFiducial);
			planeWidget->SetOrigin(lastPlanePoint2);
			planeWidget->SetPoint1(lastPlanePoint3);
			planeWidget->SetPoint2(newPlanePoint2);
			planeWidget->GeneratePoint3();
			delete []newPlanePoint2;
		}
		else
		{
			planeWidget->SetOrigin(lastPlanePoint2);
			planeWidget->SetPoint1(lastPlanePoint3);
			planeWidget->SetPoint2(coordinateOfNewPlaneFiducial);
			planeWidget->SetPoint3(newPlanePoint3);
	        
			//adjust the boundary of the plane if the number of planes is greater than 2
			double *newPlanePoint2=CalIntersectionPointOfPlaneAndLine(numOfPlanes-1);
			planeList.last()->SetPoint2(newPlanePoint2);
			vtkMath::Add(newPlanePoint2,lastPlaneVetor23,newPlanePoint3);
			planeWidget->SetPoint3(newPlanePoint3);
			delete []newPlanePoint2;
		}
		(numOfPlanes%2 == 0)?planeWidget->SetPlaneColor(0.6,0.3,0.8):planeWidget->SetPlaneColor(1,1,1);
	}
	//flagOfFirstTimeClickingDelete=1;
	
	setButtonState();

	renderWindowInteractor->Initialize();
	planeWidget->SetInteractor(renderWindowInteractor);
	planeWidget->On();
	renderWindow->Render();
	/*renderWindowInteractor->Start();*/ //cause error if uncommeted
}

void qSlicerSmartModelClipModuleWidget::deletePlane()
{
	
	planeList.at(numOfPlanes-1)->SetEnabled(0);
	planeList.at(numOfPlanes-1)->Delete();
	planeList.removeLast();
	numOfPlanes--;
	if(numOfPlanes==0)
		numOfFiducials=numOfFiducials-3;

	if(numOfPlanes!=0)
	{
		planeList.at(numOfPlanes-1)->SetHandlesVisibility(1);
		numOfFiducials--; //decrease the numOfFiducials because we have deleted a plane
	}
	if(numOfPlanes==1)
	{
		vtkQuadPlaneWidgetPlus* pFirstPlane = dynamic_cast<vtkQuadPlaneWidgetPlus*>(planeList.at(0));
		pFirstPlane->planeFixing(0);
	}

	renderWindow->Render();
	setButtonState();
}

void qSlicerSmartModelClipModuleWidget::clearPlanes()
{
	Q_D(qSlicerSmartModelClipModuleWidget);

	for(int i=numOfPlanes-1;i>=0;i--)
	{
	    planeList.at(i)->SetEnabled(0);
		planeList.at(i)->Delete();
	}
	planeList.clear();
	numOfPlanes=0;
	numOfFiducials=0;

	//delete the depth plane
	if(d->depthButton->text() == tr("Remove Depth Plane"))
	{
		DepthPlaneWidget->SetEnabled(0);
		DepthPlaneWidget->Delete();
	}

	setButtonState();

	//another way to clear planes,but it is slower.
	//for (int i = 0; i < numOfPlanes; i++) //this is wrong because numOfPlanes is not a constant
	/*for (int i = numOfPlanes; i > 0; i--) 
	{
	deletePlane();
	}*/
}

void qSlicerSmartModelClipModuleWidget::SetPlaneVisibility()
{
	Q_D(qSlicerSmartModelClipModuleWidget);

	int i;
	if((d->hidePlaneBox->isChecked()) == 1)
	{
		for(i=0;i<numOfPlanes;i++)
			planeList.at(i)->SetPlaneVisibility(0);
	    if(d->depthButton->text() == tr("Remove Depth Plane"))  //it means that the depth plane has been created
			DepthPlaneWidget->SetPlaneVisibility(0);             //set the depth plane invisible
	}
	else
	{
		for(i=0;i<numOfPlanes-1;i++)
		{
			planeList.at(i)->SetPlaneVisibility(1);
			planeList.at(i)->SetHandlesVisibility(0);
		}
		planeList.at(numOfPlanes-1)->SetPlaneVisibility(1);
		if(d->depthButton->text()==tr("Remove Depth Plane"))  //it means that the depth plane has been created
			DepthPlaneWidget->SetPlaneVisibility(1);           //set the depth plane visible
	}

	renderWindow->Render();
	setButtonState();
}

void qSlicerSmartModelClipModuleWidget::setDepthPlane()
{
	Q_D(qSlicerSmartModelClipModuleWidget);

	if (d->depthButton->text() == tr("Create Depth Plane"))  //create the depth plane
	{
		DepthPlaneWidget = vtkQuadPlaneWidgetPlus::New();
		DepthPlaneWidget->SetPlaneColor(0.8,0.4,0.2);
		double Dpt2[3];//DepthPlane's point2
		double Dpt3[3];//DepthPlane's point3
		double Do2[3]; //DepthPlane's vector o2
		double Do3[3];//DepthPlane's vector o3
		double vo1[3];//First Plane's vector o1
		double vo2[3];//First Plane's vector o2

		vtkMath::Subtract(planeList.at(0)->GetPoint1(),planeList.at(0)->GetOrigin(),vo1);
		vtkMath::Subtract(planeList.at(0)->GetPoint2(),planeList.at(0)->GetOrigin(),vo2);
		vtkMath::Cross(vo2,vo1,Do2);
		double lo1 = vtkMath::Norm(vo1); //length of lo1;
		vtkMath::Normalize(Do2);
		vtkMath::MultiplyScalar(Do2,lo1);
		vtkMath::Add(Do2,planeList.at(0)->GetOrigin(),Dpt2);
		vtkMath::Add(Dpt2,vo2,Dpt3);

		DepthPlaneWidget->SetOrigin(planeList.at(0)->GetOrigin());
		DepthPlaneWidget->SetPoint1(planeList.at(0)->GetPoint2());
		DepthPlaneWidget->SetPoint2(Dpt2);
		DepthPlaneWidget->SetPoint3(Dpt3);
		//this->depthPlane->SetPlaceFactor(1.0f);
		DepthPlaneWidget->SetRepresentationToSurface();

		d->depthButton->setText(tr("Remove Depth Plane"));
		renderWindowInteractor->Initialize();
		DepthPlaneWidget->SetInteractor(renderWindowInteractor);
		DepthPlaneWidget->On();
		renderWindow->Render();
	} 
	else //delete the depth plane
	{
		DepthPlaneWidget->SetEnabled(0);
		DepthPlaneWidget->Delete();
		d->depthButton->setText(tr("Create Depth Plane"));
		renderWindow->Render();
	}
}

void qSlicerSmartModelClipModuleWidget::setButtonState()
{
	Q_D(qSlicerSmartModelClipModuleWidget);

	if(d->hidePlaneBox->isChecked()== 1)
	{//hide all planes and all the buttons should be hided
		d->createButton->setEnabled(0);
		d->deleteButton->setEnabled(0);
		d->clearButton->setEnabled(0);
		d->depthButton->setEnabled(0);
		d->clipButton->setEnabled(0);
		d->reverseClippingPlaneButton->setEnabled(0);
        d->reverseDepthPlaneButton->setEnabled(0);
	}
	else//show all planes
	{
		if(numOfPlanes==0)
		{//no planes on the screen
			d->createButton->setEnabled(1);
			d->deleteButton->setEnabled(0);
			d->clearButton->setEnabled(0);
			d->hidePlaneBox->setEnabled(0);
			d->depthButton->setEnabled(0);
			d->clipButton->setEnabled(0);
			d->reverseClippingPlaneButton->setEnabled(0);
		    d->reverseDepthPlaneButton->setEnabled(0);
		}
		else
		{
			d->createButton->setEnabled(1);
			d->deleteButton->setEnabled(1);
			d->clearButton->setEnabled(1);
			d->hidePlaneBox->setEnabled(1);
			d->depthButton->setEnabled(1);
			d->clipButton->setEnabled(1);
			d->reverseClippingPlaneButton->setEnabled(1);
		    d->reverseDepthPlaneButton->setEnabled(1);
		}
	}
}

void qSlicerSmartModelClipModuleWidget::reverseClippingPlane()
{
	isReversedClippingPlane = !isReversedClippingPlane;
    MessageBox(NULL,"The direction of the clipping plane has been successfully reversed！\n Press the \"Clip the Model\" button to clip the model.","Message", MB_OKCANCEL );
}

void qSlicerSmartModelClipModuleWidget::reverseDepthPlane()
{
	isReversedDepthPlane = !isReversedDepthPlane;

	try{
	double* point1=DepthPlaneWidget->GetPoint1();
	double* point2=DepthPlaneWidget->GetPoint2();
	double savePoint1[3];
	savePoint1[0]=point1[0];
	savePoint1[1]=point1[1];
	savePoint1[2]=point1[2];

	DepthPlaneWidget->SetPoint1(point2);
	DepthPlaneWidget->SetPoint2(savePoint1);
	MessageBox(NULL,"The direction of the Depth plane has been successfully reversed！\n Press the \"Clip the Model\" button to clip the model.","Message", MB_OKCANCEL );
	}
	catch(...)
	{
		MessageBox(NULL,"No depth plane found！\n Press \"Create Depth plane\" button to create a depth plane.","Error Message", MB_ICONHAND );
		return;
	}
}

void qSlicerSmartModelClipModuleWidget::clip()
{
	Q_D(qSlicerSmartModelClipModuleWidget);

	vtkSmartPointer<vtkClipPolyData> clipper=vtkSmartPointer<vtkClipPolyData>::New();
	clipper->GenerateClippedOutputOn();
	this->timesOfClip++;
	
	if(numOfPlanes>2)
	{
		double *newPoint=new double[3];
		newPoint=CalIntersectionPointOfPlaneAndLine(numOfPlanes-1);
		planeList.last()->SetPoint2(newPoint);
		delete []newPoint;
	}
	//obtain the source model for clipping
	qSlicerApplication *app = qSlicerApplication::application();
	vtkMRMLScene *mrmlScene = app->mrmlScene();
	QString nodeID;
	char *charNodeID;
	nodeID = d->clipNodeComboBox->currentNodeId();
	charNodeID = nodeID.toLatin1().data();
	vtkMRMLNode *sourceNode = mrmlScene->GetNodeByID(charNodeID);
	vtkSmartPointer<vtkMRMLModelNode> sourceModel = vtkSmartPointer<vtkMRMLModelNode>::New();
	sourceModel->Copy(sourceNode);    //Copy the node's attributes to this object.
	vtkSmartPointer<vtkPolyData> sourcePolyData = vtkSmartPointer<vtkPolyData>::New();
	sourcePolyData->DeepCopy(sourceModel->GetPolyData());   //Shallow and Deep copy.
	clipper->SetInput(sourcePolyData);

	long start = 0;  
    long end = 0;  

	//start = clock();  
  LARGE_INTEGER m_liPerfFreq = {0};  
  QueryPerformanceFrequency( &m_liPerfFreq );  
  
  LARGE_INTEGER m_liPerfStart = {0};  
  QueryPerformanceCounter( &m_liPerfStart );    

	if(!isReversedClippingPlane && !isReversedDepthPlane)
	{
		clipper->SetClipFunction(SetClipDepth(makeBody(determineFirstPlaneOfClipping(),numOfPlanes-1))); 
	}
	else if(isReversedClippingPlane && !isReversedDepthPlane)
	{
		clipper->SetClipFunction(makeBody(determineFirstPlaneOfClipping(),numOfPlanes-1)); 
		clipper->InsideOutOn();
		vtkImplicitFunction* temptImplicitFunction=clipper->GetClipFunction();
		clipper->SetClipFunction(SetClipDepth(temptImplicitFunction));
	}
	else if(!isReversedClippingPlane && isReversedDepthPlane)
	{
		clipper->SetClipFunction(SetClipDepth(makeBody(determineFirstPlaneOfClipping(),numOfPlanes-1))); 
	}
	else
	{
		clipper->SetClipFunction(makeBody(determineFirstPlaneOfClipping(),numOfPlanes-1)); 
		clipper->InsideOutOn();
		vtkImplicitFunction* temptImplicitFunction=clipper->GetClipFunction();
		clipper->SetClipFunction(SetClipDepth(temptImplicitFunction));
	}
	//int time = start - end;  
    //TCHAR   buffer[100];  
    //wsprintf(buffer, L"執行時間   %d   millisecond   ",time);   
    //MessageBox(NULL,buffer, L"計算時間 ",MB_OK);  
	//QueryPerformanceCounter(&EndTime);    
 //   double realTime=(double)( EndTime.QuadPart - BegainTime.QuadPart )/ Frequency.QuadPart;

	//		char str[32];
	//		TCHAR  buffer[200]; 
	//		double realTime=double(end-start)/CLOCKS_PER_SEC;
	//	sprintf( buffer, "%d", realTime );

	//	::MessageBox(NULL,buffer,"Error Message",MB_OK);

	  LARGE_INTEGER liPerfNow = {0};  
  QueryPerformanceCounter( &liPerfNow );  
  
  int time1=( ((liPerfNow.QuadPart - m_liPerfStart.QuadPart) * 1000000)/m_liPerfFreq.QuadPart);   
  /*TCHAR buffer[100];  
  wsprintf(buffer, L"执行时间 %d millisecond   ",time);   
  MessageBox(NULL,buffer, L"计算时间 ",MB_OK);   */
  //		char buffer1[100];
  //		//sprintf( buffer1, "%d", time );
		//wsprintf(buffer1, "执行时间 %d millisecond   ",time);   
		//MessageBox(NULL,buffer1,"us",MB_OK);

	clipper->Update();	

	vtkSmartPointer<vtkPolyData> reservedPolyData = vtkSmartPointer<vtkPolyData>::New();
	vtkSmartPointer<vtkPolyData> clippedPolyData = vtkSmartPointer<vtkPolyData>::New();
	reservedPolyData->DeepCopy(clipper->GetOutput());
	this->reservedList.append(reservedPolyData);
	clippedPolyData->DeepCopy(clipper->GetClippedOutput());
	this->clippedList.append(clippedPolyData);

	QueryPerformanceCounter( &liPerfNow );  
	int time2=( ((liPerfNow.QuadPart - m_liPerfStart.QuadPart) * 1000000)/m_liPerfFreq.QuadPart);   

			//display and store the result model
	vtkSmartPointer<vtkMRMLModelNode> resultModel = vtkSmartPointer<vtkMRMLModelNode>::New();
	QString resultName = tr("Reserved Part_");
	resultName.append(QString::number(this->timesOfClip));
	resultModel->SetName(resultName.toLatin1().data());
	resultModel->SetAndObservePolyData(this->reservedList.at(this->timesOfClip - 1));
	//resultModel->SetAndObservePolyData(reservedPolyData);
	//mrmlScene->SaveStateForUndo();
	resultModel->SetScene(mrmlScene);

	QueryPerformanceCounter( &liPerfNow );  
	int time3=( ((liPerfNow.QuadPart - m_liPerfStart.QuadPart) * 1000000)/m_liPerfFreq.QuadPart);   

	vtkSmartPointer<vtkMRMLModelDisplayNode> resultDisplay = vtkSmartPointer<vtkMRMLModelDisplayNode>::New();
	vtkSmartPointer<vtkMRMLModelStorageNode> resultStorage = vtkSmartPointer<vtkMRMLModelStorageNode>::New();
	resultDisplay->SetScene(mrmlScene);
	resultStorage->SetScene(mrmlScene);
	resultDisplay->SetInputPolyData(resultModel->GetPolyData());
	resultDisplay->SetColor(1.0, 0.0, 0.0);
	resultStorage->SetFileName(resultName.toLatin1().data());
	mrmlScene->AddNode(resultDisplay);
	mrmlScene->AddNode(resultStorage);
	resultModel->SetAndObserveDisplayNodeID(resultDisplay->GetID());
	resultModel->SetAndObserveStorageNodeID(resultStorage->GetID());

	QueryPerformanceCounter( &liPerfNow );  
	int time4=( ((liPerfNow.QuadPart - m_liPerfStart.QuadPart) * 1000000)/m_liPerfFreq.QuadPart);   

	//	display and store the clipped model
	vtkSmartPointer<vtkMRMLModelNode> clippedModel =
		vtkSmartPointer<vtkMRMLModelNode>::New();
	QString clippedName = tr("Clipped Part_");
	clippedName.append(QString::number(this->timesOfClip));
	clippedModel->SetName(clippedName.toLatin1().data());
	clippedModel->SetAndObservePolyData(this->clippedList.at(this->timesOfClip - 1));
	//clippedModel->SetAndObservePolyData(clippedPolyData);
	//mrmlScene->SaveStateForUndo();
	clippedModel->SetScene(mrmlScene);

	QueryPerformanceCounter( &liPerfNow );  
	int time5=( ((liPerfNow.QuadPart - m_liPerfStart.QuadPart) * 1000000)/m_liPerfFreq.QuadPart);   


	vtkSmartPointer<vtkMRMLModelDisplayNode> clippedDisplay =
		vtkSmartPointer<vtkMRMLModelDisplayNode>::New();
	vtkSmartPointer<vtkMRMLModelStorageNode> clippedStorage =
		vtkSmartPointer<vtkMRMLModelStorageNode>::New();
	clippedDisplay->SetScene(mrmlScene);
	clippedStorage->SetScene(mrmlScene);
	clippedDisplay->SetInputPolyData(clippedModel->GetPolyData());
	clippedDisplay->SetColor(0.0, 1.0, 0.0);
	clippedStorage->SetFileName(clippedName.toLatin1().data());
	mrmlScene->AddNode(clippedDisplay);
	mrmlScene->AddNode(clippedStorage);
	clippedModel->SetAndObserveDisplayNodeID(clippedDisplay->GetID());
	clippedModel->SetAndObserveStorageNodeID(clippedStorage->GetID());

	mrmlScene->AddNode(resultModel);
	mrmlScene->AddNode(clippedModel);

    QueryPerformanceCounter( &liPerfNow );  
  
  int time6=( ((liPerfNow.QuadPart - m_liPerfStart.QuadPart) * 1000000)/m_liPerfFreq.QuadPart);   
  //char buffer5[100];
  //sprintf( buffer5, "%d", time );

  //::MessageBox(NULL,buffer5,"Error Message",MB_OK);
   //ofstream outfile("file.txt", ios::app);
   ofstream outfile("file.txt",ios::app);
   //FILE *fp= fopen("file.txt");
  //FILE * outfile;
  //outfile = fopen ( "file.txt" , "w" );

  if(outfile.is_open())
{
   	outfile.seekp(0,ios::end);
	if((size_t)outfile.tellp()==0)
		outfile<<"numOfPlanes"<<"\t"<<"timeOfAlgorithm"<<"\t"<<"\t"<<"timeOfClip"<<endl;
	
  //outfile<<numOfPlanes<<"\t"<<"\t"<<time1<<"\t"<<"\t"<<"\t"<<time2<<endl;    //message是程序中处理的数据
  outfile<<numOfPlanes<<"\t"<<time1<<"\t"<<time2<<"\t"<<time3<<"\t"<<time4<<"\t"<<time5<<"\t"<<time6<<endl;    //message是程序中处理的数据
  outfile.close();
}
else
{
  cout<<"不能打开文件!"<<endl;
}



	
}

// ---------------------------TOOLS USED TO CREATE A PLANE----------------------------------

// Point2 coordinates of first two planes satisfy such requirements that the line segment of Point2 and Point3 is
// perpendicular with the line segment of Origin and Point1.
double* qSlicerSmartModelClipModuleWidget::
	CalculatePoint2CoordinatesOfFirstTwoPlanes(double* newPlaneOrigin,double* newPlanePoint1,double* newPlanePoint2)
{
	double* newPoint2=new double[3];
	double vectorFromPointOriginTOPoint1[3];
	double vectorFromPointOriginTOPoint2[3];
	vtkMath::Subtract(newPlanePoint1,newPlaneOrigin,vectorFromPointOriginTOPoint1);
	vtkMath::Subtract(newPlanePoint2,newPlaneOrigin,vectorFromPointOriginTOPoint2);

	double tempt = vtkMath::Dot(vectorFromPointOriginTOPoint1,vectorFromPointOriginTOPoint2)
		/vtkMath::Norm(vectorFromPointOriginTOPoint1)/vtkMath::Norm(vectorFromPointOriginTOPoint1);
	vtkMath::MultiplyScalar(vectorFromPointOriginTOPoint1,tempt);
	vtkMath::Subtract(vectorFromPointOriginTOPoint2,vectorFromPointOriginTOPoint1,vectorFromPointOriginTOPoint2);
	vtkMath::Add(vectorFromPointOriginTOPoint2,newPlaneOrigin,newPoint2); 
	return newPoint2;
}

// get the position of a fiducial and return its coordinates.The function returns 0 if no fiducial is on the scenery.
double* qSlicerSmartModelClipModuleWidget::getPositionOfFiducials()
{
	Q_D(qSlicerSmartModelClipModuleWidget);
	int tempt=numOfFiducials; //use this variable to prevent the situation that numOfFiducials count but actually there's no more fiducials
	tempt++;

	qSlicerApplication *app = qSlicerApplication::application();
	vtkMRMLScene *mrmlScene = app->mrmlScene();
	QString fiducialsID = tr("vtkMRMLAnnotationFiducialNode");
	fiducialsID.append(QString::number(tempt));

	int loopTimes=0;
	double *data;
	
	while(loopTimes<50)
	{
		try{
			vtkMRMLAnnotationFiducialNode* m_fnode1=vtkMRMLAnnotationFiducialNode::SafeDownCast(mrmlScene->GetNodeByID(fiducialsID.toLatin1().data()));
			data=m_fnode1->GetFiducialCoordinates();
			numOfFiducials=tempt;    //if there's no more fiducials,this block goes to catch(...),so numOfFiducials changes only when there's indeed a fiducial
			return data;
		}
		catch(...)
		{
			tempt++;
			fiducialsID = tr("vtkMRMLAnnotationFiducialNode");
			fiducialsID.append(QString::number(tempt));
		}
		loopTimes++;
	}
	return 0;  //hasn't found any subsequent fiducials,return null
}

// calculate the intersection point of the plane(which pass through Origin of plane m-2,
// Point2 of plane m-2 and Point2 of plane m-1)and the line (which pass through Point3 and Point2 
// of plane m).We define this intersection point as the plane m 's new coordinates of point2's.
// We do this because it's convenient to judge whether the plane m is intersected with the previous plane
double* qSlicerSmartModelClipModuleWidget::CalIntersectionPointOfPlaneAndLine(int m)
{
	//the second last plane previous the plane m
	double* SLPO = planeList.at(m-2)->GetOrigin();
	double* SLP2 = planeList.at(m-2)->GetPoint2();  
	//previous plane of the plane m
	double* PP2 = planeList.at(m-1)->GetPoint2();
	//plane m
	double *P2=planeList.at(m)->GetPoint2();
	double *P3=planeList.at(m)->GetPoint3();

   double *SLPO2 = new double[3];
	vtkMath::Subtract(
	this->planeList.at(m-2)->GetPoint2(),
	this->planeList.at(m-2)->GetOrigin(),SLPO2);
	double *PPO2 = new double[3];
	vtkMath::Subtract(
	this->planeList.at(m-1)->GetPoint2(),
	this->planeList.at(m-1)->GetOrigin(),PPO2);

	double *norm = new double[3];
	vtkMath::Cross(SLPO2,PPO2,norm);

	double *lineVector =new double[3];
	vtkMath::Subtract(P2,P3,lineVector);

	double *temp=new double[3];
	vtkMath::Subtract(SLPO,P3,temp);

	double t =vtkMath::Dot(norm,temp) /vtkMath::Dot(norm,lineVector);

	double *Point = new double[3];
	Point[0]=P3[0]+lineVector[0]*t;
	Point[1]=P3[1]+lineVector[1]*t;
	Point[2]=P3[2]+lineVector[2]*t;

	delete []SLPO2;
	delete []PPO2;
	delete []norm;
	delete []lineVector;
	delete []temp;

	return Point;
}


//---------------------------TOOLS USED TO CLIP THE MODEL-----------------------------------

//The algorithm of model clipping is based on recursion
vtkSmartPointer<vtkImplicitBoolean> qSlicerSmartModelClipModuleWidget:: makeBody(int m,int n)
{
	vtkSmartPointer<vtkImplicitBoolean> temptBody = vtkSmartPointer<vtkImplicitBoolean>::New();

	if(m==n)
	{
		temptBody->AddFunction(planeList.at(m)->GetPlane());
		return temptBody;
	}
	else if((n-m)==1)
	{
		if(isPlaneInside(n))
			temptBody->SetOperationTypeToIntersection();
		else
			temptBody->SetOperationTypeToUnion();
		temptBody->AddFunction(planeList.at(m)->GetPlane());
		temptBody->AddFunction(planeList.at(n)->GetPlane());
		return temptBody;
	}
	else
	{
		int i=n;
		while (isIntersect1(m,i) || isIntersect2(i-1,n))
		{
			i--;
			if((i-m)==1)
				break;
		}
		//char str[32];
		//sprintf( str, "%d", i );

		//::MessageBox(NULL,str,"Error Message",MB_ICONHAND);

		if(isPlaneInside(i))
			temptBody->SetOperationTypeToIntersection();
		else 
			temptBody->SetOperationTypeToUnion();

		temptBody->AddFunction(makeBody(m,i-1));
		temptBody->AddFunction(makeBody(i,n));
		return temptBody;
	}
}

//Specify the depth plane to clip the model 
vtkSmartPointer<vtkImplicitBoolean> qSlicerSmartModelClipModuleWidget::
	SetClipDepth(vtkSmartPointer<vtkImplicitFunction> clipFunction)
{
	Q_D(qSlicerSmartModelClipModuleWidget);

	vtkSmartPointer<vtkImplicitBoolean> bodyWithDepth = 
		vtkSmartPointer<vtkImplicitBoolean>::New();
	if (d->depthButton->text() == tr("Remove Depth Plane")) 
	{
		vtkSmartPointer<vtkPlane> depthFunction = 
			vtkSmartPointer<vtkPlane>::New();
		DepthPlaneWidget->GetPlane(depthFunction);
		bodyWithDepth->SetOperationTypeToIntersection();
		bodyWithDepth->AddFunction(clipFunction);
		bodyWithDepth->AddFunction(depthFunction);
	} 
	else 
	{
		bodyWithDepth->AddFunction(clipFunction);
	}
		return bodyWithDepth;
}

// If the last plane's line segment is intersected with its previous planes' line segments twice,we define the 
// first plane Of clipping model is the larger sequence number.Or we define the first plane Of clipping model 
// is the plane 0
int qSlicerSmartModelClipModuleWidget::determineFirstPlaneOfClipping()
{
	if(numOfPlanes<4)
		return 0;
	else
	{
		int i=numOfPlanes-3;//numOfPlanes>=4;i>=1
		int firstPlaneNum=0;
		int intersectPlaneNum;
		while(i>=0)
		{
			if(isInterSectWithTheSingleLineSegment(i))
			{
			    intersectPlaneNum=i;
				if(firstPlaneNum==0)
					firstPlaneNum=i;
				if(intersectPlaneNum!=firstPlaneNum)
					return firstPlaneNum;
			}
			i--;
		}
        return 0;  //return the  0th plane
	}
}

// judge whether the plane i is inside plane i-1(i>=1)
int qSlicerSmartModelClipModuleWidget::isPlaneInside(int i)
{
	double *oP2 = new double[3];
	vtkMath::Subtract(
		this->planeList.at(i)->GetPoint2(),
		this->planeList.at(i)->GetOrigin(),
		oP2);
	double dot = vtkMath::Dot(
		this->planeList.at(i-1)->GetNormal(),
		oP2);
	delete []oP2;
	return (dot<0)?1:0;
}

// extend a line segment on plane i,the line of which pass through Point2,
// from the Origin on the plane to infinitive
double* qSlicerSmartModelClipModuleWidget::extendLineSegment(int i)
{
	double *pt2 = this->planeList.at(i)->GetPoint2();
	double *o = this->planeList.at(i)->GetOrigin();
	double *output = new double[3];
	double *vector = new double[3];

	vtkMath::Subtract(pt2, o, vector);
	vtkMath::MultiplyScalar(vector, 100);
	vtkMath::Add(vector, o, output);

	delete []vector;
	return output;
}

// extend a line segment on plane i reversely,which pass through Origin,
// from the Point2 on the plane to infinitive
double* qSlicerSmartModelClipModuleWidget::reverseExtendLineSegment(int i)//the first plane is m
{
	double *pt2 = this->planeList.at(i)->GetPoint2();
	double *o = this->planeList.at(i)->GetOrigin();
	double *output = new double[3];
	double *vector = new double[3];
	vtkMath::Subtract(o,pt2,vector);
	vtkMath::MultiplyScalar(vector, 100);
	vtkMath::Add(vector, pt2, output);
	delete []vector;
	return output;
}

// judge whether the plane i will intersect with the previous plane from plane m to plane i-2
// the line segment of plane i is extended infinitely on both of the line segment direction
bool qSlicerSmartModelClipModuleWidget::isIntersect1(int m,int i)
{
	if ((i-m) == 1)
		return false;

	//	create a vtkPoints object and store the points in it
	vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
	int numOfLines = i-m - 1;      //number of line segments
	double* originExtend;

	for (int j = m; j <= i-1; j++) 
	{
		if(j==m)
		{
			originExtend = reverseExtendLineSegment(m);
			points->InsertNextPoint(originExtend);
		    delete []originExtend;
		}
		else
			points->InsertNextPoint(this->planeList.at(j)->GetOrigin());
	}


	//	create a vtkPolyLine object and store the polyline in it
	vtkSmartPointer<vtkPolyLine> polyLine = vtkSmartPointer<vtkPolyLine>::New();
		polyLine->GetPoints()->DeepCopy(points);
		polyLine->GetPointIds()->SetNumberOfIds(numOfLines+1);
		for(int j = 0; j <=i-m-1; j++) 
			polyLine->GetPointIds()->SetId(j, j);

	double tolerance = 0.001;
	double t;
	double x[3];
	double pcoords[3];
	int subId;
	int intersection;
	int intersection2;

	double* output = extendLineSegment(i);//第i号平面延长
	intersection = polyLine->IntersectWithLine(
		this->planeList.at(i)->GetOrigin(),   
		output, tolerance, t, x, pcoords, subId);

	double* output2 = reverseExtendLineSegment(i);//第i号平面反向延长
	intersection2 = polyLine->IntersectWithLine(
		this->planeList.at(i)->GetOrigin(),
		output2, tolerance, t, x, pcoords, subId);

	delete []output;
	delete []output2;

	if ((intersection2 != 0) || (intersection != 0))
		return true;
	else 
		return false;
}

bool qSlicerSmartModelClipModuleWidget::isIntersect2(int a,int n)
{
	if ((n-a) == 1)
		return false;

	//	create a vtkPoints object and store the points in it
	vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
	int numOfLines = n-a - 1;      //number of line segments
	double* lastExtend;

	for (int j = a+2; j <= n; j++) 
	{
		points->InsertNextPoint(this->planeList.at(j)->GetOrigin());
		if(j==n)
		{
			lastExtend = extendLineSegment(n);
			points->InsertNextPoint(lastExtend);
		    delete []lastExtend;
		}	
	}


	//	create a vtkPolyLine object and store the polyline in it
	vtkSmartPointer<vtkPolyLine> polyLine = vtkSmartPointer<vtkPolyLine>::New();
		polyLine->GetPoints()->DeepCopy(points);
		polyLine->GetPointIds()->SetNumberOfIds(numOfLines+1);
		for(int j = 0; j <= numOfLines; j++) 
			polyLine->GetPointIds()->SetId(j, j);

	double tolerance = 0.001;
	double t;
	double x[3];
	double pcoords[3];
	int subId;
	int intersection;
	int intersection2;

	double* output = extendLineSegment(a);//第a号平面延长
	intersection = polyLine->IntersectWithLine(
		this->planeList.at(a)->GetOrigin(),   
		output, tolerance, t, x, pcoords, subId);

	//double* output2 = reverseExtendLineSegment(a);//第a号平面反向延长
	//intersection2 = polyLine->IntersectWithLine(
	//	this->planeList.at(n)->GetOrigin(),
	//	output2, tolerance, t, x, pcoords, subId);

	delete []output;
	//delete []output2;

	
	//if ((intersection2 != 0) || (intersection != 0))
	if (intersection != 0)
	{
		//MessageBox(NULL,"1","Error Message",MB_ICONHAND);
		return true;
	}
	else 
	{
		//MessageBox(NULL,"2","Error Message",MB_ICONHAND);
		return false;
	}
}

// If the last plane's line segment is intersected with its previous planes' line segments twice,we define the 
// first plane Of clipping model is the larger sequence number.This function checks whteher the final plane's extended
// line segment will intersected with the line segment of plane m.
bool qSlicerSmartModelClipModuleWidget::isInterSectWithTheSingleLineSegment(int m)
{
	//	create a vtkPoints object and store the points in it
	vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
	int numOfLines = 1;

	double* originExtend;
	if(m==0)
	{
		originExtend = reverseExtendLineSegment(m);
		points->InsertNextPoint(originExtend);
		points->InsertNextPoint(planeList.at(1)->GetOrigin());
		delete []originExtend;
	}
	else
	{
		points->InsertNextPoint(this->planeList.at(m)->GetOrigin());
		points->InsertNextPoint(this->planeList.at(m)->GetPoint2());
	}


	//	create a vtkPolyLine object and store the polyline in it
	vtkSmartPointer<vtkPolyLine> polyLine = vtkSmartPointer<vtkPolyLine>::New();
	polyLine->GetPoints()->DeepCopy(points);
	polyLine->GetPointIds()->SetNumberOfIds(numOfLines+1);
	polyLine->GetPointIds()->SetId(0, 0);
	polyLine->GetPointIds()->SetId(1, 1);

	double tolerance = 0.001;
	double t;
	double x[3];
	double pcoords[3];
	int subId;
	double* output = extendLineSegment(numOfPlanes-1);
	int intersection = polyLine->IntersectWithLine(
		this->planeList.at(numOfPlanes-1)->GetOrigin(),   
		output, tolerance, t, x, pcoords, subId);

	delete []output;

	if (intersection != 0) 
		return true;
	else
		return false;
}






