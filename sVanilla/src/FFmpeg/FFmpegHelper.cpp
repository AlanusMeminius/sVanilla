#include <chrono>
#include <future>
#include <thread>
#include <QStandardPaths>
#include <QApplication>
#include <QDebug>
#include <QProcess>
#include <QString>

#include "FFmpegHelper.h"
#include "Aria2Net/AriaLog.h"

namespace ffmpeg
{
constexpr char ffmpegCommand[] = "-i \"%1\" -i \"%2\" -acodec copy -vcodec copy -f mp4 \"%3\"";

FFmpegHelper::FFmpegHelper()
{
}

FFmpegHelper::~FFmpegHelper()
{
    closeFFmpeg();
}

void FFmpegHelper::megerVideo(const MergeInfo& mergeInfo)
{
    FFmpegHelper::megerVideo(
        mergeInfo, [] {}, [] {});
}

void FFmpegHelper::megerVideo(const MergeInfo& mergeInfo, std::function<void()> errorFunc, std::function<void()> finishedFunc)
{
    FFmpegHelper::globalInstance().startFFpmegAsync(mergeInfo, errorFunc, finishedFunc);
}

FFmpegHelper& FFmpegHelper::globalInstance()
{
    static FFmpegHelper ffmpegHelper;
    return ffmpegHelper;
}

void FFmpegHelper::startFFpmegAsync(const MergeInfo& mergeInfo, std::function<void()> errorFunc, std::function<void()> finishedFunc)
{
    // 检测上一次已经完成的合并移除
    m_futures.remove_if([](const std::future<bool>& future) { return future.wait_for(std::chrono::seconds(0)) == std::future_status::ready; });

    // 启动新的合并
    std::future<bool> result = std::async(std::launch::async, [mergeInfo, errorFunc = std::move(errorFunc), finishedFunc = std::move(finishedFunc)]() -> bool {
        QString executablePath = QApplication::applicationDirPath();
        QString ffmpegExecutable = QStandardPaths::findExecutable("ffmpeg", QStringList() << executablePath);
        QString ffmpegArg(ffmpegCommand);
        ffmpegArg = ffmpegArg.arg(mergeInfo.audio.c_str()).arg(mergeInfo.video.c_str()).arg(mergeInfo.targetVideo.c_str());
        qDebug() << ffmpegArg;

        QProcess ffmpegProcess;
        ffmpegProcess.setProgram(ffmpegExecutable);
        ffmpegProcess.setArguments(ffmpegArg.split("-"));
        ffmpegProcess.start();
        ffmpegProcess.waitForFinished(-1);

        if (ffmpegProcess.exitStatus() == QProcess::NormalExit && ffmpegProcess.exitCode() == 0)
        {
            // ffmpeg正常结束且返回值为0，表示执行成功
            if (finishedFunc)
            {
                finishedFunc();
            }
            return true;
        }
        else
        {
            // 否则，表示ffmpeg执行失败
            std::string errorString = ffmpegProcess.errorString().toStdString();
            int exitCode = ffmpegProcess.exitCode();
            ARIA_LOG_ERROR("{}:Error starting ffmpeg process: {}", exitCode, errorString);
            if (errorFunc)
            {
                errorFunc();
            }
            return false;
        }

        return true;
    });
    m_futures.push_back(std::move(result));
}

void FFmpegHelper::closeFFmpeg()
{
    for (auto& future : m_futures)
    {
        future.wait();
    }
    m_futures.clear();
}

}  // namespace ffmpeg
