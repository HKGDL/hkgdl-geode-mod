#include "HKGDManager.hpp"

HKGDManager* HKGDManager::s_instance = nullptr;

HKGDManager::HKGDManager() {
    m_apiUrl = "https://api.hkgdl.dpdns.org/api";
    m_websiteUrl = "https://hkgdl.dpdns.org";
}

HKGDManager* HKGDManager::get() {
    if (!s_instance) {
        s_instance = new HKGDManager();
    }
    return s_instance;
}

std::string HKGDManager::getApiUrl() {
    auto mod = Mod::get();
    auto url = mod->getSettingValue<std::string>("api-url");
    if (url.empty()) {
        return m_apiUrl;
    }
    return url;
}

std::string HKGDManager::getWebsiteUrl() {
    auto mod = Mod::get();
    auto url = mod->getSettingValue<std::string>("website-url");
    if (url.empty()) {
        return m_websiteUrl;
    }
    return url;
}

bool HKGDManager::isExtremeClassicDemon(GJGameLevel* level) {
    if (!level) return false;
    
    // Check if it's a demon
    if (!level->m_demon) return false;
    
    // Check if demon difficulty is Extreme (6)
    if (level->m_demonDifficulty != 6) return false;
    
    // Check if it's a platformer - platformer levels have m_levelLength = 5
    // Also check isPlatformer() method
    if (level->isPlatformer()) return false;
    if (level->m_levelLength == 5) return false; // Platformer length type
    
    return true;
}

bool HKGDManager::isPlatformerExtremeDemon(GJGameLevel* level) {
    if (!level) return false;
    
    // Check if it's a demon
    if (!level->m_demon) return false;
    
    // Check if demon difficulty is Extreme (6) - only extreme demons
    if (level->m_demonDifficulty != 6) return false;
    
    // Check if it's a platformer - platformer levels have m_levelLength = 5
    // Also check isPlatformer() method
    if (!level->isPlatformer() && level->m_levelLength != 5) return false;
    
    return true;
}

// Detect platformer DEMONS only (not all platformer levels)
bool HKGDManager::isPlatformerLevel(GJGameLevel* level) {
    if (!level) return false;
    
    // Check if it's a demon
    if (!level->m_demon) return false;
    
    // Check if it's a platformer - platformer levels have m_levelLength = 5
    // Also check isPlatformer() method
    if (!level->isPlatformer() && level->m_levelLength != 5) return false;
    
    return true;
}

void HKGDManager::checkLevelOnHKGDL(int levelId, std::function<void(bool, int)> callback) {
    auto url = fmt::format("{}/levels", getApiUrl());
    
    geode::async::spawn(
        web::WebRequest().get(url),
        [callback, levelId](web::WebResponse const& response) {
            if (!response.ok()) {
                callback(false, -1);
                return;
            }
            
            auto jsonResult = response.json();
            if (jsonResult.isErr()) {
                callback(false, -1);
                return;
            }
            
            auto& json = jsonResult.unwrap();
            
            try {
                if (json.isArray()) {
                    auto arrResult = json.asArray();
                    if (arrResult.isOk()) {
                        for (auto& levelData : arrResult.unwrap()) {
                            // Check if levelId matches (as string)
                            auto levelIdResult = levelData["levelId"].asString();
                            if (levelIdResult.isOk()) {
                                std::string apiLevelId = levelIdResult.unwrap();
                                if (apiLevelId == std::to_string(levelId)) {
                                    auto rankResult = levelData["hkgdRank"].asInt();
                                    int rank = rankResult.isOk() ? static_cast<int>(rankResult.unwrap()) : -1;
                                    callback(true, rank);
                                    return;
                                }
                            }
                        }
                    }
                }
                callback(false, -1);
            } catch (...) {
                callback(false, -1);
            }
        }
    );
}

void HKGDManager::fetchVictors(int levelId, std::function<void(std::vector<HKGDRecord>)> callback) {
    auto url = fmt::format("{}/levels", getApiUrl());
    
    geode::async::spawn(
        web::WebRequest().get(url),
        [callback, levelId](web::WebResponse const& response) {
            std::vector<HKGDRecord> records;
            
            if (!response.ok()) {
                callback(records);
                return;
            }
            
            auto jsonResult = response.json();
            if (jsonResult.isErr()) {
                callback(records);
                return;
            }
            
            auto& json = jsonResult.unwrap();
            
            try {
                if (json.isArray()) {
                    auto arrResult = json.asArray();
                    if (arrResult.isOk()) {
                        for (auto& levelData : arrResult.unwrap()) {
                            // Check if levelId matches
                            auto levelIdResult = levelData["levelId"].asString();
                            if (levelIdResult.isOk()) {
                                std::string apiLevelId = levelIdResult.unwrap();
                                if (apiLevelId == std::to_string(levelId)) {
                                    // Found the level, extract records
                                    if (levelData.contains("records") && levelData["records"].isArray()) {
                                        auto recordsArrResult = levelData["records"].asArray();
                                        if (recordsArrResult.isOk()) {
                                            int rank = 1;
                                            for (auto& recordJson : recordsArrResult.unwrap()) {
                                                HKGDRecord record;
                                                record.rank = rank++;
                                                
                                                auto playerResult = recordJson["player"].asString();
                                                record.username = playerResult.isOk() ? playerResult.unwrap() : "Unknown";
                                                
                                                auto dateResult = recordJson["date"].asString();
                                                record.date = dateResult.isOk() ? dateResult.unwrap() : "";
                                                
                                                if (recordJson.contains("attempts") && !recordJson["attempts"].isNull()) {
                                                    auto attemptsResult = recordJson["attempts"].asInt();
                                                    record.attempts = attemptsResult.isOk() ? static_cast<int>(attemptsResult.unwrap()) : 0;
                                                }
                                                
                                                // Check for video URL
                                                if (recordJson.contains("videoUrl") && !recordJson["videoUrl"].isNull()) {
                                                    auto videoResult = recordJson["videoUrl"].asString();
                                                    record.videoUrl = videoResult.isOk() ? videoResult.unwrap() : "";
                                                }
                                                
                                                // Check for FPS
                                                if (recordJson.contains("fps") && !recordJson["fps"].isNull()) {
                                                    auto fpsResult = recordJson["fps"].asString();
                                                    record.fps = fpsResult.isOk() ? fpsResult.unwrap() : "";
                                                }
                                                
                                                records.push_back(record);
                                            }
                                        }
                                    }
                                    break;
                                }
                            }
                        }
                    }
                }
            } catch (...) {
            }
            
            callback(records);
        }
    );
}

void HKGDManager::fetchLevelInfo(int levelId, std::function<void(HKGDLevelInfo)> callback) {
    fetchVictors(levelId, [callback, levelId](std::vector<HKGDRecord> records) {
        HKGDLevelInfo info;
        info.levelId = levelId;
        info.records = records;
        info.hasRecords = !records.empty();
        callback(info);
    });
}

void HKGDManager::submitRecord(int levelId, std::string levelName, std::string username, int attempts, std::string videoUrl, 
                                std::string fps, std::string date, std::function<void(HKGDSubmissionResult)> callback) {
    auto url = fmt::format("{}/pending-submissions", getApiUrl());
    
    // Generate a unique ID
    auto now = std::chrono::system_clock::now();
    auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    std::string submissionId = fmt::format("pending-{}-{}", timestamp, std::rand() % 1000000);
    
    // Create record data as a JSON string
    std::string attemptsStr = attempts > 0 ? std::to_string(attempts) : "null";
    std::string fpsVal = fps.empty() ? "null" : "\"" + fps + "\"";
    std::string recordData = fmt::format(
        "{{\"player\":\"{}\",\"date\":\"{}\",\"videoUrl\":\"{}\",\"fps\":{},\"attempts\":{}}}",
        username, date, videoUrl, fpsVal, attemptsStr
    );
    
    // Create JSON body with all required fields - no undefined values
    auto body = matjson::makeObject({
        {"id", submissionId},
        {"levelId", std::to_string(levelId)},
        {"levelName", levelName},
        {"isNewLevel", false},  // Admin will decide difficulty placement for new levels
        {"submittedAt", date},
        {"submittedBy", username},
        {"status", std::string("pending")},
        {"record_data", recordData},
        {"adminDecidesDifficulty", true}  // Admin will decide where this goes
    });
    
    auto request = web::WebRequest();
    request.bodyJSON(body);
    
    geode::async::spawn(
        request.post(url),
        [callback](web::WebResponse const& response) {
            HKGDSubmissionResult result;
            
            if (!response.ok()) {
                result.success = false;
                result.message = fmt::format("Server error: {}", static_cast<int>(response.code()));
                callback(result);
                return;
            }
            
            auto jsonResult = response.json();
            if (jsonResult.isErr()) {
                result.success = false;
                result.message = "Invalid server response";
                callback(result);
                return;
            }
            
            auto& json = jsonResult.unwrap();
            
            try {
                // Check for success or error in response
                if (json.contains("error")) {
                    auto errorResult = json["error"].asString();
                    result.success = false;
                    result.message = errorResult.isOk() ? errorResult.unwrap() : "Unknown error";
                } else if (json.contains("success")) {
                    auto successResult = json["success"].asBool();
                    result.success = successResult.isOk() ? successResult.unwrap() : false;
                    result.message = "Record submitted for approval!";
                } else {
                    result.success = true;
                    result.message = "Record submitted for approval!";
                }
            } catch (...) {
                result.success = false;
                result.message = "Error parsing response";
            }
            
            callback(result);
        }
    );
}

void HKGDManager::submitPlatformerRecord(int levelId, std::string levelName, std::string username, int attempts, std::string videoUrl,
                                        std::string fps, std::string date, std::function<void(HKGDSubmissionResult)> callback) {
    auto url = fmt::format("{}/platformer-submissions", getApiUrl());
    
    // Generate a unique ID
    auto now = std::chrono::system_clock::now();
    auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    std::string submissionId = fmt::format("platformer-pending-{}-{}", timestamp, std::rand() % 1000000);
    
    // Create record data as a JSON string
    std::string attemptsStr = attempts > 0 ? std::to_string(attempts) : "null";
    std::string fpsVal = fps.empty() ? "null" : "\"" + fps + "\"";
    std::string recordData = fmt::format(
        "{{\"player\":\"{}\",\"date\":\"{}\",\"videoUrl\":\"{}\",\"fps\":{},\"attempts\":{}}}",
        username, date, videoUrl, fpsVal, attemptsStr
    );
    
    // Create JSON body with all required fields
    auto body = matjson::makeObject({
        {"id", submissionId},
        {"levelId", std::to_string(levelId)},
        {"levelName", levelName},
        {"isNewLevel", false},  // Admin will decide difficulty placement for new levels
        {"submittedAt", date},
        {"submittedBy", username},
        {"status", std::string("pending")},
        {"record_data", recordData},
        {"isPlatformer", true},  // Mark as platformer submission
        {"adminDecidesDifficulty", true}  // Admin will decide where this goes
    });
    
    auto request = web::WebRequest();
    request.bodyJSON(body);
    
    geode::async::spawn(
        request.post(url),
        [callback](web::WebResponse const& response) {
            HKGDSubmissionResult result;
            
            if (!response.ok()) {
                result.success = false;
                result.message = fmt::format("Server error: {}", static_cast<int>(response.code()));
                callback(result);
                return;
            }
            
            auto jsonResult = response.json();
            if (jsonResult.isErr()) {
                result.success = false;
                result.message = "Invalid server response";
                callback(result);
                return;
            }
            
            auto& json = jsonResult.unwrap();
            
            try {
                // Check for success or error in response
                if (json.contains("error")) {
                    auto errorResult = json["error"].asString();
                    result.success = false;
                    result.message = errorResult.isOk() ? errorResult.unwrap() : "Unknown error";
                } else if (json.contains("success")) {
                    auto successResult = json["success"].asBool();
                    result.success = successResult.isOk() ? successResult.unwrap() : false;
                    result.message = "Platformer record submitted for approval!";
                } else {
                    result.success = true;
                    result.message = "Platformer record submitted for approval!";
                }
            } catch (...) {
                result.success = false;
                result.message = "Error parsing response";
            }
            
            callback(result);
        }
    );
}

void HKGDManager::fetchAllLevels(std::function<void(std::vector<HKGDLevelInfo>)> callback) {
    auto url = fmt::format("{}/levels", getApiUrl());
    
    geode::async::spawn(
        web::WebRequest().get(url),
        [callback](web::WebResponse const& response) {
            std::vector<HKGDLevelInfo> levels;
            
            if (!response.ok()) {
                callback(levels);
                return;
            }
            
            auto jsonResult = response.json();
            if (jsonResult.isErr()) {
                callback(levels);
                return;
            }
            
            auto& json = jsonResult.unwrap();
            
            try {
                if (json.isArray()) {
                    auto arrResult = json.asArray();
                    if (arrResult.isOk()) {
                        for (auto& levelData : arrResult.unwrap()) {
                            HKGDLevelInfo info;
                            
                            auto levelIdResult = levelData["levelId"].asString();
                            if (levelIdResult.isOk()) {
                                info.levelId = std::stoi(levelIdResult.unwrap());
                            }
                            
                            auto nameResult = levelData["name"].asString();
                            info.name = nameResult.isOk() ? nameResult.unwrap() : "Unknown";
                            
                            auto rankResult = levelData["hkgdRank"].asInt();
                            info.position = rankResult.isOk() ? static_cast<int>(rankResult.unwrap()) : -1;
                            
                            info.hasRecords = false;
                            
                            levels.push_back(info);
                        }
                    }
                }
            } catch (...) {
            }
            
            callback(levels);
        }
    );
}

void HKGDManager::getMappedPlayerName(std::string gameName, std::function<void(std::string)> callback) {
    auto url = fmt::format("{}/player-mapping?gameName={}", getApiUrl(), gameName);
    
    geode::async::spawn(
        web::WebRequest().get(url),
        [callback, gameName](web::WebResponse const& response) {
            if (!response.ok()) {
                callback(gameName); // Fallback to original name
                return;
            }
            
            auto jsonResult = response.json();
            if (jsonResult.isErr()) {
                callback(gameName); // Fallback to original name
                return;
            }
            
            auto& json = jsonResult.unwrap();
            
            try {
                if (json.contains("dbName") && json["dbName"].isString()) {
                    auto dbNameResult = json["dbName"].asString();
                    if (dbNameResult.isOk()) {
                        callback(dbNameResult.unwrap());
                        return;
                    }
                }
            } catch (...) {
                // Fallback to original name
            }
            
            callback(gameName);
        }
    );
}

void HKGDManager::fetchPlayerRecordsWithMapping(std::string gameName, std::function<void(std::vector<HKGDRecord>)> callback) {
    // First check for name mapping
    getMappedPlayerName(gameName, [this, callback](std::string dbName) {
        // Use the mapped name to fetch records
        fetchPlayerRecords(dbName, callback);
    });
}

void HKGDManager::fetchPlayerRecords(std::string username, std::function<void(std::vector<HKGDRecord>)> callback) {
    // Since there's no /player endpoint, we fetch all levels and search for the player in records
    auto url = fmt::format("{}/levels", getApiUrl());
    
    geode::async::spawn(
        web::WebRequest().get(url),
        [callback, username](web::WebResponse const& response) {
            std::vector<HKGDRecord> records;
            
            if (!response.ok()) {
                callback(records);
                return;
            }
            
            auto jsonResult = response.json();
            if (jsonResult.isErr()) {
                callback(records);
                return;
            }
            
            auto& json = jsonResult.unwrap();
            
            try {
                if (json.isArray()) {
                    auto arrResult = json.asArray();
                    if (arrResult.isOk()) {
                        for (auto& levelData : arrResult.unwrap()) {
                            // Get level info
                            auto levelNameResult = levelData["name"].asString();
                            std::string levelName = levelNameResult.isOk() ? levelNameResult.unwrap() : "Unknown";
                            
                            auto rankResult = levelData["hkgdRank"].asInt();
                            int hkgdRank = rankResult.isOk() ? static_cast<int>(rankResult.unwrap()) : 0;
                            
                            // Search through records for this player
                            if (levelData.contains("records") && levelData["records"].isArray()) {
                                auto recordsArrResult = levelData["records"].asArray();
                                if (recordsArrResult.isOk()) {
                                    for (auto& recordJson : recordsArrResult.unwrap()) {
                                        // Check if this record belongs to the player
                                        auto playerResult = recordJson["player"].asString();
                                        if (playerResult.isOk()) {
                                            std::string playerName = playerResult.unwrap();
                                            // Case-insensitive comparison
                                            if (playerName == username || 
                                                (playerName.length() == username.length() &&
                                                 std::equal(playerName.begin(), playerName.end(), username.begin(),
                                                     [](char a, char b) { return std::tolower(a) == std::tolower(b); }))) {
                                                HKGDRecord record;
                                                record.rank = hkgdRank; // Use level rank instead of victor rank
                                                record.username = playerName;
                                                record.levelName = levelName;
                                                
                                                // Video URL
                                                if (recordJson.contains("videoUrl") && !recordJson["videoUrl"].isNull()) {
                                                    auto videoResult = recordJson["videoUrl"].asString();
                                                    record.videoUrl = videoResult.isOk() ? videoResult.unwrap() : "";
                                                }
                                                
                                                // Date
                                                auto dateResult = recordJson["date"].asString();
                                                record.date = dateResult.isOk() ? dateResult.unwrap() : "";
                                                
                                                // Attempts
                                                if (recordJson.contains("attempts") && !recordJson["attempts"].isNull()) {
                                                    auto attemptsResult = recordJson["attempts"].asInt();
                                                    record.attempts = attemptsResult.isOk() ? static_cast<int>(attemptsResult.unwrap()) : 0;
                                                }
                                                
                                                // FPS
                                                if (recordJson.contains("fps") && !recordJson["fps"].isNull()) {
                                                    auto fpsResult = recordJson["fps"].asString();
                                                    record.fps = fpsResult.isOk() ? fpsResult.unwrap() : "";
                                                }
                                                
                                                records.push_back(record);
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            } catch (...) {
            }
            
            // Sort records by level rank (hardest first)
            std::sort(records.begin(), records.end(), [](const HKGDRecord& a, const HKGDRecord& b) {
                return a.rank < b.rank;
            });
            
            callback(records);
        }
    );
}
