#include <QProcess>
#include <QDir>
#include <QPushButton>
#include <QMouseEvent>
#include <QTimer>
#include <QMenu>
#include <QFileInfo>
#include <QDir>
#include <QScrollBar>

#include <utility>
#include <limits>

#include <BaseVideoView.h>

#include "UiDownloader.h"
#include "DownloadingListWidget.h"
#include "ui_DownloadingListWidget.h"
#include "DownloadingInfoWidget.h"
#include "Storage/DownloadingItemStorage.h"
#include "Utils/InfoPanelVisibleHelper.h"
#include "BaseQt/Utility.h"
#include "MainWindow/SApplication.h"
#include "Utils/SpeedUtil.h"
#include "Util/TimerUtil.h"
#include "ClientLog.h"
#include "const_string.h"
#include "Storage/StorageManager.h"

DownloadingItemWidget::DownloadingItemWidget(std::shared_ptr<UiDownloader> downloader, QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::DownloadingItemWidget)
    , m_downloader(std::move(downloader))
{
    ui->setupUi(this);
    setUi();
    signalsAndSlots();
    m_status.Status = download::AbstractDownloader::Ready;
}

DownloadingItemWidget::~DownloadingItemWidget()
{
    delete ui;
}

void DownloadingItemWidget::setListWidget(DownloadingListWidget* listWidget, QListWidgetItem* widgetItem)
{
    m_listWidget = listWidget;
    m_listWidgetItem = widgetItem;
}

DownloadingListWidget* DownloadingItemWidget::listWidget() const
{
    return m_listWidget;
}

std::shared_ptr<UiDownloader> DownloadingItemWidget::downloaoder() const
{
    return m_downloader;
}

void DownloadingItemWidget::setStart()
{
    const auto status = m_downloader->status();
    MLogI(svanilla::cDownloadModule, "DownloadingItemWidget setStart, status: {} | fileName: {} | guid: {}", static_cast<int>(status),
          m_downloader->videoInfoFull()->fileName(), m_downloader->videoInfoFull()->getGuid());
    if (status == download::AbstractDownloader::Waitting)
    {
        m_downloader->setStatus(download::AbstractDownloader::Ready);
    }
    else if (status == download::AbstractDownloader::Paused)
    {
        m_downloader->setStatus(download::AbstractDownloader::Resumed);
        ui->btnPause->setChecked(false);
    }
}

void DownloadingItemWidget::setPause()
{
    const auto status = m_downloader->status();
    MLogI(svanilla::cDownloadModule, "DownloadingItemWidget setPause, status: {} | fileName: {} | guid: {}", static_cast<int>(status),
          m_downloader->videoInfoFull()->fileName(), m_downloader->videoInfoFull()->getGuid());
    if (status == download::AbstractDownloader::Downloading)
    {
        m_downloader->setStatus(download::AbstractDownloader::Paused);
        ui->btnPause->setChecked(true);
    }
    else if (status == download::AbstractDownloader::Ready)
    {
        m_downloader->setStatus(download::AbstractDownloader::Waitting);
    }
}

void DownloadingItemWidget::setDelete()
{
    MLogI(svanilla::cDownloadModule, "DownloadingItemWidget setDelete,  fileName: {} | guid: {}", m_downloader->videoInfoFull()->fileName(),
          m_downloader->videoInfoFull()->getGuid());
    deleteItem();
}

DonwloadingStatus DownloadingItemWidget::status() const
{
    return m_status;
}

void DownloadingItemWidget::contextMenuEvent(QContextMenuEvent* event)
{
    if (m_contextMenu == nullptr)
    {
        m_contextMenu = new QMenu(this);
        createContextMenu();
    }
    m_contextMenu->popup(event->globalPos());
}

void DownloadingItemWidget::setUi()
{
    QFileInfo filepath(QString::fromStdString(m_downloader->filename()));
    m_status.fileName = filepath.fileName();
    m_status.folderPath = filepath.absoluteDir().absolutePath();
    m_status.uri = QString::fromStdString(m_downloader->uri());
}

void DownloadingItemWidget::signalsAndSlots()
{
    connect(ui->btnDelete, &QPushButton::clicked, this, &DownloadingItemWidget::deleteItem);
    connect(ui->btnPause, &QPushButton::clicked, this, &DownloadingItemWidget::pauseItem);
    connect(ui->btnFolder, &QPushButton::clicked, this, [&]() {
        if (ui->btnFolder->isCheckable())
        {
            restartItem();
        }
        else
        {
            openItemFolder();
        }
    });
    connect(ui->btnDetail, &QPushButton::clicked, this, &DownloadingItemWidget::showInfoPanel);

    connect(m_downloader.get(), &UiDownloader::finished, this, &DownloadingItemWidget::finishedItem);
    connect(m_downloader.get(), &UiDownloader::update, this, &DownloadingItemWidget::updateDownloadingItem);

    connect(m_downloader.get(), &UiDownloader::statusChanged, this, &DownloadingItemWidget::updateStatusIcon);
}

void DownloadingItemWidget::deleteItem()
{
    MLogI(svanilla::cDownloadModule, "DownloadingItemWidget deleteItem, fileName: {} | guid: {}", m_downloader->videoInfoFull()->fileName(),
          m_downloader->videoInfoFull()->getGuid());
    m_downloader->setStatus(download::AbstractDownloader::Stopped);
    if (m_listWidget == nullptr)
    {
        return;
    }

    if (const auto* item = m_listWidget->itemFromWidget(this))
    {
        delete item;
        deleteLater();
    }
    m_listWidget->updateDowningCount();
    m_listWidget->hideInfoPanel();
}

void DownloadingItemWidget::pauseItem(bool isResume)
{
    MLogI(svanilla::cDownloadModule, "DownloadingItemWidget pauseItem{}, fileName: {} | guid: {}", isResume, m_downloader->videoInfoFull()->fileName(),
          m_downloader->videoInfoFull()->getGuid());
    auto status = m_downloader->status();
    auto setStatus = [&](auto newStatus) {
        if (status != newStatus)
        {
            m_downloader->setStatus(newStatus);
            status = newStatus;
        }
    };
    const bool isDownloading = status == download::AbstractDownloader::Downloading;
    const bool isPaused = (status == download::AbstractDownloader::Paused || status == download::AbstractDownloader::Pause);
    const bool isWaitting = status == download::AbstractDownloader::Waitting;
    const bool isResumed = status == download::AbstractDownloader::Resumed;
    if (!isResume && (isPaused || isWaitting))
    {
        setStatus(download::AbstractDownloader::Resumed);
    }
    else if (isResume && (isDownloading || isResumed))
    {
        setStatus(download::AbstractDownloader::Pause);
    }
    else
    {
        ui->btnPause->setChecked(!isResume);
    }
}

void DownloadingItemWidget::openItemFolder()
{
    QString filePath = QString::fromStdString(m_downloader->filename());
    if (std::filesystem::u8path(m_downloader->filename()).is_relative())
    {
        filePath = SApplication::appDir() + "/" + filePath;
    }

    MLogI(svanilla::cDownloadModule, "showInFileExplorer fileName: {}", filePath.toStdString());
    QFileInfo fileInfo(filePath);
    if (fileInfo.exists())
    {
        util::showInFileExplorer(filePath);
    }
    else
    {
        util::showInFileExplorer(fileInfo.absoluteDir().absolutePath());
    }
}

void DownloadingItemWidget::updateDownloadingItem(const download::DownloadInfo& info)
{
    m_status.stage = QString::fromStdString(info.stage);
    m_status.speed = formatSpeed(info.speed);
    m_status.totalSize = formatSize(info.total);
    m_status.downloadedSize = formatSize(info.complete);
    if (info.total != 0)
    {
        const double progress = static_cast<double>(info.complete) / static_cast<double>(info.total);
        m_status.progress = progress * 100.;
        long long remainingTime = 0;
        if (info.speed == 0)
        {
            remainingTime = (progress == 1) ? 0 : std::numeric_limits<decltype(remainingTime)>::max();
        }
        else
        {
            remainingTime = (info.total - info.complete) / info.speed;
        }
        m_status.remainingTime = QString::fromStdString(formatDuration(static_cast<int>(remainingTime)));
    }

    updateItemText();

    if (m_listWidget != nullptr)
    {
        emit m_listWidget->updateInfoPanel(this);
    }
}

void DownloadingItemWidget::finishedItem()
{
    MLogI(svanilla::cDownloadModule, "DownloadingItemWidget finished! fileName: {} | guid: {} ", m_downloader->videoInfoFull()->fileName(),
          m_downloader->videoInfoFull()->getGuid());
    constexpr auto maxProgress = 100;
    ui->progressBar->setValue(maxProgress);
    ui->labelStatus->setText("finished");

    if (const auto* item = m_listWidget->itemFromWidget(this))
    {
        emit m_listWidget->finished(m_downloader->videoInfoFull());
        delete item;
        deleteLater();
    }
    emit m_listWidget->updateDowningCount();
}

void DownloadingItemWidget::updateStatusIcon(download::AbstractDownloader::Status status)
{
    if (m_status.Status == status)
    {
        return;
    }

    m_status.Status = status;
    switch (status)
    {
    case download::AbstractDownloader::Downloading:
    {
        ui->btnStatus->setIcon(QIcon(":icon/downloading.svg"));
        break;
    }
    case download::AbstractDownloader::Paused:
    {
        ui->btnStatus->setIcon(QIcon(":icon/pause.svg"));
        break;
    }
    case download::AbstractDownloader::Waitting:
    {
        ui->btnStatus->setIcon(QIcon(":icon/waiting.svg"));
        break;
    }
    case download::AbstractDownloader::Finished:
    {
        ui->btnStatus->setIcon(QIcon(":icon/completed.svg"));
        break;
    }
    case download::AbstractDownloader::Error:
    {
        ui->btnStatus->setIcon(QIcon(":icon/error.svg"));
        ui->btnStatus->setProperty("CustomIconColor", QColor(249, 84, 84));
        break;
    }
    default:
    {
        ui->btnStatus->setIcon(QIcon(":icon/downloading.svg"));
    }
    }

    if (download::AbstractDownloader::Error == status)
    {
        ui->btnFolder->setIcon(QIcon(":/icon/restart.svg"));
        ui->btnFolder->setToolTip(tr("restart"));
        ui->btnFolder->setCheckable(true);
    }
    else
    {
        ui->btnFolder->setIcon(QIcon(":/icon/folder.svg"));
        ui->btnFolder->setToolTip(tr("open floder in files explorer"));
        ui->btnFolder->setCheckable(false);
    }
}

void DownloadingItemWidget::updateItemText()
{
    ui->labelTitle->setText(m_status.fileName);
    ui->labelSpeed->setText(m_status.speed);
    ui->labelSize->setText(m_status.totalSize);
    ui->labelStatus->setText(m_status.stage);
    if (ui->progressBar->value() < m_status.progress)
    {
        ui->progressBar->setValue(static_cast<int>(m_status.progress));
    }
    ui->labelRemainingTime->setText(m_status.remainingTime);
}

void DownloadingItemWidget::showInfoPanel() const
{
    if (m_listWidget == nullptr)
    {
        return;
    }
    m_listWidget->showInfoPanel(m_listWidget->row(m_listWidgetItem));
}

void DownloadingItemWidget::createContextMenu()
{
    auto* downloadAction = new QAction(tr("Start Download"), this);
    m_contextMenu->addAction(downloadAction);
    connect(downloadAction, &QAction::triggered, this, [this]() {
        pauseItem(false);
    });

    auto* pauseAction = new QAction(tr("Pause Download"), this);
    m_contextMenu->addAction(pauseAction);
    connect(pauseAction, &QAction::triggered, this, [this]() {
        pauseItem(true);
    });
    if (m_status.Status == download::AbstractDownloader::Downloading)
    {
        downloadAction->setDisabled(true);
        pauseAction->setDisabled(false);
    }
    else
    {
        downloadAction->setDisabled(false);
        pauseAction->setDisabled(true);
    }

    auto* deleteAction = new QAction(tr("Delete"), this);
    m_contextMenu->addAction(deleteAction);
    connect(deleteAction, &QAction::triggered, this, &DownloadingItemWidget::deleteItem);

    m_contextMenu->addSeparator();

    auto* openFolderAction = new QAction(tr("Open Folder"), this);
    m_contextMenu->addAction(openFolderAction);
    connect(openFolderAction, &QAction::triggered, this, &DownloadingItemWidget::openItemFolder);

    auto* infoAction = new QAction(tr("Show Infomation"), this);
    m_contextMenu->addAction(infoAction);
    connect(infoAction, &QAction::triggered, this, &DownloadingItemWidget::showInfoPanel);
}

void DownloadingItemWidget::restartItem()
{
    auto& storageManager = sqlite::StorageManager::intance();
    bool isDownloaded = storageManager.isDownloaded(m_downloader->videoInfoFull()->getGuid());
    if (isDownloaded)
    {
        // to do
    }

    deleteItem();

    MLogI(svanilla::cDownloadModule, "downloadItem restart! name: {}, guid: {}", m_downloader->videoInfoFull()->videoView->Title,
          m_downloader->videoInfoFull()->getGuid());
    emit m_listWidget->reloadItem(m_downloader->videoInfoFull());

    if (const auto* item = m_listWidget->itemFromWidget(this))
    {
        delete item;
        deleteLater();
        return;
    }
}

DownloadingListWidget::DownloadingListWidget(QWidget* parent)
    : QListWidget(parent)
{
    setUi();
    signalsAndSlots();
    m_splitter = qobject_cast<QSplitter*>(parent);
}

void DownloadingListWidget::addDownloadItem(const std::shared_ptr<UiDownloader>& downloader)
{
    MLogI(svanilla::cDownloadModule, "DownloadingItemWidget finished! fileName: {} | guid: {} ", downloader->videoInfoFull()->fileName(),
          downloader->videoInfoFull()->getGuid());
    auto* pWidget = new DownloadingItemWidget(downloader, this);
    auto* pItem = new QListWidgetItem(this);
    pWidget->setListWidget(this, pItem);
    pItem->setSizeHint(pWidget->sizeHint());
    setItemWidget(pItem, pWidget);
    updateDowningCount();
}

void DownloadingListWidget::startAll()
{
    MLogI(svanilla::cDownloadModule, "DownloadingItemWidget startAll!");
    const int nCount = count();
    for (int i = 0; i < nCount; ++i)
    {
        if (indexOfItem(i) != nullptr)
        {
            indexOfItem(i)->setStart();
        }
    }
}

void DownloadingListWidget::pauseAll()
{
    MLogI(svanilla::cDownloadModule, "DownloadingItemWidget pauseAll!");
    const int nCount = count();
    for (int i = 0; i < nCount; ++i)
    {
        if (indexOfItem(i) != nullptr)
        {
            indexOfItem(i)->setPause();
        }
    }
}

void DownloadingListWidget::deleteAll()
{
    MLogI(svanilla::cDownloadModule, "DownloadingItemWidget deleteAll!");
    const int nCount = count();
    for (int i = 0; i < nCount; ++i)
    {
        if (indexOfItem(0) != nullptr)
        {
            indexOfItem(0)->setDelete();
        }
    }
    updateDowningCount();
}

void DownloadingListWidget::setInfoPanelSignal(DownloadingInfoWidget* infoWidget)
{
    m_infoWidget = infoWidget;
}

QListWidgetItem* DownloadingListWidget::itemFromWidget(DownloadingItemWidget* target)
{
    for (int i = 0; i < count(); ++i)
    {
        if (itemWidget(item(i)) == target)
        {
            return item(i);
        }
    }

    return nullptr;
}

void DownloadingListWidget::startSelectedItem()
{
    MLogI(svanilla::cDownloadModule, "DownloadingItemWidget startSelectedItem!");
    for (auto* widgetItem : selectedItems())
    {
        auto* const downloadingItemWidget = qobject_cast<DownloadingItemWidget*>(itemWidget(widgetItem));
        if (downloadingItemWidget != nullptr)
        {
            downloadingItemWidget->setStart();
        }
    }
}

void DownloadingListWidget::deleteSelectedItem()
{
    MLogI(svanilla::cDownloadModule, "DownloadingItemWidget deleteSelectedItem!");
    for (auto* widgetItem : selectedItems())
    {
        auto* const downloadingItemWidget = qobject_cast<DownloadingItemWidget*>(itemWidget(widgetItem));
        if (downloadingItemWidget != nullptr)
        {
            downloadingItemWidget->setPause();
            const auto row = indexFromItem(widgetItem).row();
            auto* const item = takeItem(row);
            removeItemWidget(item);
            delete item;
        }
    }
    updateDowningCount();
}

void DownloadingListWidget::mouseMoveEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        const QPoint point = event->pos();
        if (auto* const itemClicked = this->itemAt(point); itemClicked == nullptr)
        {
            this->clearSelection();
        }
        else
        {
            itemClicked->setSelected(!itemClicked->isSelected());
            return;
        }
    }
    QListWidget::mouseMoveEvent(event);
}

DownloadingItemWidget* DownloadingListWidget::indexOfItem(int row) const
{
    return qobject_cast<DownloadingItemWidget*>(itemWidget(item(row)));
}

void DownloadingListWidget::setUi()
{
    setBackgroundRole(QPalette::NoRole);
    setSelectionMode(ExtendedSelection);
    verticalScrollBar()->setSingleStep(1);
    verticalScrollBar()->setPageStep(5);
}

void DownloadingListWidget::signalsAndSlots() const
{
}

void DownloadingListWidget::updateDowningCount()
{
    int downloading = 0;
    int downloadError = 0;

    for (int i = 0; i < count(); ++i)
    {
        if (auto pWidget = qobject_cast<DownloadingItemWidget*>(itemWidget(item(i))))
        {
            pWidget->downloaoder()->status() == download::AbstractDownloader::Error ? downloadError++ : downloading++;
        }
    }

    emit downloadingCountChanged(downloading, downloadError);
}

void DownloadingListWidget::showInfoPanel(int index)
{
    setInfoPanelVisible(m_infoWidget, m_splitter, index, previousRow);
    updateInfoPanel(indexOfItem(index));
    scrollToItem(item(index));
}

void DownloadingListWidget::hideInfoPanel() const
{
    m_infoWidget->hide();
}

void DownloadingListWidget::updateInfoPanel(const DownloadingItemWidget* item) const
{
    if (m_infoWidget->isVisible())
    {
        m_infoWidget->updateInfoPanel(item);
    }
}
