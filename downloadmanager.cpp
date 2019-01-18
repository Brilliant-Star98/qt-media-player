#include "downloadmanager.h"


DownloadManager::DownloadManager(QObject *parent)
    : QObject(parent), downloadedCount(0), totalCount(0)
{
}

void DownloadManager::append(const QStringList &urlList)
{
    foreach (QString url, urlList)
        append(QUrl::fromEncoded(url.toLocal8Bit()));

    if (downloadQueue.isEmpty())
        QTimer::singleShot(0, this, SIGNAL(finished()));
}

void DownloadManager::append(const QUrl &url)
{
    if (downloadQueue.isEmpty())
        QTimer::singleShot(0, this, SLOT(startNextDownload()));

    downloadQueue.enqueue(url);
    ++totalCount;
}

void DownloadManager::clearHistory()
{
    downloadQueue.clear();
    totalCount = 0;
    downloadedCount = 0;
}

QString DownloadManager::saveFileName(const QUrl &url)
{
    QString path = url.path();
    QString basename = QFileInfo(path).fileName();

    if (basename.isEmpty())
        basename = "download";

    basename = "media/" + basename;

    if (QFile::exists(basename)) {
        /*// already exists, don't overwrite
        int i = 0;
        basename += '.';
        while (QFile::exists(basename + QString::number(i)))
            ++i;

        basename += QString::number(i);
        */

        return "--ALREADY--";
    }

    return basename;
}

void DownloadManager::startNextDownload()
{
    if (downloadQueue.isEmpty()) {
        qDebug() << "Files downloaded successfully :" << downloadedCount << "/" << totalCount;
        emit finished();
        return;
    }

    QUrl url = downloadQueue.dequeue();

    QString filename = saveFileName(url);

    if(!filename.compare("--ALREADY--"))
    {
        qDebug() << "Already exist.";

        startNextDownload();
        return;                 // skip this download

    }

    output.setFileName(filename);
    if (!output.open(QIODevice::WriteOnly)) {
        qDebug() << "Problem opening save file.";

        startNextDownload();
        return;                 // skip this download
    }

    QNetworkRequest request(url);
    currentDownload = manager.get(request);

    connect(currentDownload, SIGNAL(finished()),
            SLOT(downloadFinished()));
    connect(currentDownload, SIGNAL(readyRead()),
            SLOT(downloadReadyRead()));

    // prepare the output
    qDebug() << "Downloading ..." << url.toEncoded().constData();
    downloadTime.start();
}

void DownloadManager::downloadFinished()
{

    output.close();

    if (currentDownload->error()) {
        // download failed
        qDebug() << "Failed!";
    } else {
        qDebug() << "Succeeded!";
        ++downloadedCount;
    }

    currentDownload->deleteLater();
    startNextDownload();
}

void DownloadManager::downloadReadyRead()
{
    output.write(currentDownload->readAll());
}
