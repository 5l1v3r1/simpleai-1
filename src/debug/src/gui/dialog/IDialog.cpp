#include "IDialog.h"
#include <common/Compiler.h>

#include <QGroupBox>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>

IDialog::IDialog(const QString &title, int flags) :
		QDialog(nullptr, 0), _title(title), _applyButton(
		nullptr), _closeButton(nullptr), _buttonLayout(nullptr), _buttons(
		nullptr), _mainLayout(nullptr), _flags(flags) {
	setWindowTitle(_title);
	setModal(false);
}

IDialog::~IDialog() {
	if (!(_flags & DIALOG_NO_APPLY_BUTTON)) {
		delete _applyButton;
	}
	delete _closeButton;
	delete _buttonLayout;
	delete _buttons;
	delete _mainLayout;
}

int IDialog::run() {
	_buttonLayout = new QHBoxLayout;
	_buttons = new QWidget;
	addButtons(*_buttonLayout);
	_buttons->setLayout(_buttonLayout);

	_mainLayout = new QVBoxLayout;
	addMainWidgets(*_mainLayout);
	_mainLayout->addSpacing(12);
	_mainLayout->addWidget(_buttons);
	_mainLayout->addStretch(1);
	setLayout(_mainLayout);

	show();
	exec();
	return 0;
}

void IDialog::addButtons(QBoxLayout& layout) {
	if (!(_flags & DIALOG_NO_APPLY_BUTTON)) {
		_applyButton = new QPushButton(tr("Apply"));
		connect(_applyButton, SIGNAL(clicked()), this, SLOT(apply()));
		layout.addWidget(_applyButton);
	}
	_closeButton = new QPushButton(tr("Close"));
	connect(_closeButton, SIGNAL(clicked()), this, SLOT(reject()));
	layout.addWidget(_closeButton);
}
