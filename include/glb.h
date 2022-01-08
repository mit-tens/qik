#ifndef GLB_H
#define GLB_H

extern bool glb_running;

extern SDL_Renderer *glb_renderer;
extern SDL_Window *glb_window;
extern Uint32 glb_format;

extern float glb_time, glb_time_old, glb_time_frame;

extern qik_bind glb_bind;
extern qik_mtex glb_mtex;
extern qik_mspr glb_mspr;
extern qik_cfg glb_cfg;
extern qik_map glb_map;
extern qik_usr glb_usr;

extern config_t glb_cfg_cfg_t, glb_map_cfg_t;

#endif
