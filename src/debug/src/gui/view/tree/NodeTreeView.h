#pragma once

#include "IGraphicsView.h"
#include <QGraphicsScene>

#include "NodeTreeItem.h"

namespace ai {
namespace debug {

class AINodeStaticResolver;
class AIDebugger;

/**
 * @brief Shows the behaviour tree for the current selected entity
 */
class NodeTreeView: public IGraphicsView {
Q_OBJECT
private:
	AIDebugger& _debugger;
	QGraphicsScene _scene;
	AINodeStaticResolver& _resolver;

	NodeTreeItem* buildTreeItems(const AIStateNode& node, NodeTreeItem* parent);
public:
	NodeTreeView(AIDebugger& debugger, AINodeStaticResolver& resolver, QWidget* parent = nullptr);
	virtual ~NodeTreeView();

	void updateTreeWidget();
};

}
}
