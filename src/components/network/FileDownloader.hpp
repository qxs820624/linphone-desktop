/*
 * FileDownloader.hpp
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

#include <QObject>
#include <QtNetwork>

// =============================================================================

class QSslError;

using namespace std;

class FileDownloader : public QObject {
  Q_OBJECT;
  
  Q_PROPERTY(QUrl url READ getUrl WRITE setUrl NOTIFY urlChanged);
  Q_PROPERTY(QString outputPath READ getOutputPath WRITE setOutputPath NOTIFY outputPathChanged);
  Q_PROPERTY(qint64 readBytes READ getReadBytes NOTIFY readBytesChanged);
  Q_PROPERTY(qint64 totalBytes READ getTotalBytes NOTIFY totalBytesChanged);
  Q_PROPERTY(bool downloading READ getDownloading NOTIFY downloadingChanged);

public: 
  Q_INVOKABLE void download ();

signals:
  void urlChanged (const QUrl &url);
  void outputPathChanged (const QString &outputPath);
  void readBytesChanged (qint64 readBytes);
  void totalBytesChanged (qint64 totalBytes);
  void downloadingChanged (bool downloading);
  void downloadFinished ();
  void downloadFailed();
 
private:
  QUrl getUrl () const;
  void setUrl (const QUrl &url);
  
  QString getOutputPath () const;
  void setOutputPath (const QString &outputPath);
  
  qint64 getReadBytes () const;
  qint64 getTotalBytes () const;
  bool getDownloading () const;
  
  void handleSslErrors (const QList<QSslError> &errors);
  void handleUpdateDownloadProgress (qint64 readBytes,qint64 totalBytes);

  void readData();
  void finishDownload();
  static QString saveFileName (const QUrl &url);
  static bool isHttpRedirect (QNetworkReply *reply);
  
  QUrl mUrl;
  QString mOutputPath;
  qint64 mReadBytes;
  qint64 mTotalBytes;
  bool mDownloading = false;
  QFile destinationFile;
  QPointer<QNetworkReply> networkReply;
  QNetworkAccessManager manager;
  QVector<QNetworkReply *> currentDownloads;
};


