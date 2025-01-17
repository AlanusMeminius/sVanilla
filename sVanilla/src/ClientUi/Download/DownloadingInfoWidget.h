
#ifndef DOWNLOADINGINFOWIDGET_H
#define DOWNLOADINGINFOWIDGET_H

#include <QWidget>

#include "VideoList/VideoData.h"
#include "Download/AbstractDownloader.h"

class DownloadingItemWidget;
QT_BEGIN_NAMESPACE
namespace Ui
{
class DownloadingInfoWidget;
}
QT_END_NAMESPACE

struct DownloadingInfo
{
    const download::DownloadInfo& downloadingInfo;
    std::shared_ptr<VideoInfoFull> videoInfoFull;
    const std::string& filePath;
};

class DownloadingInfoWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DownloadingInfoWidget(QWidget* parent = nullptr);
    ~DownloadingInfoWidget() override;

    void updateInfoPanel(const DownloadingItemWidget* itemWidget);

private:
    void signalsAndSlots();

private:
    Ui::DownloadingInfoWidget* ui;
};

#endif  // DOWNLOADINGINFOWIDGET_H
