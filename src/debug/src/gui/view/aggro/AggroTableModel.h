#pragma once

#include <QAbstractTableModel>
#include <QTableView>

#include "AIDebugger.h"

namespace ai {
namespace debug {

class AggroTableModel: public QAbstractTableModel {
Q_OBJECT
private:
	AIDebugger& _debugger;
	QTableView* _parent;
public:
	AggroTableModel(AIDebugger& debugger, QTableView *parent);
	~AggroTableModel();

	inline const AIDebugger::Entities& getEntities() const {
		return _debugger.getEntities();
	}

	inline const AIStateWorld* getEntity(const QModelIndex &index) const {
		const int size = getEntities().size();
		if (size > index.row() && index.row() >= 0)
			return &getEntities().values().at(index.row());
		return nullptr;
	}

	void update();

	int rowCount(const QModelIndex &parent = QModelIndex()) const override;
	int columnCount(const QModelIndex &parent = QModelIndex()) const override;
	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
	QVariant headerData(int section, Qt::Orientation orientation,
			int role) const override;
};

}
}
