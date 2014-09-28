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
			return false;
		}
		int status = checkDownloadAvailable();
		if (status == 1) {
			// check if model ready, be verbose
			qDebug() << "Download IS available";
			downloadModel();
		} else if (status == -1) {
			qDebug() << "Download IS error";
			QMessageBox::warning(parent->parentWidget(),
								 "PHOV model download",
								 "There was an error during the process. Try another image set.");
			isWaiting = false;
			phovID = "";
			saveSettings();
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
		if (phovID.isEmpty()) {
			QMessageBox::warning(parent->parentWidget(),
								 "PHOV image upload",
								 "The server seems to be busy. Please try again later.");
		}
		else {
			uploadImages();
		}
	}

	return false;
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
		QRegExp rgx(rxId);
		int pos = rgx.indexIn(reply->readAll());
		if (pos > -1) {
			phovID = rgx.cap(1);
			qDebug() << "PHOV ID: " << phovID;
			saveSettings();
		} else {
			qDebug() << "no PHOV ID";
		}
    }
    else {
        //failure
        qDebug() << "Failure" <<reply->errorString();
    }
	delete reply;
	qDebug() << "getPhovId end";
}

int EditPhovPlugin::checkDownloadAvailable() {
	int isAvailable = 0;
	
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
		QString response(reply->readAll());
		qDebug() << response;
		
		QRegExp rgx(rxError);
		int pos = rgx.indexIn(response);
		if (pos != -1 ) {
			isAvailable = -1;
		} else {
			QRegExp rgx2(rxSuccess);
			int pos = rgx2.indexIn(response);
			if (pos != -1 ) {
				isAvailable = 1;
			}
		}
    }
    else {
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

	int numOfFiles = files.size();

	if (numOfFiles <= 0) {
		return;
	}
	
	QProgressDialog progress("Uploading files to PHOV ...",
							 "Abort Upload",
							 0, numOfFiles,
							 parent->parentWidget());
	progress.setWindowModality(Qt::WindowModal);
	progress.show();
	progress.setValue(0);
	for (int i = 0; i < numOfFiles; ++i) {
		QString filepath = files.at(i);
		qDebug() << "Uploading " << filepath;
		QFile file(filepath);
		
		QString filename = QFileInfo(filepath).baseName();
	
		// HTTP request
		QHttp *http = new QHttp(this);
		QEventLoop eventLoop;
		connect(http, SIGNAL(done(bool)),
				&eventLoop, SLOT(quit()));
		
		
		QString boundary = "---------------------------193971182219750";
	
		QByteArray datas(QString("--" + boundary + "\r\n").toAscii());
		datas += "Content-Disposition: form-data; name=\"file\"; filename=\"";
		datas += filename.toAscii();
		datas += "\"\r\n";
		datas += "Content-Type: image/jpeg\r\n\r\n";
	
		if (!file.open(QIODevice::ReadOnly))
			return;
 
		datas += file.readAll();
		datas += "\r\n";
		datas += QString("--" + boundary + "\r\n").toAscii();
		datas += "Content-Disposition: form-data; name=\"";
		datas += "file";
		datas += "\"\r\n\r\n";
		datas += "Uploader\r\n";
		datas += QString("--" + boundary + "--\r\n").toAscii();

		QUrl url(apiUrlUpload.arg(phovID));

		QHttpRequestHeader header("POST", url.toString());
		header.setValue("Host", url.host());
		header.setValue("Content-Type",
						"multipart/form-data; boundary=" + boundary);
		header.setValue("Content-Length",
						QString::number(datas.length()));
		
 
		http->setHost(url.host());
		http->request(header, datas);

		eventLoop.exec();
 
		progress.setValue(i + 1);
		if (progress.wasCanceled()) {
			qDebug() << "Canceled upload";
			isSuccess = false;
            break;
		}
		qDebug() << "Finished " << filepath << "!";

		delete http;
	}
	if (isSuccess) {
		int status = QMessageBox::question(parent->parentWidget(),
										   "PHOV model upload",
										   "Do you wish to finish the upload and start the model processing?",
			 QMessageBox::Yes | QMessageBox::No);
		if (status == QMessageBox::Yes) {
			finishUpload();
		}
	} else {
		progress.setValue(numOfFiles);
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

// void EditPhovPlugin::deleteModel() {
// 	QNetworkAccessManager nm(this);
// 	QEventLoop eventLoop;
	
// 	QUrl url(apiUrlDelete.arg(phovID));
// 	QByteArray postData;
	
// 	QNetworkRequest request(url);    
// 	request.setHeader(QNetworkRequest::ContentTypeHeader, 
// 					  "application/x-www-form-urlencoded");
	
// 	QObject::connect(&nm, SIGNAL(finished(QNetworkReply*)),
// 					 &eventLoop, SLOT(quit()));
// 	QNetworkReply* reply = nm.post(request, postData);
// 	eventLoop.exec(); 
// 	if (reply->error() == QNetworkReply::NoError) {
// 		qDebug() << "Success";
//     }
//     else {
//         //failure
//         qDebug() << "Failure" <<reply->errorString();
//     }
// 	delete reply;
// 	qDebug() << "delete end";
// }
