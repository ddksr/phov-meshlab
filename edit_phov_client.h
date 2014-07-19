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

#ifndef EDIT_PHOV_CLIENT_H
#define EDIT_PHOV_CLIENT_H

#include <QString>
#include <QUrl>
#include <Qt>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QUrl>
#include <QNetworkReply>
 

using namespace std;

class EditPhovClient {
public:
	EditPhovClient(QObject &qObj);
	virtual ~EditPhovClient() {}

	QString getPhovID();
	void finishedSlotGetPhovID(QNetworkReply* reply);
	void downloadFile(const QString &filename, QString &phovId) {}
	void uploadFile(const QString &filename, QString &phovId);

private:
	QString apiUrl;

	QNetworkAccessManager *am;
	QNetworkRequest request;
	QString *resultGetPhovID;
};

#endif
