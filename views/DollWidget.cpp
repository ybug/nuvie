/*
 *  DollWidget.cpp
 *  Nuvie
 *
 *  Created by Eric Fry on Mon Sep 01 2003.
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

// FIX need to subclass this class for U6, MD & SE

#include "nuvieDefs.h"
// wont need all these includes with PlayerActions
#include "MsgScroll.h"
#include "MapWindow.h"
#include "Player.h"
#include "UseCode.h"
#include "Event.h"

#include "Actor.h" 
#include "DollWidget.h"

static SDL_Rect item_hit_rects[8] = { {24, 0,16,16},   // ACTOR_HEAD
                                     { 0, 8,16,16},   // ACTOR_NECK
                                     {48, 8,16,16},   // ACTOR_BODY
                                     { 0,24,16,16},   // ACTOR_ARM
                                     {48,24,16,16},   // ACTOR_ARM_2 
                                     { 0,40,16,16},   // ACTOR_HAND
                                     {48,40,16,16},   // ACTOR_HAND_2
                                     {24,48,16,16} }; // ACTOR_FOOT

DollWidget::DollWidget(Configuration *cfg): GUI_Widget(NULL, 0, 0, 0, 0)
{
 config = cfg;
 actor = NULL;
 selected_obj = NULL;
}

DollWidget::~DollWidget()
{

}

bool DollWidget::init(Actor *a, uint16 x, uint16 y, TileManager *tm, ObjManager *om)
{
 tile_manager = tm;
 obj_manager = om;
 
 area.x = x;
 area.y = y;
 
 area.w = 64;
 area.h = 64;
 
 set_actor(a);
 
 return true;
}

void DollWidget::set_actor(Actor *a)
{
 actor = a;
 Redraw();
}


SDL_Rect *DollWidget::get_item_hit_rect(uint8 location)
{
    if(location < 8)
        return(&item_hit_rects[location]);
    return(NULL);
}


void DollWidget::Display(bool full_redraw)
{
 //if(full_redraw || update_display)
 // {
   update_display = false;
   screen->fill(0x31, area.x, area.y, area.w, area.h);
   if(actor != NULL)
     display_doll();
   screen->update(area.x, area.y, area.w, area.h);
//  }
 
}

inline void DollWidget::display_doll()
{
 Tile *tile, *empty_tile;
 uint16 i,j;
 
 empty_tile = tile_manager->get_tile(410);
 
 for(i=0;i<2;i++)
   {
    for(j=0;j<2;j++) // draw doll
      {
       tile = tile_manager->get_tile(368+i*2+j);
       screen->blit(area.x+16+j*16,area.y+16+i*16,tile->data,8,16,16,16,true);
      }
   }

 display_readied_object(ACTOR_NECK, area.x, (area.y+8) + 0 * 16, actor, empty_tile);
 display_readied_object(ACTOR_BODY, area.x+3*16, (area.y+8) + 0 * 16, actor, empty_tile);

 display_readied_object(ACTOR_ARM, area.x, (area.y+8) + 1 * 16, actor, empty_tile);
 display_readied_object(ACTOR_ARM_2, area.x+3*16, (area.y+8) + 1 * 16, actor, empty_tile);
 
 display_readied_object(ACTOR_HAND, area.x, (area.y+8) + 2 * 16, actor, empty_tile);
 display_readied_object(ACTOR_HAND_2, area.x+3*16, (area.y+8) + 2 * 16, actor, empty_tile);

 display_readied_object(ACTOR_HEAD, area.x+16+8, area.y, actor, empty_tile);
 display_readied_object(ACTOR_FOOT, area.x+16+8, area.y+3*16, actor, empty_tile);

 return;
}

inline void DollWidget::display_readied_object(uint8 location, uint16 x, uint16 y, Actor *actor, Tile *empty_tile)
{
 Obj *obj;
 Tile *tile;
 
 obj = actor->inventory_get_readied_object(location);
 
 if(obj)
   tile = tile_manager->get_tile(obj_manager->get_obj_tile_num(obj->obj_n)+obj->frame_n);
 else
   tile = empty_tile;
 
 screen->blit(x,y,tile->data,8,16,16,16,true);
 
 return;
}


GUI_status DollWidget::MouseDown(int x, int y, int button)
{
 Event *event = Game::get_game()->get_event();
 MsgScroll *scroll = Game::get_game()->get_scroll();
 MapWindow *map_window = Game::get_game()->get_map_window();
 uint8 location;
 Obj *obj;
 
 x -= area.x;
 y -= area.y;
 
 if(actor && selected_obj == NULL)
   {
    for(location=0;location<8;location++)
      {
       if(HitRect(x,y,item_hit_rects[location]))
         {
          printf("Hit %d\n",location);
          obj = actor->inventory_get_readied_object(location);
          if(obj)
           {
            selected_obj = obj;
     // ABOEING
     switch (event->get_mode()) {
     case LOOK_MODE:
      event->look(selected_obj);
      scroll->display_prompt();
      map_window->set_show_cursor(false);
      break;
     case USE_MODE:
      event->use(selected_obj);
      break;
     default:
      break;
     }            return GUI_YUM;
           }
         }
      }
   }

	return GUI_PASS;
}




GUI_status DollWidget::MouseUp(int x,int y,int button)
{
 Event *event = Game::get_game()->get_event();
 UseCode *usecode = Game::get_game()->get_usecode();
 // ABOEING
 if (event->get_mode() == LOOK_MODE) {
  event->set_mode(MOVE_MODE);
  selected_obj = NULL;
  return GUI_YUM;
 }

 // only act now if not usable, else wait for possible double-click
 if(selected_obj && !usecode->has_usecode(selected_obj)) // un-ready selected item.
   {
//    actor->remove_readied_object(selected_obj);
    event->unready(selected_obj);
    selected_obj = NULL;
    Redraw();
    finish_mouseclick();
   }

 selected_obj = NULL;
 return GUI_YUM;
}


// waited for double-click
GUI_status DollWidget::MouseClick(int x, int y, int button)
{
 Event *event = Game::get_game()->get_event();
 Obj *selected_obj = NULL;
 for(uint8 location=0;location<8;location++)
    if(HitRect(x-area.x,y-area.y,item_hit_rects[location]))
       selected_obj = actor->inventory_get_readied_object(location);
 if(selected_obj) // un-ready selected item.
   {
//    actor->remove_readied_object(selected_obj);
    event->unready(selected_obj);
    Redraw();
   }

 return GUI_YUM;
}


GUI_status DollWidget::MouseMotion(int x,int y,Uint8 state)
{
 Tile *tile;

 if(selected_obj && !dragging)
   {
    dragging = true;
    tile = tile_manager->get_tile(obj_manager->get_obj_tile_num(selected_obj->obj_n)+selected_obj->frame_n);
    return gui_drag_manager->start_drag(this, GUI_DRAG_OBJ, selected_obj, tile->data, 16, 16, 8);
   }
   
	return GUI_PASS;
}

void DollWidget::drag_drop_success(int x, int y, int message, void *data)
{
 dragging = false;
 actor->remove_readied_object(selected_obj);
 actor->inventory_remove_obj(selected_obj);
 selected_obj = NULL;
 Redraw();
}

void DollWidget::drag_drop_failed(int x, int y, int message, void *data)
{
 printf("Drop Failed\n");
 dragging = false;
 selected_obj = NULL;
}

bool DollWidget::drag_accept_drop(int x, int y, int message, void *data)
{
 if(message == GUI_DRAG_OBJ) //FIX add weight check and ready check
   {
    return true;
   }
   
 return false;
}

void DollWidget::drag_perform_drop(int x, int y, int message, void *data)
{
 Event *event = Game::get_game()->get_event();
 MsgScroll *scroll = Game::get_game()->get_scroll();
 Obj *obj;
 
 x -= area.x;
 y -= area.y;
 
 if(message == GUI_DRAG_OBJ)
   {
    printf("Ready item.\n");
    obj = (Obj *)data;

#if 0 /* trying to connect dragndrop to Actions, but it cant be done yet */
    if(!(obj->status & OBJ_STATUS_IN_INVENTORY)) // get
      {
       scroll->display_string("Get-");
       event->get(obj, NULL, actor);
      }
    if(obj->status & OBJ_STATUS_IN_INVENTORY) // ready
      {
       if((obj->status & OBJ_STATUS_READIED) != OBJ_STATUS_READIED)
         {
          assert(obj->x == actor->get_actor_num());
          event->ready(obj);
         }
      }
#endif
    actor->inventory_add_object(obj);
    actor->add_readied_object(obj);
    Redraw();
   }
   
 return;
}

void DollWidget::drag_draw(int x, int y, int message, void* data)
{
	Tile* tile;

	if (!selected_obj)
		return;
		
	tile = tile_manager->get_tile(obj_manager->get_obj_tile_num (selected_obj->obj_n) + selected_obj->frame_n);

	int	nx = x - 8;
	int	ny = y - 8;

	if (nx + 16 >= 320)
		nx = 303;
	else if (nx < 0)
		nx = 0;

	if (ny + 16 >= 200)
		ny = 183;
	else if (ny < 0)
		ny = 0;

	screen->blit(nx, ny, tile->data, 8, 16, 16, 16, true);
	screen->update(nx, ny, 16, 16);
}


/* Use object. (FIXME: this is duplicating code from InventoryWidget)
 */
GUI_status DollWidget::MouseDouble(int x, int y, int button)
{
    UseCode *uc = Game::get_game()->get_usecode();
    MsgScroll *scroll = Game::get_game()->get_scroll();
    Converse *converse = Game::get_game()->get_converse();
    MapWindow *map_window = Game::get_game()->get_map_window();
    Event *event = Game::get_game()->get_event();
    const char *target_name;
    Obj *obj = NULL;
    for(uint8 location = 0; location < 8; location++)
        if(HitRect(x - area.x, y - area.y, item_hit_rects[location]))
            obj = actor->inventory_get_readied_object(location);

    if(!(actor && obj && button == 1 && event->get_mode() == MOVE_MODE))
        return(GUI_YUM);

    target_name = obj_manager->look_obj(obj);
    scroll->display_string("Use-");
    scroll->display_string(target_name);
    scroll->display_string("\n");

    if(uc->has_usecode(obj))
        uc->use_obj(obj, Game::get_game()->get_player()->get_actor());
    else
    {
        fprintf(stderr, "Obj %d:%d, (no usecode)\n", obj->obj_n, obj->frame_n);
        scroll->display_string("\nNot usable\n");
    }

    map_window->updateBlacking();

    // if selecting, select_obj will return to MOVE_MODE (and no need to here)
    if(event->get_mode() == USE_MODE || event->get_mode() == MOVE_MODE)
    {
        scroll->display_string("\n");
        scroll->display_prompt();
        event->set_mode(MOVE_MODE);
    }

    return(GUI_PASS);
}
