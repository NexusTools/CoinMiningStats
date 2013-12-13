#include "manageminers.h"

#include <QInputDialog>
#include <QMessageBox>
#include <QFileDialog>
#include <QDebug>

ManageMiners::ManageMiners(QWidget *parent) :
	QDialog(parent) {
	setupUi(this);
	show();

	setAttribute(Qt::WA_DeleteOnClose);
	connect(addMiner, SIGNAL(clicked()), this, SLOT(addMinerEntry()));
	connect(removeMiner, SIGNAL(clicked()), this, SLOT(removeMinerEntry()));
	connect(minerList, SIGNAL(currentRowChanged(int)), this, SLOT(updateMinerPage()));
	connect(arguments, SIGNAL(currentRowChanged(int)), this, SLOT(updateArgumentControls()));
	connect(removeArgument, SIGNAL(clicked()), this, SLOT(removeArg()));
	connect(addArgument, SIGNAL(clicked()), this, SLOT(addArg()));
	connect(moveUp, SIGNAL(clicked()), this, SLOT(moveArgUp()));
	connect(moveDown, SIGNAL(clicked()), this, SLOT(moveArgDown()));
//	connect(host, SIGNAL(currentIndexChanged(int)), this, SLOT(storePage()));
//	connect(key, SIGNAL(textChanged(QString)), this, SLOT(storePage()));
//	connect(secert, SIGNAL(textChanged(QString)), this, SLOT(storePage()));
	connect(program, SIGNAL(textChanged(QString)), this, SLOT(storePage()));
	connect(browse, SIGNAL(clicked()), this, SLOT(browseProgram()));
	connect(discard, SIGNAL(clicked()), this, SLOT(close()));
	connect(store, SIGNAL(clicked()), this, SLOT(save()));
}

void ManageMiners::browseProgram(){
	blockSignals(true);
	QString file = QFileDialog::getOpenFileName(this, "Select Mining Program", QDir::homePath());
	if(!file.isNull())
		program->setText(file);
	blockSignals(false);
	storePage();
}

void ManageMiners::save(){
	emit dataUpdated(minerData);
	close();
}

void ManageMiners::storePage() {
	if(signalsBlocked())
		return;

	if(minerList->count() == 0) { //User probably removed the miners.
		store->setEnabled(true);
		return;
	}

	QListWidgetItem* item = minerList->currentItem();
	if(item) {
		QMap<QString, QVariant> minerEntry;
//		minerEntry.insert("host", host->currentIndex());
//		minerEntry.insert("hostKey", key->text());
//		minerEntry.insert("hostSecert", secert->text());
		minerEntry.insert("program", program->text());
		QStringList args;
		for(int i=0; i<arguments->count(); i++)
			args.append(arguments->item(i)->text());
		minerEntry.insert("arguments", args);
		qDebug() << "Storing Page" << minerEntry;
		minerData.insert(item->text(), minerEntry);
		store->setEnabled(true);
	} else
		qWarning() << "Store Called with no Active Miner";
}

void ManageMiners::addArg() {
	blockSignals(true);
	QInputDialog inputDiag(this);
	inputDiag.setInputMode(QInputDialog::TextInput);
	inputDiag.setLabelText("Launch Argument");
	inputDiag.exec();
	QString argu = inputDiag.textValue();
	if(!argu.isNull())
		arguments->addItem(argu);
	blockSignals(false);
	storePage();
}

void ManageMiners::removeArg() {
	blockSignals(true);
	arguments->model()->removeRow(arguments->currentRow());
	blockSignals(false);
	storePage();
}

void ManageMiners::moveArgUp() {
	blockSignals(true);
	QListWidgetItem* upItem = arguments->item(arguments->currentRow()-1);
	QString upVal = upItem->text();
	upItem->setText(arguments->currentItem()->text());
	arguments->currentItem()->setText(upVal);
	arguments->setCurrentRow(arguments->currentRow()-1);
	updateArgumentControls();
	blockSignals(false);
	storePage();
}

void ManageMiners::moveArgDown() {
	blockSignals(true);
	QListWidgetItem* downItem = arguments->item(arguments->currentRow()+1);
	QString upVal = downItem->text();
	downItem->setText(arguments->currentItem()->text());
	arguments->currentItem()->setText(upVal);
	arguments->setCurrentRow(arguments->currentRow()+1);
	updateArgumentControls();
	blockSignals(false);
	storePage();
}

void ManageMiners::setMinerData(QVariant var) {
	blockSignals(true);
	minerData = var.toMap();
	minerList->clear();
	foreach(QString name, minerData.keys())
		minerList->addItem(name);

	minerList->setFocus();
	minerList->setCurrentRow(0);
	blockSignals(false);
}

void ManageMiners::updateArgumentControls() {
	moveUp->setEnabled(arguments->currentRow() > 0);
	moveDown->setEnabled(arguments->currentRow() > -1 && arguments->currentRow() < arguments->count()-1);
	removeArgument->setEnabled(arguments->currentRow() > -1);
}

void ManageMiners::updateMinerPage() {
	blockSignals(true);
	arguments->clear();
	minerPage->setEnabled(minerList->currentItem());
	removeMiner->setEnabled(minerPage->isEnabled());

	if(minerPage->isEnabled()) {
		QMap<QString, QVariant> minerEntry = minerData.value(minerList->currentItem()->text()).toMap();
//		if(minerEntry.value("host").canConvert(QVariant::Int))
//			host->setCurrentIndex(minerEntry.value("host").toInt());

//		if(minerEntry.value("hostKey").canConvert(QVariant::String))
//			key->setText(minerEntry.value("hostKey").toString());

//		if(minerEntry.value("hostSecert").canConvert(QVariant::String))
//			secert->setText(minerEntry.value("hostSecert").toString());

		if(minerEntry.value("program").canConvert(QVariant::String))
			program->setText(minerEntry.value("program").toString());

		if(minerEntry.value("arguments").canConvert(QVariant::StringList))
			foreach(QString arg, minerEntry.value("arguments").toStringList())
				arguments->addItem(arg);
		qDebug() << minerEntry;
	} else
		program->setText("");

	updateArgumentControls();
	blockSignals(false);
}

void ManageMiners::removeMinerEntry() {
	blockSignals(true);
	qDebug() << "Removing" << minerList->currentIndex();
	if(minerList->currentItem()) {
		minerData.remove(minerList->currentItem()->text());
		minerList->model()->removeRow(minerList->currentRow());
	}
	updateMinerPage();
	storePage();
}

void ManageMiners::addMinerEntry() {
	blockSignals(true);
	QInputDialog inputDiag(this);
	inputDiag.setInputMode(QInputDialog::TextInput);
	inputDiag.setLabelText("Name for Miner");
	inputDiag.exec();
	QString minerName = inputDiag.textValue();
	if(!minerName.isNull()) {
		if(minerData.contains(minerName))
			QMessageBox::warning(this, "Failed", "A miner with that name already exists.");
		else {
			QListWidgetItem* newItem = new QListWidgetItem(minerName);
			minerList->addItem(newItem);
			minerList->setCurrentItem(newItem);
		}
	}
	updateMinerPage();
	storePage();
}

void ManageMiners::changeEvent(QEvent *e) {
	QDialog::changeEvent(e);
	switch (e->type()) {
	case QEvent::LanguageChange:
		retranslateUi(this);
		break;
	default:
		break;
	}
}
