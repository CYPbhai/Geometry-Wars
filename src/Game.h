#pragma once
#include <fstream>
#include <random>

#include "Entity.hpp"
#include "EntityManager.hpp"

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

#include "imgui.h"
#include "imgui-SFML.h"
#include "imgui_stdlib.h"

// Configuration for the player entity
struct PlayerConfig
{
	int shapeRadius;        // Radius of the player shape
	int collisionRadius;    // Radius used for collision detection
	int fillRed, fillGreen, fillBlue;  // Fill color (RGB)
	int outlineRed, outlineGreen, outlineBlue; // Outline color (RGB)
	int outlineThickness;   // Outline thickness
	int vertexCount;        // Number of vertices (e.g. 3 for triangle)
	float speed;            // Movement speed
};

// Configuration for enemy entities
struct EnemyConfig
{
	int shapeRadius;        // Radius of enemy shape
	int collisionRadius;    // Radius used for collision detection
	int outlineRed, outlineGreen, outlineBlue; // Outline color (RGB)
	int outlineThickness;   // Outline thickness
	int vertexMin;          // Minimum number of vertices (random range)
	int vertexMax;          // Maximum number of vertices (random range)
	int lifespan;           // Frames before small enemy disappears
	int spawnInterval;      // Interval (frames or ms) between enemy spawns
	float speedMin;         // Minimum speed
	float speedMax;         // Maximum speed
};

// Configuration for bullets fired by the player
struct BulletConfig
{
	int shapeRadius;        // Radius of bullet shape
	int collisionRadius;    // Radius used for collision detection
	int fillRed, fillGreen, fillBlue;  // Fill color (RGB)
	int outlineRed, outlineGreen, outlineBlue; // Outline color (RGB)
	int outlineThickness;   // Outline thickness
	int vertexCount;        // Number of vertices (for shape)
	int lifespan;           // Frames before bullet disappears
	float speed;            // Bullet speed
};

enum class GameState
{
	StartMenu,
	Playing,
	GameOver
};

// State for special weapon usage
struct SpecialWeaponState {
	bool active = false;
	int frameCount = 0;
	float currentAngleOffset = 0.0f; // degrees

	int lastScoreUsed = 0;
	int scoreThreshold = 2500;
};

class Game
{
	sf::RenderTexture	m_gameBuffer;   
	int					m_frameLimit = 60;
	sf::RenderWindow	m_window;
	EntityManager		m_entityManager;
	sf::Font			m_font;
	sf::Text			m_text;
	PlayerConfig		m_playerConfig;
	EnemyConfig			m_enemyConfig;
	BulletConfig		m_bulletConfig;
	SpecialWeaponState	m_specialWeapon;
	sf::Clock			m_deltaClock;
	int					m_score = 0;
	int					m_currentFrame = 0;
	int					m_lastEnemySpawnTime = 0;
	bool				m_paused = false;

	bool				m_debugMode = false;

	static constexpr float WORLD_WIDTH = 1920.f;
	static constexpr float WORLD_HEIGHT = 1080.f;

	// Resize Window
	bool m_isFullscreen = false;
	unsigned int m_windowedWidth = 1280;
	unsigned int m_windowedHeight = 720;

	// Aspect ratio constraint
	static constexpr unsigned int MIN_WIDTH = 854;   // 480p in 16:9
	static constexpr unsigned int MIN_HEIGHT = 480;
	static constexpr float        ASPECT_RATIO = 16.f / 9.f;

	// GUI
	sf::Text			m_scoreText;
	sf::Text			m_specialWeaponText;
	sf::Text			m_pausedText;
	sf::RectangleShape	m_specialBarOutline;
	sf::RectangleShape	m_specialBarFill;

	// Camera
	sf::View			m_cameraView;
	int					m_shakeFramesRemaining = 0;
	int					m_totalShakeFrames = 0;
	float				m_shakeIntensity = 0.0f;
	Vec2f				m_originalViewCenter;

	// Game State
	GameState			m_gameState = GameState::StartMenu;
	sf::Text			m_startText;
	sf::Text			m_gameOverText;
	sf::Text			m_retryText;

	// audio settings
	float				m_masterVolume = 100.f;
	float				m_musicVolume = 20.0f;
	float				m_sfxVolume = 100.0;

	// system toggles
	bool				m_isMovement = true;
	bool				m_isLifeSpan = true;
	bool				m_isCollision = true;
	bool				m_isSpawning = true;

	// audio
	sf::Music m_music;
	std::unordered_map<std::string, sf::SoundBuffer> m_soundBuffers;
	std::vector<std::unique_ptr<sf::Sound>> m_soundPool;
	std::size_t m_nextSoundIndex = 0;
	static constexpr std::size_t SOUND_POOL_SIZE = 32;

	Game() = default;

	void init(const std::string& path);

	void sMovement();
	void sUserInput();
	void sLifeSpan();
	void sRender();
	void sGUI();
	void sEnemySpawner();
	void sCollision();
	void sSpecialWeapon();

	void cameraShake();
	void spawnPlayer();
	void spawnEnemy();
	void spawnSmallEnemies(std::shared_ptr<Entity> entity);
	void spawnBullet(std::shared_ptr<Entity> entity, const Vec2f & mousePos);
	void spawnSpecialWeapon(std::shared_ptr<Entity> entity, const Vec2f& mousePos);
	void activateSpecialWeapon(std::shared_ptr<Entity> entity, const Vec2f& mousePos);
	void triggerCameraShake(int durationFrames, float intensity);
	std::shared_ptr<Entity> player();
	void resetGame();
	void playSFX(const std::string& name);

	void onWindowResize(sf::Vector2u newSize);
	void repositionUI();
	void toggleFullscreen();

	sf::Vector2u enforceAspectRatio(sf::Vector2u requested) const;
	sf::VideoMode getBest169FullscreenMode() const;
	Vec2f mouseWorldPos() const;
public:
	Game(const std::string& config);
	
	void run();
};