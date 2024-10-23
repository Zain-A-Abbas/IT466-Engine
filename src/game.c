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
#include "Entity.h"
#include "TerrainManager.h"
#include "Player.h"
#include "Enemy.h"
#include "Reticle.h"
#include "Interactable.h"

extern int __DEBUG;

static int _done = 0;
static Uint32 frame_delay = 16;
static float fps = 0;

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
    Model *sky;
    GFC_Matrix4 skyMat,dinoMat;
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
    //gf2d_mouse_load("actors/mouse.actor");
    reticleLoad("actors/reticle.actor");
    sky = gf3d_model_load("models/sky.model");
    gfc_matrix4_identity(skyMat);
    gfc_matrix4_identity(dinoMat);
        //camera
    gf3d_camera_set_scale(gfc_vector3d(1,1,1));
    gf3d_camera_set_position(gfc_vector3d(15,-15,10));
    gf3d_camera_look_at(gfc_vector3d(0,0,0),NULL);
    gf3d_camera_set_move_step(0.2);
    gf3d_camera_set_rotate_step(0.05);
    
    gf3d_camera_enable_free_look(1);
    entitySystemInit(2048);

    // Create player
    Entity * player = createPlayer();
    assignCamera(player, gf3dGetCamera());
    player->position.z = 20;
    SDL_SetRelativeMouseMode(SDL_TRUE);

    // Create dummy enemies
    Entity* enemy1 = enemyEntityNew();
    enemy1->position = gfc_vector3d(4, 4, 0);
    Entity* enemy2 = enemyEntityNew();
    enemy2->position = gfc_vector3d(-4, 4, 0);
    
    // Create land
    Entity* testGround = terrainEntityNew();
    testGround->model = gf3d_model_load("models/primitives/testground2.model");
    testGround->position = gfc_vector3d(0, 0, 0);


    // Create interactable
    Entity* testInteractable = interactableNew(SPINNING_BOX);
    testInteractable->position = gfc_vector3d(0, -32, 0);


    //Delta time
    float delta = 0.0;
    game_frame_delay(&delta);


    // main game loop
    while(!_done)
    {
        gfc_input_update();
        gf2d_mouse_update();
        gf2d_font_update();
        entityThinkAll(delta);
        entityUpdateAll(delta);
        //camera updaes
        //gf3d_camera_controls_update();
        gf3d_camera_update_view();
        gf3d_camera_get_view_mat4(gf3d_vgraphics_get_view_matrix());

        gf3d_vgraphics_render_start();

            //3D draws
        


                gf3d_model_draw_sky(sky,skyMat,GFC_COLOR_WHITE);
                entityDrawAll();
                draw_origin();

                // Draw last player raycast
               /*PlayerData* playerData = getPlayerData(player);
                if (playerData != NULL) {
                    GFC_Triangle3D t = { 0 };
                    GFC_Vector3D gravityRaycastDir = gfc_vector3d(0, 0, -6.5);
                    GFC_Edge3D gravityRaycast = gfc_edge3d_from_vectors(player->position, gfc_vector3d_added(player->position, gravityRaycastDir));
                    gf3d_draw_edge_3d(
                        gravityRaycast,
                        gfc_vector3d(0, 0, 0),
                        gfc_vector3d(0, 0, 0),
                        gfc_vector3d(1, 1, 1),
                        0.5,
                        gfc_color(1.0, 1.0, 0.0, 1.0)
                    );
                }*/
            //2D draws
                //gf2d_mouse_draw();
                reticleDraw();
                gf2d_font_draw_line_tag("ALT+F4 to exit",FT_H1,GFC_COLOR_WHITE, gfc_vector2d(10,10));
        gf3d_vgraphics_render_end();


        if (gfc_input_command_down("exit"))_done = 1; // exit condition
        game_frame_delay(&delta);
    }    
    vkDeviceWaitIdle(gf3d_vgraphics_get_default_logical_device());    
    //cleanup
    slog("gf3d program end");
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
    //slog("fps: %f", fps);
    //slog("Delta: %f", *delta);
}
/*eol@eof*/
