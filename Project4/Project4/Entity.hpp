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

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"

enum EntityType {PLAYER, PLATFORM, ENEMY};

enum AIType { WALKER, PAUSE, PATROL, JUMP };
enum AIState { HALT, ACTIVE };

class Entity {
public:
    EntityType entityType;
    AIType aiType;
    AIState aiState;
    
    glm::vec3 position;
    glm::vec3 movement;
    glm::vec3 acceleration;
    glm::vec3 velocity;
    float speed;
    
    float width = 1;
    float height = 1;
    
    float jump = false;
    float jumpPower = 0;
        
    GLuint textureID;
    
    glm::mat4 g_model_matrix;
    
    bool isActive = true;
    
    bool collidedTop = false;
    bool collidedBottom = false;
    bool collidedLeft = false;
    bool collidedRight = false;
    
    bool defeated_aliens = false;
    bool was_defeated = false;
    
    Entity();
    
    bool CheckCollision(Entity *other);
    void CheckCollisionsY(Entity *objects, int objectCount);
    void CheckCollisionsX(Entity *objects, int objectCount);
    void CheckEnemyCollision(Entity *enemies, int enemyCount);
    
    void update(float deltaTime, Entity *player, Entity *platforms, int platformCount, Entity *enemies, int enemyCount);
    void render(ShaderProgram *g_shader_program);
    void draw_sprite(ShaderProgram *g_shader_program, GLuint textureID, int index);
    
    void AI(Entity *player);
    void AIWALKER();
    void AIPAUSE(Entity *player);
    void AIPatrol();
    void AIJump(Entity *player);
};
