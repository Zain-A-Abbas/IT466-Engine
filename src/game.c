#include <SDL.h>            

#include "simple_json.h"
#include "simple_logger.h"

#include "gfc_input.h"
#include "gfc_config_def.h"
#include "gfc_vector.h"
#include "gfc_matrix.h"
#include "gfc_audio.h"
#include "gfc_string.h"
#include "gfc_actions.h"

#include "gf2d_sprite.h"
#include "gf2d_font.h"
#include "gf2d_draw.h"
#include "gf2d_actor.h"
#include "gf2d_mouse.h"

#include "gf3d_vgraphics.h"
#include "gf3d_pipeline.h"
#include "gf3d_swapchain.h"
#include "gf3d_model.h"
#include "gf3d_camera.h"
#include "gf3d_texture.h"
#include "gf3d_draw.h"
#include "gf3d_armature.h"
#include "Entity.h"
#include "TerrainManager.h"
#include "Player.h"
#include "Enemy.h"
#include "UI.h"
#include "light.h"
#include "Interactable.h"
#include "Structure.h"
#include "Zombie.h"
#include "Level.h"
#include "Title.h"

extern int __DEBUG;

static int _done = 0;
static Uint32 frame_delay = 16;
static float fps = 0;

enum GameState {
    GS_NONE,
    GS_TITLE,
    GS_LOADING,
    GS_GAME
};

void parse_arguments(int argc,char *argv[]);
void game_frame_delay(float* delta);

void exitGame()
{
    _done = 1;
}

void draw_origin()
{
    gf3d_draw_edge_3d(
        gfc_edge3d_from_vectors(gfc_vector3d(-100,0,0),gfc_vector3d(100,0,0)),
        gfc_vector3d(0,0,0),
        gfc_vector3d(0,0,0),gfc_vector3d(1,1,1),0.1,gfc_color(1,0,0,1));
    gf3d_draw_edge_3d(
        gfc_edge3d_from_vectors(gfc_vector3d(0,-100,0),gfc_vector3d(0,100,0)),
        gfc_vector3d(0,0,0),gfc_vector3d(0,0,0),gfc_vector3d(1,1,1),0.1,gfc_color(0,1,0,1));
    gf3d_draw_edge_3d(
        gfc_edge3d_from_vectors(gfc_vector3d(0,0,-100),gfc_vector3d(0,0,100)),
        gfc_vector3d(0,0,0),gfc_vector3d(0,0,0),gfc_vector3d(1,1,1),0.1,gfc_color(0,0,1,1));
}


int main(int argc,char *argv[])
{
    //local variables
    GFC_Matrix4 dinoMat;
    //initializtion    
    parse_arguments(argc,argv);
    init_logger("gf3d.log",0);
    slog("gf3d begin");
    //gfc init
    gfc_input_init("config/input.cfg");
    gfc_config_def_init();
    gfc_action_init(1024);
    //gf3d init
    gf3d_vgraphics_init("config/setup.cfg");
    gf3d_materials_init();
    gf2d_font_init("config/font.cfg");
    gf2d_actor_init(1000);
    
    gf3d_draw_init();//3D
    gf2d_draw_manager_init(1000);//2D
    
    //game init
    srand(SDL_GetTicks());
    slog_sync();

    //game setup

    //armatures
    gf3d_armature_system_init(256);

    gfc_matrix4_identity(dinoMat);

    //camera
    gf3d_camera_set_scale(gfc_vector3d(1,1,1));
    gf3d_camera_set_position(gfc_vector3d(15,-15,10));
    gf3d_camera_look_at(gfc_vector3d(0,0,0),NULL);
    gf3d_camera_set_move_step(0.2);
    gf3d_camera_set_rotate_step(0.05);
    
    gf3d_camera_enable_free_look(1);
    // Setup audio
    gfc_audio_init(128, 8, 1, 1, 0, 0);
    
    entitySystemInit(2048);

    // Setup lights
    initLights();

    SDL_SetRelativeMouseMode(SDL_TRUE);

    enum GameState gameState = GS_TITLE;
    createTitle();


    // Create player
    Entity *player = NULL;
    // Create level
    LevelData* levelData = NULL;
    
    // UI setup
    //initializeUI();

    // Create dummy enemies
    Entity* enemy1 = NULL;// = createZombie(player);
    //enemy1->position = gfc_vector3d(0, -40, 0);
    //entityScalePreserveModel(enemy1, gfc_vector3d(0.08, 0.08, 0.08));
    /*Entity* enemy2 = enemyEntityNew();
    enemy2->position = gfc_vector3d(-4, 4, 0);*/
    
  

    //Delta time
    float delta = 0.0;
    game_frame_delay(&delta);

    for (int i = 0; i < entityManager.entityMax; i++) {
        if (entityManager.entityList[i].type == ENEMY) {
            enemy1 = &entityManager.entityList[i];
            break;
        }
    }

    // main game loop
    while(!_done)
    {
        gfc_input_update();
        gf2d_mouse_update();
        gf2d_font_update();
        levelProcess(delta);
        entityThinkAll(delta);
        entityUpdateAll(delta);
        //camera updaes
        //gf3d_camera_controls_update();
        gf3d_camera_update_view();
        gf3d_camera_get_view_mat4(gf3d_vgraphics_get_view_matrix());

        gf3d_vgraphics_render_start();

            if (gameState == GS_TITLE) {
                processTitle();
                if (titleOption() == 1) {
                    createForestLevel(&player);
                    initializeUI();
                    gameState = GS_GAME;

                } else if (titleOption() == 2) {
                    gameState = GS_NONE;
                    _done = 1;
                }
            }
            else if (gameState == GS_GAME) {
                levelDraw();
                entityDrawAll(delta);
                draw_origin();

                   // Draw last player raycast
                PlayerData* playerData = getPlayerData(player);
                if (playerData != NULL) {
                    if (playerData->hp <= 0) {
                        _done = 1;
                    }
                    if (playerData->raycastTests) {
                        int i = 0;
                        for (i = 0; i < 1; i++) {
                            GFC_Edge3D drawEdge;
                            GFC_Edge3D* edgeptr = (GFC_Edge3D*)gfc_list_get_nth(playerData->raycastTests, i);
                            if (!edgeptr) {
                                continue;
                            }
                            drawEdge.a = edgeptr->a;
                            drawEdge.b = edgeptr->b;
                            gf3d_draw_edge_3d(
                                drawEdge,
                                gfc_vector3d(0, 0, 0),
                                gfc_vector3d(0, 0, 0),
                                gfc_vector3d(1, 1, 1),
                                0.25,
                                gfc_color(1.0, 1.0, 0.0, 1.0)
                            );


                        }
                    }

                    Character3DData* playerChar3dData = playerData->character3dData;

                    GFC_Edge3D drawEdge = playerChar3dData->gravityRaycast;
                    gf3d_draw_edge_3d(
                        drawEdge,
                        gfc_vector3d(0, 0, 0),
                        gfc_vector3d(0, 0, 0),
                        gfc_vector3d(1, 1, 1),
                        0.25,
                        gfc_color(1.0, 1.0, 0.0, 1.0)
                    );

                    //if (playerData->boundingBoxTest.x != 0) {
                      //  gf3d_draw_cube_solid(playerData->boundingBoxTest, gfc_vector3d(0, 0, 0), gfc_vector3d(0, 0, 0), gfc_vector3d(1, 1, 1), gfc_color(0.5, 0.2, 0.2, 0.8));
                    //}
                }

                /*for (int i = 0; i < gfc_list_get_count(testGround->entityCollision->quadTree->leaves); i++) {
                    QuadtreeNode* currentLeaf = (QuadtreeNode*)gfc_list_get_nth(testGround->entityCollision->quadTree->leaves, i);
                    drawBoundingBox(currentLeaf->AABB, gfc_color(0.3, 0.3, 0.3, 0.4), 1);
                }*/

                //drawBoundingBox(capsuleToBox(enemy1->entityCollision->collisionPrimitive->s.c), gfc_color(0.1, 0.3, 0.3, 0.5), 0);

                //if (gfc_box_overlap(testBox, player->entityCollision->AABB)) {
                //    printf("klasjsd");
                //}
                //2D draws

                if (enemy1) {
                    EnemyData* enemyData = (EnemyData*)enemy1->data;
                    if (enemyData != NULL) {
                    //printf("\nAttack sphere: %f, %f, %f, %f", enemyData->attackSphere.x, enemyData->attackSphere.y, enemyData->attackSphere.z, enemyData->attackSphere.r);
                        gf3d_draw_sphere_solid(
                            enemyData->attackSphere,
                            gfc_vector3d(0, 0, 0),
                            gfc_vector3d(0, 0, 0),
                            gfc_vector3d(1, 1, 1),
                            gfc_color(0.7, 0, 0, 0.5),
                            gfc_color(1, 1, 1, 1)
                        );
                    }
                }

                    //gf2d_mouse_draw();
                    drawUI();
            }

            //3D draws
        



                //gf2d_font_draw_line_tag("ALT+F4 to exit",FT_H1,GFC_COLOR_WHITE, gfc_vector2d(10,10));
        gf3d_vgraphics_render_end();


        if (gfc_input_command_down("exit"))_done = 1; // exit condition
        game_frame_delay(&delta);
    }    
    vkDeviceWaitIdle(gf3d_vgraphics_get_default_logical_device());    
    //cleanup
    slog("\ngf3d program end");
    exit(0);
    slog_sync();
    return 0;
}

void parse_arguments(int argc,char *argv[])
{
    int a;

    for (a = 1; a < argc;a++)
    {
        if (strcmp(argv[a],"--debug") == 0)
        {
            __DEBUG = 1;
        }
    }    
}

void game_frame_delay(float * delta)
{
    Uint32 diff;
    static Uint32 now;
    static Uint32 then;
    then = now;
    slog_sync();// make sure logs get written when we have time to write it
    now = SDL_GetTicks();
    diff = (now - then);
    *delta = (float)diff / 1000;
    if (diff < frame_delay)
    {
        SDL_Delay(frame_delay - diff);
    }
    fps = 1000.0/MAX(SDL_GetTicks() - then,0.001);
    //printf("\nfps: %f", fps);
    //printf("\nDelta: %f", *delta);
}
/*eol@eof*/
