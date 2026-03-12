#pragma once
#include <string>
#include <map>

class LocalizationManager {
public:
    // 指定した言語ファイルをロードする (例: "Assets/Languages/jp.txt")
    bool LoadLanguage(const std::string& filename);

    // キーに対応する文字列を返す。見つからなければキーそのものを返す
    std::string Get(const std::string& key) const;

private:
    std::map<std::string, std::string> mDictionary;

    // 文字列の前後から空白を取り除くヘルパー関数
    std::string trim(const std::string& str);
};