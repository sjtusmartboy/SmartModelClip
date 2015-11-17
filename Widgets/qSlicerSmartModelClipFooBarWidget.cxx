/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// FooBar Widgets includes
#include "qSlicerSmartModelClipFooBarWidget.h"
#include "ui_qSlicerSmartModelClipFooBarWidget.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_SmartModelClip
class qSlicerSmartModelClipFooBarWidgetPrivate
  : public Ui_qSlicerSmartModelClipFooBarWidget
{
  Q_DECLARE_PUBLIC(qSlicerSmartModelClipFooBarWidget);
protected:
  qSlicerSmartModelClipFooBarWidget* const q_ptr;

public:
  qSlicerSmartModelClipFooBarWidgetPrivate(
    qSlicerSmartModelClipFooBarWidget& object);
  virtual void setupUi(qSlicerSmartModelClipFooBarWidget*);
};

// --------------------------------------------------------------------------
qSlicerSmartModelClipFooBarWidgetPrivate
::qSlicerSmartModelClipFooBarWidgetPrivate(
  qSlicerSmartModelClipFooBarWidget& object)
  : q_ptr(&object)
{
}

// --------------------------------------------------------------------------
void qSlicerSmartModelClipFooBarWidgetPrivate
::setupUi(qSlicerSmartModelClipFooBarWidget* widget)
{
  this->Ui_qSlicerSmartModelClipFooBarWidget::setupUi(widget);
}

//-----------------------------------------------------------------------------
// qSlicerSmartModelClipFooBarWidget methods

//-----------------------------------------------------------------------------
qSlicerSmartModelClipFooBarWidget
::qSlicerSmartModelClipFooBarWidget(QWidget* parentWidget)
  : Superclass( parentWidget )
  , d_ptr( new qSlicerSmartModelClipFooBarWidgetPrivate(*this) )
{
  Q_D(qSlicerSmartModelClipFooBarWidget);
  d->setupUi(this);
}

//-----------------------------------------------------------------------------
qSlicerSmartModelClipFooBarWidget
::~qSlicerSmartModelClipFooBarWidget()
{
}
