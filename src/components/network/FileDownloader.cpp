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

void FileDownloader::download () {
  QNetworkRequest request(mUrl);
  networkReply = manager.get(request);
  if(mOutputPath != NULL)  destinationFile.setFileName(QFileInfo(mOutputPath).fileName());
  else destinationFile.setFileName(saveFileName(mUrl));
 
  if(!destinationFile.open(QIODevice::WriteOnly)) {
    qWarning() << QStringLiteral("Could not open %1 for writing: %2").arg(qPrintable(destinationFile.fileName())).arg(qPrintable(destinationFile.errorString()));
    emit downloadFailed();
    return;
  }
#if QT_CONFIG(ssl)
  connect(networkReply, &QNetworkReply::sslErrors, this, &FileDownloader::handleSslErrors);
#endif
  connect(networkReply, &QNetworkReply::downloadProgress, this, &FileDownloader::handleUpdateDownloadProgress);
  connect(networkReply, &QNetworkReply::readyRead, this, &FileDownloader::readData);
  connect(networkReply, &QNetworkReply::finished, this, &FileDownloader::finishDownload);
  emit downloadingChanged(true);
  currentDownloads.append(networkReply);
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

bool FileDownloader::isHttpRedirect(QNetworkReply *reply) {
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    return statusCode == 301 || statusCode == 302 || statusCode == 303
           || statusCode == 305 || statusCode == 307 || statusCode == 308;
}

void FileDownloader::readData(){
  QByteArray data= networkReply->readAll();
  destinationFile.write(data);
}

void FileDownloader::finishDownload(){
   if(networkReply->error() != QNetworkReply::NoError){
     //failed download
     qWarning() << QStringLiteral("Download of %1 failed: %2").arg(mUrl.toEncoded().constData()).arg(qPrintable(networkReply->errorString()));
     emit downloadFailed();
   } else {
       // TODO: Deal with redirection.
       if (isHttpRedirect(networkReply)) {
         qWarning() << QStringLiteral("Request was redirected.");
         emit downloadFailed();
       } else {
           //successful download
          QByteArray data= networkReply->readAll();
          destinationFile.write(data);
          destinationFile.close();
          networkReply->deleteLater();
       }
   }
   emit downloadingChanged(false);
   emit downloadFinished();
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
