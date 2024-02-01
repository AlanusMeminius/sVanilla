#include <QApplication>
#include <sstream>
#include <utility>
#include "ThreadPool/ThreadPool.h"
#include "ThreadPool/Task.h"
#include "ClientUi/Config/SingleConfig.h"
#include "Aria2Net/AriaServer/AriaServer.h"
#include "App.h"

#include <QStandardPaths>

void App::init()
{
    setHighDpi();
    downloadManager = std::make_shared<DownloadManager>();

    maimWindow = std::make_unique<MainWindow>();
    maimWindow->show();
    signalsAndSlots();
    startAriaServer();

    // option.dir = SingleConfig::instance().getAriaConfig().downloadDir;
    const QString downloadPath = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    option.dir = downloadPath.toStdString();
}

void App::setHighDpi()
{
#if QT_VERSION < QT_VERSION_CHECK(6, 2, 0)
    qputenv("QT_DPI_ADJUSTMENT_POLICY", "AdjustDpi");
#endif  // Qt < 6.2.0

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling, true);
#endif  // Qt < 6.0.0

    QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
}

void App::startAriaServer()
{
    if (const auto aria2Config = SingleConfig::instance().getAriaConfig(); !aria2Config.isRemote)
    {
        aria2net::AriaServer ariaServer;
        PRINT("start aria2 local server")
        ariaServer.setErrorFunc([] {});
        ariaServer.startServerAsync();
    }
}

void App::signalsAndSlots()
{
    connect(maimWindow.get(), &MainWindow::AddUri, this, &App::addUri);
    connect(maimWindow.get(), &MainWindow::onSettingPage, this, &App::updateAria2Version);
    connect(downloadManager.get(), &DownloadManager::toRuquestStatus, this, &App::updateDownloadStatus);
}

void App::updateAria2Version()
{
    auto taskFunc = [this]() {
        return ariaClient.GetAriaVersionAsync();
    };
    auto task = std::make_shared<TemplateSignalReturnTask<decltype(taskFunc)>>(taskFunc);
    connect(task.get(), &SignalReturnTask::result, this, [this](const std::any& res) {
        try
        {
            auto result = std::any_cast<aria2net::AriaVersion>(res);
            if (!result.id.empty() && result.error.message.empty())
            {
                isConnect = true;
            }
            maimWindow->updateAria2Version(std::make_shared<aria2net::AriaVersion>(result));
        }
        catch (const std::bad_any_cast& e)
        {
            updateHomeMsg(e.what());
        }
    });
    ThreadPool::instance().enqueue(task);
}
void App::updateDownloadStatus(const std::string& gid)
{
    auto taskFunc = [this, gid]() {
        return ariaClient.TellStatus(gid);
    };
    const auto task = std::make_shared<TemplateSignalReturnTask<decltype(taskFunc)>>(taskFunc);
    connect(task.get(), &SignalReturnTask::result, this, [this](const std::any& res) {
        try
        {
            const auto& result = std::any_cast<aria2net::AriaTellStatus>(res);
            const auto status = std::make_shared<aria2net::AriaTellStatus>(result);
            const auto error = downloadManager->updateDownloaderStatus(status);
            if (const auto e = error.message; !e.empty())
            {
                updateHomeMsg(e);
            }
            else
            {
                const auto g = status->result.gid;
                if (downloadManager->neededUpdate.count(g) > 0)
                {
                    maimWindow->updateDownloadStatus(status);
                }
            }
        }
        catch (const std::bad_any_cast& e)
        {
            updateHomeMsg(e.what());
        }
    });
    ThreadPool::instance().enqueue(task);
}

void App::addUri(const std::list<std::string>& uris)
{
    auto taskFunc = [this, uris]() {
        return ariaClient.AddUriAsync(uris, option, -1);
    };
    auto task = std::make_shared<TemplateSignalReturnTask<decltype(taskFunc)>>(taskFunc);
    connect(task.get(), &SignalReturnTask::result, this, [this](const std::any& res) {
        try
        {
            const auto& result = std::any_cast<aria2net::AriaAddUri>(res);
            if (!result.result.empty() && result.error.message.empty())
            {
                auto gid = result.result;
                maimWindow->AddDownloadTask(gid);
                downloadManager->addDownloader(gid);
                updateHomeMsg("Add success");
            }
            else
            {
                updateHomeMsg(result.error.message);
            }
        }
        catch (const std::bad_any_cast& e)
        {
            updateHomeMsg(e.what());
        }
    });
    ThreadPool::instance().enqueue(task);
}

void App::updateHomeMsg(const std::string& msg) const
{
    maimWindow->updateHomeMsg(msg);
}