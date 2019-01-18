#ifndef DOWNLOADMANAGER_H
#define DOWNLOADMANAGER_H

#include <QFile>
#include <QObject>
#include <QQueue>
#include <QTime>
#include <QUrl>
#include <QNetworkAccessManager>
#include <QFileInfo>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QString>
#include <QStringList>
#include <QTimer>
#include <QObject>

class DownloadManager: public QObject
{
    Q_OBJECT
public:
    DownloadManager(QObject *parent = 0);

    void append(const QUrl &url);
    void append(const QStringList &urlList);
    void clearHistory();
    QString saveFileName(const QUrl &url);

signals:
    void finished();

public slots:
    void startNextDownload();
    void downloadFinished();
    void downloadReadyRead();

private:
    QNetworkAccessManager manager;
    QQueue<QUrl> downloadQueue;
    QNetworkReply *currentDownload;
    QFile output;
    QTime downloadTime;

    int downloadedCount;
    int totalCount;
};

#endif // DOWNLOADMANAGER_H
