#include "Game.h"

Game::Game(const std::string& config)
	:m_text(m_font, "Default", 24), 
	m_scoreText(m_font, "Score: 0", 24), 
	m_specialWeaponText(m_font, "Special Weapon Ready!", 20),
	m_pausedText(m_font, "PAUSED", 48),
	m_startText(m_font, "Press ENTER to Start", 36),
	m_gameOverText(m_font, "GAME OVER", 48),
	m_retryText(m_font, "Press ENTER to Retry", 36)
{
	init(config);
}

void Game::init(const std::string& path)
{
	std::ifstream fin(path);
	std::string start;
	while (fin >> start)
	{
		if (start == "Player")
		{
			fin >> m_playerConfig.shapeRadius >> m_playerConfig.collisionRadius
				>> m_playerConfig.fillRed >> m_playerConfig.fillGreen
				>> m_playerConfig.fillBlue >> m_playerConfig.outlineRed
				>> m_playerConfig.outlineGreen >> m_playerConfig.outlineBlue
				>> m_playerConfig.outlineThickness >> m_playerConfig.vertexCount
				>> m_playerConfig.speed;
		}
		else if (start == "Enemy")
		{
			fin >> m_enemyConfig.shapeRadius >> m_enemyConfig.collisionRadius
				>> m_enemyConfig.outlineRed >> m_enemyConfig.outlineGreen >> m_enemyConfig.outlineBlue
				>> m_enemyConfig.outlineThickness >> m_enemyConfig.vertexMin
				>> m_enemyConfig.vertexMax >> m_enemyConfig.lifespan
				>> m_enemyConfig.spawnInterval >> m_enemyConfig.speedMin
				>> m_enemyConfig.speedMax;
		}
		else if (start == "Bullet")
		{
			fin >> m_bulletConfig.shapeRadius >> m_bulletConfig.collisionRadius >> m_bulletConfig.fillRed
				>> m_bulletConfig.fillGreen >> m_bulletConfig.fillBlue
				>> m_bulletConfig.outlineRed >> m_bulletConfig.outlineGreen
				>> m_bulletConfig.outlineBlue >> m_bulletConfig.outlineThickness
				>> m_bulletConfig.vertexCount >> m_bulletConfig.lifespan >> m_bulletConfig.speed;
		}
		else if (start == "Window")
		{
			unsigned int width, height;
			int frameLimit, isFullScreen;
			fin >> width >> height >> frameLimit >> isFullScreen;
			if (isFullScreen == 1)
			{
				m_window.create(
					sf::VideoMode::getDesktopMode(), 
					"Geometry Wars", 
					sf::Style::None, 
					sf::State::Fullscreen
				);
				m_isFullscreen = true;
			}
			else
			{
				m_window.create(
					sf::VideoMode({ width, height }),
					"Geometry Wars",
					sf::Style::Titlebar | sf::Style::Close | sf::Style::Resize,
					sf::State::Windowed
				);
				m_windowedWidth = width;
				m_windowedHeight = height;
				m_isFullscreen = false;
			}
			m_frameLimit = frameLimit;
			m_window.setFramerateLimit(m_frameLimit);
			m_window.setKeyRepeatEnabled(false);
			m_originalViewCenter = m_cameraView.getCenter();

			if (!m_gameBuffer.resize({ static_cast<unsigned>(WORLD_WIDTH),
										static_cast<unsigned>(WORLD_HEIGHT) }))
				std::cerr << "Failed to create render buffer\n";

			if (!ImGui::SFML::Init(m_window))
			{
				std::cerr << "Could not initialize the window\n";
				std::exit(-1);
			}
		}
		else if (start == "Font")
		{
			std::string fontPath;
			int size;
			int color[3];
			fin >> fontPath >> size >> color[0] >> color[1] >> color[2];
			if (!m_font.openFromFile(fontPath))
			{
				throw std::runtime_error("Failed to load font from " + fontPath);
			}

			m_text.setFont(m_font);
			m_text.setCharacterSize(size);
			m_text.setFillColor(sf::Color(color[0], color[1], color[2]));
			m_text.setString("Default");
		}
		else if (start == "DebugMode")
		{
			fin >> m_debugMode;

			if (m_debugMode == 1)
			{
				ImGui::GetStyle().ScaleAllSizes(2.0f);
				ImGui::GetIO().FontGlobalScale = 2.0f;
			}
		}
	}

	m_scoreText.setFillColor(sf::Color::White);
	m_scoreText.setPosition({ 10.f, 10.f });

	m_specialWeaponText.setFillColor(sf::Color::Cyan);
	m_specialWeaponText.setPosition({ 10.f, 40.f });

	m_pausedText.setFillColor(sf::Color::Red);
	sf::FloatRect bounds = m_pausedText.getLocalBounds();

	m_specialBarOutline.setSize(sf::Vector2f(200.f, 15.f));
	m_specialBarOutline.setFillColor(sf::Color::Transparent);
	m_specialBarOutline.setOutlineThickness(2.f);
	m_specialBarOutline.setOutlineColor(sf::Color::White);
	m_specialBarOutline.setPosition({ 10.f, 70.f });

	m_specialBarFill.setSize(sf::Vector2f(0.f, 15.f));
	m_specialBarFill.setFillColor(sf::Color::Green);
	m_specialBarFill.setPosition({ 10.f, 70.f });

	m_startText.setFillColor(sf::Color::White);
	sf::FloatRect startBounds = m_startText.getLocalBounds();

	m_gameOverText.setFillColor(sf::Color::Red);
	sf::FloatRect overBounds = m_gameOverText.getLocalBounds();

	m_retryText.setFillColor(sf::Color::White);
	sf::FloatRect retryBounds = m_retryText.getLocalBounds();

	repositionUI();

	m_cameraView.setSize({ WORLD_WIDTH, WORLD_HEIGHT });
	m_cameraView.setCenter({ WORLD_WIDTH / 2.f, WORLD_HEIGHT / 2.f });
	m_originalViewCenter = Vec2f(WORLD_WIDTH / 2.f, WORLD_HEIGHT / 2.f);

	spawnPlayer();

	m_soundPool.resize(SOUND_POOL_SIZE);
	m_nextSoundIndex = 0;

	try {
		if (!m_music.openFromFile("assets/audio/background.mp3")) {
			std::cerr << "Failed to load background music\n";
		}
		else {
			m_music.setLooping(true);
			m_music.setVolume(m_musicVolume);
			m_music.play();
		}

		sf::SoundBuffer buffer;
		if (buffer.loadFromFile("assets/audio/shoot.wav")) {
			m_soundBuffers["shoot"] = buffer;
		}
		else {
			std::cerr << "Failed to load shoot.wav\n";
		}

		if (buffer.loadFromFile("assets/audio/explosion.wav")) {
			m_soundBuffers["explosion"] = buffer;
		}
		else {
			std::cerr << "Failed to load explosion.wav\n";
		}
		if(buffer.loadFromFile("assets/audio/tap.wav")) {
			m_soundBuffers["tap"] = buffer;
		}
		else {
			std::cerr << "Failed to load start.wav\n";
		}
		if(buffer.loadFromFile("assets/audio/explosionBig.wav")) {
			m_soundBuffers["explosionBig"] = buffer;
		}
		else {
			std::cerr << "Failed to load special.wav\n";
		}
		if(buffer.loadFromFile("assets/audio/gameover.wav")) {
			m_soundBuffers["gameover"] = buffer;
		}
		else {
			std::cerr << "Failed to load gameover.wav\n";
		}
	}
	catch (const std::exception& e) {
		std::cerr << "Audio init error: " << e.what() << std::endl;
	}
}

void Game::run()
{
	while (m_window.isOpen())
	{
		m_entityManager.update();

		ImGui::SFML::Update(m_window, m_deltaClock.restart());

		sUserInput();
		if (!m_paused && m_gameState == GameState::Playing && player())
		{
			sEnemySpawner();
			sSpecialWeapon();
			sMovement();
			sCollision();
			sLifeSpan();
			cameraShake();
		}
		sGUI();
		sRender();

		m_currentFrame++;
	}
}

void Game::cameraShake()
{
	if (m_shakeFramesRemaining > 0)
	{
		m_shakeFramesRemaining--;
		float strength = static_cast<float>(m_shakeFramesRemaining) / m_totalShakeFrames;
		float ox = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 2.f * m_shakeIntensity * strength;
		float oy = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 2.f * m_shakeIntensity * strength;
		m_cameraView.setCenter(m_originalViewCenter + sf::Vector2f(ox, oy));
	}
	else
	{
		m_cameraView.setCenter(m_originalViewCenter);
	}
}

void Game::triggerCameraShake(int durationFrames, float intensity)
{
	m_shakeFramesRemaining = durationFrames;
	m_totalShakeFrames = durationFrames;
	m_shakeIntensity = intensity;
}

void Game::sMovement()
{
	if (!m_isMovement) return;
	for (const auto& entity : m_entityManager.getEntities())
	{
		if(entity->has<CInput>() && entity->has<CTransform>())
		{
			auto& input = entity->get<CInput>();
			auto& transform = entity->get<CTransform>();
			Vec2f direction(0.0f, 0.0f);
			if (input.up)
				direction.y = -1.0f;
			if (input.down)
				direction.y = 1.0f;
			if (input.left)
				direction.x = -1.0f;
			if (input.right)
				direction.x = 1.0f;
			if (direction.x != 0.0f || direction.y != 0.0f)
			{
				direction.normalize();
				transform.velocity = direction * m_playerConfig.speed;
			}
			else
			{
				transform.velocity = Vec2f(0.0f, 0.0f);
			}
			if(input.shoot)
			{
				if(m_currentFrame % m_bulletConfig.collisionRadius == 0)
				{
					spawnBullet(entity, mouseWorldPos());
				}
			}
			if(input.specialWeapon)
			{
				activateSpecialWeapon(entity, mouseWorldPos());
			}
		}
		if (entity->has<CTransform>())
		{
			auto& transform = entity->get<CTransform>();
			transform.position += transform.velocity;
			auto windowSize = m_window.getSize();
			float radius = entity->get<CCollision>().radius;

			if (entity->tag() == "player")
			{
				if(transform.position.x - radius < 0.0f)
					transform.position.x = radius;
				if(transform.position.x + radius > WORLD_WIDTH)
					transform.position.x = WORLD_WIDTH - radius;
				if(transform.position.y - radius < 0.0f)
					transform.position.y = radius;
				if (transform.position.y + radius > WORLD_HEIGHT)
					transform.position.y = WORLD_HEIGHT - radius;
			} else
			{
				if(transform.position.x - radius < 0.0f || 
					transform.position.x + radius > WORLD_WIDTH)
					transform.velocity.x = -transform.velocity.x;
				if (transform.position.y - radius < 0.0f || 
					transform.position.y + radius > WORLD_HEIGHT)
					transform.velocity.y = -transform.velocity.y;
			}
		}
	}
}

void Game::sUserInput()
{
	while (auto event = m_window.pollEvent())
	{
		ImGui::SFML::ProcessEvent(m_window, *event);

		if (event->is<sf::Event::Closed>())
		{
			m_window.close();
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape))
		{
			m_window.close();
		}

		if (const auto* resized = event->getIf<sf::Event::Resized>())
		{
			onWindowResize({ resized->size.x, resized->size.y });
		}
	}
	if (m_gameState == GameState::StartMenu)
	{
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Enter))
		{
			playSFX("tap");
			m_gameState = GameState::Playing;
		}
	}
	if (m_gameState == GameState::Playing)
	{
		static bool pauseKeyWasPressed = false;
		bool pauseKeyPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::P);
		if (pauseKeyPressed && !pauseKeyWasPressed)
		{
			playSFX("tap");
			m_paused = !m_paused;
			if (m_paused) {
				if (m_music.getStatus() == sf::Music::Status::Playing) m_music.pause();
			}
			else {
				if (m_music.getStatus() == sf::Music::Status::Paused) m_music.play();
			}
		}
		pauseKeyWasPressed = pauseKeyPressed;
	}
	if (m_gameState == GameState::GameOver)
	{
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Enter))
		{
			playSFX("tap");
			resetGame();
			m_gameState = GameState::Playing;
		}
	}
	for (const auto& entity : m_entityManager.getEntities())
	{
		if (entity->has<CInput>())
		{
			auto& input = entity->get<CInput>();
			input.up = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W);
			input.down = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S);
			input.left = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A);
			input.right = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D);
			input.shoot = sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);
			input.specialWeapon = sf::Mouse::isButtonPressed(sf::Mouse::Button::Right);
		}
	}

	static bool muteKeyWasPressed = false;
	bool muteKeyPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::M);
	if (muteKeyPressed && !muteKeyWasPressed)
	{
		playSFX("tap");
		if (m_musicVolume > 0.f) {
			m_musicVolume = 0.f;
		}
		else {
			m_musicVolume = 20.0f;
		}
		m_music.setVolume(m_musicVolume);
	}
	muteKeyWasPressed = muteKeyPressed;


	static bool f11WasPressed = false;
	bool f11Pressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::F11);
	if (f11Pressed && !f11WasPressed)
		toggleFullscreen();
	f11WasPressed = f11Pressed;
}

void Game::sLifeSpan()
{
	if (!m_isLifeSpan) return;
	for (const auto& entity : m_entityManager.getEntities())
	{
		if (entity->has<CLifeSpan>())
		{
			auto& lifespan = entity->get<CLifeSpan>();
			auto& shape = entity->get<CShape>();
			lifespan.remaining--;
			float lifeRatio = static_cast<float>(lifespan.remaining) / static_cast<float>(lifespan.lifespan);
			sf::Color color = shape.circle.getFillColor();
			sf::Color outlineColor = shape.circle.getOutlineColor();
			color.a = (lifeRatio * 255);
			outlineColor.a = (lifeRatio * 255);
			shape.circle.setFillColor(color);	
			shape.circle.setOutlineColor(outlineColor);
			if (lifespan.remaining <= 0)
			{
				entity->destroy();
			}
		}
	}
}

void Game::sRender()
{
	float t = static_cast<float>(m_currentFrame) * 0.005f;
	unsigned int r = static_cast<unsigned>((std::sin(t) * 0.5f + 0.5f) * 155);
	unsigned int g = static_cast<unsigned>((std::sin(t + 2.0f) * 0.5f + 0.5f) * 155);
	unsigned int b = static_cast<unsigned>((std::sin(t + 4.0f) * 0.5f + 0.5f) * 155);

	m_gameBuffer.clear(sf::Color(r, g, b));
	m_gameBuffer.setView(m_cameraView);

	if (m_gameState == GameState::Playing)
	{
		for (const auto& entity : m_entityManager.getEntities())
		{
			if (entity->has<CTransform>() && entity->has<CShape>())
			{
				auto& transform = entity->get<CTransform>();
				auto& shapeComp = entity->get<CShape>();
				shapeComp.circle.setPosition({ transform.position.x, transform.position.y });
				transform.angle = fmod(transform.angle + 4.f, 360.f);
				shapeComp.circle.setScale({ transform.scale.x, transform.scale.y });
				shapeComp.circle.setRotation(sf::degrees(transform.angle));
				m_gameBuffer.draw(shapeComp.circle);
			}
		}
		m_gameBuffer.draw(m_scoreText);
		m_gameBuffer.draw(m_specialWeaponText);
		m_gameBuffer.draw(m_specialBarOutline);
		m_gameBuffer.draw(m_specialBarFill);
		if (m_paused) m_gameBuffer.draw(m_pausedText);
	}
	else if (m_gameState == GameState::StartMenu) { m_gameBuffer.draw(m_startText); }
	else if (m_gameState == GameState::GameOver) {
		m_gameBuffer.draw(m_gameOverText);
		m_gameBuffer.draw(m_retryText);
	}

	m_gameBuffer.display();

	auto wSize = m_window.getSize();
	sf::Sprite frame(m_gameBuffer.getTexture());
	frame.setScale({
		static_cast<float>(wSize.x) / WORLD_WIDTH,
		static_cast<float>(wSize.y) / WORLD_HEIGHT
		});

	m_window.clear(sf::Color::Black);

	m_window.setView(sf::View(sf::FloatRect(
		{ 0.f, 0.f },
		sf::Vector2f(wSize)
	)));

	m_window.draw(frame);
	ImGui::SFML::Render(m_window);
	m_window.display();
}

void Game::sGUI()
{
	m_scoreText.setString("Score: " + std::to_string(m_score));

	int gainedSinceUse = m_score - m_specialWeapon.lastScoreUsed;
	int needed = m_specialWeapon.scoreThreshold;
	float ratio = std::clamp(static_cast<float>(gainedSinceUse) / needed, 0.0f, 1.0f);

	m_specialBarFill.setSize(sf::Vector2f(200.f * ratio, 15.f));

	if (m_specialWeapon.active)
	{
		m_specialWeaponText.setString("Special Weapon: ACTIVE");
		m_specialWeaponText.setFillColor(sf::Color::Yellow);
	}
	else if (ratio >= 1.0f)
	{
		m_specialWeaponText.setString("Special Weapon: READY!");
		m_specialWeaponText.setFillColor(sf::Color::Green);
	}
	else
	{
		int remaining = needed - gainedSinceUse;
		m_specialWeaponText.setString("Special Weapon: Locked (" + std::to_string(remaining) + " pts)");
		m_specialWeaponText.setFillColor(sf::Color::Red);
	}

	if (m_debugMode == 1)
	{
		ImGui::Begin("Geometry Wars");

		if (ImGui::BeginTabBar("MainTabs"))
		{
			if (ImGui::BeginTabItem("Systems"))
			{
				ImGui::Checkbox("Movement", &m_isMovement);
				ImGui::Checkbox("Lifespan", &m_isLifeSpan);
				ImGui::Checkbox("Collision", &m_isCollision);
				ImGui::Checkbox("Spawning", &m_isSpawning);
				ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize("Enemy Spawn Interval").x - ImGui::GetStyle().ItemSpacing.x);
				ImGui::SliderInt("Enemy Spawn Interval", &m_enemyConfig.spawnInterval, 1, 300);
				if(ImGui::Button("Spawn Manual"))
				{
					spawnEnemy();
				}
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Entity Manager"))
			{
				if (ImGui::CollapsingHeader("All Entities"))
				{
					for (auto& entity : m_entityManager.getEntities())
					{
						ImGui::PushID(entity->id());
						if (entity->tag() != "player")
						{
							if (ImGui::Button("Delete"))
							{
								entity->destroy();
							}
							ImGui::SameLine();
						}
						ImGui::Text("ID: %d, Type: %s, Pos: (%.1f, %.1f)",
							entity->id(), entity->tag().c_str(),
							entity->has<CTransform>() ? entity->get<CTransform>().position.x : 0.0f,
							entity->has<CTransform>() ? entity->get<CTransform>().position.y : 0.0f);

						ImGui::PopID();
					}
				}
				if (ImGui::CollapsingHeader("Entities By Tag"))
				{
					std::vector<std::string> tags = { "player", "enemy", "small_enemy", "bullet" };
					for (const auto& tag : tags)
					{
						if (ImGui::CollapsingHeader(tag.c_str()))
						{
							auto& entitiesByTag = m_entityManager.getEntities(tag);
							for (auto& entity : entitiesByTag)
							{
								ImGui::PushID(entity->id());
								if (entity->tag() != "player")
								{
									if (ImGui::Button("Delete"))
									{
										entity->destroy();
									}
									ImGui::SameLine();
								}
								ImGui::Text("ID: %d, Pos: (%.1f, %.1f)",
									entity->id(),
									entity->has<CTransform>() ? entity->get<CTransform>().position.x : 0.0f,
									entity->has<CTransform>() ? entity->get<CTransform>().position.y : 0.0f);
								ImGui::PopID();
							}
						}
					}
				}
				if (ImGui::CollapsingHeader("Audio"))
				{
					ImGui::SliderFloat("Master", &m_masterVolume, 0.f, 100.f);
					ImGui::SliderFloat("Music", &m_musicVolume, 0.f, 100.f);
					ImGui::SliderFloat("SFX", &m_sfxVolume, 0.f, 100.f);

					if (ImGui::Button("Mute")) {
						m_masterVolume = 0.f;
					}
				}
				m_music.setVolume(m_musicVolume * (m_masterVolume / 100.f));
				ImGui::EndTabItem();
			}

			ImGui::EndTabBar();
		}

		ImGui::End();
	}
}

void Game::sEnemySpawner()
{
	if (!m_isSpawning) return;
	if(m_currentFrame - m_lastEnemySpawnTime >= m_enemyConfig.spawnInterval)
	{
		spawnEnemy();
		m_lastEnemySpawnTime = m_currentFrame;
	}
}

void Game::sCollision()
{
	if (!m_isCollision) return;
	auto& bullets = m_entityManager.getEntities("bullet");
	auto& enemies = m_entityManager.getEntities("enemy");
	auto& smallEnemies = m_entityManager.getEntities("small_enemy");
	auto player = Game::player();
	for (auto& bullet : bullets)
	{
		if (!bullet->has<CCollision>() || !bullet->has<CTransform>()) continue;
		auto& bTrans = bullet->get<CTransform>();
		auto& bColl = bullet->get<CCollision>();

		for (auto& enemy : enemies)
		{
			if (!enemy->has<CCollision>() || !enemy->has<CTransform>()) continue;
			auto& eTrans = enemy->get<CTransform>();
			auto& eColl = enemy->get<CCollision>();

			float dx = bTrans.position.x - eTrans.position.x;
			float dy = bTrans.position.y - eTrans.position.y;
			float radiusSum = bColl.radius + eColl.radius;

			if (dx * dx + dy * dy > radiusSum * radiusSum)
				continue;

			m_score += enemy->get<CScore>().score;

			spawnSmallEnemies(enemy);

			bullet->destroy();
			enemy->destroy();
			triggerCameraShake(15, 15.0f);
			break;
		}
	}

	for (auto& bullet : bullets)
	{
		if (!bullet->isAlive()) continue;
		if (!bullet->has<CCollision>() || !bullet->has<CTransform>()) continue;

		auto& bTrans = bullet->get<CTransform>();
		auto& bColl = bullet->get<CCollision>();

		for (auto& small : smallEnemies)
		{
			if (!small->has<CCollision>() || !small->has<CTransform>()) continue;
			auto& sTrans = small->get<CTransform>();
			auto& sColl = small->get<CCollision>();

			float dx = bTrans.position.x - sTrans.position.x;
			float dy = bTrans.position.y - sTrans.position.y;
			float radiusSum = bColl.radius + sColl.radius;

			if (dx * dx + dy * dy > radiusSum * radiusSum) continue;

			m_score += small->get<CScore>().score;
			bullet->destroy();
			small->destroy();
			playSFX("explosion");
			triggerCameraShake(10, 8.0f);
			break;
		}
	}

	if (player && player->has<CCollision>() && player->has<CTransform>())
	{
		auto& pTrans = player->get<CTransform>();
		auto& pColl = player->get<CCollision>();

		for (auto& enemy : enemies)
		{
			if (!enemy->has<CCollision>() || !enemy->has<CTransform>()) continue;
			auto& eTrans = enemy->get<CTransform>();
			auto& eColl = enemy->get<CCollision>();

			float dx = pTrans.position.x - eTrans.position.x;
			float dy = pTrans.position.y - eTrans.position.y;
			float radiusSum = pColl.radius + eColl.radius;

			if (dx * dx + dy * dy > radiusSum * radiusSum) continue;

			pTrans.position = { WORLD_WIDTH / 2.f, WORLD_HEIGHT / 2.f };
			spawnSmallEnemies(enemy);
			enemy->destroy();
			player->destroy();
			playSFX("gameover");
			m_gameState = GameState::GameOver;
		}

		for (auto& small : smallEnemies)
		{
			if (!small->has<CCollision>() || !small->has<CTransform>()) continue;
			auto& sTrans = small->get<CTransform>();
			auto& sColl = small->get<CCollision>();

			float dx = pTrans.position.x - sTrans.position.x;
			float dy = pTrans.position.y - sTrans.position.y;
			float radiusSum = pColl.radius + sColl.radius;

			if (dx * dx + dy * dy > radiusSum * radiusSum) continue;

			pTrans.position = { WORLD_WIDTH / 2.f, WORLD_HEIGHT / 2.f };
			player->destroy();
			small->destroy();
			playSFX("gameover");
			m_gameState = GameState::GameOver;
		}
	}
}

void Game::sSpecialWeapon()
{
	if (!m_specialWeapon.active) return;

	spawnSpecialWeapon(player(), mouseWorldPos());

	m_specialWeapon.frameCount++;
	m_specialWeapon.currentAngleOffset += 6.0f;

	if (m_specialWeapon.frameCount >= 60)
	{
		m_specialWeapon.active = false;
	}
}

void Game::spawnPlayer()
{
	auto e = m_entityManager.addEntity("player");
	e->add<CTransform>(
		Vec2f(WORLD_WIDTH / 2.0f, WORLD_HEIGHT / 2.0f),
		Vec2f{0.0f, 0.0f},
		Vec2f{ 1.0f, 1.0f },
		0.0f
	);

	e->add<CShape>(
		m_playerConfig.shapeRadius,
		m_playerConfig.vertexCount,
		sf::Color(m_playerConfig.fillRed, m_playerConfig.fillGreen, m_playerConfig.fillBlue),
		sf::Color(m_playerConfig.outlineRed, m_playerConfig.outlineGreen, m_playerConfig.outlineBlue),
		m_playerConfig.outlineThickness
	);

	e->add<CCollision>(static_cast<float>(m_playerConfig.collisionRadius));

	e->add<CInput>();
}

void Game::spawnEnemy()
{
	auto e = m_entityManager.addEntity("enemy");
	auto windowSize = m_window.getSize();
	int radius = m_enemyConfig.collisionRadius;

	auto playerPos = Game::player()->get<CTransform>().position;
	int safeRadius = 150;

	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<int> xDist(radius, static_cast<int>(WORLD_WIDTH) - radius);
	std::uniform_int_distribution<int> yDist(radius, static_cast<int>(WORLD_HEIGHT) - radius);
	std::uniform_real_distribution<float> speedDist(m_enemyConfig.speedMin, m_enemyConfig.speedMax);
	std::uniform_real_distribution<float> angleDist(0.0f, 360.0f);
	std::uniform_int_distribution<int> vertexDist(m_enemyConfig.vertexMin, m_enemyConfig.vertexMax);
	std::uniform_int_distribution<int> colorDist(0, 255);
	float dx = 0;
	float dy = 0;

	Vec2f enemyPos;
	do {
		enemyPos.x = static_cast<float>(xDist(gen));
		enemyPos.y = static_cast<float>(yDist(gen));
		dx = enemyPos.x - playerPos.x;
		dy = enemyPos.y - playerPos.y;
	} while ((dx * dx + dy * dy) < (safeRadius * safeRadius));

	float angle = angleDist(gen);
	float speed = speedDist(gen);
	float rad = angle * (3.14159265f / 180.0f);
	Vec2f velocity(std::cos(rad) * speed, std::sin(rad) * speed);

	e->add<CTransform>(
		enemyPos,
		velocity,
		Vec2f{1.0f, 1.0f},
		0.0f
	);

	e->add<CShape>(
		m_enemyConfig.shapeRadius,
		static_cast<size_t>(vertexDist(gen)),
		sf::Color(colorDist(gen), colorDist(gen), colorDist(gen)),
		sf::Color(m_enemyConfig.outlineRed, m_enemyConfig.outlineGreen, m_enemyConfig.outlineBlue),
		m_enemyConfig.outlineThickness
	);

	e->add<CCollision>(m_enemyConfig.collisionRadius);
	e->add<CScore>(10);
}

void Game::spawnSmallEnemies(std::shared_ptr<Entity> entity)
{
	float scaledRadius = m_enemyConfig.shapeRadius * 0.5f;
	float scaledCollision = m_enemyConfig.collisionRadius * 0.5f;
	playSFX("explosionBig");
	auto& shape = entity->get<CShape>().circle;
	auto& transform = entity->get<CTransform>();
	int vertexCount = shape.getPointCount();

	const float degToRad = 3.14159265f / 180.0f;
	float rotRad = transform.angle * degToRad;
	float cosR = std::cos(rotRad);
	float sinR = std::sin(rotRad);

	sf::Vector2f origin = shape.getOrigin();
	Vec2f scale(transform.scale.x, transform.scale.y);

	float spawnSpeed = (m_enemyConfig.speedMin + m_enemyConfig.speedMax) * 0.5f;

	for (int i = 0; i < vertexCount; ++i)
	{
		sf::Vector2f localPoint = shape.getPoint(i);

		Vec2f scaled = Vec2f((localPoint.x - origin.x) * scale.x,
			(localPoint.y - origin.y) * scale.y);

		Vec2f rotated;
		rotated.x = scaled.x * cosR - scaled.y * sinR;
		rotated.y = scaled.x * sinR + scaled.y * cosR;

		Vec2f cornerWorld = transform.position + rotated;

		Vec2f direction = cornerWorld - transform.position;
		direction.normalize();

		Vec2f spawnPos = transform.position;

		Vec2f velocity = direction * spawnSpeed;

		auto e = m_entityManager.addEntity("small_enemy");
		e->add<CTransform>(
			spawnPos,
			velocity,
			Vec2f{1.0f, 1.0f},
			0.0f
		);

		e->add<CShape>(
			scaledRadius,
			vertexCount,
			shape.getFillColor(),
			shape.getOutlineColor(),
			shape.getOutlineThickness()
		);

		e->add<CCollision>(scaledCollision);
		e->add<CLifeSpan>(m_enemyConfig.lifespan);
		e->add<CScore>(20);
	}
}

void Game::spawnBullet(std::shared_ptr<Entity> entity, const Vec2f& mousePos)
{
	playSFX("shoot");
	auto e = m_entityManager.addEntity("bullet");
	auto& playerTransform = entity->get<CTransform>();

	Vec2f direction = mousePos - playerTransform.position;
	direction.normalize();

	e->add<CTransform>(
		playerTransform.position,
		direction * m_bulletConfig.speed,
		Vec2f{1.0f, 1.0f},
		0.0f
	);

	e->add<CShape>(
		m_bulletConfig.shapeRadius,
		m_bulletConfig.vertexCount,
		sf::Color(m_bulletConfig.fillRed, m_bulletConfig.fillGreen, m_bulletConfig.fillBlue),
		sf::Color(m_bulletConfig.outlineRed, m_bulletConfig.outlineGreen, m_bulletConfig.outlineBlue),
		m_bulletConfig.outlineThickness
	);

	e->add<CCollision>(m_bulletConfig.collisionRadius);

	e->add<CLifeSpan>(m_bulletConfig.lifespan);
}

void Game::spawnSpecialWeapon(std::shared_ptr<Entity> entity, const Vec2f& mousePos)
{
	auto& shape = entity->get<CShape>().circle;
	auto& playerTransform = entity->get<CTransform>();
	int vertexCount = shape.getPointCount();
	const float degToRad = 3.14159265f / 180.0f;

	float angleOffsetRad = m_specialWeapon.currentAngleOffset * degToRad;

	for (int i = 0; i < vertexCount; ++i)
	{
		sf::Vector2f localPoint = shape.getPoint(i);
		Vec2f direction(localPoint.x - shape.getOrigin().x,
			localPoint.y - shape.getOrigin().y);
		direction.normalize();

		float cosA = cos(angleOffsetRad);
		float sinA = sin(angleOffsetRad);
		Vec2f rotatedDir(
			direction.x * cosA - direction.y * sinA,
			direction.x * sinA + direction.y * cosA
		);

		Vec2f spawnPos = playerTransform.position + rotatedDir * static_cast<float>(m_playerConfig.shapeRadius);
		Vec2f velocity = rotatedDir * m_bulletConfig.speed;

		auto e = m_entityManager.addEntity("bullet");
		e->add<CTransform>(spawnPos, velocity, Vec2f{ 1.0f, 1.0f }, 0.0f);
		e->add<CShape>(
			m_bulletConfig.shapeRadius,
			m_bulletConfig.vertexCount,
			sf::Color(m_bulletConfig.fillRed, m_bulletConfig.fillGreen, m_bulletConfig.fillBlue),
			sf::Color(m_bulletConfig.outlineRed, m_bulletConfig.outlineGreen, m_bulletConfig.outlineBlue),
			m_bulletConfig.outlineThickness
		);
		e->add<CCollision>(static_cast<float>(m_bulletConfig.collisionRadius));
		e->add<CLifeSpan>(m_bulletConfig.lifespan);
	}
}

std::shared_ptr<Entity> Game::player()
{
	auto& players = m_entityManager.getEntities("player");
	if (players.empty()) return nullptr;
	return players.back();
}

void Game::activateSpecialWeapon(std::shared_ptr<Entity> entity, const Vec2f& mousePos)
{
	if (m_specialWeapon.active) return;
	if (m_score - m_specialWeapon.lastScoreUsed < m_specialWeapon.scoreThreshold)
	{
		return;
	}
	m_specialWeapon.active = true;
	m_specialWeapon.frameCount = 0;
	m_specialWeapon.currentAngleOffset = 0.0f;
	m_specialWeapon.lastScoreUsed = m_score;
}

void Game::playSFX(const std::string& name)
{
	auto it = m_soundBuffers.find(name);
	if (it == m_soundBuffers.end()) return;

	const sf::SoundBuffer& buf = it->second;

	for (std::size_t i = 0; i < m_soundPool.size(); ++i)
	{
		std::size_t idx = (m_nextSoundIndex + i) % m_soundPool.size();
		auto& ptr = m_soundPool[idx];

		if (!ptr)
		{
			ptr = std::make_unique<sf::Sound>(buf);
			ptr->setVolume(m_sfxVolume * (m_masterVolume / 100.f));
			ptr->play();
			m_nextSoundIndex = (idx + 1) % m_soundPool.size();
			return;
		}

		if (ptr->getStatus() == sf::Sound::Status::Stopped)
		{
			ptr.reset(std::make_unique<sf::Sound>(buf).release());
			ptr->setVolume(m_sfxVolume * (m_masterVolume / 100.f));
			ptr->play();
			m_nextSoundIndex = (idx + 1) % m_soundPool.size();
			return;
		}
	}

	{
		auto& ptr = m_soundPool[m_nextSoundIndex];
		ptr.reset(std::make_unique<sf::Sound>(buf).release());
		ptr->setVolume(m_sfxVolume * (m_masterVolume / 100.f));
		ptr->play();
		m_nextSoundIndex = (m_nextSoundIndex + 1) % m_soundPool.size();
	}
}

void Game::onWindowResize(sf::Vector2u newSize)
{
	if (!m_isFullscreen)
	{
		sf::Vector2u constrained = enforceAspectRatio(newSize);
		if (newSize != constrained)
		{
			m_window.setSize(constrained);
			return;
		}
		m_windowedWidth = constrained.x;
		m_windowedHeight = constrained.y;
	}

	m_cameraView.setSize({ WORLD_WIDTH, WORLD_HEIGHT });
	m_cameraView.setCenter({ WORLD_WIDTH / 2.f, WORLD_HEIGHT / 2.f });
	m_originalViewCenter = Vec2f(WORLD_WIDTH / 2.f, WORLD_HEIGHT / 2.f);

	repositionUI();
}

void Game::repositionUI()
{
	m_scoreText.setPosition({ 10.f, 10.f });
	m_specialWeaponText.setPosition({ 10.f, 40.f });
	m_specialBarOutline.setPosition({ 10.f, 70.f });
	m_specialBarFill.setPosition({ 10.f, 70.f });

	auto reanchor = [](sf::Text& t, float x, float y) {
		sf::FloatRect b = t.getLocalBounds();
		t.setOrigin({ b.size.x / 2.f, b.size.y / 2.f });
		t.setPosition({ x, y });
		};

	reanchor(m_pausedText, WORLD_WIDTH / 2.f, WORLD_HEIGHT / 2.f);
	reanchor(m_startText, WORLD_WIDTH / 2.f, WORLD_HEIGHT / 2.f);
	reanchor(m_gameOverText, WORLD_WIDTH / 2.f, WORLD_HEIGHT / 2.f - 30.f);
	reanchor(m_retryText, WORLD_WIDTH / 2.f, WORLD_HEIGHT / 2.f + 30.f);
}

void Game::toggleFullscreen()
{
	m_isFullscreen = !m_isFullscreen;

	ImGui::SFML::Shutdown();

	if (m_isFullscreen)
		m_window.create(getBest169FullscreenMode(), "Geometry Wars",
			sf::Style::None, sf::State::Fullscreen);
	else
		m_window.create(sf::VideoMode({ m_windowedWidth, m_windowedHeight }), "Geometry Wars",
			sf::Style::Titlebar | sf::Style::Close | sf::Style::Resize,
			sf::State::Windowed);

	m_window.setKeyRepeatEnabled(false);
	m_window.setFramerateLimit(m_frameLimit);

	auto wSize = m_window.getSize();
	sf::Sprite snap(m_gameBuffer.getTexture());
	snap.setScale({ static_cast<float>(wSize.x) / WORLD_WIDTH,
					static_cast<float>(wSize.y) / WORLD_HEIGHT });
	m_window.clear(sf::Color::Black);
	m_window.draw(snap);
	m_window.display();

	if (!ImGui::SFML::Init(m_window))
		std::cerr << "ImGui re-init failed\n";

	if (m_debugMode)
	{
		ImGui::GetStyle().ScaleAllSizes(2.0f);
		ImGui::GetIO().FontGlobalScale = 2.0f;
	}
	ImGui::SFML::Update(m_window, m_deltaClock.restart());

	onWindowResize(m_window.getSize());
}

sf::Vector2u Game::enforceAspectRatio(sf::Vector2u requested) const
{
	float wdWidth = static_cast<float>(std::max(requested.x, MIN_WIDTH));
	float wdHeight = wdWidth / ASPECT_RATIO;

	float hdHeight = static_cast<float>(std::max(requested.y, MIN_HEIGHT));
	float hdWidth = hdHeight * ASPECT_RATIO;

	sf::Vector2u result;
	result.x = static_cast<unsigned int>((wdWidth + hdWidth) / 2.f);
	result.y = static_cast<unsigned int>((wdHeight + hdHeight) / 2.f);

	// Final minimum clamp as a safety net
	if (result.x < MIN_WIDTH)
	{
		result.x = MIN_WIDTH;
		result.y = static_cast<unsigned int>(MIN_WIDTH / ASPECT_RATIO);
	}
	if (result.y < MIN_HEIGHT)
	{
		result.y = MIN_HEIGHT;
		result.x = static_cast<unsigned int>(MIN_HEIGHT * ASPECT_RATIO);
	}

	return result;
}
sf::VideoMode Game::getBest169FullscreenMode() const
{
	const auto& modes = sf::VideoMode::getFullscreenModes();

	for (const auto& mode : modes)
	{
		float ratio = static_cast<float>(mode.size.x) / static_cast<float>(mode.size.y);
		if (std::abs(ratio - ASPECT_RATIO) < 0.02f)
			return mode;
	}

	return sf::VideoMode::getDesktopMode();
}

Vec2f Game::mouseWorldPos() const
{
	sf::View stableView = m_cameraView;
	stableView.setCenter(sf::Vector2f(m_originalViewCenter.x, m_originalViewCenter.y));

	return Vec2f(m_window.mapPixelToCoords(
		sf::Mouse::getPosition(m_window), stableView));
}

void Game::resetGame()
{
	m_entityManager.clearAll();
	m_score = 0;
	m_lastEnemySpawnTime = 0;
	m_currentFrame = 0;
	m_paused = false;

	// Reset special weapon
	m_specialWeapon.active = false;
	m_specialWeapon.frameCount = 0;
	m_specialWeapon.currentAngleOffset = 0.0f;
	m_specialWeapon.lastScoreUsed = 0;

	// Reset camera shake
	m_shakeFramesRemaining = 0;
	m_totalShakeFrames = 0;
	m_shakeIntensity = 0.0f;

	// Reset game state
	m_gameState = GameState::Playing;

	spawnPlayer();
}