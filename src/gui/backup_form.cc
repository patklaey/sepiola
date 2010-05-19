/*
#| sepiola - Open Source Online Backup Client
#| Copyright (C) 2007, 2008  stepping stone GmbH
#|
#| This program is free software; you can redistribute it and/or
#| modify it under the terms of the GNU General Public License
#| Version 2 as published by the Free Software Foundation.
#|
#| This program is distributed in the hope that it will be useful,
#| but WITHOUT ANY WARRANTY; without even the implied warranty of
#| MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#| GNU General Public License for more details.
#|
#| You should have received a copy of the GNU General Public License
#| along with this program; if not, write to the Free Software
#| Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include <QDebug>
#include <QMessageBox>
#include <QShortcut>

#include "gui/backup_form.hh"
#include "gui/pattern_dialog.hh"
#include "model/scheduled_task.hh"

#include "utils/log_file_utils.hh"

// default const values
/* it's not worth at this moment to add a new settings-variable, therefore default-schedule-values are set here */
const QTime BackupForm::default_schedule_time = QTime(12,0,0,0);
const int BackupForm::default_schedule_minutesAfterStartup = 10;


BackupForm::BackupForm( QWidget *parent, MainModel *model ) : QWidget ( parent )
{
	setupUi ( this );
	Settings* settings = Settings::getInstance();
	this->model = model;
	this->localDirModel = this->model->getLocalDirModel();
	this->localDirModel->setSelectionRules(	settings->getLastBackupSelectionRules() );

	this->treeView->setModel( localDirModel );
	this->treeView->setSelectionMode( QAbstractItemView::ExtendedSelection );
	this->treeView->setColumnWidth( 0, 300 );
	this->expandSelectedBranches();

		// disable option "minutes after booting" if the scheduler does not support it
	if ( !this->model->isSchedulingOnStartSupported() )
	{
		this->radioButtonMinutesAfterBooting->setEnabled( false );
		this->spinBoxMinutesAfterBooting->setEnabled( false );
	}

	ScheduledTask rule = settings->getScheduleRule();
	switch (rule.getType()) {
		case ScheduleRule::NEVER:
			this->radioButtonNoSchedule->setChecked(true);
			on_radioButtonNoSchedule_clicked();
		break;
		case ScheduleRule::AFTER_BOOT:
			this->radioButtonMinutesAfterBooting->setChecked(true);
			on_radioButtonMinutesAfterBooting_clicked();
			this->spinBoxMinutesAfterBooting->setValue(rule.getMinutesAfterStartup());
		break;
		case ScheduleRule::AT_WEEKDAYS_AND_TIME:
			this->radioButtonDaily->setChecked(true);
			on_radioButtonDaily_clicked();
			this->checkBoxMonday->setChecked(rule.getWeekdays().contains(ScheduleRule::MONDAY));
			this->checkBoxTuesday->setChecked(rule.getWeekdays().contains(ScheduleRule::TUESDAY));
			this->checkBoxWednesday->setChecked(rule.getWeekdays().contains(ScheduleRule::WEDNESDAY));
			this->checkBoxThursday->setChecked(rule.getWeekdays().contains(ScheduleRule::THURSDAY));
			this->checkBoxFriday->setChecked(rule.getWeekdays().contains(ScheduleRule::FRIDAY));
			this->checkBoxSaturday->setChecked(rule.getWeekdays().contains(ScheduleRule::SATURDAY));
			this->checkBoxSunday->setChecked(rule.getWeekdays().contains(ScheduleRule::SUNDAY));
			this->timeEditTime->setTime(rule.getTimeToRun());
		break;
	}

	new QShortcut( Qt::Key_F5, this, SLOT( refreshLocalDirModel() ) );
	QObject::connect( this, SIGNAL( updateOverviewFormScheduleInfo() ),
					  parent, SIGNAL ( updateOverviewFormScheduleInfo() ) );
	QObject::connect( this, SIGNAL( updateOverviewFormLastBackupsInfo() ),
					  parent, SIGNAL ( updateOverviewFormLastBackupsInfo() ) );
}

BackupForm::~BackupForm()
{}

void BackupForm::expandSelectedBranches()
{
	foreach ( QString location, this->localDirModel->getSelectionRules().keys() ) {
		QString loc = StringUtils::parentDir(location);
		QModelIndex index = localDirModel->index( loc );
		while (index.isValid()) {
			this->treeView->expand( index ); // if possible
			index = index.parent();
		}
	}
}

void BackupForm::on_btnSchedule_clicked()
{
	if ( this->localDirModel->getSelectionRules().size() == 0 && !this->radioButtonNoSchedule->isChecked() )
	{
		QMessageBox::information( this,
									tr( "Empty backup list" ),
									tr( "No items have been selected for scheduling" )
								);
		return;
	}
	/*ScheduleDialog scheduleDialog( this->model, backupItems, includePatternList, excludePatternList );
	scheduleDialog.exec();*/
	this->schedule();
}

void BackupForm::schedule()
{
	ScheduledTask scheduleTask;
	if ( this->radioButtonDaily->isChecked() )
	{
		QTime time = this->timeEditTime->time();
		QSet<ScheduleRule::Weekdays> wd;
		if (this->checkBoxMonday->checkState() == Qt::Checked) wd.insert(ScheduleRule::MONDAY);
		if (this->checkBoxTuesday->checkState() == Qt::Checked) wd.insert(ScheduleRule::TUESDAY);
		if (this->checkBoxWednesday->checkState() == Qt::Checked) wd.insert(ScheduleRule::WEDNESDAY);
		if (this->checkBoxThursday->checkState() == Qt::Checked) wd.insert(ScheduleRule::THURSDAY);
		if (this->checkBoxFriday->checkState() == Qt::Checked) wd.insert(ScheduleRule::FRIDAY);
		if (this->checkBoxSaturday->checkState() == Qt::Checked) wd.insert(ScheduleRule::SATURDAY);
		if (this->checkBoxSunday->checkState() == Qt::Checked) wd.insert(ScheduleRule::SUNDAY);

		bool validSelection = wd.size() > 0;;

		if ( !validSelection )
		{
			this->model->infoDialog( tr( "Check at least one day" ) );
			return;
		}
		scheduleTask = ScheduledTask(wd, time);
	}
	else if ( this->radioButtonMinutesAfterBooting->isChecked() )
	{
		int delay = this->spinBoxMinutesAfterBooting->value();
		scheduleTask = ScheduledTask(delay);
	}
	else if ( this->radioButtonNoSchedule->isChecked() )
	{
		scheduleTask = ScheduledTask();
	}
	this->model->schedule( this->model->getLocalDirModel()->getSelectionRules(), scheduleTask );
	qDebug() << "BackupForm::schedule(): emit updateOverviewFormScheduleInfo()";
	emit updateOverviewFormScheduleInfo();
}


void BackupForm::on_btnBackup_clicked()
{
	if ( this->localDirModel->getSelectionRules().size() == 0 )
	{
		QMessageBox::information( this, tr( "Empty backup list" ), tr( "No items have been selected for backup" ));
		return;
	}
	this->model->showProgressDialogSlot( tr( "Backup" ) );
	qDebug() << "BackupForm::on_btnBackup_clicked()" << this->model->getLocalDirModel()->getSelectionRules();
	this->model->backup( this->model->getLocalDirModel()->getSelectionRules(), false );
	//this->model->backup( backupItems, includePatternList, excludePatternList, deleteExtraneous, false );
	emit updateOverviewFormLastBackupsInfo();
}

QString BackupForm::patternListToString( QStringList patternList )
{
	QString result;
	for ( int i=0; i<patternList.size(); i++ )
	{
		result.append( patternList.at( i ) );
		if ( i < patternList.size() -1 )
		{
			result.append( ", ");
		}
	}
	return result;
}

void BackupForm::disableScheduleOptions() {
	this->spinBoxMinutesAfterBooting->setEnabled(false);
	this->spinBoxMinutesAfterBooting->setValue( default_schedule_minutesAfterStartup );
	this->timeEditTime->setEnabled(false);
	this->timeEditTime->setTime( default_schedule_time );
	QList<QCheckBox*> weekdaysCheckboxes;
	weekdaysCheckboxes << this->checkBoxMonday << this->checkBoxTuesday << this->checkBoxWednesday << this->checkBoxThursday << this->checkBoxFriday << this->checkBoxSaturday << this->checkBoxSunday;
	foreach (QCheckBox* cb, weekdaysCheckboxes ) {
		cb->setChecked(true);
		cb->setEnabled(false);
	}
}

void BackupForm::on_radioButtonNoSchedule_clicked() {
	disableScheduleOptions();
}

void BackupForm::on_radioButtonMinutesAfterBooting_clicked() {
	disableScheduleOptions();
	this->spinBoxMinutesAfterBooting->setEnabled(true);
}

void BackupForm::on_radioButtonDaily_clicked() {
	disableScheduleOptions();
	this->timeEditTime->setEnabled(true);
	QList<QCheckBox*> weekdaysCheckboxes;
	weekdaysCheckboxes << this->checkBoxMonday << this->checkBoxTuesday << this->checkBoxWednesday << this->checkBoxThursday << this->checkBoxFriday << this->checkBoxSaturday << this->checkBoxSunday;
	foreach (QCheckBox* cb, weekdaysCheckboxes ) {
		cb->setEnabled(true);
	}
}


QStringList BackupForm::getSelectedFilesAndDirs()
{
	QModelIndexList indexes = this->treeView->selectionModel()->selectedIndexes();
	QStringList items;
	// save current value of localDirModel->resolveSymlinks to reset after getting filePaths of the items
	bool bkup_ResolveSymbolicLinks = localDirModel->resolveSymlinks();
	localDirModel->setResolveSymlinks(false);
	foreach ( QModelIndex index, indexes )
	{
		if ( index.column() == 0 )
		{
			items << localDirModel->filePath( index );
		}
	}
	localDirModel->setResolveSymlinks(bkup_ResolveSymbolicLinks);
	return items;
}

void BackupForm::refreshLocalDirModel()
{
	if (Settings::getInstance()->getShowHiddenFilesAndFolders()) {
		this->localDirModel->setFilter(this->localDirModel->filter() | QDir::Hidden);
	} else {
		this->localDirModel->setFilter(this->localDirModel->filter() & (~QDir::Hidden));
	}
	this->localDirModel->refresh();
}

void BackupForm::on_btnRefresh_clicked()
{
	refreshLocalDirModel();
}
