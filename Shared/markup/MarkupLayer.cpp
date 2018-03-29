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

// PCH header
#include "pch.hpp"

// example app headers
#include "MarkupLayer.h"
#include "MarkupConstants.h"

// C++ API headers
#include "GraphicsOverlay.h"
#include "SimpleLineSymbol.h"
#include "FeatureCollection.h"
#include "FeatureCollectionLayer.h"
#include "FeatureCollectionTable.h"
#include "Feature.h"
#include "SimpleLineSymbol.h"
#include "SimpleRenderer.h"
#include "Polyline.h"
#include "Field.h"

// Qt headers
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QString>
#include <QHash>

using namespace Esri::ArcGISRuntime;

/*
 \internal
 \brief Constructor that takes a \a featureCollection and an optional \a parent.
 */
MarkupLayer::MarkupLayer(const QString& json, FeatureCollection* featureCollection, QObject* parent) :
  FeatureCollectionLayer(featureCollection, parent),
  m_json(json),
  m_featureCollection(featureCollection)
{
  const QJsonDocument markupDoc = QJsonDocument::fromJson(json.toUtf8());
  const QJsonObject markupJson = markupDoc.object();

  // Clear Hash to keep track of features/symbols added to the table
  m_featureHash.clear();

  // Get the table
  auto table = m_featureCollection->tables()->at(0);

  // Connect to know when addFeature successfully completes
  connect(table, &FeatureCollectionTable::addFeatureCompleted, this, [this, table](QUuid id, bool success)
  {
    if (!success || !m_featureHash.contains(id))
      return;

    auto pair = m_featureHash.value(id);
    table->setSymbolOverride(pair.first, pair.second);
  });

  // Loop through the markup elements and add them as Features to the table
  QJsonArray markupElements = markupJson.value(MarkupConstants::MARKUP).toObject().value(MarkupConstants::ELEMENTS).toArray();
  int markupSize = markupElements.size();
  for (int i = 0; i < markupSize; i++)
  {
    const QJsonObject element = markupElements.at(i).toObject();
    Feature* feature = table->createFeature(table);
    const QString geomString = QString(QJsonDocument(element.value(MarkupConstants::GEOMETRY).toObject()).toJson(QJsonDocument::Compact));
    feature->setGeometry(Geometry::fromJson(geomString));
    QUuid id = table->addFeature(feature).taskId();
    SimpleLineSymbol* sls = new SimpleLineSymbol(SimpleLineSymbolStyle::Solid, QColor(MarkupLayer::colors().at(element.value(MarkupConstants::COLOR).toInt())), 12.0f, parent);
    m_featureHash[id] = QPair<Feature*, SimpleLineSymbol*>(feature, sls);
  }

  setName(markupJson.value(MarkupConstants::MARKUP).toObject().value(MarkupConstants::NAME).toString());
}

/*
 \brief Destructor
 */
MarkupLayer::~MarkupLayer()
{
}

/*
 \brief Sets the layer path to \a path.
*/
void MarkupLayer::setPath(const QString& path)
{
  m_path = path;
}

/*
 \brief Returns the layer path.
*/
QString MarkupLayer::path() const
{
  return m_path;
}

/*
 \brief Returns the JSON string in \c .markup format.
*/
QString MarkupLayer::toJson() const
{
  return m_json;
}

/*
 \brief Returns the FeatureCollection used by this layer.
*/
FeatureCollection* MarkupLayer::featureCollection()
{
  return m_featureCollection;
}

/*
 \brief Converts the input \a graphicsOverlay to \c .markup JSON.
 */
MarkupLayer* MarkupLayer::createFromGraphics(GraphicsOverlay* graphicsOverlay, const QString& authorName, QObject* parent)
{
  // get the sceneview instance
  SceneView* sceneView = dynamic_cast<SceneView*>(Toolkit::ToolResourceProvider::instance()->geoView());

  // create the markup
  QJsonObject markup;
  QJsonArray elements;
  const int graphicCount = graphicsOverlay->graphics()->size();
  for (int i = 0; i < graphicCount; i++)
  {
    Graphic* graphic = graphicsOverlay->graphics()->at(i);
    QJsonObject element;
    QJsonDocument geomDoc = QJsonDocument::fromJson(graphic->geometry().toJson().toUtf8());
    element[MarkupConstants::GEOMETRY] = QJsonValue(geomDoc.object());
    element[MarkupConstants::FILLED] = false;
    element[MarkupConstants::ARROW] = false;
    SimpleLineSymbol* sls = dynamic_cast<SimpleLineSymbol*>(graphic->symbol());
    element[MarkupConstants::COLOR] = sls ? colors().indexOf(sls->color().name()) : 0;
    QJsonValue value(element);
    elements.append(value);
  }
  markup[MarkupConstants::ELEMENTS] = elements;
  markup[MarkupConstants::VERSION] = MarkupConstants::VERSIONNUMBER;
  markup[MarkupConstants::NAME] = graphicsOverlay->overlayId();

  // create the markup item json
  QJsonObject markupJson;

  // set center
  markupJson[MarkupConstants::SCALE] = sceneView ? (int)sceneView->currentViewpointCamera().location().z() : -1;
  markupJson[MarkupConstants::VERSION] = "1.0";
  QJsonDocument centerDoc = sceneView ?
        QJsonDocument::fromJson(sceneView->currentViewpointCamera().location().toJson().toUtf8()) :
        QJsonDocument();
  markupJson[MarkupConstants::CENTER] = sceneView ? QJsonValue(centerDoc.object()) : -1;

  // add the markups to the markup item json
  markupJson[MarkupConstants::MARKUP] = markup;

  // add the name of the sharer
  markupJson[MarkupConstants::SHAREDBY] = authorName;

  return MarkupLayer::createFromJson(QJsonDocument(markupJson).toJson(QJsonDocument::Compact), parent);
}

/*
 \brief Returns a MarkupLayer for the input \a json.
*/
MarkupLayer* MarkupLayer::createFromJson(const QString& json, QObject* parent)
{
  // Create the FeatureCollectionTable
  FeatureCollectionTable* table = new FeatureCollectionTable(QList<Field>{}, GeometryType::Polyline, SpatialReference(4326), true, false, parent);
  SimpleRenderer* defaultRenderer = new SimpleRenderer(parent);
  defaultRenderer->setSymbol(new SimpleLineSymbol(SimpleLineSymbolStyle::Solid, QColor("red"), 12.0f, parent));
  table->setRenderer(defaultRenderer);

  // Add the table to a Collection
  FeatureCollection* featureCollection = new FeatureCollection(QList<FeatureCollectionTable*>{table}, parent);

  // Create a MarkupLayer
  MarkupLayer* markupLayer = new MarkupLayer(json, featureCollection, parent);

  return markupLayer;
}

/*
 \brief Creates a new MarkupLayer from a \a path to a \c .markup JSON file.
*/
MarkupLayer* MarkupLayer::createFromPath(const QString& path)
{

}

/*
 \brief Returns a list of colors to be used in the Markup tool.
*/
QStringList MarkupLayer::colors()
{
  return QStringList{QStringLiteral("#ff0000"), QStringLiteral("#ffd700"),
        QStringLiteral("#32cd32"), QStringLiteral("#00ffff"),
        QStringLiteral("#800080"), QStringLiteral("#ff00ff")};
}
