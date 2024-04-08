#pragma once
#include <string>
#include <list>

#include "AbstractDownloader.h"
#include "AriaDownloader.h"

namespace download
{

struct FinishedItem
{
    std::string uniqueId;
    std::string filePath;
    std::string bvid;
    std::string title;
    int duration;
};

class BiliDownloader : public AbstractDownloader
{
public:
    BiliDownloader();
    BiliDownloader(std::list<std::string> videoUris, std::list<std::string> audioUri = {}, std::string path = "", std::string filename = "");
    explicit BiliDownloader(ResourseInfo info);
    ~BiliDownloader() = default;

    void start() override;
    void stop() override;
    void pause() override;
    void resume() override;
    void downloadStatus() override;
    void finish() override;

    void setVideoUris(const std::list<std::string>& videoUris);
    void setAudioUris(const std::list<std::string>& audioUri);
    void setPath(std::string path);
    const std::string& path() const;
    void setFilename(std::string filename);
    const std::string& filename() const;

    bool isFinished() const;

    std::string nowDownload() const;

private:
    ResourseInfo m_resourseInfo;
    std::string m_path;
    std::string m_filename;
    std::string m_uniqueId;
    AriaDownloader m_videoDownloader;
    AriaDownloader m_audioDownloader;
    bool m_finished;
    bool m_haveTwoPart;
};

}  // namespace download