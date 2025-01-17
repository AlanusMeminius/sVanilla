#pragma once
#include <memory>

#include <QListWidget>
#include <QSplitter>

struct DownloadingInfo;
namespace Ui
{
class DownloadingItemWidget;
}  // namespace Ui

struct VideoInfoFull;
class UiDownloader;
class DownloadingListWidget;
class DownloadingInfoWidget;

struct DonwloadingStatus
{
    QString uri;
    QString stage;
    QString fileName;
    QString folderPath;
    QString speed;
    QString totalSize;
    QString downloadedSize;
    QString remainingTime;
    double progress;
    download::AbstractDownloader::Status Status;
};

class DownloadingItemWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DownloadingItemWidget(std::shared_ptr<UiDownloader> downloader, QWidget* parent = nullptr);
    ~DownloadingItemWidget();

    void setListWidget(DownloadingListWidget* listWidget, QListWidgetItem* widgetItem);
    [[nodiscard]] DownloadingListWidget* listWidget() const;
    [[nodiscard]] std::shared_ptr<UiDownloader> downloaoder() const;

    void setStart();
    void setPause();
    void setDelete();

    [[nodiscard]] DonwloadingStatus status() const;

protected:
    void contextMenuEvent(QContextMenuEvent* event) override;

private:
    void setUi();
    void signalsAndSlots();

    void deleteItem();
    void pauseItem(bool isResume);
    void openItemFolder();
    void updateDownloadingItem(const download::DownloadInfo& info);
    void finishedItem();

    void updateStatusIcon(download::AbstractDownloader::Status status);
    void updateItemText();
    void showInfoPanel() const;

    void createContextMenu();
    void restartItem();

private:
    Ui::DownloadingItemWidget* ui;
    DownloadingListWidget* m_listWidget = nullptr;
    QListWidgetItem* m_listWidgetItem = nullptr;
    std::shared_ptr<UiDownloader> m_downloader;
    DonwloadingStatus m_status;
    QMenu* m_contextMenu = nullptr;
};

class DownloadingListWidget : public QListWidget
{
    Q_OBJECT

public:
    explicit DownloadingListWidget(QWidget* parent = nullptr);

    void addDownloadItem(const std::shared_ptr<UiDownloader>& downloader);

    void startAll();
    void pauseAll();
    void deleteAll();

    void setInfoPanelSignal(DownloadingInfoWidget* infoWidget);

    QListWidgetItem* itemFromWidget(DownloadingItemWidget* target);
    void showInfoPanel(int index);
    void hideInfoPanel() const;
    void updateInfoPanel(const DownloadingItemWidget* item) const;

    void startSelectedItem();
    void deleteSelectedItem();
    void updateDowningCount();

signals:
    void finished(std::shared_ptr<VideoInfoFull> videoInfoFull);
    void reloadItem(std::shared_ptr<VideoInfoFull> videoInfoFull);
    void downloadingCountChanged(int downloading, int downloadError);

protected:
    void mouseMoveEvent(QMouseEvent* event) override;

private:
    [[nodiscard]] DownloadingItemWidget* indexOfItem(int row) const;
    void setUi();
    void signalsAndSlots() const;

private:
    QSplitter* m_splitter = nullptr;
    DownloadingInfoWidget* m_infoWidget = nullptr;
    int previousRow = -1;
};
