/****************************************************************************
 * MeshLab															 o o	 *
 * A versatile mesh processing toolbox							   o	 o	 *
 *																  _	  O	 _	 *
 * Copyright(C) 2005												\/)\/	 *
 * Visual Computing Lab											   /\/|		 *
 * ISTI - Italian National Research Council							  |		 *
 *																	  \		 *
 * All rights reserved.														 *
 *																			 *
 * This program is free software; you can redistribute it and/or modify		 *	 
 * it under the terms of the GNU General Public License as published by		 *
 * the Free Software Foundation; either version 2 of the License, or		 *
 * (at your option) any later version.										 *
 *																			 *
 * This program is distributed in the hope that it will be useful,			 *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of			 *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the			 *
 * GNU General Public License (http://www.gnu.org/licenses/gpl.txt)			 *
 * for more details.														 *
 *																			 *
 ****************************************************************************/
/****************************************************************************
  History
$Log: meshedit.cpp,v $
****************************************************************************/
#include <Qt>
#include <QSettings>
#include <QDir>
#include <QFileDialog>
#include "edit_phov.h"
#include "edit_phov_client.h"

using namespace std;
using namespace vcg;

EditPhovPlugin::EditPhovPlugin() {
	qFont.setFamily("Helvetica");
	qFont.setPixelSize(12);

	qDebug() << "Init Edit PHOV";

	settingsFile = QDir::homePath()+ QDir::separator() + "meshlab_phov.ini";
	loadSettings();

	isEnabled = false;
	if (phovID.isEmpty()) {
		EditPhovClient client(*this);
		phovID = client.getPhovID();
		qDebug() << "test init: " << phovID;
	} else {
		isEnabled = true;
		saveSettings();
	}
	qDebug() << phovID;
}

void EditPhovPlugin::loadSettings() {
	QSettings settings(settingsFile, QSettings::NativeFormat);
	phovID = settings.value("phov_id", "").toString();
	isWaiting = settings.value("waiting", false).toBool();
}

void EditPhovPlugin::saveSettings() {
	QSettings settings(settingsFile, QSettings::NativeFormat);
	settings.setValue("phov_id", phovID);
	settings.setValue("waiting", isWaiting);
	settings.sync();
}

const QString EditPhovPlugin::Info() 
{
	return tr("PHOV photogrammetry service for Meshlab.");
}

bool EditPhovPlugin::StartEdit(MeshDocument &_md, GLArea *_gla ) {
	this->md = &_md;
	gla = _gla;

	if (phovID.isEmpty()) {
		phovID = 
	}
	
	if (isWaiting) {
		// check if model ready, be verbose
		downloadModel();
	}
	else {
		uploadImages();
	}
	
	return true;
}

void EditPhovPlugin::uploadImages() {
    QWidget* parent = gla->window();

	QString filename = QFileDialog::getOpenFileName(parent->parentWidget(),
													tr("Open images for PHOV"),
													QDir::homePath(),
													tr("JPG Image Files ZIP (*.zip)"));
	qDebug() << filename;
	
}
