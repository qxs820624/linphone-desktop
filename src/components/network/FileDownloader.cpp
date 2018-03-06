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

#include "../core/CoreManager.hpp"

#include "FileDownloader.hpp"

// =============================================================================

namespace {
  constexpr char cDefaultFileName[] = "download";
}

static QString getDownloadFilePath (const QString &folder, const QUrl &url) {
  QFileInfo fileInfo(url.path());
  QString fileName = fileInfo.fileName();
  if (fileName.isEmpty())
    fileName = cDefaultFileName;

  fileName.prepend(folder);
  if (!QFile::exists(fileName))
    return fileName;

  // Already exists, don't overwrite.
  QString baseName = fileInfo.baseName();
  if (baseName.isEmpty())
    baseName = cDefaultFileName;

  QString suffix = fileInfo.completeSuffix();
  if (!suffix.isEmpty())
    suffix.prepend(".");

  for (int i = 1; true; ++i) {
    fileName = folder + baseName + "(" + QString::number(i) + ")" + suffix;
    if (!QFile::exists(fileName))
      break;
  }
  return fileName;
}

static bool isHttpRedirect (QNetworkReply *reply) {
  Q_CHECK_PTR(reply);
  int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
  return statusCode == 301 || statusCode == 302 || statusCode == 303
    || statusCode == 305 || statusCode == 307 || statusCode == 308;
}

// -----------------------------------------------------------------------------

void FileDownloader::download () {
  if (mDownloading) {
    qWarning() << "Unable to download file. Already downloading!";
    return;
  }
  setDownloading(true);

  QNetworkRequest request(mUrl);
  mNetworkReply = mManager.get(request);
  if (mDownloadFolder.isEmpty()) {
    mDownloadFolder = CoreManager::getInstance()->getSettingsModel()->getDownloadFolder();
    emit downloadFolderChanged(mDownloadFolder);
  }

  Q_ASSERT(!mDestinationFile.isOpen());
  mDestinationFile.setFileName(getDownloadFilePath(QDir::cleanPath(mDownloadFolder) + QDir::separator(), mUrl));

  if (!mDestinationFile.open(QIODevice::WriteOnly)) {
    qWarning() << QStringLiteral("Could not open %1 for writing: %2")
      .arg(mDestinationFile.fileName()).arg(mDestinationFile.errorString());
    emit downloadFailed();
    return;
  }

  #if QT_CONFIG(ssl)
    connect(mNetworkReply, &QNetworkReply::sslErrors, this, &FileDownloader::handleSslErrors);
  #endif

  connect(mNetworkReply, &QNetworkReply::downloadProgress, this, &FileDownloader::handleDownloadProgress);
  connect(mNetworkReply, &QNetworkReply::readyRead, this, &FileDownloader::handleReadyData);
  connect(mNetworkReply, &QNetworkReply::finished, this, &FileDownloader::handleDownloadFinished);
}

void FileDownloader::handleReadyData () {
  QByteArray data = mNetworkReply->readAll();
  mDestinationFile.write(data);
}

void FileDownloader::handleDownloadFinished() {
  if (mNetworkReply->error() != QNetworkReply::NoError) {
    qWarning() << QStringLiteral("Download of %1 failed: %2")
      .arg(mUrl.toString()).arg(mNetworkReply->errorString());
    emit downloadFailed();
  } else {
    // TODO: Deal with redirection.
    if (isHttpRedirect(mNetworkReply)) {
      qWarning() << QStringLiteral("Request was redirected.");
      emit downloadFailed();
    } else
      emit downloadFinished();
  }

  mDestinationFile.close();
  mNetworkReply->deleteLater();
  setDownloading(false);
}

void FileDownloader::handleSslErrors (const QList<QSslError> &sslErrors) {
  #if QT_CONFIG(ssl)
    for (const QSslError &error : sslErrors)
      qWarning() << QStringLiteral("SSL error: %1").arg(error.errorString());
  #else
    Q_UNUSED(sslErrors);
  #endif
}

void FileDownloader::handleDownloadProgress (qint64 readBytes, qint64 totalBytes) {
  setReadBytes(readBytes);
  setTotalBytes(totalBytes);
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

QString FileDownloader::getDownloadFolder () const {
  return mDownloadFolder;
}

void FileDownloader::setDownloadFolder (const QString &downloadFolder) {
  if (mDownloading) {
    qWarning() << QStringLiteral("Unable to set download folder, a file is downloading.");
    return;
  }

  if (mDownloadFolder != downloadFolder) {
    mDownloadFolder = downloadFolder;
    emit downloadFolderChanged(mDownloadFolder);
  }
}

qint64 FileDownloader::getReadBytes () const {
  return mReadBytes;
}

void FileDownloader::setReadBytes (qint64 readBytes) {
  if (mReadBytes != readBytes) {
    mReadBytes = readBytes;
    emit readBytesChanged(readBytes);
  }
}

qint64 FileDownloader::getTotalBytes () const {
  return mTotalBytes;
}

void FileDownloader::setTotalBytes (qint64 totalBytes) {
  if (mTotalBytes != totalBytes) {
    mTotalBytes = totalBytes;
    emit totalBytesChanged(totalBytes);
  }
}

bool FileDownloader::getDownloading () const {
  return mDownloading;
}

void FileDownloader::setDownloading (bool downloading) {
  if (mDownloading != downloading) {
    mDownloading = downloading;
    emit downloadingChanged(downloading);
  }
}
