#include "PauseMenu.h"

PauseMenu::PauseMenu(class Game* game)
	: mGame(game)
	, mFont(new Font())
	, mTitle(new sf::Texture())
	, mBackground(new sf::Texture())
	, mButtonOn(new sf::Texture())
	, mButtonOff(new sf::Texture())
	, mButtons(std::vector<Button*>{})
	, mState(IUIScreen::UIState::EActive)
{
	mGame->PushUI(this);
	// �|�[�Y���j���[�̓Q�[�����[�v�Ɋ��荞�ޏ����Ȃ̂ŁA
	// �|�[�Y���j���[�̋N�����̂݃C���X�^���X�����݂���悤�ɂ���
	mGame->SetState(Game::GameState::EPaused);

	// TODO �e�N�X�`����Game�N���X�����荞�ނ悤�ɕς���
	// �t�H���g�����[�h���A�摜��ǂݍ���Ńe�N�X�`���ɂ��Ă���
	mFont->Load("Assets/fonts/arial.ttf");
	mButtonOn->loadFromFile("Assets/ButtonYellow.png");
	mButtonOff->loadFromFile("Assets/ButtonBlue.png");

	// �|�[�Y�����ƃQ�[���I���̃{�^����ǉ�
	// TODO �{�^���̍��W���΍��W���瑊�΍��W�ɕς���
	// Note: SetTitle�֐���mFont->Load���Ăяo������
	// Note: �{�^���̍��W�͘g�̍���A���W�n���E�B���h�E�̍��オ(0,0)
	SetTitle("PAUSED");
	mTitlePos = sf::Vector2f{ static_cast<float>(game->GetWindowSize().x - mTitle->getSize().x) / 2.0f, 100.0f};
	mNextButtonPos = sf::Vector2f{ static_cast<float>(game->GetWindowSize().x - mButtonOn->getSize().x) / 2.0f, 200.0f };
	mBGPos = sf::Vector2f{ 0.0f, 0.0f };
	AddButton("Resume", [this]() {
		Close();
		}); 
	AddButton("Quit", [this]() {
		mGame->SetState(Game::EQuit);
		}); 
}

PauseMenu::~PauseMenu()
{
	// Button�N���X�̃����o��Font�N���X�̃����o�ϐ����܂܂�邽�߁A��������ɏ���
	while (!mButtons.empty())
	{
		delete mButtons.back();
		mButtons.pop_back();
	}
	delete mFont;
	delete mTitle;
	delete mBackground;
	delete mButtonOn;
	delete mButtonOff;
	mGame->SetState(Game::GameState::EGamePlay);
}

void PauseMenu::Update(float deltaTime)
{

}

void PauseMenu::Draw(sf::RenderWindow* mWindow)
{
	// �w�i��ǉ�����ꍇ�͂����`��
	if (mBackground)
	{
		DrawTexture(mWindow, mBackground, mBGPos);
	}
	// �^�C�g�������݂���ꍇ�͂����`��
	if (mTitle)
	{
		DrawTexture(mWindow, mTitle, mTitlePos);
	}
	// �{�^���{�̂̕`��
	for (auto b : mButtons)
	{
		// �{�^���̔w�i��`��
		sf::Texture* tex = b->GetHighlighted() ? mButtonOn : mButtonOff;
		DrawTexture(mWindow, tex, b->GetPosition());
		// �{�^���̃e�L�X�g��`��
		DrawTexture(mWindow, b->GetNameTex(), b->GetPosition());
	}
}

void PauseMenu::ProcessInput(const sf::Event* event, const sf::Vector2i& mousePos)
{
	// �{�^�������邩�H
	if (!mButtons.empty())
	{
		// �}�E�X�̈ʒu���擾
		sf::Vector2f mouse_pos{ static_cast<float>(mousePos.x), static_cast<float>(mousePos.y) };

		// �{�^���̋���
		for (auto b : mButtons)
		{
			if (b->ContainsPoint(mouse_pos))
			{
				b->SetHighlighted(true);
			}
			else
			{
				b->SetHighlighted(false);
			}
		}
	}

	// Esc�L�[�������ꂽ��|�[�Y���j���[�����
	switch (event->key.code)
	{
	case sf::Keyboard::Escape:
		//Close();
		break;
	case sf::Keyboard::Insert:
		Close();
		break;
	default:
		break;
	}

	// �{�^����Ń}�E�X���{�^���������ꂽ��A���̃{�^���̏������s��
	switch (event->mouseButton.button)
	{
	case sf::Mouse::Left:
		// �A�N�e�B�u�ȃ{�^�������邩���ׂ�
		if (!mButtons.empty())
		{
			for (auto b : mButtons)
			{
				if (b->GetHighlighted())
				{
					b->OnClick();
					break;
				}
			}
		}
		break;
	default:
		break;
	}

}

void PauseMenu::Close()
{
	mState = IUIScreen::UIState::EClosing;
}

void PauseMenu::SetTitle(const std::string& text, const sf::Color& color /*= sf::Color::White*/, int pointSize /*= 40*/)
{
	mTitle = mFont->RenderText(text, color, pointSize);
}

void PauseMenu::AddButton(const std::string& name, std::function<void()> onClick)
{
	sf::Vector2f dims{ static_cast<float>(mButtonOn->getSize().x), static_cast<float>(mButtonOn->getSize().y) };
	Button* b = new Button(name, mFont, onClick, mNextButtonPos, dims);
	b->SetNameCenter(name, dims);
	mButtons.emplace_back(b);

	// TODO �{�^���̍��W���΍��W���瑊�΍��W�ɕς���
	// �{�^���̍��� + �]���@�̕������ʒu��������
	mNextButtonPos.y += mButtonOff->getSize().y + 20.0f;
}

void PauseMenu::DrawTexture(sf::RenderWindow* mWindow, class sf::Texture* texture, const sf::Vector2f& offset /*= sf::Vector2f{ 0.0f, 0.0f }*/, float scale /*= 1.0f*/)
{
	sf::Sprite spr;
	spr.setTexture(*texture);

	// �X�v���C�g�̃X�P�[�����O�ƈړ����s��
	spr.setScale(scale, scale);
	spr.setPosition(offset);

	mWindow->draw(spr);
}