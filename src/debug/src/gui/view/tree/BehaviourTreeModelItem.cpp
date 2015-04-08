#include "BehaviourTreeModelItem.h"
#include "TreeViewCommon.h"
#include "AINodeStaticResolver.h"
#include <QFile>

namespace ai {
namespace debug {

BehaviourTreeModelItem::BehaviourTreeModelItem(AIStateNode* node, AINodeStaticResolver& resolver, BehaviourTreeModelItem* parent) :
		_node(node), _staticNodeData(resolver.get(node->getNodeId())), _populated(false), _rowCount(0), _parent(parent) {
	for (const AIStateNode& node : _node->getChildren()) {
		_rows.push_back(new BehaviourTreeModelItem(const_cast<AIStateNode*>(&node), resolver, this));
	}
	const QString type = QString::fromStdString(_staticNodeData.getType()).toLower();
	const QString path = ":/images/" + type + ".png";
	if (QFile::exists(path))
		_icon = QIcon(path);
	else if (type.contains("selector"))
		_icon = QIcon(":/images/selector.png");
	else
		_icon = QIcon(":/images/node.png");
}

BehaviourTreeModelItem::~BehaviourTreeModelItem() {
	qDeleteAll(_rows);
}

BehaviourTreeModelItem* BehaviourTreeModelItem::child(int row) {
	return _rows.value(row);
}

QVariant BehaviourTreeModelItem::color() const {
	const TreeNodeStatus status = _node->getStatus();
	switch (status) {
	case UNKNOWN:
	case CANNOTEXECUTE:
		return QColor(Qt::gray);
	case RUNNING:
	case FINISHED:
		return QColor(Qt::darkGreen);
	case FAILED:
	case EXCEPTION:
		return QColor(Qt::red);
	case MAX_TREENODESTATUS:
		break;
	}
	return QVariant();
}

int BehaviourTreeModelItem::row() {
	if (_parent != nullptr)
		return _parent->_rows.indexOf(this);
	return 0;
}

QIcon BehaviourTreeModelItem::icon() const {
	return _icon;
}

QString BehaviourTreeModelItem::tooltip(int column) const {
	if (column == COL_NAME)
		return QString::fromStdString(_staticNodeData.getType());
	return QString();
}

void BehaviourTreeModelItem::setData(int column, const QVariant& data) {
	switch (column) {
	case COL_NAME:
		_editedName = data.toString();
		break;
	case COL_TYPE:
		_editedType = data.toString();
		break;
	case COL_CONDITION:
		_editedCondition = data.toString();
		break;
	}
}

QVariant BehaviourTreeModelItem::headerData(int column) const {
	switch (column) {
	case COL_NAME:
		return QObject::tr("Name");
	case COL_TYPE:
		return QObject::tr("Type");
	case COL_CONDITION:
		return QObject::tr("Condition");
	case COL_STATE:
		return QObject::tr("State");
	case COL_LASTRUN:
		return QObject::tr("Last run");
	}
	return QVariant();
}

QVariant BehaviourTreeModelItem::data(int column) const {
	switch (column) {
	case COL_NAME:
		if (!_editedName.isEmpty())
			return _editedName;
		return QString::fromStdString(_staticNodeData.getName());
	case COL_TYPE:
		if (!_editedType.isEmpty())
			return _editedType;
		return QString::fromStdString(_staticNodeData.getType());
	case COL_CONDITION:
		if (!_editedCondition.isEmpty())
			return _editedCondition;
		return QString::fromStdString(_node->getCondition());
	case COL_STATE:
		return stateNames[_node->getStatus()];
	case COL_LASTRUN:
		return QString::number(_node->getLastRun() / 1000);
	}
	return QVariant();
}

}
}
