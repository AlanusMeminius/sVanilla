#pragma once

#include "Sqlite/Storage/BaseStorage.h"
#include "Sqlite/Storage/StorageFactory.h"

struct DownloadedItem
{
    std::string uniqueId;
    int pluginType;
    std::string filePath;
    std::string coverPath;
    std::string bvid;
    std::string title;
    std::string auther;
    std::string url;
    std::string aid;
    std::string cid;
    int duration;
    int type;
    bool fileExist;

    // move to sqlite
    int bind(sqlite::SQLiteStatement& stmt) const;
    void setValue(sqlite::SQLiteStatement& stmt, int startIndex = 0);
};

// clang-format off
TABLESTRUCTINFO_BEGIN(DownloadedItem)
    TABLESTRUCTINFO_COMLUNM(uniqueId, uniqueId, false, true)
    TABLESTRUCTINFO_COMLUNM(pluginType)
    TABLESTRUCTINFO_COMLUNM(filePath)
    TABLESTRUCTINFO_COMLUNM(coverPath)
    TABLESTRUCTINFO_COMLUNM(bvid)
    TABLESTRUCTINFO_COMLUNM(title)
    TABLESTRUCTINFO_COMLUNM(auther)
    TABLESTRUCTINFO_COMLUNM(url)
    TABLESTRUCTINFO_COMLUNM(cid)
    TABLESTRUCTINFO_COMLUNM(aid)
    TABLESTRUCTINFO_COMLUNM(duration)
    TABLESTRUCTINFO_COMLUNM(type)
    TABLESTRUCTINFO_COMLUNM(fileExist)
TABLESTRUCTINFO_END(FinishedItem)
// clang-format on

class DownloadedItemStorage : public sqlite::BaseStorage
{
public:
    using Entity = DownloadedItem;
    using BaseStorage::BaseStorage;

    bool isDownload(const std::string& guid);

    std::vector<Entity> lastItems();

    void updateFileExist();
    void updateFileExist(bool exist, const std::string& guid);

private:
    static constexpr int maxQueryNum = 4000;
};
