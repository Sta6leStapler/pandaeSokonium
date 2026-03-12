#include "MoveAnimationComponent.h"
#include "IActor.h"

MoveAnimationComponent::MoveAnimationComponent(IActor* owner, int updateOrder)
    : mOwner(owner)
    , mUpdateOrder(updateOrder)
    , mStartPos(0.0f, 0.0f)
    , mEndPos(0.0f, 0.0f)
    , mDuration(0.0f)
    , mTimer(0.0f)
    , mIsAnimating(false)
{
    // 所有者であるアクターのコンポーネントリストに追加します。
    mOwner->AddComponent(this);
}

MoveAnimationComponent::~MoveAnimationComponent()
{
    mOwner->RemoveComponent(this);
}

void MoveAnimationComponent::Start(const sf::Vector2f& start, const sf::Vector2f& end, float duration) {
    mStartPos = start;
    mEndPos = end;
    mDuration = duration;
    mTimer = 0.0f;
    mIsAnimating = true;

    // アニメーション開始直後の位置をアクターに反映
    mOwner->SetPosition(mStartPos);
}

void MoveAnimationComponent::Update(float deltaTime) {
    if (mIsAnimating) {
        mTimer += deltaTime;

        // 進捗率 t (0.0 ～ 1.0) の算出
        // $t = \frac{mTimer}{mDuration}$
        float t = mTimer / mDuration;

        // 完了判定
        if (t >= 1.0f) {
            t = 1.0f;
            mIsAnimating = false;
        }

        // 線形補間（Lerp）による現在座標の計算
        // $P(t) = (1 - t)P_{start} + tP_{end}$
        sf::Vector2f currentPos = mStartPos + t * (mEndPos - mStartPos);

        // アクターの物理座標（スクリーン座標）を更新
        // これにより SpriteComponent 等が新しい位置で描画されるようになります。
        mOwner->SetPosition(currentPos);
    }
}