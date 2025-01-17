#include <QDir>
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>
#include <QPushButton>
#include <QScrollBar>
#include <QMenu>
#include <QShortcut>
#include <QContextMenuEvent>

#include "VideoGridWidget.h"
#include "VideoInfoWidget.h"
#include "ui_VideoGridWidget.h"
#include "ClientUi/Utils/InfoPanelVisibleHelper.h"
#include "SUI/RoundImageWidget.h"
#include "ClientUi/VideoList/VideoData.h"
#include "ClientUi/MainWindow/SApplication.h"
#include "Utils/UrlProcess.h"
#include "ClientLog.h"
#include "const_string.h"

void elideText(QLabel* label, const QString& text)
{
    if (const QFontMetrics fontMetrics(label->font()); fontMetrics.horizontalAdvance(text) > label->width())
    {
        const auto elidedText = fontMetrics.elidedText(text, Qt::ElideRight, label->width());
        label->setText(elidedText);
        label->setToolTip(text);
    }
    else
    {
        label->setText(text);
    }
}

VideoGridItemWidget::VideoGridItemWidget(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::VideoGridItemWidget)
{
    ui->setupUi(this);
    setUi();
    signalsAndSlots();
}

VideoGridItemWidget::~VideoGridItemWidget()
{
    delete ui;
}

void VideoGridItemWidget::saveWidgetItem(QListWidgetItem* widgetItem)
{
    m_listWidgetItem = widgetItem;
}

void VideoGridItemWidget::setVideoInfo(const std::shared_ptr<VideoInfoFull>& infoFull)
{
    m_infoFull = infoFull;
    updateVideoCard();
}

std::shared_ptr<VideoInfoFull> VideoGridItemWidget::getVideoInfo()
{
    return m_infoFull;
}

void VideoGridItemWidget::setUi()
{
    const QPixmap pixmap(":/CoverSpace.svg");
    ui->cover->setPixmap(pixmap);
}

void VideoGridItemWidget::signalsAndSlots()
{
    connect(ui->btnInfo, &QPushButton::clicked, this, &VideoGridItemWidget::showInfoTigger);
    connect(ui->btnDownload, &QPushButton::clicked, this, &VideoGridItemWidget::downloadTrigger);
}

void VideoGridItemWidget::updateCard()
{
    elideText(ui->labelTitle, m_cardInfo.title);
    ui->labelDuration->setText(m_cardInfo.duration);
    elideText(ui->labelAuthor, m_cardInfo.author);
    elideText(ui->labelPublishDate, m_cardInfo.publishDate);
}

void VideoGridItemWidget::createContextMenu()
{
#ifdef _WIN32
    auto* downloadAction = new QAction(QIcon(":icon/download.svg"), tr("Download\tCTRL D"), this);
    auto* infoAction = new QAction(QIcon(":icon/info.svg"), tr("Show Infomation\tCTRL I"), this);
    auto* similarAction = new QAction(tr("Find Similar"), this);
#else
    auto* downloadAction = new QAction(QIcon(":icon/download.svg"), "Download\t⌘ D", this);
    auto* infoAction = new QAction(QIcon(":icon/info.svg"), "Show Infomation\t⌘ I", this);
    auto* similarAction = new QAction("Find Similar", this);
#endif

    connect(downloadAction, &QAction::triggered, this, &VideoGridItemWidget::downloadTrigger);
    m_menu->addAction(downloadAction);

    m_menu->addAction(infoAction);
    connect(infoAction, &QAction::triggered, this, &VideoGridItemWidget::showInfoTigger);

    m_menu->addAction(similarAction);
}

void VideoGridItemWidget::setCover()
{
    const QString tempPath = SApplication::appDir() + QString("/") + QString(coverDir);
    const auto filePath = tempPath + QDir::separator() + QString::fromStdString(util::removeSpecialChar(m_infoFull->coverPath())) + ".jpg";
    if (const QString fullPath = QDir::cleanPath(filePath); QFile::exists(fullPath))
    {
        MLogI(svanilla::cVideoList, "setCover, paath: {}", filePath.toStdString());
        const QPixmap pixmap(fullPath);
        ui->cover->setPixmap(pixmap);
        update();
    }
}

void VideoGridItemWidget::updateVideoCard()
{
    const auto& view = m_infoFull->videoView;
    m_cardInfo = {QString::fromStdString(view->Title), QString::fromStdString(view->Duration), QString::fromStdString(view->Publisher),
                  QString::fromStdString(view->PublishDate)};

    updateCard();
}

void VideoGridItemWidget::updateCover()
{
    setCover();
    if (ui->spinner != nullptr)
    {
        ui->spinner->setVisible(false);
        ui->spinner->deleteLater();
        ui->spinner = nullptr;
    }
}

const VideoGridItemWidget::CardInfo& VideoGridItemWidget::getCardInfo() const
{
    return m_cardInfo;
}

std::string VideoGridItemWidget::getCoverPath() const
{
    return util::removeSpecialChar(m_infoFull->coverPath());
}

QSize VideoGridItemWidget::sizeHint() const
{
    return {itemBaseWidth, itemBaseHeight};
}

void VideoGridItemWidget::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    if (m_infoFull != nullptr)
    {
        if (m_listWidgetItem != nullptr)
        {
            updateCard();
        }
    }
}

void VideoGridItemWidget::contextMenuEvent(QContextMenuEvent* event)
{
    if (m_menu == nullptr)
    {
        m_menu = new QMenu(this);
        createContextMenu();
    }
    m_menu->popup(event->globalPos());
}

VideoGridWidget::VideoGridWidget(QWidget* parent)
    : QListWidget(parent)
{
    setUi();
}

void VideoGridWidget::addVideoItem(const std::shared_ptr<VideoInfoFull>& videoView)
{
    MLogI(svanilla::cVideoList, "addVideoItem {}", videoView->videoView->Title);
    auto* const videoItem = new VideoGridItemWidget(this);
    auto* const item = new VideoListWidgetItem(videoView, count());
    addItem(item);
    videoItem->saveWidgetItem(item);
    item->setSizeHint(calculateItemSize());
    setItemWidget(item, videoItem);
    videoItem->setVideoInfo(videoView);
    connect(videoItem, &VideoGridItemWidget::downloadTrigger, this, [this, item]() {
        downloadItem(item);
    });
    connect(videoItem, &VideoGridItemWidget::showInfoTigger, this, [this, item]() {
        showInfo(item);
    });
}

void VideoGridWidget::clearVideo()
{
    clear();
    update();
}

void VideoGridWidget::setOrderType(OrderType orderType)
{
    int num = count();
    for (int i = 0; i < num; ++i)
    {
        auto widgetItem = dynamic_cast<VideoListWidgetItem*>(item(i));
        widgetItem->setOrderType(orderType);
    }
}

void VideoGridWidget::coverReady(const std::string& fileName) const
{
    auto* const itemWidget = getItem(fileName);
    itemWidget->updateCover();
}

void VideoGridWidget::resizeEvent(QResizeEvent* event)
{
    QListWidget::resizeEvent(event);
    adjustItemSize();
}

void VideoGridWidget::wheelEvent(QWheelEvent* event)
{
    int sensitivity = 3;
    int delta = event->angleDelta().y() / sensitivity;
    verticalScrollBar()->setValue(verticalScrollBar()->value() - delta);
}

void VideoGridWidget::setUi()
{
    setSpacing(itemSpacing);
    setSelectionMode(ExtendedSelection);
    verticalScrollBar()->setSingleStep(1);
    verticalScrollBar()->setPageStep(5);
    setItemShortCuts();
}

void VideoGridWidget::setItemShortCuts()
{
    auto* const downloadShortcut = new QShortcut(this);
    connect(downloadShortcut, &QShortcut::activated, this, [this]() {
        downloadItem(currentItem());
    });
    auto* const infoShortcut = new QShortcut(this);
    connect(infoShortcut, &QShortcut::activated, this, [this]() {
        showInfo(currentItem());
    });

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    downloadShortcut->setKey(QKeySequence(Qt::CTRL | Qt::Key_D));
    infoShortcut->setKey(QKeySequence(Qt::CTRL | Qt::Key_I));
#else
    downloadShortcut->setKey(QKeySequence(Qt::CTRL | Qt::Key_D));
    infoShortcut->setKey(QKeySequence(Qt::CTRL | Qt::Key_I));
#endif
}

void VideoGridWidget::adjustItemSize() const
{
    auto size = calculateItemSize();
    setItemSize(size);
}

void VideoGridWidget::setItemSize(const QSize& size) const
{
    for (int i = 0; i < count(); ++i)
    {
        item(i)->setSizeHint(size);
    }
}

void VideoGridWidget::downloadAllItem()
{
    for (int i = 0; i < count(); ++i)
    {
        downloadItem(item(i));
    }
}

void VideoGridWidget::downloadSelectedItem()
{
    for (const auto& item : selectedItems())
    {
        downloadItem(item);
    }
}

void VideoGridWidget::downloadItem(QListWidgetItem* item)
{
    const auto gridWidget = getItem(item);
    if (gridWidget == nullptr)
    {
        return;
    }
    MLogI(svanilla::cVideoList, "downloadItem {}", gridWidget->getVideoInfo()->getGuid());
    emit downloandBtnClick(gridWidget->getVideoInfo());
}

void VideoGridWidget::showInfo(QListWidgetItem* item)
{
    const auto gridWidget = getItem(item);
    if (gridWidget == nullptr)
    {
        return;
    }
    const auto index = row(item);
    emit infoBtnClick({previousRow, index, gridWidget->getVideoInfo()});
    scrollToItem(item);
    previousRow = index;
}

QSize VideoGridWidget::calculateItemSize() const
{
    const int n = (width() - verticalScrollBar()->width() - 5) / (itemBaseWidth + itemWidthSpacing);
    const int itemWidth = (width() - verticalScrollBar()->width() - 5 - (n - 1) * itemWidthSpacing) / n;
    const int itemHeight = static_cast<int>(static_cast<float>(itemWidth) / aspectRatio);
    return QSize(itemWidth, itemHeight);
}

void VideoGridWidget::updateCovers()
{
    for (int i = 0; i < count(); ++i)
    {
        const auto coverPath = getItem(i)->getCoverPath();
        if (coverPath.empty())
        {
            return;
        }
        coverReady(coverPath);
    }
}

VideoGridItemWidget* VideoGridWidget::getItem(const std::string& fileName) const
{
    for (int i = 0; i < count(); ++i)
    {
        if (const auto item = getItem(i); item->getCoverPath() == fileName)
        {
            return item;
        }
    }
    return nullptr;
}

VideoGridItemWidget* VideoGridWidget::getItem(QListWidgetItem* item) const
{
    if (item == nullptr)
    {
        return nullptr;
    }
    auto* const widgetItem = itemWidget(item);
    return qobject_cast<VideoGridItemWidget*>(widgetItem);
}

VideoGridItemWidget* VideoGridWidget::getItem(const int index) const
{
    return getItem(item(index));
}

std::vector<VideoGridItemWidget*> VideoGridWidget::getItems() const
{
    std::vector<VideoGridItemWidget*> items;
    items.reserve(count());
    for (int i = 0; i < count(); ++i)
    {
        items.emplace_back(getItem(i));
    }
    return items;
}

std::vector<QListWidgetItem*> VideoGridWidget::getWidgetItems() const
{
    std::vector<QListWidgetItem*> items;
    items.reserve(count());
    for (int i = 0; i < count(); ++i)
    {
        items.emplace_back(item(i));
    }
    return items;
}

std::vector<std::shared_ptr<VideoInfoFull>> VideoGridWidget::getVideoInfo() const
{
    std::vector<std::shared_ptr<VideoInfoFull>> infos;
    infos.reserve(count());
    for (int i = 0; i < count(); ++i)
    {
        infos.emplace_back(getItem(i)->getVideoInfo());
    }
    return infos;
}