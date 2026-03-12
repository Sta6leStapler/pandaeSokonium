#pragma once

#include "IComponent.h"
#include "SFML/Graphics.hpp"

// アクターの滑らかな座標移動を管理するコンポーネント
// 物理的な座標移動を移動間隔と同期させ、線形補間によるアニメーションを行う
class MoveAnimationComponent : public IComponent {
public:
    // コンストラクタ
    MoveAnimationComponent(class IActor* owner, int updateOrder = 10);
    // デストラクタ
    ~MoveAnimationComponent();

    // 毎フレームの更新処理
    // 座標の補間計算を行う
    void Update(float deltaTime) override;

    // 入力処理はこのコンポーネントでは不要なため空実装とする
    void ProcessInput(const sf::Event* event,
        const std::map<sf::Keyboard::Key, float>& key_held_duration,
        const std::map<sf::Keyboard::Key, float>& auto_repeat_timer) override {
    }

    // アニメーションを開始する
    // 開始時のスクリーン座標と目標となるスクリーン座標とアニメーションにかける秒数を受け取る
    // Playerの移動間隔と同期させる 
    void Start(const sf::Vector2f& start, const sf::Vector2f& end, float duration);

    // アニメーションが実行中かどうかを取得する
    bool IsAnimating() const { return mIsAnimating; }

    // アニメーションの進捗率（0.0～1.0）を取得
    float GetProgress() const {
        if (!mIsAnimating || mDuration <= 0.0f) return 1.0f;
        return mTimer / mDuration;
    }

    // 終了間際（先行入力許容範囲）かどうかを判定
    bool CanAcceptNextInput(float threshold = 0.9f) const {
        return !mIsAnimating || GetProgress() >= threshold;
    }

    // IComponent インターフェースのゲッター
    int GetUpdateOrder() override { return mUpdateOrder; }

private:
    class IActor* mOwner;
    int mUpdateOrder;

    sf::Vector2f mStartPos; // アニメーション開始地点
    sf::Vector2f mEndPos;   // アニメーション終了地点
    float mDuration;        // アニメーション所要時間
    float mTimer;           // 経過時間
    bool mIsAnimating;      // アニメーション中フラグ
};