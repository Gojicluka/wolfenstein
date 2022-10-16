#pragma once
#include <iostream>
#include <sdl.h>
#include <sdl_image.h>
#include <vector>
enum class GameState {PLAY, EXIT};

class Game
{
public:
	Game();
	~Game();

	GameState gameState;
	SDL_Texture* loadTexture(const char* filePath);
	void render(SDL_Texture* tex);
	void gameLoop();
private:
	void handleEvents();
	void display();
	void clear();
	
	SDL_Window* window;
	SDL_Renderer* renderer;

	int screenWidth = 1000;
	int screenHeight = 1000;
	
};

