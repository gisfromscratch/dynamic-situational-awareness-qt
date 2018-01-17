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

#include "IdentifyFeaturesController.h"

#include "ToolManager.h"
#include "ToolResourceProvider.h"

#include "GeoElement.h"
#include "GeoView.h"
#include "Popup.h"
#include "PopupManager.h"

using namespace Esri::ArcGISRuntime;

IdentifyFeaturesController::IdentifyFeaturesController(QObject* parent /* = nullptr */):
  Toolkit::AbstractTool(parent)
{
  Toolkit::ToolManager::instance().addTool(this);

  connect(Toolkit::ToolResourceProvider::instance(), &Toolkit::ToolResourceProvider::mouseClicked,
          this, &IdentifyFeaturesController::onMouseClicked);

  connect(Toolkit::ToolResourceProvider::instance(), &Toolkit::ToolResourceProvider::identifyLayersCompleted,
          this, &IdentifyFeaturesController::onIdentifyLayersCompleted);
}

IdentifyFeaturesController::~IdentifyFeaturesController()
{
}

QString IdentifyFeaturesController::toolName() const
{
  return QStringLiteral("identify features");
}

void IdentifyFeaturesController::setActive(bool active)
{
  if (active == m_active)
    return;

  if (busy())
    m_taskWatcher.cancel();

  m_active = active;
  emit activeChanged();
}

bool IdentifyFeaturesController::busy() const
{
  return m_taskWatcher.isValid() && !m_taskWatcher.isDone() && !m_taskWatcher.isCanceled();
}

QVariantList IdentifyFeaturesController::popupManagers() const
{
  QVariantList res;

  for (PopupManager* mgr : m_popupManagers)
  {
    QVariant v = QVariant::fromValue(mgr);
    res.push_back(v);
  }

  return res;
}

void IdentifyFeaturesController::onMouseClicked(QMouseEvent& event)
{
  if (!isActive())
    return;

  if (m_taskWatcher.isValid() && !m_taskWatcher.isDone())
    return;

  GeoView* geoView = Toolkit::ToolResourceProvider::instance()->geoView();
  if (!geoView)
    return;

  m_taskWatcher = geoView->identifyLayers(event.pos().x(), event.pos().y(), m_tolerance, false);
  emit busyChanged();

  m_popupManagers.clear();
  emit popupManagersChanged();

  event.accept();
}

void IdentifyFeaturesController::onIdentifyLayersCompleted(const QUuid&, const QList<IdentifyLayerResult*>& identifyResults)
{
  emit busyChanged();
  m_taskWatcher = TaskWatcher();

  auto it = identifyResults.begin();
  auto itEnd = identifyResults.end();
  for (; it != itEnd; ++it)
  {
    IdentifyLayerResult* res = *it;
    if (!res)
      continue;

    const QString resTitle = res->layerContent()->name();

    const QList<GeoElement*> geoElements = res->geoElements();
    auto geoElemIt = geoElements.begin();
    auto geoElemEnd = geoElements.end();
    for (; geoElemIt != geoElemEnd; ++geoElemIt)
    {
      GeoElement* geoElement = *geoElemIt;
      if (!geoElement)
        continue;

      Popup* newPopup = new Popup(geoElement, this);
      newPopup->popupDefinition()->setTitle(resTitle);
      PopupManager* newManager = new PopupManager(newPopup, this);

      m_popupManagers.push_back(newManager);
    }
  }

  if (!m_popupManagers.empty())
    emit popupManagersChanged();
}
