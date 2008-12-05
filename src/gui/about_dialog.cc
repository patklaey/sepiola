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

#include <QLabel>

#include "gui/about_dialog.hh"
#include "settings/settings.hh"

const int AboutDialog::MAX_IMAGE_HEIGHT = 75; 
const int AboutDialog::MAX_IMAGE_WIDTH = 150; 

AboutDialog::AboutDialog()
{
	setupUi( this );
	this->setBackgroundRole( QPalette::Base );
	Settings* settings = Settings::getInstance();
	this->labelVersion->setText( tr( "%1 Version %2" ).arg( settings->getApplicationName(), Settings::VERSION ) );

	// rescale images if needed
	if ( this->imgSteppingStone->pixmap()->height() > AboutDialog::MAX_IMAGE_HEIGHT || this->imgSteppingStone->pixmap()->width() > AboutDialog::MAX_IMAGE_WIDTH ) {
		this->imgSteppingStone->setPixmap(this->imgSteppingStone->pixmap()->scaled(AboutDialog::MAX_IMAGE_WIDTH, AboutDialog::MAX_IMAGE_HEIGHT, Qt::KeepAspectRatio, Qt::SmoothTransformation));
	}
	if ( this->imgPuzzleITC->pixmap()->height() > AboutDialog::MAX_IMAGE_HEIGHT || this->imgPuzzleITC->pixmap()->width() > AboutDialog::MAX_IMAGE_WIDTH ) {
		this->imgPuzzleITC->setPixmap(this->imgPuzzleITC->pixmap()->scaled(AboutDialog::MAX_IMAGE_WIDTH, AboutDialog::MAX_IMAGE_HEIGHT, Qt::KeepAspectRatio, Qt::SmoothTransformation));
	}
	
	if (settings->isReseller())
	{
		this->groupBox1->setHidden(false);
		this->labelReseller->setText(settings->getResellerAddress());
		// rescale image if needed
		if ( this->imgReseller->pixmap()->height() > AboutDialog::MAX_IMAGE_HEIGHT || this->imgReseller->pixmap()->width() > AboutDialog::MAX_IMAGE_WIDTH ) {
			this->imgReseller->setPixmap(this->imgReseller->pixmap()->scaled(AboutDialog::MAX_IMAGE_WIDTH, AboutDialog::MAX_IMAGE_HEIGHT, Qt::KeepAspectRatio, Qt::SmoothTransformation));
		}
	}
	else
	{
		this->groupBox1->setHidden(true);
	}
}

AboutDialog::~AboutDialog()
{
}