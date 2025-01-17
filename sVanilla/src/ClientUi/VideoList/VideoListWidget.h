#pragma once

#include <QWidget>
#include <QListWidget>
#include <QSplitter>

#include <BaseVideoView.h>

#include "VideoData.h"

class VideoListWidget;
class VideoInfoWidget;
namespace Ui
{
class VideoListItemWidget;
}
struct VideoInfoFull;

class VideoListItemWidget : public QWidget
{
    Q_OBJECT
public:
    explicit VideoListItemWidget(QWidget* parent = nullptr);
    ~VideoListItemWidget();

    void setListWidget(VideoListWidget* listWidget, QListWidgetItem* widgetItem);
    void setVideoInfo(const std::shared_ptr<VideoInfoFull>& infoFull);
    const std::shared_ptr<VideoInfoFull>& videoInfo() const;

    void updateVideoItem();
    void downloadItem() const;

private:
    void signalsAndSlots();
    void showInfoPanel() const;

    void createContextMenu();

protected:
    void contextMenuEvent(QContextMenuEvent* event) override;

private:
    std::shared_ptr<VideoInfoFull> m_infoFull;
    VideoListWidget* m_listWidget = nullptr;
    QListWidgetItem* m_listWidgetItem = nullptr;
    Ui::VideoListItemWidget* ui;
    QMenu* m_menu = nullptr;
};

class VideoListWidget : public QListWidget
{
    Q_OBJECT
public:
    explicit VideoListWidget(QWidget* parent = nullptr);
    void addVideoItem(const std::shared_ptr<VideoInfoFull>& infoFull);
    void clearVideo();
    void setOrderType(OrderType orderType);

    void setInfoPanelSignalPointer(VideoInfoWidget* infoWidget, QSplitter* splitter);
    void showInfoPanel(int row);
    void updateInfoPanel(const std::shared_ptr<VideoInfoFull>& infoFull) const;

    Q_SIGNAL void downloandBtnClick(const std::shared_ptr<VideoInfoFull>& infoFull);

private:
    void downloadAllItem() const;
    void downloadSelectedItem() const;
    void downloadItem(QListWidgetItem* item) const;

protected:
    void resizeEvent(QResizeEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;

private:
    QSplitter* m_splitter = nullptr;
    VideoInfoWidget* m_infoWidget = nullptr;
    int previousRow = -1;
};
