#pragma once

#include <QColor>
#include <QPainter>
#include <QGraphicsItem>
#include <QStyleOptionGraphicsItem>
#include <QFontMetrics>

#include <server/AIStubTypes.h>

namespace ai {
namespace debug {

class AIDebugger;

class NodeTreeItem : public QGraphicsItem {
protected:
	const AIStateNode& _node;
	NodeTreeItem* _parent;
	QList<NodeTreeItem*> _children;
	QString _condition;
	QString _name;
	int _width;
	int _height;
	int _lineHeight;
	int _horizontalSpacing;
	int _verticalSpacing;
	QRectF _size;
	QPointF _offset;

	QRectF fullSize();
	inline QPointF getChildPos (NodeTreeItem* child) const;
	void setOffset(const QPointF& offset);
	QRectF boundingRect() const override;
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *item, QWidget *widget) override;

public:
	NodeTreeItem (QGraphicsItem* parentItem, const AIStateNode& node, NodeTreeItem* parent, int height, int horizontalSpacing, int verticalSpacing);
	virtual ~NodeTreeItem ();

	void init();

	inline NodeTreeItem* getParent() { return _parent; }
	inline const AIStateNode& getNode() { return _node; }
	inline int width() const { return _width; }
	inline int height() const { return _height; }
	inline int getSize() const { return _node.getChildren().size(); }
	inline void addChildren(NodeTreeItem* node) { _children.append(node); }
};

}
}
