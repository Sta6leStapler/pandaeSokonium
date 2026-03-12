#include "LocalizationManager.h"
#include <fstream>
#include <sstream>
#include <iostream>

bool LocalizationManager::LoadLanguage(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Could not open language file: " << filename << std::endl;
        return false;
    }

    mDictionary.clear();
    std::string line;
    while (std::getline(file, line)) {
        // 空行やコメント（#で始まる行）をスキップ
        if (line.empty() || line[0] == '#') continue;

        size_t delimiterPos = line.find('=');
        if (delimiterPos != std::string::npos) {
            std::string key = trim(line.substr(0, delimiterPos));
            std::string value = trim(line.substr(delimiterPos + 1));

            // TGUI等で日本語を扱うため、そのままmapに格納
            mDictionary[key] = value;
        }
    }
    return true;
}

std::string LocalizationManager::Get(const std::string& key) const {
    auto it = mDictionary.find(key);
    if (it != mDictionary.end()) {
        return it->second;
    }
    // キーが見つからない場合は、デバッグしやすいようにキー名をそのまま返す
    return key;
}

std::string LocalizationManager::trim(const std::string& str) {
    const std::string whitespace = " \t\r\n";
    const auto strBegin = str.find_first_not_of(whitespace);
    if (strBegin == std::string::npos) return "";

    const auto strEnd = str.find_last_not_of(whitespace);
    const auto strRange = strEnd - strBegin + 1;

    return str.substr(strBegin, strRange);
}