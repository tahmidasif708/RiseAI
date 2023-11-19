/**
* Author: Tahmid Asif
* Assignment: Rise of the AI
* Date due: 2023-11-18, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "stb_image.h"
#include <vector>
#include "Entity.hpp"

#define PLATFORM_COUNT 25
#define ENEMY_COUNT 3

struct GameState {
    Entity *player;
    Entity *platforms;
    Entity *enemies;
};

GameState g_game_state;

SDL_Window* g_display_window;
bool g_game_is_running = true;

ShaderProgram g_shader_program;
glm::mat4 g_view_matrix, g_model_matrix, g_projection_matrix;

const char KENNY_FILEPATH[] = "/Users/tahmidasif/Desktop/RiseAI/Project4/Project4/kenny.png";
const char ALIEN1_FILEPATH[] = "/Users/tahmidasif/Desktop/RiseAI/Project4/Project4/alien1.png";
const char ALIEN2_FILEPATH[] = "/Users/tahmidasif/Desktop/RiseAI/Project4/Project4/alien2.png";
const char ALIEN3_FILEPATH[] = "/Users/tahmidasif/Desktop/RiseAI/Project4/Project4/alien3.png";
const char PLATFORM_FILEPATH[] = "/Users/tahmidasif/Desktop/RiseAI/Project4/Project4/platform.png";


const char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
           F_SHADER_PATH[] = "shaders/fragment_textured.glsl";


GLuint load_texture(const char* filePath) {
    int w, h, n;
    unsigned char* image = stbi_load(filePath, &w, &h, &n, STBI_rgb_alpha);
    
    if (image == NULL) {
        std::cout << "Unable to load image. Make sure the path is correct\n";
        assert(false);
    }
    
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    stbi_image_free(image);
    return textureID;
}


void initialise() {
    SDL_Init(SDL_INIT_VIDEO);
    g_display_window = SDL_CreateWindow("Kenny Vs. Aliens!", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);
    
#ifdef _WINDOWS
    glewInit();
#endif
    
    glViewport(0, 0, 640, 480);
    
    g_shader_program.load(V_SHADER_PATH, F_SHADER_PATH);
    
    g_view_matrix = glm::mat4(1.0f);
    g_model_matrix = glm::mat4(1.0f);
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);
    
    g_shader_program.set_projection_matrix(g_projection_matrix);
    g_shader_program.set_view_matrix(g_view_matrix);
    
    glUseProgram(g_shader_program.get_program_id());
    
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glEnable(GL_BLEND);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    g_game_state.player = new Entity();
    g_game_state.player->entityType = PLAYER;

    GLuint playerTextureID = load_texture(KENNY_FILEPATH);
    g_game_state.player->textureID = playerTextureID;
    g_game_state.player->height = 1.0f;
    g_game_state.player->width = 1.0f;

    g_game_state.player->position = glm::vec3(-4, -1, 0);
    g_game_state.player->movement = glm::vec3(0);
    g_game_state.player->acceleration = glm::vec3(0, -2.81f, 0);
    g_game_state.player->speed = 2.85f;
    g_game_state.player->jumpPower = 3.2f;
    
    g_game_state.enemies = new Entity[ENEMY_COUNT];
    
    GLuint enemyTextureID;
    for (int i = 0; i < ENEMY_COUNT; i ++) {
        g_game_state.enemies[i].entityType = ENEMY;
        
        if (i == 0) {
            g_game_state.enemies[i].position = glm::vec3(4, 1.75f, 0);
            g_game_state.enemies[i].acceleration = glm::vec3(0, -9.81f, 0);
            g_game_state.enemies[i].aiType = PAUSE;
            g_game_state.enemies[i].aiState = HALT;
            enemyTextureID = load_texture(ALIEN1_FILEPATH);
        }
        else if (i == 1) {
            g_game_state.enemies[i].position = glm::vec3(-2.5, 2.75f, 0);
            g_game_state.enemies[i].acceleration = glm::vec3(0, -9.81f, 0);
            g_game_state.enemies[i].aiType = PATROL;
            g_game_state.enemies[i].aiState = ACTIVE;
            enemyTextureID = load_texture(ALIEN2_FILEPATH);
        }
        else {
            g_game_state.enemies[i].position = glm::vec3(-2.0f, -1.25f, 0);
            g_game_state.enemies[i].acceleration = glm::vec3(0, -9.81f, 0);
            g_game_state.enemies[i].aiType = JUMP;
            g_game_state.enemies[i].aiState = HALT;
            enemyTextureID = load_texture(ALIEN3_FILEPATH);
        }
        
        g_game_state.enemies[i].textureID = enemyTextureID;
        g_game_state.enemies[i].height = 1.0f;
        g_game_state.enemies[i].width = 1.0f;
        
        g_game_state.enemies[i].movement = glm::vec3(0);
        g_game_state.enemies[i].speed = 1;
    }

    g_game_state.platforms = new Entity[PLATFORM_COUNT];
    GLuint platformTextureID = load_texture(PLATFORM_FILEPATH);
    
    int x = 0;
    for (int i = 0; i < PLATFORM_COUNT; i++) {
        g_game_state.platforms[i].entityType = PLATFORM;
        g_game_state.platforms[i].textureID = platformTextureID;
        
        if (i < 10) {
            g_game_state.platforms[i].position = glm::vec3(-4.5 + i, -3.25, 0);
        }
        else if (i < 14) {
            x = i - 4;
            g_game_state.platforms[i].position = glm::vec3(-4.5 + x, -2.25, 0);
        }
        else if (i < 17) {
            x = i - 7;
            g_game_state.platforms[i].position = glm::vec3(-4.5 + x, -1.25, 0);
        }
        else if (i < 19) {
            x = i - 9;
            g_game_state.platforms[i].position = glm::vec3(-4.5 + x, -0.25, 0);
        }
        else {
            x = i - 19;
            g_game_state.platforms[i].position = glm::vec3(-4.5 + x, 0.75, 0);
        }
        
        g_game_state.platforms[i].update(0, NULL, NULL, 0, NULL, NULL);
    }
}

void draw_text(ShaderProgram *g_shader_program, GLuint fontTextureID, std::string text, float size, float spacing, glm::vec3 position)
{
    float width = 1.0f / 16.0f;
    float height = 1.0f / 16.0f;

    std::vector<float> vertices;
    std::vector<float> texture_coordinates;

    for (int i = 0; i < text.size(); i++) {
        int index = (int)text[i];
        float offset = (size + spacing) * i;
        float u = (float)(index % 16) / 16.0f;
        float v = (float)(index / 16) / 16.0f;
        
        vertices.insert(vertices.end(), {
            offset + (-0.5f * size), 0.5f * size,
            offset + (-0.5f * size), -0.5f * size,
            offset + (0.5f * size), 0.5f * size,
            offset + (0.5f * size), -0.5f * size,
            offset + (0.5f * size), 0.5f * size,
            offset + (-0.5f * size), -0.5f * size,
        });
        
        texture_coordinates.insert(texture_coordinates.end(), {
            u, v,
            u, v + height,
            u + width, v,
            u + width, v + height,
            u + width, v,
            u, v + height,
        });

    }
    
    glm::mat4 fontModelMatrix = glm::mat4(1.0f);
    g_model_matrix = glm::translate(fontModelMatrix, position);
    g_shader_program->set_model_matrix(fontModelMatrix);
    
    glUseProgram(g_shader_program->get_program_id());

    glVertexAttribPointer(g_shader_program->get_position_attribute(), 2, GL_FLOAT, false, 0, vertices.data());
    glEnableVertexAttribArray(g_shader_program->get_position_attribute());

    glVertexAttribPointer(g_shader_program->get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, texture_coordinates.data());
    glEnableVertexAttribArray(g_shader_program->get_tex_coordinate_attribute());

    glDrawArrays(GL_TRIANGLES, 0, (int)(text.size() * 6));

    glDisableVertexAttribArray(g_shader_program->get_position_attribute());
    glDisableVertexAttribArray(g_shader_program->get_tex_coordinate_attribute());
}

void process_input() {
    g_game_state.player->movement = glm::vec3(0);
    
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
            case SDL_WINDOWEVENT_CLOSE:
                g_game_is_running = false;
                break;
                
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                    case SDLK_LEFT:
                        break;
                        
                    case SDLK_RIGHT:
                        break;
                        
                    case SDLK_SPACE:
                        if (g_game_state.player->collidedBottom) {
                            g_game_state.player->jump = true;
                        }
                        break;
                }
                break;
        }
    }
    
    const Uint8 *keys = SDL_GetKeyboardState(NULL);

    if (keys[SDL_SCANCODE_LEFT]) {
        g_game_state.player->movement.x = -1.0f;
    }
    else if (keys[SDL_SCANCODE_RIGHT]) {
        g_game_state.player->movement.x = 1.0f;
    }

    if (glm::length(g_game_state.player->movement) > 1.0f) {
        g_game_state.player->movement = glm::normalize(g_game_state.player->movement);
    }

}

#define FIXED_TIMESTEP 0.0166666f
float lastTicks = 0;
float accumulator = 0.0f;

void update() {
    float ticks = (float)SDL_GetTicks() / 1000.0f;
    float deltaTime = ticks - lastTicks;
    lastTicks = ticks;

    deltaTime += accumulator;
    if (deltaTime < FIXED_TIMESTEP) {
        accumulator = deltaTime;
        return;
    }

    while (deltaTime >= FIXED_TIMESTEP) {
        g_game_state.player->update(FIXED_TIMESTEP, g_game_state.player, g_game_state.platforms, PLATFORM_COUNT, g_game_state.enemies, ENEMY_COUNT);
        
        for (int i = 0; i < ENEMY_COUNT; i++) {
            g_game_state.enemies[i].update(FIXED_TIMESTEP, g_game_state.player, g_game_state.platforms, PLATFORM_COUNT, g_game_state.enemies, ENEMY_COUNT);
        }
        
        deltaTime -= FIXED_TIMESTEP;
    }
    
    accumulator = deltaTime;
}

void render() {
    glClear(GL_COLOR_BUFFER_BIT);

    for (int i = 0; i < PLATFORM_COUNT; i++) {
        g_game_state.platforms[i].render(&g_shader_program);
    }
    
    for (int i = 0; i < ENEMY_COUNT; i++) {
        g_game_state.enemies[i].render(&g_shader_program);
    }
    
    g_game_state.player->render(&g_shader_program);
    
    int alien_defeat = 0;
    for (int i = 0; i < ENEMY_COUNT; i++) {
        if (g_game_state.enemies[i].was_defeated) {
            alien_defeat += 1;
        }
    }
    std::cout << "Aliens Defeated: " << alien_defeat << "\n";
    if (alien_defeat == ENEMY_COUNT) {
        g_game_state.player->defeated_aliens = true;
    }
    
    if (g_game_state.player->defeated_aliens == true) {
        draw_text(&g_shader_program, load_texture("font1.png"), "You Win!", 0.5f, -0.25f, glm::vec3(1.0, 0, 0));
        std::cout << "win\n";
        g_game_state.player->isActive = false;
    }
    else if (g_game_state.player->was_defeated == true) {
        draw_text(&g_shader_program, load_texture("font1.png"), "Game Over", 0.5f, -0.25f, glm::vec3(1.0, 0, 0));
        std::cout << "lose\n";
        g_game_state.player->isActive = false;
    }
    
    SDL_GL_SwapWindow(g_display_window);
}


void shutdown() {
    SDL_Quit();
}

int main(int argc, char* argv[]) {
    initialise();
    
    while (g_game_is_running) {
        process_input();
        update();
        render();
    }
    
    shutdown();
    return 0;
}
