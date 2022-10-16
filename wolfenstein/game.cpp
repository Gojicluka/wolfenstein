#include "Game.h"

Game::Game() {
	window = nullptr;
	renderer = nullptr;
	gameState = GameState::PLAY;
	SDL_Init(SDL_INIT_EVERYTHING);
	window = SDL_CreateWindow("Wolfenstein", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screenWidth, screenHeight, SDL_WINDOW_SHOWN);
	renderer = SDL_CreateRenderer(window, -1, 0);
};
Game::~Game() {
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
};

SDL_Texture* Game::loadTexture(const char* filePath) {
	SDL_Texture* tex = NULL;
	tex = IMG_LoadTexture(renderer,filePath);
	if (tex == NULL) std::cout << "img can't load, Error:" << SDL_GetError()<<"\n";
	return tex;
}
void Game::render(SDL_Texture* tex) {
	/*SDL_Rect src;
	src.x = 0;
	src.y = 0;
	src.w = 128;
	src.h = 128;
	SDL_Rect dst;
	dst.x = 0;
	dst.y = 0;
	dst.w = 128;
	dst.h = 128;*/
	SDL_RenderCopy(renderer, tex, NULL, NULL);
}

void triangle(SDL_Renderer* rend) {
	const std::vector< SDL_Vertex > verts =
	{
		{ SDL_FPoint{ 0,0 }, SDL_Color{ 204, 115, 12, 255 }, SDL_FPoint{ 0 }, },
		{ SDL_FPoint{ 0,500 }, SDL_Color{ 204, 115, 12, 255}, SDL_FPoint{ 0 }, },
		{ SDL_FPoint{ 1000,0}, SDL_Color{ 204, 115, 12, 255}, SDL_FPoint{ 0 }, },
		{ SDL_FPoint{ 1000,500 }, SDL_Color{ 204, 115, 12, 255}, SDL_FPoint{ 0 }, }
	};
	SDL_RenderGeometry(rend, nullptr, verts.data(), verts.size(), nullptr, 0);
}

void Game::gameLoop() {
	SDL_Texture* finger = loadTexture("img/finger.jpg");
	while (gameState != GameState::EXIT) {
		handleEvents();
		//render(finger);
		triangle(renderer);
		display();
		clear();
	}
}

void Game::handleEvents() {
	SDL_Event evnt;
	while (SDL_PollEvent(&evnt)) {
		switch (evnt.type) {
		case SDL_QUIT:
			gameState = GameState::EXIT;
			break;
		}
	}
}
void Game::display(){
	SDL_RenderPresent(renderer);
}
void Game::clear(){
	SDL_RenderClear(renderer);
}

