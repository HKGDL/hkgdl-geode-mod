#pragma once

#include <cctype>
#include <Geode/Geode.hpp>
#include <Geode/loader/Mod.hpp>
#include <Geode/utils/web.hpp>
#include <Geode/utils/async.hpp>
#include <Geode/binding/GJGameLevel.hpp>

using namespace geode::prelude;

struct HKGDRecord {
    int rank;
    std::string username;
    std::string date;
    std::string videoUrl;
    std::string levelName;  // For player stats popup
    std::string fps;
    int attempts = 0;
};

struct HKGDLevelInfo {
    int levelId;
    std::string name;
    int position;
    std::vector<HKGDRecord> records;
    bool hasRecords;
};

struct HKGDSubmissionResult {
    bool success;
    std::string message;
};

class HKGDManager {
public:
    static HKGDManager* get();
    
    // Get the API base URL from settings
    std::string getApiUrl();
    
    // Get the website URL
    std::string getWebsiteUrl();
    
    // Fetch level info from HKGDL API
    void fetchLevelInfo(int levelId, std::function<void(HKGDLevelInfo)> callback);
    
    // Check if a level is on HKGDL
    void checkLevelOnHKGDL(int levelId, std::function<void(bool, int)> callback);
    
    // Fetch victors for a level
    void fetchVictors(int levelId, std::function<void(std::vector<HKGDRecord>)> callback);
    
    // Submit a record to HKGDL
    void submitRecord(int levelId, std::string levelName, std::string username, int attempts, std::string videoUrl, 
                      std::string fps, std::string date, std::function<void(HKGDSubmissionResult)> callback);
    
    // Fetch all HKGDL levels
    void fetchAllLevels(std::function<void(std::vector<HKGDLevelInfo>)> callback);
    
    // Fetch player's beaten extreme demons
    void fetchPlayerRecords(std::string username, std::function<void(std::vector<HKGDRecord>)> callback);
    
    // Fetch player records with name mapping
    void fetchPlayerRecordsWithMapping(std::string gameName, std::function<void(std::vector<HKGDRecord>)> callback);
    
    // Get mapped database name for a game name
    void getMappedPlayerName(std::string gameName, std::function<void(std::string)> callback);
    
    // Check if level is an extreme demon classic
    bool isExtremeClassicDemon(GJGameLevel* level);
    
    // Check if level is a platformer extreme demon
    bool isPlatformerExtremeDemon(GJGameLevel* level);
    
    // Check if level is ANY platformer level (not just extreme)
    bool isPlatformerLevel(GJGameLevel* level);
    
    // Submit a platformer record to HKGDL
    void submitPlatformerRecord(int levelId, std::string levelName, std::string username, int attempts, std::string videoUrl,
                                std::string fps, std::string date, std::function<void(HKGDSubmissionResult)> callback);

private:
    static HKGDManager* s_instance;
    std::string m_apiUrl;
    std::string m_websiteUrl;
    
    HKGDManager();
};
