#include <QMenuBar>
#include <QMessageBox>
#include <QStatusBar>
#include <QSplitter>
#include <QTreeView>
#include <QDesktopServices>
#include <QPushButton>
#include <QUrl>
#include <QHeaderView>

#include "AIDebugger.h"
#include "AIDebuggerWidget.h"
#include "ConnectDialog.h"
#include "StateTable.h"
#include "EntityList.h"
#include "AggroTable.h"
#include "MapView.h"
#include "ZoomFrame.h"
#include "NodeTreeView.h"

namespace ai {
namespace debug {

AIDebuggerWidget::AIDebuggerWidget(AIDebugger& debugger, AINodeStaticResolver& resolver) :
		QWidget(), _resolver(resolver), _model(debugger, resolver), _debugger(debugger), _proxy(this) {
	createView();
	createActions();

	_statusBarLabel = new QLabel(tr("not connected"));
	_selectedLabel = new QLabel(tr("nothing selected"));

	connect(_namesComboBox, SIGNAL(activated(const QString &)), SLOT(change(const QString &)));
	connect(&_debugger, SIGNAL(onPause(bool)), SLOT(setPause(bool)));
	connect(&_debugger, SIGNAL(disconnected()), SLOT(onDisconnect()));

	connect(&_debugger, SIGNAL(onSelected()), &_proxy, SLOT(selected()), Qt::QueuedConnection);
	connect(&_debugger, SIGNAL(onNamesReceived()), &_proxy, SLOT(namesReceived()), Qt::QueuedConnection);
	connect(&_debugger, SIGNAL(onEntitiesUpdated()), &_proxy, SLOT(entitiesUpdated()), Qt::QueuedConnection);

	connect(&_proxy, SIGNAL(onSelected()), this, SLOT(onSelected()));
	connect(&_proxy, SIGNAL(onEntitiesUpdated()), this, SLOT(onEntitiesUpdated()));
	connect(&_proxy, SIGNAL(onNamesReceived()), this, SLOT(onNamesReceived()));
}

AIDebuggerWidget::~AIDebuggerWidget() {
	delete _nodeTree;
	delete _nodeTreeFrame;
	delete _mapFrame;
	delete _entityList;
	delete _statusBarLabel;
	delete _selectedLabel;
	delete _stateTable;
	delete _aggroTable;
	delete _pauseAction;
	delete _resetAction;
	delete _stepAction;
	delete _connectAction;
	delete _aboutAction;
	delete _documentationAction;
	delete _bugAction;
	delete _namesComboBox;
	delete _tree;
}

void AIDebuggerWidget::onEntitiesUpdated() {
	_entityList->updateEntityList();
	_mapWidget->updateMapView();
}

void AIDebuggerWidget::onSelected() {
	const ai::CharacterId& id = _debugger.getSelected();
	if (id == -1) {
		_selectedLabel->setText(tr("nothing selected"));
	} else {
		_selectedLabel->setText(tr("selected %1").arg(id));
	}
	_stateTable->updateStateTable();
	_nodeTree->updateTreeWidget();
	_model.setRootNode(const_cast<AIStateNode*>(&_debugger.getNode()));
	_tree->expandAll();
	_aggroTable->updateAggroTable();
}

void AIDebuggerWidget::onNamesReceived() {
	const QString name = _namesComboBox->currentText();
	_namesComboBox->clear();
	const QStringList& names = _debugger.getNames();
	if (names.empty()) {
		_namesComboBox->insertItem(0, tr("None"));
		_namesComboBox->setEnabled(false);
	} else {
		_namesComboBox->insertItems(0, names);
		_namesComboBox->setEnabled(true);
	}
	const int index = _namesComboBox->findText(name);
	if (index == -1) {
		if (!names.empty())
			_namesComboBox->setCurrentIndex(0);
		return;
	}

	_namesComboBox->setCurrentIndex(index);
}

void AIDebuggerWidget::contributeToStatusBar(QStatusBar* statusBar) {
	statusBar->addWidget(_statusBarLabel);
	statusBar->addWidget(_selectedLabel);
}

void AIDebuggerWidget::contributeToToolBar(QToolBar* toolBar) {
	toolBar->addAction(_connectAction);
	toolBar->addAction(_pauseAction);
	toolBar->addAction(_stepAction);
	toolBar->addAction(_resetAction);
}

void AIDebuggerWidget::contributeToFileMenu(QMenu *fileMenu) {
	fileMenu->addAction(_connectAction);
}

void AIDebuggerWidget::contributeToHelpMenu(QMenu *helpMenu) {
	helpMenu->addAction(_documentationAction);
	helpMenu->addAction(_bugAction);
	helpMenu->addAction(_aboutAction);
}

void AIDebuggerWidget::removeFromStatusBar(QStatusBar* statusBar) {
	statusBar->removeWidget(_statusBarLabel);
	statusBar->removeWidget(_selectedLabel);
}

void AIDebuggerWidget::removeFromToolBar(QToolBar* toolBar) {
	toolBar->removeAction(_connectAction);
	toolBar->removeAction(_pauseAction);
	toolBar->removeAction(_stepAction);
	toolBar->removeAction(_resetAction);
}

void AIDebuggerWidget::removeFromFileMenu(QMenu *fileMenu) {
	fileMenu->removeAction(_connectAction);
}

void AIDebuggerWidget::removeFromHelpMenu(QMenu *helpMenu) {
	helpMenu->removeAction(_bugAction);
	helpMenu->removeAction(_documentationAction);
	helpMenu->removeAction(_aboutAction);
}

void AIDebuggerWidget::createView() {
	QVBoxLayout *layout = new QVBoxLayout;
	QSplitter* splitter = new QSplitter(Qt::Orientation::Vertical);
	splitter->addWidget(createTopWidget());
	splitter->addWidget(createBottomWidget());
	layout->addWidget(splitter);
	setLayout(layout);
}

QWidget *AIDebuggerWidget::createTopWidget() {
	QSplitter *splitter = new QSplitter;

	_mapWidget = _debugger.createMapWidget();
	_mapFrame = new ZoomFrame(_mapWidget);

	_entityFilter = new QLineEdit();
	_entityList = new EntityList(_debugger, _entityFilter);
	_namesComboBox = new QComboBox();
	_namesComboBox->setFixedWidth(_entityList->width());
	_namesComboBox->setInsertPolicy(QComboBox::InsertAlphabetically);
	_namesComboBox->addItem(tr("None"));

	splitter->addWidget(_mapFrame);

	QVBoxLayout *vbox = new QVBoxLayout();
	vbox->setMargin(0);
	vbox->addWidget(_namesComboBox);
	vbox->addWidget(_entityFilter);
	vbox->addWidget(_entityList);

	QWidget *widget = new QWidget();
	widget->setFixedWidth(_entityList->width());
	widget->setLayout(vbox);
	splitter->addWidget(widget);

	return splitter;
}

void AIDebuggerWidget::showContextMenu(const QPoint &pos) {
	const QModelIndex& index = _tree->indexAt(pos);

	QMenu *menu = new QMenu(this);
	// TODO:
	menu->addAction(new QAction(tr("Add node"), this));
	menu->addAction(new QAction(tr("Delete node"), this));
	menu->popup(_tree->viewport()->mapToGlobal(pos));
}

QWidget *AIDebuggerWidget::createTreePanelWidget() {
	QWidget* treePanel = new QWidget();
	treePanel->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Expanding);

	_nodeTree = new NodeTreeView(_debugger, _resolver);
	_nodeTreeFrame = new ZoomFrame(_nodeTree, treePanel);
	_nodeTreeFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	_nodeTreeFrame->setVisible(false);
	_aggroTable = new AggroTable(_debugger);
	_stateTable = new StateTable(_debugger);

	_tree = new QTreeView(treePanel);
	_tree->setUniformRowHeights(true);
	_tree->setAlternatingRowColors(true);
	_tree->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	_tree->setModel(&_model);

	connect(_tree, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(showContextMenu(const QPoint &)));

	QHeaderView *header = _tree->header();
	header->setStretchLastSection(false);
	header->setSectionResizeMode(COL_NAME, QHeaderView::Stretch);
	header->setSectionResizeMode(COL_TYPE, QHeaderView::Stretch);
	header->setSectionResizeMode(COL_CONDITION, QHeaderView::Stretch);
	header->setSectionResizeMode(COL_STATE, QHeaderView::ResizeToContents);
	header->setSectionResizeMode(COL_LASTRUN, QHeaderView::ResizeToContents);

	QPushButton *toggle = new QPushButton(QIcon(":/images/switch.png"), "", treePanel);
	toggle->setFlat(true);
	toggle->setCheckable(true);
	toggle->setFixedSize(16, 16);
	toggle->setToolTip(tr("Switch between tree views"));
	connect(toggle, SIGNAL(released()), this, SLOT(toggleTreeView()));
	toggle->raise();

	QGridLayout *treeLayout = new QGridLayout();
	treeLayout->setColumnStretch(0, 10);
	treeLayout->setRowStretch(0, 10);
	treeLayout->addWidget(_nodeTreeFrame, 0, 0);
	treeLayout->addWidget(_tree, 0, 0);
	treeLayout->addWidget(toggle, 0, 0, Qt::AlignRight | Qt::AlignTop);

	treePanel->setLayout(treeLayout);
	treePanel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	return treePanel;
}

QWidget *AIDebuggerWidget::createBottomWidget() {
	QSplitter *splitter = new QSplitter;

	QWidget* treePanel = createTreePanelWidget();
	splitter->addWidget(treePanel);
	splitter->setStretchFactor(splitter->indexOf(treePanel), 5);
	splitter->addWidget(_aggroTable);
	splitter->setStretchFactor(splitter->indexOf(_aggroTable), 1);
	splitter->addWidget(_stateTable);
	splitter->setStretchFactor(splitter->indexOf(_stateTable), 1);
	return splitter;
}

void AIDebuggerWidget::onDisconnect() {
	_statusBarLabel->setText(tr("not connected"));
}

void AIDebuggerWidget::change(const QString &name) {
	_name = name;
	_debugger.change(name);
}

void AIDebuggerWidget::setPause(bool pause) {
	if (pause) {
		_pauseAction->setIcon(QIcon(":/images/continue.png"));
	} else {
		_pauseAction->setIcon(QIcon(":/images/pause.png"));
	}
}

void AIDebuggerWidget::requestStep() {
	_debugger.step();
}

void AIDebuggerWidget::requestReset() {
	_debugger.reset();
}

void AIDebuggerWidget::requestPause() {
	_debugger.togglePause();
}

void AIDebuggerWidget::connectToAIServer(const QString& hostname, short port) {
	if (_debugger.connectToAIServer(hostname, port)) {
		_statusBarLabel->setText(tr("connected to %1:%2").arg(hostname).arg(port));
	} else {
		_statusBarLabel->setText(tr("connection to %1:%2 failed").arg(hostname).arg(port));
	}
}

void AIDebuggerWidget::connectToAIServer() {
	ConnectDialog d;
	const int state = d.run();
	if (state != QDialog::Accepted) {
		_statusBarLabel->setText(tr("not connected"));
		return;
	}
	const short port = d.getPort();
	const QString& hostname = d.getHostname();
	connectToAIServer(hostname, port);
}

void AIDebuggerWidget::about() {
	QMessageBox::about(this, tr("About"), tr("AI debug visualization for libsimpleai.<br />Grab the latest version at <a href=\"https://github.com/mgerhardy/simpleai\">github</a>"));
}

void AIDebuggerWidget::documentation() {
	QDesktopServices::openUrl(QUrl("https://github.com/mgerhardy/simpleai/wiki"));
}

void AIDebuggerWidget::bug() {
	QDesktopServices::openUrl(QUrl("https://github.com/mgerhardy/simpleai/issues"));
}

void AIDebuggerWidget::toggleTreeView() {
	if (_nodeTreeFrame->isVisible()) {
		_nodeTreeFrame->setVisible(false);
		_tree->setVisible(true);
	} else {
		_nodeTreeFrame->setVisible(true);
		_tree->setVisible(false);
	}
}

void AIDebuggerWidget::createActions() {
	_connectAction = new QAction(tr("C&onnect"), this);
	_connectAction->setShortcuts(QKeySequence::Open);
	_connectAction->setStatusTip(tr("Connect to AI server"));
	_connectAction->setIcon(QIcon(":/images/connect.png"));
	connect(_connectAction, SIGNAL(triggered()), this, SLOT(connectToAIServer()));

	_pauseAction = new QAction(tr("Pause"), this);
	_pauseAction->setStatusTip(tr("Freeze the ai controlled entities"));
	_pauseAction->setIcon(QIcon(":/images/pause.png"));
	connect(_pauseAction, SIGNAL(triggered()), this, SLOT(requestPause()));

	_stepAction = new QAction(tr("Step"), this);
	_stepAction->setStatusTip(tr("Performs one step while ai is in pause mode"));
	_stepAction->setIcon(QIcon(":/images/step.png"));
	connect(_stepAction, SIGNAL(triggered()), this, SLOT(requestStep()));

	_resetAction = new QAction(tr("Reset"), this);
	_resetAction->setStatusTip(tr("Resets the states of the ai"));
	_resetAction->setIcon(QIcon(":/images/reset.png"));
	connect(_resetAction, SIGNAL(triggered()), this, SLOT(requestReset()));

	_aboutAction = new QAction(tr("&About"), this);
	_aboutAction->setStatusTip(tr("Show the application's About box"));
	_aboutAction->setIcon(QIcon(":/images/about.png"));
	connect(_aboutAction, SIGNAL(triggered()), this, SLOT(about()));

	_documentationAction = new QAction(tr("&Documentation"), this);
	_documentationAction->setStatusTip(tr("Open the libsimpleai documentation"));
	_documentationAction->setIcon(QIcon(":/images/docs.png"));
	connect(_documentationAction, SIGNAL(triggered()), this, SLOT(documentation()));

	_bugAction = new QAction(tr("&Report a bug"), this);
	_bugAction->setStatusTip(tr("Report a bug"));
	_bugAction->setIcon(QIcon(":/images/bug.png"));
	connect(_bugAction, SIGNAL(triggered()), this, SLOT(bug()));
}

QLabel *AIDebuggerWidget::createLabel(const QString &text) const {
	QLabel *label = new QLabel(text);
	label->setAlignment(Qt::AlignCenter);
	label->setMargin(2);
	label->setFrameStyle(QFrame::Box | QFrame::Sunken);
	return label;
}

}
}
