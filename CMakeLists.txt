cmake_minimum_required(VERSION 2.8.9)

#-----------------------------------------------------------------------------
if(NOT Slicer_SOURCE_DIR)
  set(EXTENSION_NAME SmartModelClip)
  set(EXTENSION_HOMEPAGE "http://www.slicer.org/slicerWiki/index.php/Documentation/Nightly/Extensions/SmartModelClip")
  set(EXTENSION_CATEGORY "Surface Models")
  set(EXTENSION_CONTRIBUTORS "QiQin ZHAN, Xiaojun CHEN, Ph.D (SJTU)")
  set(EXTENSION_DESCRIPTION "This is an extension module to set the osteotomy trajectory with multiple planes and clip with just one click.")
  set(EXTENSION_ICONURL "http://www.slicer.org/slicerWiki/index.php/File:ModelClipIcon.png")
  set(EXTENSION_SCREENSHOTURLS "http://www.slicer.org/slicerWiki/index.php/File:ModelClipScreenShot.png")
endif()

#-----------------------------------------------------------------------------
set(CMAKE_BUILD_TYPE "Release")
set(MODULE_NAME SmartModelClip)
set(MODULE_TITLE ${MODULE_NAME})

string(TOUPPER ${MODULE_NAME} MODULE_NAME_UPPER)

#-----------------------------------------------------------------------------
if(NOT Slicer_SOURCE_DIR)
  find_package(Slicer REQUIRED)
  include(${Slicer_USE_FILE})
endif()

#-----------------------------------------------------------------------------

add_subdirectory(Logic)
add_subdirectory(Widgets)

#-----------------------------------------------------------------------------
set(MODULE_EXPORT_DIRECTIVE "Q_SLICER_QTMODULES_${MODULE_NAME_UPPER}_EXPORT")

# Current_{source,binary} and Slicer_{Libs,Base} already included
set(MODULE_INCLUDE_DIRECTORIES
  ${CMAKE_CURRENT_SOURCE_DIR}/Logic
  ${CMAKE_CURRENT_BINARY_DIR}/Logic
  ${CMAKE_CURRENT_SOURCE_DIR}/Widgets
  ${CMAKE_CURRENT_BINARY_DIR}/Widgets
  )

set(MODULE_SRCS
  qSlicer${MODULE_NAME}Module.cxx
  qSlicer${MODULE_NAME}Module.h
  qSlicer${MODULE_NAME}ModuleWidget.cxx
  qSlicer${MODULE_NAME}ModuleWidget.h
  vtkQuadPlaneSource.cxx
  vtkQuadPlaneSource.h
  vtkQuadPlaneWidget.cxx
  vtkQuadPlaneWidget.h
  vtkQuadPlaneWidgetPlus.cpp
  vtkQuadPlaneWidgetPlus.h
  vtkSpinningPlaneWidget.cxx
  vtkSpinningPlaneWidget.h
  )

set(MODULE_MOC_SRCS
  qSlicer${MODULE_NAME}Module.h
  qSlicer${MODULE_NAME}ModuleWidget.h
  )

set(MODULE_UI_SRCS
  Resources/UI/qSlicer${MODULE_NAME}ModuleWidget.ui
  )

set(MODULE_TARGET_LIBRARIES
  vtkSlicer${MODULE_NAME}ModuleLogic
  qSlicer${MODULE_NAME}ModuleWidgets
  )

set(MODULE_RESOURCES
  Resources/qSlicer${MODULE_NAME}Module.qrc
  )

#-----------------------------------------------------------------------------
slicerMacroBuildQtModule(
  NAME ${MODULE_NAME}
  TITLE ${MODULE_TITLE}
  EXPORT_DIRECTIVE ${MODULE_EXPORT_DIRECTIVE}
  INCLUDE_DIRECTORIES ${MODULE_INCLUDE_DIRECTORIES}
  SRCS ${MODULE_SRCS}
  MOC_SRCS ${MODULE_MOC_SRCS}
  UI_SRCS ${MODULE_UI_SRCS}
  TARGET_LIBRARIES ${MODULE_TARGET_LIBRARIES}
  RESOURCES ${MODULE_RESOURCES}
  WITH_GENERIC_TESTS
  )

#-----------------------------------------------------------------------------
if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()

#-----------------------------------------------------------------------------
if(NOT Slicer_SOURCE_DIR)
  include(${Slicer_EXTENSION_CPACK})
endif()

#ADD_LIBRARY(qSlicerAnnotationsModule STATIC IMPORTED)
#add_dependencies(qSlicer${MODULE_NAME}ModuleWidget vtkSlicerAnnotationsModuleMRML)

#LINK_DIRECTORIES(D:/Slicer/Slicer_build/Slicer-build/lib/Slicer-4.2/qt-loadable-modules/Release)
#TARGET_LINK_LIBRARIES(qSlicer${MODULE_NAME}ModuleWidgets vtkSlicerAnnotationsModuleMRML.lib)
#add_dependencies(qSlicer${MODULE_NAME}ModuleWidgets vtkSlicerAnnotationsModuleMRML.lib)