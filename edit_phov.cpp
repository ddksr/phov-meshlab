/****************************************************************************
 * MESHLAB															 o o	 *
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
#include <QProgressDialog>
#include <QHttpRequestHeader>
#include "edit_phov.h"
#include "edit_phov_api.h"

// TODO: remove
#include <unistd.h>


using namespace std;
using namespace vcg;

EditPhovPlugin::EditPhovPlugin() {
	qFont.setFamily("Helvetica");
	qFont.setPixelSize(12);
	
	qDebug() << "Init Edit PHOV";

	settingsFile = QDir::homePath()+ QDir::separator() + "meshlab_phov.ini";
	loadSettings();

	isEnabled = true;
	// if (phovID.isEmpty()) {
	// 	getPhovId();
	// } else {
	// 	isEnabled = true;
	// 	saveSettings();
	// }
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
	QWidget* parent = gla->window();

	if (isWaiting) {
		if (phovID.isEmpty()) {
			QMessageBox::warning(parent->parentWidget(),
								 "PHOV model download",
								 "There was something terribly wrong. Please reupload the images.");
			isWaiting = false;
			saveSettings();
		}
		else if (checkDownloadAvailable()) {
			// check if model ready, be verbose
			qDebug() << "Download IS available";
			downloadModel();
		}
		else {
			qDebug() << "Download NOT available";
			
			QMessageBox::information(parent->parentWidget(),
									 "PHOV model downlaod",
									 "PHOV model is not available yet.");
			//TODO: inform no download available
		}
	}
	else {
		if (phovID.isEmpty()) {
			getPhovId();
		}
		uploadImages();
	}

	return true;
}

void EditPhovPlugin::getPhovId() {
	QNetworkAccessManager nm(this);
	QEventLoop eventLoop;
	
	qDebug() << "getPhovId";
	QUrl url(apiUrlGetId);
	QByteArray postData;
	postData.append("notify=mailto:sigi.kajzer@gmail.com");
	qDebug() << url;

	QNetworkRequest request(url);    
	request.setHeader(QNetworkRequest::ContentTypeHeader, 
					  "application/x-www-form-urlencoded");
	
	QObject::connect(&nm, SIGNAL(finished(QNetworkReply*)),
					 &eventLoop, SLOT(quit()));
	QNetworkReply* reply = nm.post(request, postData);
	eventLoop.exec(); 
	if (reply->error() == QNetworkReply::NoError) {
        //success
        phovID = QString(reply->readAll());
		qDebug() << "PHOV ID";
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
	QUrl url(apiUrlCheckDownload.arg(phovID));

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
	QUrl url(apiUrlDownload.arg(phovID));

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

		isWaiting = false;
		phovID = "";
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
	bool isSuccess = true;
    QWidget* parent = gla->window();
	QStringList files = QFileDialog::getOpenFileNames(parent->parentWidget(),
													  tr("Open images for PHOV"),
													  tr("JPEG/PNG Image Files"));
	
	QProgressDialog progress("Uploading files to PHOV ...",
							 "Abort Upload",
							 0, files.size(),
							 parent->parentWidget());
	progress.setWindowModality(Qt::WindowModal);
	progress.show();
	for (int i = 0; i < files.size(); ++i) {
		QString filepath = files.at(i);
		qDebug() << "Uploading " << filepath;
		QFile file(filepath);
		QString filename = QFileInfo(filepath).baseName();
	
		// HTTP request
		QHttp *http = new QHttp(this);
		QString boundary = "---------------------------193971182219750";
	
		QByteArray datas(QString("--" + boundary + "\r\n").toAscii());
		datas += "Content-Disposition: form-data; name=\"uploadedImage\"; filename=\"";
		datas += filename.toAscii();
		datas += "\"\r\n";
		datas += "Content-Type: image/jpeg\r\n\r\n";
	
		if (!file.open(QIODevice::ReadOnly))
			return;
 
		datas += file.readAll();
		datas += "\r\n";
		datas += QString("--" + boundary + "\r\n").toAscii();
		datas += "Content-Disposition: form-data; name=\"";
		datas += "uploadedImage";
		datas += "\"\r\n\r\n";
		datas += "Uploader\r\n";
		datas += QString("--" + boundary + "--\r\n").toAscii();

		QUrl url(apiUrlUpload.arg(phovID));

		QHttpRequestHeader header("POST", url.toString());
		header.setValue("Host", url.host());
		header.setValue("Content-Type", "multipart/form-data; boundary=" + boundary);
		header.setValue("Content-Length", QString::number(datas.length()));
 
		http->setHost(url.host());
		http->request(header, datas);
		
		progress.setValue(i);
		if (progress.wasCanceled()) {
			qDebug() << "Canceld upload";
			isSuccess = false;
            break;
		}
		qDebug() << "Finished " << filepath << "!";
	}
	progress.setValue(files.size());
	if (isSuccess) {
		finishUpload();
	}
	
}
void EditPhovPlugin::finishUpload() {
	QNetworkAccessManager nm(this);
	QEventLoop eventLoop;
	
	QUrl url(apiUrlFinish.arg(phovID));

	qDebug() << url;

	QNetworkRequest request(url);    
	request.setHeader(QNetworkRequest::ContentTypeHeader, 
					  "application/x-www-form-urlencoded");
	
	QObject::connect(&nm, SIGNAL(finished(QNetworkReply*)),
					 &eventLoop, SLOT(quit()));
	QNetworkReply* reply = nm.get(request);
	eventLoop.exec(); 
	if (reply->error() == QNetworkReply::NoError) {
		isWaiting = true;
		saveSettings();
    }
    else {
        qDebug() << "Failure" <<reply->errorString();
    }
	delete reply;
	
}
