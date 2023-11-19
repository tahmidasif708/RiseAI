/**
* Author: Tahmid Asif
* Assignment: Rise of the AI
* Date due: 2023-11-18, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/
#include "Entity.hpp"

Entity::Entity()
{
    position = glm::vec3(0);
    movement = glm::vec3(0);
    acceleration = glm::vec3(0);
    velocity = glm::vec3(0);
    speed = 0;
    
    g_model_matrix = glm::mat4(1.0f);
}

bool Entity::CheckCollision(Entity *other) {
    if (isActive == false || other->isActive == false) return false;
    
    float xdist = fabs(position.x - other->position.x) - ((width + other->width) / 2.0);
    float ydist = fabs(position.y - other->position.y) - ((height + other->height) / 2.0);
    
    if (xdist < 0 && ydist < 0) return true;
    
    return false;
}

void Entity::CheckCollisionsY(Entity *objects, int objectCount)
{
    for (int i = 0; i < objectCount; i++)
    {
        Entity *object = &objects[i];

        if (CheckCollision(object))
        {
            float ydist = fabs(position.y - object->position.y);
            float penetrationY = fabs(ydist - (height / 2.0f) - (object->height / 2.0f));
            
            if (velocity.y > 0) {
                position.y -= penetrationY;
                velocity.y = 0;
                collidedTop = true;
            }
            else if (velocity.y < 0) {
                position.y += penetrationY;
                velocity.y = 0;
                collidedBottom = true;
            }
        }
    }
}

void Entity::CheckCollisionsX(Entity *objects, int objectCount)
{
    for (int i = 0; i < objectCount; i++)
    {
        Entity *object = &objects[i];

        if (CheckCollision(object))
        {
            float xdist = fabs(position.x - object->position.x);
            float penetrationX = fabs(xdist - (width / 2.0f) - (object->width / 2.0f));
            if (velocity.x > 0) {
                position.x -= penetrationX;
                velocity.x = 0;
                collidedRight = true;
            }
            else if (velocity.x < 0) {
                position.x += penetrationX;
                velocity.x = 0;
                collidedLeft = true;
            }
        }
    }
}

void Entity::CheckEnemyCollision(Entity *enemies, int enemyCount) {
    for (int i = 0; i < enemyCount; i++)
    {
        Entity *enemy = &enemies[i];

        if (CheckCollision(enemy)) {
            if (velocity.y < 0) {
                if (enemy->position.y <= position.y) {
                    enemy->was_defeated = true;
                    enemy->isActive = false;
                }
            }
            else {
                was_defeated = true;
            }
        }
    }
}

void Entity::AI(Entity *player) {
    switch(aiType) {
        case WALKER:
            AIWALKER();
            break;
            
        case PAUSE:
            AIPAUSE(player);
            break;
            
        case PATROL:
            AIPatrol();
            break;
            
        case JUMP:
            AIJump(player);
            break;
            
    }
}

void Entity::AIWALKER() {
    movement = glm::vec3(-1, 0, 0);
}

void Entity::AIPAUSE(Entity *player) {
    switch(aiState) {
        case HALT:
            if (glm::distance(position, player->position) < 3.0f) {
                aiState = ACTIVE;
            }
            break;
            
        case ACTIVE:
            if (glm::distance(position, player->position) > 3.5f) {
                aiState = HALT;
            }
            
            if (glm::distance(position, player->position) < 2.5f) {
                speed = 1.45;
            }
            else {
                speed = 1;
            }
            
            if (player->position.x < position.x) {
                movement = glm::vec3(-1, 0, 0);
            } else {
                movement = glm::vec3(1, 0, 0);
            }
            
            break;
    }
}

void Entity::AIPatrol() {
    switch(aiState) {
        case HALT:
            break;
            
        case ACTIVE:
            if (position.x < 0.5) {
                movement = glm::vec3(1, 0, 0);
            }
            else {
                position.x = -4.5f;
                movement = glm::vec3(1, 0, 0);
            }
            movement = glm::vec3(1, 0, 0);
            break;
    }
}

void Entity::AIJump(Entity *player) {
    switch(aiState) {
        case HALT:
            std::cout << aiState << "\n";
            if (glm::distance(position, player->position) < 1.5f) {
                aiState = ACTIVE;
            }
            else {
                velocity.y = -4.75;
            }
            break;
            
        case ACTIVE:
            std::cout << aiState << "\n";
            velocity.y = 4;
            
            if (glm::distance(position, player->position) > 1.5f) {
                aiState = HALT;
            }
            
            break;
    }
}

void Entity::update(float deltaTime, Entity *player, Entity *platforms, int platformCount, Entity *enemies, int enemyCount)
{
    if (isActive == false) return;
    
    collidedTop = false;
    collidedBottom = false;
    collidedLeft = false;
    collidedRight = false;
    
    if (entityType == ENEMY) {
        AI(player);
    }
    else {
        CheckEnemyCollision(enemies, enemyCount);
    }
    
    if (jump) {
        jump = false;
        velocity.y += jumpPower;
    }
    
    velocity.x = movement.x * speed;
    velocity += acceleration * deltaTime;
    
    position.y += velocity.y * deltaTime;
    CheckCollisionsY(platforms, platformCount);

    position.x += velocity.x * deltaTime;
    CheckCollisionsX(platforms, platformCount);     
    
    g_model_matrix = glm::mat4(1.0f);
    g_model_matrix = glm::translate(g_model_matrix, position);
}

void Entity::draw_sprite(ShaderProgram *g_shader_program, GLuint textureID, int index)
{
    float u = 0;
    float v = 0;

    float width = 1.0f;
    float height = 1.0f;
    
    float texture_coordinates[] = { u, v + height, u + width, v + height, u + width, v,
        u, v + height, u + width, v, u, v};
    
    float vertices[]  = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5 };
    
    glBindTexture(GL_TEXTURE_2D, textureID);
    
    glVertexAttribPointer(g_shader_program->get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(g_shader_program->get_position_attribute());
    
    glVertexAttribPointer(g_shader_program->get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, texture_coordinates);
    glEnableVertexAttribArray(g_shader_program->get_tex_coordinate_attribute());
    
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    glDisableVertexAttribArray(g_shader_program->get_position_attribute());
    glDisableVertexAttribArray(g_shader_program->get_tex_coordinate_attribute());
}

void Entity::render(ShaderProgram *g_shader_program) {
    if (isActive == false) return;
    
    g_shader_program->set_model_matrix(g_model_matrix);
    
    float vertices[]  = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5 };
    float texture_coordinates[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };
    
    glBindTexture(GL_TEXTURE_2D, textureID);
    
    glVertexAttribPointer(g_shader_program->get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(g_shader_program->get_position_attribute());
    
    glVertexAttribPointer(g_shader_program->get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, texture_coordinates);
    glEnableVertexAttribArray(g_shader_program->get_tex_coordinate_attribute());
    
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    glDisableVertexAttribArray(g_shader_program->get_position_attribute());
    glDisableVertexAttribArray(g_shader_program->get_tex_coordinate_attribute());
}
