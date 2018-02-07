/*
 * FileDownloader.cpp
 * Copyright (C) 2017-2018  Belledonne Communications, Grenoble, France
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *  Created on: February 6, 2018
 *      Author: Danmei Chen
 */

#include "FileDownloader.hpp"

// =============================================================================

namespace {
  constexpr qint64 cMaxBytes = 10000000;
}

FileDownloader::FileDownloader () {
  connect(&manager, &QNetworkAccessManager::finished, this, &FileDownloader::handleDownloadFinished);
}

void FileDownloader::download () {
  QNetworkRequest request(mUrl);
  QNetworkReply *reply = manager.get(request);

#if QT_CONFIG(ssl)
  connect(reply, &QNetworkReply::sslErrors, this, &FileDownloader::handleSslErrors);
#endif
  connect(reply, &QNetworkReply::downloadProgress, this, &FileDownloader::handleUpdateDownloadProgress);
  emit downloadingChanged(true);
  currentDownloads.append(reply);
}

QString FileDownloader::saveFileName (const QUrl &url) {
	QString basename = QFileInfo(url.path()).fileName();
	
    if (basename.isEmpty())
        basename = "download";

    if (QFile::exists(basename)) {
        // already exists, don't overwrite
        int i = 0;
		basename += '.';
        while (QFile::exists(basename + QString::number(i)))
            ++i;

        basename += QString::number(i);
    }

    return basename;
}

bool FileDownloader::saveToDisk (const QString &filename, QIODevice *data) {
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << QStringLiteral("Could not open %1 for writing: %2").arg(qPrintable(filename)).arg(qPrintable(file.errorString()));
        return false;
    }

    file.write(data->readAll());

    return true;
}

bool FileDownloader::isHttpRedirect(QNetworkReply *reply) {
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    return statusCode == 301 || statusCode == 302 || statusCode == 303
           || statusCode == 305 || statusCode == 307 || statusCode == 308;
}

void FileDownloader::handleDownloadFinished (QNetworkReply *reply) {
    QUrl url = reply->url();
	bool failed = false;
    if (reply->error()) {
        qWarning() << QStringLiteral("Download of %1 failed: %2").arg(url.toEncoded().constData()).arg(qPrintable(reply->errorString()));
		failed = true;
    } else {
		// TODO: Deal with redirection.
        if (isHttpRedirect(reply)) {
            qWarning() << QStringLiteral("Request was redirected.");
			failed = true;
        } else {
            QString filename = saveFileName(url);
            if (saveToDisk(filename, reply)) {
                qInfo() << QStringLiteral("Download of %1 succeeded (saved to %2)").arg(url.toEncoded().constData()).arg(qPrintable(filename));
            }
        }
    }

    currentDownloads.removeAll(reply);
    reply->deleteLater();
	
	//download finished
	emit downloadingChanged(false);
	if(failed) emit downloadFailed();
	else emit downloadFinished();
}

void FileDownloader::handleSslErrors (const QList<QSslError> &sslErrors) {
#if QT_CONFIG(ssl)
    for (const QSslError &error : sslErrors)
       qWarning() << QStringLiteral("SSL error: %1").arg(qPrintable(error.errorString()));
#else
    Q_UNUSED(sslErrors);
#endif
}

void FileDownloader::handleUpdateDownloadProgress (qint64 readBytes,qint64 totalBytes) {
  emit readBytesChanged(readBytes);
  if(totalBytes == -1) emit totalBytesChanged(cMaxBytes); 
  else emit totalBytesChanged(totalBytes);
}
// -----------------------------------------------------------------------------

QUrl FileDownloader::getUrl () const {
  return mUrl;
}

void FileDownloader::setUrl (const QUrl &url) {
  if (mDownloading) {
    qWarning() << QStringLiteral("Unable to set url, a file is downloading.");
    return;
  } 
  
  if (mUrl != url) { 
    mUrl = url;
    emit urlChanged(mUrl);
  }
}

QString FileDownloader::getOutputPath () const {
  return mOutputPath;
}

void FileDownloader::setOutputPath (const QString &outputPath) {
  if (mDownloading) {
    qWarning() << QStringLiteral("Unable to set output path, a file is downloading.");
    return;
  } 
  
  if (mOutputPath != outputPath) {
    mOutputPath = outputPath;
    emit outputPathChanged(mOutputPath);
  }
}

qint64 FileDownloader::getReadBytes () const {
  return mReadBytes;
}

qint64 FileDownloader::getTotalBytes () const {
  return mTotalBytes;
}

bool FileDownloader::getDownloading () const {
  return mDownloading;
}
