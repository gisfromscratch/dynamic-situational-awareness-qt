// Copyright 2016 ESRI
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

#ifndef DSACONTROLLER_H
#define DSACONTROLLER_H

#include <QObject>

#include <QAbstractListModel>
#include <QUrl>

namespace Esri
{
namespace ArcGISRuntime
{
  class Error;
  class Scene;
  class GraphicsOverlay;
}
}

class DsaController : public QObject
{
  Q_OBJECT

public:
  DsaController(QObject* parent = nullptr);
  ~DsaController();

  Esri::ArcGISRuntime::Scene* scene() const;

  QList<Esri::ArcGISRuntime::GraphicsOverlay*> overlays() const;

  void init();

public slots:
  void onError(const Esri::ArcGISRuntime::Error& error);

private:
  Esri::ArcGISRuntime::Scene* m_scene = nullptr;
  QList<Esri::ArcGISRuntime::GraphicsOverlay*> m_overlays;

  QString                     m_dataPath;
};

#endif // DSACONTROLLER_H
