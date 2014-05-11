#pragma once

#include <QGraphicsView>
#include <AI.h>
#include <server/AIStubTypes.h>
#include <QGraphicsScene>

namespace ai {
namespace debug {

class AIDebugger;
class MapItem;

/**
 * @brief The view that renders your map with all the ai controlled entities.
 *
 * @note If you want to render additional details to an entity, extend this class to override
 * MapView::createMapItem and provide your own @c MapItem there.
 */
class MapView: public QGraphicsView {
	Q_OBJECT
protected:
	QGraphicsScene _scene;
	AIDebugger& _debugger;
public:
	MapView(AIDebugger& debugger);
	virtual ~MapView();

	void updateMapView();

	/**
	 * @brief Creates a @c MapItem and allows you to create your own instances to render extra details
	 */
	virtual MapItem* createMapItem(const AIStateWorld& state);
};

}
}
