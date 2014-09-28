/****************************************************************************
* MeshLab                                                           o o     *
* A versatile mesh processing toolbox                             o     o   *
*                                                                _   O  _   *
* Copyright(C) 2005                                                \/)\/    *
* Visual Computing Lab                                            /\/|      *
* ISTI - Italian National Research Council                           |      *
*                                                                    \      *
* All rights reserved.                                                      *
*                                                                           *
* This program is free software; you can redistribute it and/or modify      *   
* it under the terms of the GNU General Public License as published by      *
* the Free Software Foundation; either version 2 of the License, or         *
* (at your option) any later version.                                       *
*                                                                           *
* This program is distributed in the hope that it will be useful,           *
* but WITHOUT ANY WARRANTY; without even the implied warranty of            *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
* GNU General Public License (http://www.gnu.org/licenses/gpl.txt)          *
* for more details.                                                         *
*                                                                           *
****************************************************************************/

#ifndef EDIT_PHOV_H
#define EDIT_PHOV_H

#include <QObject>
#include <common/interfaces.h>
#include <meshlab/glarea.h>
#include <Qt>
#include <QMessageBox>
#include <QDialog>
#include <QDockWidget>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QUrl>
#include <QNetworkReply>

class EditPhovPlugin : public QObject, public MeshEditInterface
{
	Q_OBJECT
	Q_INTERFACES(MeshEditInterface)
		
public:
    EditPhovPlugin();
    virtual ~EditPhovPlugin() {}

	bool StartEdit(MeshDocument &/*m*/, GLArea * /*parent*/);
	static const QString Info();
	
    void EndEdit(MeshModel &/*m*/, GLArea * /*parent*/) {}
    void Decorate(MeshModel &/*m*/, GLArea * /*parent*/, QPainter *p) {}
    void Decorate (MeshModel &/*m*/, GLArea * ) {}
    void mousePressEvent(QMouseEvent *, MeshModel &, GLArea * ) {}
    void mouseMoveEvent(QMouseEvent *, MeshModel &, GLArea * ) {}
    void mouseReleaseEvent(QMouseEvent *event, MeshModel &/*m*/, GLArea * ) {}
	void drawFace(CMeshO::FacePointer fp,MeshModel &m, GLArea *gla, QPainter *p) {}

	// IMPORTANT: because if true, dialog will fail
	bool isSingleMeshEdit() const {return false;}
	
	QFont qFont;

	MeshDocument *md; 
	GLArea *gla;

private:
	void loadSettings();
	void saveSettings();
	void uploadImages();
	void getPhovId();
	bool checkDownloadAvailable();
	void downloadModel();
	void finishUpload();
	
	QString phovID;
	QString settingsFile;
	bool isEnabled;
	bool isWaiting;

public slots:
	void uploadImagesCallback() {}
};

#endif

