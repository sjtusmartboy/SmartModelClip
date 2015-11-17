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

// Qt includes
#include <QtPlugin>

// SmartModelClip Logic includes
#include <vtkSlicerSmartModelClipLogic.h>

// SmartModelClip includes
#include "qSlicerSmartModelClipModule.h"
#include "qSlicerSmartModelClipModuleWidget.h"

//-----------------------------------------------------------------------------
Q_EXPORT_PLUGIN2(qSlicerSmartModelClipModule, qSlicerSmartModelClipModule);

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qSlicerSmartModelClipModulePrivate
{
public:
  qSlicerSmartModelClipModulePrivate();
};

//-----------------------------------------------------------------------------
// qSlicerSmartModelClipModulePrivate methods

//-----------------------------------------------------------------------------
qSlicerSmartModelClipModulePrivate
::qSlicerSmartModelClipModulePrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerSmartModelClipModule methods

//-----------------------------------------------------------------------------
qSlicerSmartModelClipModule
::qSlicerSmartModelClipModule(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerSmartModelClipModulePrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerSmartModelClipModule::~qSlicerSmartModelClipModule()
{
}

//-----------------------------------------------------------------------------
QString qSlicerSmartModelClipModule::helpText()const
{
  return "This is a loadable module bundled in an extension";
}

//-----------------------------------------------------------------------------
QString qSlicerSmartModelClipModule::acknowledgementText()const
{
  return "This work was was partially funded by NIH grant 3P41RR013218-12S1";
}

//-----------------------------------------------------------------------------
QStringList qSlicerSmartModelClipModule::contributors()const
{
  QStringList moduleContributors;
  moduleContributors << QString("Jean-Christophe Fillion-Robin (Kitware)");
  return moduleContributors;
}

//-----------------------------------------------------------------------------
QIcon qSlicerSmartModelClipModule::icon()const
{
  return QIcon(":/Icons/SmartModelClip.png");
}

//-----------------------------------------------------------------------------
QStringList qSlicerSmartModelClipModule::categories() const
{
  return QStringList() << "Examples";
}

//-----------------------------------------------------------------------------
QStringList qSlicerSmartModelClipModule::dependencies() const
{
  return QStringList();
}

//-----------------------------------------------------------------------------
void qSlicerSmartModelClipModule::setup()
{
  this->Superclass::setup();
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation * qSlicerSmartModelClipModule
::createWidgetRepresentation()
{
  return new qSlicerSmartModelClipModuleWidget;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerSmartModelClipModule::createLogic()
{
  return vtkSlicerSmartModelClipLogic::New();
}
