// Copyright 2017 ESRI
//
// All rights reserved under the copyright laws of the United States
// and applicable international laws, treaties, and conventions.
//
// You may freely redistribute and use this sample code, with or
// without modification, provided you include the original copyright
// notice and use restrictions.
//
// See the Sample code usage restrictions document for further information.
//

#include "AlertSource.h"
#include "WithinAreaAlertConditionData.h"

#include "GeoElement.h"
#include "Point.h"

using namespace Esri::ArcGISRuntime;

WithinAreaAlertConditionData::WithinAreaAlertConditionData(AlertCondition* condition,
                                                           AlertSource* source,
                                                           AlertTarget* target):
  AlertConditionData(condition, source, target)
{

}

WithinAreaAlertConditionData::~WithinAreaAlertConditionData()
{

}