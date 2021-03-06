#ifndef __PortraitView_h__
#define __PortraitView_h__

/*
 *  PortraitView.h
 *  Nuvie
 *
 *  Created by Eric Fry on Tue May 20 2003.
 *  Copyright (c) 2003. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */

#include "View.h"


class Configuration;
class TileManager;
class Portrait;
class Screen;
class Font;
class ObjManager;
class Party;
class Player;
class Actor;
class DollWidget;
class U6Shape;

class PortraitView : public View {

 nuvie_game_t gametype;

 uint8 cur_actor_num;
 Portrait *portrait;
 std::string *name_string;

 unsigned char *portrait_data;
 U6Shape *bg_data;
 uint8 portrait_width;
 uint8 portrait_height;

 bool waiting; // waiting for input, then will return to previous view
 bool show_cursor; // indicate waiting for input
// uint16 cursor_x, cursor_y;

 DollWidget *doll_widget;
 bool display_doll;

 public:
 PortraitView(Configuration *cfg);
 ~PortraitView();

 bool init(uint16 x, uint16 y, Font *f, Party *p, Player *player, TileManager *tm, ObjManager *om, Portrait *port);
 void Display(bool full_redraw);
 GUI_status HandleEvent(const SDL_Event *event);

 bool set_portrait(Actor *actor, const char *name);
 void set_show_cursor(bool state) { show_cursor = state; }
 void set_waiting(bool state);
 bool get_waiting()     { return(waiting); }

 protected:

 void display_name(uint16 y_offset);

 private:
 void load_background(const char *filename, uint8 lib_offset);

};

#endif /* __PortraitView_h__ */

