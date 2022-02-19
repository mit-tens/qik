#ifndef TYPES_H
#define TYPES_H

typedef struct qik_mtex qik_mtex;
struct qik_mtex {
    unsigned n;
    SDL_Surface **ref;
};

typedef struct qik_spr qik_spr;
struct qik_spr {
    double x;
    double y;
    unsigned tex;
};

typedef struct qik_mspr qik_mspr;
struct qik_mspr {
    unsigned n;
    qik_spr *ref;
};

typedef struct qik_map_cell qik_map_cell;
struct qik_map_cell {
    double h;
    unsigned w;
    unsigned f;
};

typedef struct qik_map qik_map;
struct qik_map {
    char *title;
    char *version;
    char *music;
    int speed;
    struct {
	bool walls;
	bool floor;
	bool ceil;
	bool tile;
	bool roof;
	bool sprites;
	bool sky;
	bool shadows;
	bool fog;
    } cfg;
    struct {
	int f;
	SDL_Color c;
    } fog;
    struct {
	double master;
	unsigned w;
	unsigned h;
	qik_map_cell **tile;
	qik_map_cell **roof;
    } world;
    struct {
	double x_pos;
	double y_pos;
	double x_dir;
	double y_dir;
	double x_plane;
	double y_plane;
    } usr;
};

typedef struct qik_usr qik_usr;
struct qik_usr {
    char *name;
    double x_pos;
    double y_pos;
    double z_pos;
    double x_dir;
    double x_dir_old;
    double y_dir;
    double x_plane;
    double x_plane_old;
    double y_plane;
    double mov_speed;
    double rot_speed;
};

typedef struct qik_bind qik_bind;
struct qik_bind {
    SDL_Scancode ulook;
    SDL_Scancode dlook;
    SDL_Scancode rlook;
    SDL_Scancode llook;
    SDL_Scancode fmove;
    SDL_Scancode bmove;
    SDL_Scancode rmove;
    SDL_Scancode lmove;
    SDL_Scancode pfire;
    SDL_Scancode sfire;
    SDL_Scancode tfire;
};

typedef struct qik_cfg qik_cfg;
struct qik_cfg {
    char *usr;
    char *map;
    unsigned w;
    unsigned h;
};

#endif
