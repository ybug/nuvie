#ifndef __MapWindow_h__
#define __MapWindow_h__

/*
 *  MapWindow.h
 *  Nuive
 *
 *  Created by Eric Fry on Fri Mar 21 2003.
 *  Copyright (c) 2003. All rights reserved.
 *
 */

#include "U6def.h"
#include "Configuration.h"
#include "Surface.h"
#include "Screen.h"
#include "TileManager.h"
#include "ObjManager.h"
#include "ActorManager.h"

class MapWindow
{
 Configuration *config;
 Screen *screen;
 
 Map *map;
 
 uint16 *tmp_buf; // tempory buffer for flood fill, hide rooms. 
 
 TileManager *tile_manager;
 ObjManager *obj_manager;
 ActorManager *actor_manager;
 
 uint16 cur_x, cur_y;
 uint16 cursor_x, cursor_y;
 Tile *cursor_tile;
 Tile *use_tile;
 
 bool show_cursor;
 bool show_use_cursor;
 
 uint16 win_width, win_height;
 uint8 cur_level;

 public:
 
 MapWindow(Configuration *cfg);
 ~MapWindow();
 
 bool init(Screen *s, Map *m, TileManager *tm, ObjManager *om, ActorManager *am);
 
 bool set_windowSize(uint16 width, uint16 height);
 void set_show_cursor(bool state);
 void set_show_use_cursor(bool state);
 
 void moveLevel(uint8 new_level);
 void moveMap(sint16 new_x, sint16 new_y, sint8 new_level);
 void moveMapRelative(sint16 rel_x, sint16 rel_y);
 
 void centerMapOnActor(Actor *actor);
 void centerCursor();
 
 void moveCursor(sint16 new_x, sint16 new_y);
 void moveCursorRelative(sint16 rel_x, sint16 rel_y);
 
 char *lookAtCursor();
 
 void get_level(uint8 *level);
 void get_pos(uint16 *x, uint16 *y);
 
 void updateBlacking();
 
 void drawMap();
 
protected:

 void drawActors();
 void drawObjs();
 void drawObjSuperBlock(U6LList *superblock, bool toptile);
 inline void drawObj(Obj *obj, bool toptile);
 inline void drawTile(Tile *tile, uint16 x, uint16 y, bool toptile);
 void drawBorder();
 inline void MapWindow::drawTopTile(Tile *tile, uint16 x, uint16 y, bool toptile);

 void generateTmpMap();
 void boundaryFill(unsigned char *map_ptr, uint16 pitch, uint16 x, uint16 y);
 bool boundaryLookThroughWindow(uint16 tile_num, uint16 x, uint16 y);
 
 void reshapeBoundary();
 bool tmpBufTileIsBoundary(uint16 x, uint16 y);
 bool tmpBufTileIsWall(uint16 x, uint16 y);
};

#endif /* __MapWindow_h__ */