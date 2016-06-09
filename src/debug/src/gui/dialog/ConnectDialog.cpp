#include "ConnectDialog.h"
#include <QSettings>
#include "Settings.h"

ConnectDialog::ConnectDialog(const QString& defaultHostname, short defaultPort) :
		IDialog(tr("Connect to AI server")), _hostnameText(nullptr), _portText(nullptr), _group(
				nullptr) {
	_hostname = Settings::getHostname(defaultHostname);
	_port = Settings::getPort(defaultPort);
}

ConnectDialog::~ConnectDialog() {
	delete _hostnameText;
	delete _portText;
	delete _group;
}

void ConnectDialog::addMainWidgets(QBoxLayout& layout) {
	_group = new QGroupBox(tr("Server"));
	QVBoxLayout *boxLayout = new QVBoxLayout;
	_hostnameText = new QLineEdit(_hostname);
	boxLayout->addWidget(_hostnameText);
	_portText = new QLineEdit(QString::number(_port));
	boxLayout->addWidget(_portText);
	_group->setLayout(boxLayout);
	layout.addWidget(_group);
}

void ConnectDialog::onApply() {
	_hostname = _hostnameText->text();
	_port = _portText->text().toShort();

	Settings::setHostname(_hostname);
	Settings::setPort(_portText->text().toInt());

	close();
}
