// Platformer level detection
bool isPlatformerExtremeDemon(GJGameLevel* level) {
    if (!level) return false;
    
    // Check if it's a demon
    if (!level->m_demon) return false;
    
    // Check if demon difficulty is Extreme (6)
    if (level->m_demonDifficulty != 6) return false;
    
    // Check if it's a platformer - platformer levels have m_levelLength = 5
    // Also check isPlatformer() method
    if (!level->isPlatformer() && level->m_levelLength != 5) return false;
    
    return true;
}

// Submit a platformer record to HKGDL
void submitPlatformerRecord(int levelId, std::string levelName, std::string username, int attempts, std::string videoUrl,
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
        {"isNewLevel", false},
        {"submittedAt", date},
        {"submittedBy", username},
        {"status", std::string("pending")},
        {"record_data", recordData},
        {"isPlatformer", true}  // Mark as platformer submission
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
