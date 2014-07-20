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
#include <QHttpRequestHeader>
#include "edit_phov.h"

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
		getPhovId();
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
		// TODO: message that something is wrong
	}
	else {
		if (isWaiting) {
			// check if model ready, be verbose
			if (checkDownloadAvailable()) {
				qDebug() << "Download IS available";
				downloadModel();
			}
			else {
				qDebug() << "Download NOT available";
				//TODO: inform no download available
			}
		}
		else {
			uploadImages();
		}
	}
	return true;
}

void EditPhovPlugin::getPhovId() {
	QNetworkAccessManager nm(this);
	QEventLoop eventLoop;
	
	qDebug() << "getPhovId";
	QUrl url(apiURL);
	url.addQueryItem("method", QString("get_id"));
	qDebug() << url;
	QObject::connect(&nm, SIGNAL(finished(QNetworkReply*)),
					 &eventLoop, SLOT(quit()));
	QNetworkReply* reply = nm.get(QNetworkRequest(url));
	eventLoop.exec(); 
	if (reply->error() == QNetworkReply::NoError) {
        //success
        phovID = QString(reply->readAll());
		saveSettings();
    }
    else {
        //failure
        qDebug() << "Failure" <<reply->errorString();
    }
	delete reply;
	qDebug() << "getPhovId end";
}

bool EditPhovPlugin::checkDownloadAvailable() {
	bool isAvailable = false;
	
	QNetworkAccessManager nm(this);
	QEventLoop eventLoop;
	qDebug() << "downloadAvaiblable";
	QUrl url(apiURL);
	url.addQueryItem("method", QString("check_download"));
	url.addQueryItem("id", phovID);

	qDebug() << url;
	QObject::connect(&nm, SIGNAL(finished(QNetworkReply*)),
	 				 &eventLoop, SLOT(quit()));
	QNetworkReply* reply = nm.get(QNetworkRequest(url));
	eventLoop.exec(); 
	if (reply->error() == QNetworkReply::NoError) {
		isAvailable = QString(reply->readAll()).toInt() == 1;
    }
    else {
        //failure
        qDebug() << "Failure" <<reply->errorString();
    }
	delete reply;
	return isAvailable;
}

void EditPhovPlugin::downloadModel() {
	QWidget* parent = gla->window();
	QString filename = QFileDialog::getSaveFileName(parent->parentWidget(),
													tr("Save PHOV PLY model"),
													QDir::homePath(),
													tr("PLY mesh (*.ply)"));

	
	QNetworkAccessManager nm(this);
	QEventLoop eventLoop;
	qDebug() << "downloadModel";
	QUrl url(apiURL);
	url.addQueryItem("method", QString("download"));
	url.addQueryItem("id", phovID);

	qDebug() << url;
	QObject::connect(&nm, SIGNAL(finished(QNetworkReply*)),
	 				 &eventLoop, SLOT(quit()));
	QNetworkReply* reply = nm.get(QNetworkRequest(url));
	eventLoop.exec(); 
	if (reply->error() == QNetworkReply::NoError) {
		qDebug() << "SUCCESS - CAN DOWNLOAD !!!! ";
		QFile file(filename);
		file.open(QIODevice::WriteOnly);
		file.write(reply->readAll());
		file.close();

		isWaiting = true;
		saveSettings();
    }
    else {
        //failure
        qDebug() << "Failure" <<reply->errorString();
    }

	delete reply;	
	qDebug() << "downloadModelEnd end";
}


void EditPhovPlugin::uploadImages() {
    QWidget* parent = gla->window();

	QString filepath = QFileDialog::getOpenFileName(parent->parentWidget(),
													tr("Open images for PHOV"),
													QDir::homePath(),
													tr("JPG Image Files ZIP (*.zip)"));
	qDebug() << filepath;
	// filepath -> filename
	
	QFile file(filepath);
	QString filename = QFileInfo(filepath).baseName();
	
	// HTTP request
	QHttp *http = new QHttp(this);
	QString boundary = "---------------------------193971182219750";
	
	QByteArray datas(QString("--" + boundary + "\r\n").toAscii());
	datas += "Content-Disposition: form-data; name=\"uploadedZip\"; filename=\"";
	datas += filename.toAscii();
	datas += "\"\r\n";
	datas += "Content-Type: application/zip\r\n\r\n";
 
	
	if (!file.open(QIODevice::ReadOnly))
		return;
 
	datas += file.readAll();
	datas += "\r\n";
	datas += QString("--" + boundary + "\r\n").toAscii();
	datas += "Content-Disposition: form-data; name=\"";
	datas += "uploadedZip";
	datas += "\"\r\n\r\n";
	datas += "Uploader\r\n";
	datas += QString("--" + boundary + "--\r\n").toAscii();

	QUrl url(apiURL);
	url.addQueryItem("method", QString("upload"));
	url.addQueryItem("id", phovID);
	QHttpRequestHeader header("POST", url.toString());
	header.setValue("Host", url.host());
	header.setValue("Content-Type", "multipart/form-data; boundary=" + boundary);
	header.setValue("Content-Length", QString::number(datas.length()));
 
	http->setHost(url.host());
	http->request(header, datas);

	isWaiting = true;
	saveSettings();
}
