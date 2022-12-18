#include <iostream>
using namespace std;
#include <SDL.h>
#include <vector>
#include <string>

#define WIN_WIDTH 512
#define WIN_HEIGHT 512

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

class Player {
public:
    float x, y, w, h;
    float view_direction = M_PI/2;//1.53;
    float FOV = M_PI / 3;
    Player(float x,float y,float w,float h) {
        this->x = x;
        this->y = y;
        this->w = w;
        this->h = h;
    }
};
class Texture {
public:
    int size=0,cnt=0;
    std::vector<uint32_t> arr;
    Texture() {}
};

struct Point {
    double x, y;
};
struct Ray {
    Point start;
    double direction;
};
struct Line {
    Point p1, p2;
};

pair<Point,double> horizontalIntersection(Ray r, Point p1, Point p2) {
    r.direction = fmod(r.direction, 2 * M_PI);
    double ogDirection = r.direction;
    if (r.direction <= M_PI)
        r.direction = abs(M_PI / 2 - r.direction);
    else if (r.direction <= 2 * M_PI)
        r.direction = abs(3 * M_PI / 2 - r.direction);

    if (p1.y < r.start.y) {
        p1.y += 32; p2.y += 32;
    }
    //Fiksiran je y
    double yKor = p1.y;
    double yDuzina = abs(r.start.y - p1.y);
    double xDuzina = yDuzina * abs(tan(r.direction));
    double xKor = -1;
    if (ogDirection <= M_PI / 2 || ogDirection >= (M_PI * 3 / 2))
        xKor = r.start.x + xDuzina;
    else
        xKor = r.start.x - xDuzina;
    double distance = sqrt(pow(xDuzina, 2) + pow(yDuzina, 2));
    if (p1.x <= xKor && xKor <= p2.x)
        return {{ xKor,yKor }, distance};
    else 
        return { { -1,-1 } ,distance};
}

pair<Point,double> verticalIntersection(Ray r, Point p1, Point p2) {
    r.direction = fmod(r.direction, 2 * M_PI);
    double ogDirection = r.direction;
    if (( r.direction>=0&& r.direction <= M_PI / 2)|| (r.direction >= M_PI && r.direction <= 3*M_PI / 2)) 
        r.direction = fmod(r.direction, M_PI);
    else  
        r.direction = abs(M_PI-fmod(r.direction, M_PI));
    
    if (p1.x < r.start.x) {
        p1.x += 32; p2.x += 32;
    }
    //Fiksiran je x
    double xKor = p1.x;
    double xDuzina = abs(r.start.x - p1.x);
    double yDuzina = xDuzina * abs(tan(r.direction));
    double yKor = -1;

    if (ogDirection >= M_PI  && ogDirection <= 2*M_PI)
        yKor = r.start.y - yDuzina;
    else 
        yKor = r.start.y + yDuzina;

    double distance = sqrt(pow(xDuzina,2)+pow(yDuzina,2));
    if (p1.y <= yKor && yKor <= p2.y) 
        return { { xKor,yKor } ,distance};
    else 
        return {{ -1,-1 },distance };
}

//Textura mora biti kvadrat
bool load_texture(const std::string filename, Texture& texture) {
    int nchannels = -1, w, h;
    unsigned char* pixmap = stbi_load(filename.c_str(), &w, &h, &nchannels, 0);
    if (!pixmap) {
        std::cerr << "Error: can not load the textures" << std::endl;
        return false;
    }
    if (!(nchannels==4||nchannels==3)) {
        std::cerr << "Error: the texture must be a 32 bit image" << std::endl;
        stbi_image_free(pixmap);
        return false;
    }
    texture.cnt = w / h;
    texture.size = w / texture.cnt;
    texture.arr = std::vector<uint32_t>(w * h*4);
    for (int j = 0; j < h; j++) {
        for (int i = 0; i < w; i++) {
            texture.arr[(i + j * h)*4] = pixmap[(i + j * w) * nchannels + 0];
            texture.arr[(i + j * h) * 4 +1]  = pixmap[(i + j * w) * nchannels + 1];
            texture.arr[(i + j * h) * 4 +2] = pixmap[(i + j * w) * nchannels + 2];
            if (nchannels == 4) texture.arr[(i + j * h) * 4 + 3] = pixmap[(i + j * w) * nchannels + 3];
            else texture.arr[(i + j * h) * 4 + 3] = 0;
        }
    }
    stbi_image_free(pixmap);
    return true;
}

void draw_rectangle(uint8_t* img, size_t img_w, size_t img_h,size_t x, size_t y, size_t w, size_t h, char r,char g,char b,char a) {
    //assert(img.size() == img_w * img_h);
    for (size_t i = 0; i < w; i++) {
        for (size_t j = 0; j < h; j++) {
            size_t cx = x + i;
            size_t cy = y + j;
            img[cx * 4 + cy * img_w * 4] = r;
            img[cx * 4 + cy * img_w * 4 + 1] = g;
            img[cx * 4 + cy * img_w * 4 + 2] = b;
            img[cx * 4 + cy * img_w * 4 + 3] = a;
        }
    }
}
template <typename T> int sgn(T val) {
    return (T(0) < val) - (val < T(0));
}
void draw(uint8_t* pixels, uint8_t* pixels2,const char* map,int map_w,int map_h,Player& player, SDL_Texture* texture, SDL_Texture* sdlTexture2,Texture& wall) {
    memset(pixels, 0, WIN_WIDTH * WIN_HEIGHT * 4 * sizeof(uint8_t));
    memset(pixels2, 0, WIN_WIDTH * WIN_HEIGHT * 4 * sizeof(uint8_t));
    int win_h = WIN_HEIGHT, win_w = WIN_WIDTH;
    for (size_t i = 0; i < win_h; i++) { // fill the screen with color gradients
        for (size_t j = 0; j < win_w; j++) {
            pixels[i * win_w * 4 + j * 4] = 255 * i / float(win_h);
            pixels[i * win_w * 4 + j * 4 + 1] = 0;
            pixels[i * win_w * 4 + j * 4 + 2] = 255 * j / float(win_w);
        }
    }
    const size_t rect_w = win_w / map_w;
    const size_t rect_h = win_h / map_h;
    for (size_t j = 0; j < map_h; j++) { // draw the map
        for (size_t i = 0; i < map_w; i++) {
            if (map[i + j * map_w] == ' ') continue; // skip empty spaces
            size_t rect_x = i * rect_w;
            size_t rect_y = j * rect_h;
            int r = rand() % 255;
            draw_rectangle(pixels, win_w, win_h, rect_x, rect_y, rect_w, rect_h,  60, 179,113,0 );
        }
    }

    draw_rectangle(pixels, win_w, win_h, player.x * rect_w, player.y * rect_h, 5, 5,  255,255,255,0 );
    //castovanje
    for (int i = 0; i < win_w; i++) {
        float angle = (player.view_direction - player.FOV / 2 + player.FOV * i / float(win_w));
        if (angle < 0) angle = 2*M_PI + angle;
        else if (angle > 2 * M_PI) angle = angle - 2 * M_PI;

        float dy=0, dx=0;
        pair<int, int> step = { sgn<float>(cos(angle)),sgn<float>(sin(angle)) };
        pair<double,double> dir = { cos(angle),sin(angle) };
        pair<double, double> stepSize = { sqrt(1 + (dir.second / dir.first) *(dir.second / dir.first)),sqrt(1 + (dir.first / dir.second) *(dir.first / dir.second)) };
        //pair<double, double> vMapCheck = { player.x,player.y };
        pair<double, double> vMapCheck = { (int)player.x,(int)player.y };
        pair<double, double> vRayLength1D = { 0,0 };
        if (step.first < 0) {
            vRayLength1D.first = (player.x - double(vMapCheck.first)) * stepSize.first;
        }
        else {
            vRayLength1D.first = (double(vMapCheck.first + 1) - player.x) * stepSize.first;
        }
        if (step.second < 0) {
            vRayLength1D.second= (player.y - double(vMapCheck.second)) * stepSize.second;
        }
        else {
            vRayLength1D.second = (double(vMapCheck.second + 1) - player.y) * stepSize.second;
        }
        //VrayDir da ide do kraja mape
        bool bTileFound = false;
        float fMaxDistance = 100.0f;
        float fDistance = 0.0f;
        while (!bTileFound && fDistance < fMaxDistance)
        {
            // Walk along shortest path
            if (vRayLength1D.first < vRayLength1D.second)
            {
                vMapCheck.first += step.first;
                fDistance = vRayLength1D.first;
                vRayLength1D.first += stepSize.first;
            }
            else
            {
                vMapCheck.second += step.second;
                fDistance = vRayLength1D.second; 
                vRayLength1D.second += stepSize.second;
            }
            int pix_x = vMapCheck.first*rect_w;
            int pix_y = vMapCheck.second*rect_h;
            pixels[pix_x * 4 + pix_y * win_w * 4] = 255;
            pixels[pix_x * 4 + pix_y * win_w * 4 + 1] = 255;
            pixels[pix_x * 4 + pix_y * win_w * 4 + 2] = 255;
            pixels[pix_x * 4 + pix_y * win_w * 4 + 3] = 0;
            int a = static_cast<int>(vMapCheck.second);
            if (vMapCheck.first >= 0 && vMapCheck.first < 16 && vMapCheck.second >= 0 && vMapCheck.second < 16)
            {
                if (map[(int)(vMapCheck.second) * 16 + (int)(vMapCheck.first)] != ' ')
                {
                    vMapCheck.first = ceil(vMapCheck.first);
                    bTileFound = true;
                }
            }
        }
        Point p = {-1,-1};
        double distance;
        Point nadjenP = { ((int)vMapCheck.first) * rect_w,((int)vMapCheck.second) * rect_h };
        if (bTileFound)
        {
            pair<Point, double> par = horizontalIntersection({ {(double)player.x * rect_w,(double)player.y * rect_h},angle }, nadjenP, { nadjenP.x + rect_w,nadjenP.y });
            pair<Point, double> par2 = verticalIntersection({ {(double)player.x * rect_w,(double)player.y * rect_h},angle }, nadjenP, { nadjenP.x ,nadjenP.y + rect_h });
            if (par2.second < par.second&&par2.first.x!=-1&&par.first.x!=-1||par.first.x==-1) {
                p = par2.first;
                distance = par2.second;
            }
            else {
                p = par.first;
                distance = par.second;
            }
        }
        if (p.x == -1) {
            continue;
        }
        //CRTANJE ZIDA
        int pix_x = p.x;
        int pix_y = p.y;
        float cx = p.x/32;
        float cy = p.y/32; 
        float column_height = (float)win_h / ((distance/32)* cos(angle - player.view_direction));
        float hitx = cx - floor(cx + .5); 
        float hity = cy - floor(cy + .5); 
        int x_texcoord = hitx * wall.size;
        if (std::abs(hity) > std::abs(hitx)) {
            x_texcoord = wall.size- hity * wall.size;
        }
                
        if (x_texcoord < 0) x_texcoord += wall.size;
        if (x_texcoord > 255) x_texcoord -= wall.size;
        pix_x = win_w  + i;
        for (size_t j = 0; j < column_height; j++) {
            size_t texture_x2 = 0 * wall.size + x_texcoord;
            size_t texture_y2 = (j * wall.size) / column_height;
            pix_y = j + win_h / 2 - column_height / 2;
            if (pix_y < 0 || pix_y >= (int)win_h) continue;
            pixels2[(pix_x + pix_y * win_w)*4] = wall.arr[(texture_x2 + texture_y2 * wall.size) * 4];
            pixels2[(pix_x + pix_y * win_w)*4+1] = wall.arr[(texture_x2 + texture_y2 * wall.size) * 4+1];
            pixels2[(pix_x + pix_y * win_w)*4+2] = wall.arr[(texture_x2 + texture_y2 * wall.size) * 4+2];
            pixels2[(pix_x + pix_y * win_w)*4+3] = wall.arr[(texture_x2 + texture_y2 * wall.size) * 4+3];
        }
        
        
        
    }
    //// update texture with new data
    int texture_pitch = 0;
    void* texture_pixels = NULL;
    if (SDL_LockTexture(texture, NULL, &texture_pixels, &texture_pitch) != 0) {
        SDL_Log("Unable to lock texture: %s", SDL_GetError());
    }
    else {
        memcpy(texture_pixels, pixels, texture_pitch * WIN_HEIGHT);
    }
    SDL_UnlockTexture(texture);
    int texture_pitch2 = 0;
    void* texture_pixels2 = NULL;
    SDL_LockTexture(sdlTexture2, NULL, &texture_pixels2, &texture_pitch2);
    memcpy(texture_pixels2, pixels2, texture_pitch2 * WIN_HEIGHT);
    SDL_UnlockTexture(sdlTexture2);
}
int main(int argc, char** argv) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        return 1;
    }
    SDL_Window* window = SDL_CreateWindow("2d",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WIN_WIDTH *2,
        WIN_HEIGHT *2,
        SDL_WINDOW_RESIZABLE);
    SDL_Window* window2 = SDL_CreateWindow("3d",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WIN_WIDTH * 2,
        WIN_HEIGHT * 2,
        SDL_WINDOW_RESIZABLE);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    SDL_Renderer* renderer2 = SDL_CreateRenderer(window2, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    SDL_RenderSetLogicalSize(renderer, WIN_WIDTH, WIN_HEIGHT);
    SDL_RenderSetLogicalSize(renderer2, WIN_WIDTH, WIN_HEIGHT);
    SDL_Texture* sdltexture = SDL_CreateTexture(renderer,SDL_PIXELFORMAT_RGBA32,SDL_TEXTUREACCESS_STREAMING, WIN_WIDTH,WIN_HEIGHT);
    SDL_Texture* sdlTexture2 = SDL_CreateTexture(renderer2, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, WIN_WIDTH, WIN_HEIGHT);
    if (sdltexture == NULL) {
        SDL_Log("Unable to create sdltexture: %s", SDL_GetError());
        return 1;
    }

    // array of pixels
    static uint8_t pixels[WIN_WIDTH * WIN_HEIGHT * 4] = { 0 };
    static uint8_t pixels2[WIN_WIDTH * WIN_HEIGHT * 4] = { 0 };
    const size_t map_w = 16; // map width
    const size_t map_h = 16; // map height
    const char map[] =  "1111111111111111"\
                        "1              1"\
                        "1      11111   1"\
                        "1     1        1"\
                        "1     3  1111111"\
                        "1     3        1"\
                        "1   11111      1"\
                        "1   1   11111  1"\
                        "1   1   1      1"\
                        "1   1   1  11111"\
                        "1       1      1"\
                        "1       1      1"\
                        "1       1      1"\
                        "1 1111111      1"\
                        "1              1"\
                        "1111111111111111";
    Player player(2.3, 2.5, 5, 5);
    //Player player(2, 2, 5, 5);
    Texture wall;
    if (!load_texture("milodrip.png", wall)) {
        std::cerr << "Failed to load wall textures" << std::endl;
        return -1;
    }
    
    // main loop
    bool should_quit = false;
    SDL_Event e;
    while (!should_quit) {
        Uint64 start = SDL_GetPerformanceCounter();
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                should_quit = true;
            }
            else if (e.type == SDL_KEYDOWN){
                switch (e.key.keysym.scancode) {
                case SDL_SCANCODE_P:
                    should_quit = true;
                    break;
                case SDL_SCANCODE_D:
                    player.view_direction += 0.1;
                    if (player.view_direction >= 2 * M_PI) player.view_direction = 0;
                    break;
                case SDL_SCANCODE_A:
                    player.view_direction -= 0.1;
                    if (player.view_direction <=0) player.view_direction = 2*M_PI;
                    break;
                }
            }
        }
        draw(pixels, pixels2, map, map_w, map_h, player,sdltexture,sdlTexture2,wall);
        // render on screen
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, sdltexture, NULL, NULL);
        SDL_RenderPresent(renderer);

        SDL_RenderClear(renderer2);
        SDL_RenderCopy(renderer2, sdlTexture2, NULL, NULL);
        SDL_RenderPresent(renderer2);
        //SDL_Delay(1000 / 60);
        Uint64 end = SDL_GetPerformanceCounter();

        float elapsed = (end - start) / (float)SDL_GetPerformanceFrequency();
        cout << "Current FPS: " << to_string(1.0f / elapsed) << "\n";
        //cout << "Current Direction: " << player.view_direction << "\n";
    }

    SDL_DestroyTexture(sdltexture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_DestroyTexture(sdlTexture2);
    SDL_DestroyRenderer(renderer2);
    SDL_DestroyWindow(window2);
    SDL_Quit();
    return 0;
}