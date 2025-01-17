#pragma once

#include <QListWidget>
#include <QSplitter>

#include "DownloadedInfoWidget.h"

namespace Ui
{
class DownloadedItemWidget;
}  // namespace Ui

struct VideoInfoFull;
class DownloadedListWidget;
class DownloadedItemWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DownloadedItemWidget(std::shared_ptr<VideoInfoFull> videoInfoFull, QWidget* parent = nullptr);
    ~DownloadedItemWidget();

    void setListWidget(DownloadedListWidget* listWidget, QListWidgetItem* widgetItem);
    DownloadedListWidget* listWidget() const;
    std::shared_ptr<VideoInfoFull> videoInfoFull() const;

    void clearItem();
    void reloadItem();
    void updateStatus();

private:
    void setUi();
    void signalsAndSlots();
    void deleteItem();
    void restartItem();
    void openItemFolder();
    void deleteDbFinishItem();

    void showInfoPanel() const;

private:
    Ui::DownloadedItemWidget* ui;
    DownloadedListWidget* m_listWidget = nullptr;
    QListWidgetItem* m_listWidgetItem = nullptr;

    std::shared_ptr<VideoInfoFull> m_videoInfoFull;
};

class DownloadedListWidget : public QListWidget
{
    Q_OBJECT

public:
    explicit DownloadedListWidget(QWidget* parent = nullptr);

    void addDownloadedItem(const std::shared_ptr<VideoInfoFull>& videoInfoFull);

    void clearAll();
    void reloadAll();
    void scan();

    QListWidgetItem* itemFromWidget(QWidget* target) const;
    void setInfoPanelSignal(DownloadedInfoWidget* infoWidget);
    void showInfoPanel(const std::shared_ptr<VideoInfoFull>& videoInfo, int index);
    void hideInfoPanel() const;

signals:
    void reloadItem(std::shared_ptr<VideoInfoFull> videoInfoFull);
    void downloadedCountChanged(int count);

protected:
    void mouseMoveEvent(QMouseEvent* event) override;

private:
    [[nodiscard]] DownloadedItemWidget* indexOfItem(int row) const;
    void signalsAndSlots() const;

private:
    QSplitter* m_splitter = nullptr;
    DownloadedInfoWidget* m_infoWidget = nullptr;
    int previousRow = -1;
};
