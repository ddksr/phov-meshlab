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
/****************************************************************************
  History
$Log: meshedit.cpp,v $
****************************************************************************/

#include "edit_phov_client.h"

using namespace std;

EditPhovClient::EditPhovClient(QObject &qObj) {
	am = new QNetworkAccessManager(&qObj);
	apiUrl = QString("http://localhost/~sigi/phov/phov.php");
}

void EditPhovClient::uploadFile(const QString &filename, QString &phovId) {
	QUrl url(apiUrl + "?method=upload&id=");
	request = QNetworkRequest(url);
}

QString EditPhovClient::getPhovID() {
	if (!resultGetPhovID->isEmpty()) {
return QString(*resultGetPhovID);
	}

	QUrl url(QString(apiUrl + "?method=get_id"));

	QObject::connect(am, SIGNAL(finished(QNetworkReply*)),
					 am->parent(), SLOT(finishedSlotGetPhovID(QNetworkReply*)));

	QNetworkReply* reply = am->get(QNetworkRequest(url));

	QString test("test 123 123 123 ");
	resultGetPhovID = &test;

return QString("");
}

void EditPhovClient::finishedSlotGetPhovID(QNetworkReply* reply) {
	QString test("blah 123");
	resultGetPhovID = &test;
}

