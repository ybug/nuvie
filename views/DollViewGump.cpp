/*
 *  DollViewGump.cpp
 *  Nuvie
 *
 *  Created by Eric Fry on Mon Mar 19 2012.
 *  Copyright (c) 2012. All rights reserved.
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
#include <cassert>
#include <math.h>
#include "nuvieDefs.h"
#include "U6misc.h"
#include "Event.h"
#include "GUI.h"
#include "GUI_button.h"

#include "Party.h"
#include "Actor.h"
#include "ViewManager.h"

#include "ContainerViewGump.h"
#include "DollWidget.h"
#include "DollViewGump.h"


DollViewGump::DollViewGump(Configuration *cfg) : DraggableView(cfg),
	gump_button(NULL), combat_button(NULL), heart_button(NULL), party_button(NULL), inventory_button(NULL),
	doll_widget(NULL), font(NULL), actor(NULL), cursor_tile(NULL)
{
	bg_image = NULL;
	actor_doll = NULL;
	is_avatar = false;
	show_cursor = true;
	cursor_pos = CURSOR_HEAD;
}

DollViewGump::~DollViewGump()
{
	if(font)
		delete font;
	if(actor_doll)
	  SDL_FreeSurface(actor_doll);
}

bool DollViewGump::init(Screen *tmp_screen, void *view_manager, uint16 x, uint16 y, Actor *a, Font *f, Party *p, TileManager *tm, ObjManager *om)
{
	View::init(x,y,f,p,tm,om);

	SetRect(area.x, area.y, 108, 136);

	actor = a;
	is_avatar = actor->is_avatar();
	cursor_tile = tile_manager->get_gump_cursor_tile();
	doll_widget = new DollWidget(config, this);
	doll_widget->init(actor, 26, 16, tile_manager, obj_manager);

	AddWidget(doll_widget);

	std::string datadir = GUI::get_gui()->get_data_dir();
	std::string imagefile;
	std::string path;

	SDL_Surface *image, *image1;

	build_path(datadir, "images", path);
	datadir = path;
	build_path(datadir, "gumps", path);
	datadir = path;

	gump_button = loadButton(datadir, "gump", 0, 112);

	build_path(datadir, "left_arrow.bmp", imagefile);
	image = SDL_LoadBMP(imagefile.c_str());
	image1 = SDL_LoadBMP(imagefile.c_str());

	left_button = new GUI_Button(this, 23, 7, image, image1, this);
	this->AddWidget(left_button);

	build_path(datadir, "right_arrow.bmp", imagefile);
	image = SDL_LoadBMP(imagefile.c_str());
	image1 = SDL_LoadBMP(imagefile.c_str());

	right_button = new GUI_Button(this, 86, 7, image, image1, this);
	this->AddWidget(right_button);

	build_path(datadir, "doll", path);
	datadir = path;

	build_path(datadir, "doll_bg.bmp", imagefile);
	bg_image = SDL_LoadBMP(imagefile.c_str());

	set_bg_color_key(0, 0x70, 0xfc);

	build_path(datadir, "combat_btn_up.bmp", imagefile);
	image = SDL_LoadBMP(imagefile.c_str());
	build_path(datadir, "combat_btn_down.bmp", imagefile);
	image1 = SDL_LoadBMP(imagefile.c_str());

	combat_button = new GUI_Button(NULL, 23, 92, image, image1, this);
	this->AddWidget(combat_button);

	heart_button = loadButton(datadir, "heart", 23, 108);
	party_button = loadButton(datadir, "party", 47, 108);
	inventory_button = loadButton(datadir, "inventory", 71, 108);

	font = new GUI_Font(GUI_FONT_GUMP);
	font->SetColoring( 0x08, 0x08, 0x08, 0x80, 0x58, 0x30, 0x00, 0x00, 0x00);

	if(party->get_member_num(actor) < 0)
	{
		if(Game::get_game()->get_event()->using_control_cheat() == false)
			heart_button->Hide();
		left_button->Hide();
		right_button->Hide();
	}
	party_button->Hide();
	is_avatar = actor->is_avatar();
	ViewManager *vm = Game::get_game()->get_view_manager();
	if(is_avatar)
		actor_doll = vm->loadAvatarDollImage(actor_doll);
	else
		actor_doll = vm->loadCustomActorDollImage(actor_doll, actor->get_actor_num());
	setColorKey(actor_doll);

	return true;
}

void DollViewGump::setColorKey(SDL_Surface *image)
{
  if(image)
  {
    bg_color_key = SDL_MapRGB(image->format, 0xf1, 0x0f, 0xc4);
    SDL_SetColorKey(image, SDL_SRCCOLORKEY, bg_color_key);
  }
}

static const char combat_mode_tbl[][8] = {"COMMAND", "FRONT", "REAR", "FLANK", "BERSERK", "RETREAT", "ASSAULT"};
static const char combat_mode_tbl_se[][8] = {"COMMAND", "RANGED", "FLEE", "CLOSE"};
static const char combat_mode_tbl_md[][8] = {"COMMAND", "RANGED", "FLEE", "ATTACK"};

void DollViewGump::set_actor(Actor *a)
{
	actor = a;
	if(actor)
	{
		is_avatar = actor->is_avatar();
		ViewManager *vm = Game::get_game()->get_view_manager();
		if(is_avatar)
			actor_doll = vm->loadAvatarDollImage(actor_doll);
		else
			actor_doll = vm->loadCustomActorDollImage(actor_doll, actor->get_actor_num());
		setColorKey(actor_doll);
	}

	if(doll_widget)
		doll_widget->set_actor(actor);
}

uint8 DollViewGump::get_cursor_xoff()
{
	switch(cursor_pos) {
		case CURSOR_LEFT: return 18;
		case CURSOR_RIGHT: return 82;
		case CURSOR_HEAD: return 50;
		case CURSOR_NECK:
		case CURSOR_RIGHT_HAND: return 26;
		case CURSOR_CHEST:
		case CURSOR_LEFT_HAND: return 74;
		case CURSOR_RIGHT_RING: return 26;
		case CURSOR_LEFT_RING:  return 74;
		case CURSOR_FEET:  return 50;
		case CURSOR_CHECK: return 1;
		case CURSOR_COMBAT:  return 23;
		case CURSOR_HEART:  return 26;
		case CURSOR_PARTY: return 50;
		case CURSOR_INVENTORY:
		default : return 74;
	}
}

uint8 DollViewGump::get_cursor_yoff()
{
	switch(cursor_pos) {
		case CURSOR_LEFT:
		case CURSOR_RIGHT: return 2;
		case CURSOR_HEAD: return 16;
		case CURSOR_NECK: return 24;
		case CURSOR_CHEST: return 24;
		case CURSOR_RIGHT_HAND:
		case CURSOR_LEFT_HAND: return 40;
		case CURSOR_RIGHT_RING:
		case CURSOR_LEFT_RING: return 57;
		case CURSOR_FEET: return 63;
		case CURSOR_CHECK: return 111;
		case CURSOR_COMBAT: return 92;
		case CURSOR_HEART:
		case CURSOR_PARTY:
		case CURSOR_INVENTORY:
		default: return 109;
	}
}

void DollViewGump::Display(bool full_redraw)
{
 //display_level_text();
 //display_spell_list_text();
 SDL_Rect dst;
 dst = area;
 dst.w = 108;
 dst.h = 136;
 SDL_BlitSurface(bg_image, NULL, surface, &dst);

   if(actor_doll)
   {
     dst.x += 45;
     dst.y += 32;
     SDL_BlitSurface(actor_doll, NULL, surface, &dst);
   }

 uint8 w = font->get_center(actor->get_name(), 58);
 font->TextOut(screen->get_sdl_surface(), area.x + 29 + w, area.y + 7, actor->get_name());

 displayEquipWeight();

 DisplayChildren(full_redraw);
 displayCombatMode();
 if(show_cursor)
	screen->blit(area.x+get_cursor_xoff(),area.y+get_cursor_yoff(),(unsigned char *)cursor_tile->data,8,16,16,16,true);
 update_display = false;
 screen->update(area.x, area.y, area.w, area.h);

 return;
}

void DollViewGump::displayEquipWeight()
{
	uint8 strength = actor->get_strength();
	float equip_weight = ceilf(actor->get_inventory_equip_weight());
	char string[4]; //nnn\0

	snprintf(string, 4, "%d", (int)equip_weight);
	font->TextOut(screen->get_sdl_surface(), area.x + (((int)equip_weight > 9) ? 59 : 64), area.y + 82, string);

	snprintf(string, 4, "%d", strength);
	font->TextOut(screen->get_sdl_surface(), area.x + ((strength > 9) ? 76 : 81), area.y + 82, string);
}

void DollViewGump::displayCombatMode()
{
	if(!actor->is_in_party() || party->get_member_num(actor) == 0)
		return;
	uint8 index = get_combat_mode_index(actor);
	const char *text;
	if(Game::get_game()->get_game_type() == NUVIE_GAME_U6)
		text = combat_mode_tbl[index];
	else if(Game::get_game()->get_game_type() == NUVIE_GAME_MD)
		text = combat_mode_tbl_md[index];
	else // SE
		text = combat_mode_tbl_se[index];
	uint8 c = font->get_center(text, 55);
	font->TextOut(screen->get_sdl_surface(), area.x + 36 + c, area.y + 97, text);
}

void DollViewGump::left_arrow()
{
	uint8 party_mem_num = party->get_member_num(actor);
	if(party_mem_num > 0)
		party_mem_num--;
	else
		party_mem_num = party->get_party_size() - 1;

	set_actor(party->get_actor(party_mem_num));
}

void DollViewGump::right_arrow()
{
	set_actor(party->get_actor((party->get_member_num(actor) + 1) % party->get_party_size()));
}

GUI_status DollViewGump::callback(uint16 msg, GUI_CallBack *caller, void *data)
{
	Event *event = Game::get_game()->get_event();
	//close gump and return control to Magic class for clean up.
	if(event->get_mode() == ATTACK_MODE || caller == gump_button)
	{
		Game::get_game()->get_view_manager()->close_gump(this);
		return GUI_YUM;
	}
	else if(caller == right_button)
	{
		right_arrow();
	}
	else if(caller == left_button)
	{
		left_arrow();
	}
	else if(caller == inventory_button)
	{
		Game::get_game()->get_view_manager()->open_container_view(actor);
	}
	else if(caller == heart_button)
	{
		Game::get_game()->get_view_manager()->open_portrait_gump(actor);
	}
	else if(caller == combat_button)
	{
		activate_combat_button();
	}
	else if(caller == party_button) // FIXME: What is this supposed to do?
	{

	}
	else if(caller == doll_widget)
	{
		if(event->get_mode() != MOVE_MODE && event->get_mode() != EQUIP_MODE)
		{
			Obj *obj = (Obj *)data;
			event->select_view_obj(obj, actor);
			return GUI_YUM;
		}
	}

    return GUI_PASS;
}

GUI_status DollViewGump::moveCursorRelative(uint8 direction)
{
	dollCursorPos cursor_left = actor->is_in_party() ? CURSOR_LEFT : CURSOR_HEAD; // don't allow pickpocket or control cheat into arrow area
	dollCursorPos cursor_right = actor->is_in_party() ? CURSOR_RIGHT : CURSOR_HEAD;
	dollCursorPos cursor_party; // no party button yet so skip it
	dollCursorPos cursor_heart; // not available in pickpocket mode

	if(!actor->is_in_party() && !Game::get_game()->get_event()->using_control_cheat()) {
		if(direction == NUVIE_DIR_SW || direction == NUVIE_DIR_W)
			cursor_heart = CURSOR_CHECK;
		else
			cursor_heart = CURSOR_INVENTORY;
	} else
		cursor_heart = CURSOR_HEART;

	if(direction == NUVIE_DIR_W || direction == NUVIE_DIR_SW)
		cursor_party = cursor_heart;
	else
		cursor_party = CURSOR_INVENTORY;

	switch(cursor_pos) {
		case CURSOR_LEFT:
			switch(direction) {
				case NUVIE_DIR_NE:
				case NUVIE_DIR_E: cursor_pos = CURSOR_RIGHT; return GUI_YUM;
				case NUVIE_DIR_SW:
				case NUVIE_DIR_S: cursor_pos = CURSOR_NECK; return GUI_YUM;
				case NUVIE_DIR_SE: cursor_pos = CURSOR_HEAD; return GUI_YUM;
				default: return GUI_YUM;
			}
		case CURSOR_RIGHT:
			switch(direction) {
				case NUVIE_DIR_W:
				case NUVIE_DIR_NW: cursor_pos = CURSOR_LEFT; return GUI_YUM;
				case NUVIE_DIR_S:
				case NUVIE_DIR_SE: cursor_pos = CURSOR_CHEST; return GUI_YUM;
				case NUVIE_DIR_SW: cursor_pos = CURSOR_HEAD; return GUI_YUM;
				default: return GUI_YUM;
			}
		case CURSOR_HEAD:
			switch(direction) {
				case NUVIE_DIR_N:
				case NUVIE_DIR_NW: cursor_pos = cursor_left; return GUI_YUM;
				case NUVIE_DIR_NE: cursor_pos = cursor_right; return GUI_YUM;
				case NUVIE_DIR_W:
				case NUVIE_DIR_SW: cursor_pos = CURSOR_NECK; return GUI_YUM;
				case NUVIE_DIR_E:
				case NUVIE_DIR_SE: cursor_pos = CURSOR_CHEST; return GUI_YUM;
				case NUVIE_DIR_S: cursor_pos = CURSOR_FEET; return GUI_YUM;
				default: return GUI_YUM;
			}
		case CURSOR_NECK:
			switch(direction) {
				case NUVIE_DIR_N:
				case NUVIE_DIR_NW: cursor_pos = cursor_left; return GUI_YUM;
				case NUVIE_DIR_E:
				case NUVIE_DIR_NE: cursor_pos = CURSOR_HEAD; return GUI_YUM;
				case NUVIE_DIR_S:
				case NUVIE_DIR_SW: cursor_pos = CURSOR_RIGHT_HAND; return GUI_YUM;
				case NUVIE_DIR_SE: cursor_pos = CURSOR_LEFT_HAND; return GUI_YUM;
				default: return GUI_YUM;
			}
		case CURSOR_CHEST:
			switch(direction) {
				case NUVIE_DIR_N:
				case NUVIE_DIR_NE: cursor_pos = cursor_right; return GUI_YUM;
				case NUVIE_DIR_W:
				case NUVIE_DIR_NW: cursor_pos = CURSOR_HEAD; return GUI_YUM;
				case NUVIE_DIR_SW: cursor_pos = CURSOR_RIGHT_HAND; return GUI_YUM;
				case NUVIE_DIR_S:
				case NUVIE_DIR_SE: cursor_pos = CURSOR_LEFT_HAND; return GUI_YUM;
				default: return GUI_YUM;
			}
		case CURSOR_RIGHT_HAND:
			switch(direction) {
				case NUVIE_DIR_N:
				case NUVIE_DIR_NW: cursor_pos = CURSOR_NECK; return GUI_YUM;
				case NUVIE_DIR_NE: cursor_pos = CURSOR_CHEST; return GUI_YUM;
				case NUVIE_DIR_E: cursor_pos = CURSOR_LEFT_HAND; return GUI_YUM;
				case NUVIE_DIR_SE: cursor_pos = CURSOR_FEET; return GUI_YUM;
				case NUVIE_DIR_S:
				case NUVIE_DIR_SW: cursor_pos = CURSOR_RIGHT_RING; return GUI_YUM;
				default: return GUI_YUM;
			}
		case CURSOR_LEFT_HAND:
			switch(direction) {
				case NUVIE_DIR_N:
				case NUVIE_DIR_NE: cursor_pos = CURSOR_CHEST; return GUI_YUM;
				case NUVIE_DIR_NW: cursor_pos = CURSOR_HEAD; return GUI_YUM;
				case NUVIE_DIR_W: cursor_pos = CURSOR_RIGHT_HAND; return GUI_YUM;
				case NUVIE_DIR_SW: cursor_pos = CURSOR_FEET; return GUI_YUM;
				case NUVIE_DIR_S:
				case NUVIE_DIR_SE: cursor_pos = CURSOR_LEFT_RING; return GUI_YUM;
				default: return GUI_YUM;
			}
		case CURSOR_RIGHT_RING:
			switch(direction) {
				case NUVIE_DIR_N:
				case NUVIE_DIR_NW: cursor_pos = CURSOR_RIGHT_HAND; return GUI_YUM;
				case NUVIE_DIR_NE: cursor_pos = CURSOR_LEFT_HAND; return GUI_YUM;
				case NUVIE_DIR_S:
				case NUVIE_DIR_SE: cursor_pos = CURSOR_FEET; return GUI_YUM;
				case NUVIE_DIR_SW: cursor_pos = CURSOR_COMBAT; return GUI_YUM;
				case NUVIE_DIR_E: cursor_pos = CURSOR_LEFT_RING;
				default: return GUI_YUM;
			}
		case CURSOR_LEFT_RING:
			switch(direction) {
				case NUVIE_DIR_N:
				case NUVIE_DIR_NE: cursor_pos = CURSOR_LEFT_HAND; return GUI_YUM;
				case NUVIE_DIR_NW: cursor_pos = CURSOR_RIGHT_HAND; return GUI_YUM;
				case NUVIE_DIR_W: cursor_pos = CURSOR_RIGHT_RING; return GUI_YUM;
				case NUVIE_DIR_S:
				case NUVIE_DIR_SW: cursor_pos = CURSOR_FEET; return GUI_YUM;
				case NUVIE_DIR_SE: cursor_pos = CURSOR_COMBAT; return GUI_YUM;
				default: return GUI_YUM;
			}
		case CURSOR_FEET:
			switch(direction) {
				case NUVIE_DIR_N: cursor_pos = CURSOR_HEAD; return GUI_YUM;
				case NUVIE_DIR_W:
				case NUVIE_DIR_NW: cursor_pos = CURSOR_RIGHT_RING; return GUI_YUM;
				case NUVIE_DIR_E:
				case NUVIE_DIR_NE: cursor_pos = CURSOR_LEFT_RING; return GUI_YUM;
				case NUVIE_DIR_S:
				case NUVIE_DIR_SW:
				case NUVIE_DIR_SE: cursor_pos = CURSOR_COMBAT; return GUI_YUM;
				default: return GUI_YUM;
			}
		case CURSOR_COMBAT:
			switch(direction) {
				case NUVIE_DIR_N:
				case NUVIE_DIR_NW: cursor_pos = CURSOR_RIGHT_RING; return GUI_YUM;
				case NUVIE_DIR_NE: cursor_pos = CURSOR_FEET; return GUI_YUM;
				case NUVIE_DIR_SW: cursor_pos = CURSOR_CHECK; return GUI_YUM;
				case NUVIE_DIR_SE: cursor_pos = cursor_party; return GUI_YUM;
				case NUVIE_DIR_S: cursor_pos = cursor_heart; return GUI_YUM;
				default: return GUI_YUM;
			}
		case CURSOR_CHECK:
			switch(direction) {
				case NUVIE_DIR_NE: cursor_pos = CURSOR_COMBAT; return GUI_YUM;
				case NUVIE_DIR_E:
				case NUVIE_DIR_SE: cursor_pos = cursor_heart; return GUI_YUM;
				default: return GUI_YUM;
			}
		case CURSOR_HEART:
			switch(direction) {
				case NUVIE_DIR_N:
				case NUVIE_DIR_NW:
				case NUVIE_DIR_NE: cursor_pos = CURSOR_COMBAT; return GUI_YUM;
				case NUVIE_DIR_W:
				case NUVIE_DIR_SW: cursor_pos = CURSOR_CHECK; return GUI_YUM;
				case NUVIE_DIR_E:
				case NUVIE_DIR_SE: cursor_pos = cursor_party; return GUI_YUM;
				default: return GUI_YUM;
			}
		case CURSOR_PARTY:
			switch(direction) {
				case NUVIE_DIR_N:
				case NUVIE_DIR_NW:
				case NUVIE_DIR_NE: cursor_pos = CURSOR_COMBAT; return GUI_YUM;
				case NUVIE_DIR_W:
				case NUVIE_DIR_SW: cursor_pos = cursor_heart; return GUI_YUM;
				case NUVIE_DIR_E:
				case NUVIE_DIR_SE: cursor_pos = CURSOR_INVENTORY; return GUI_YUM;
				default: return GUI_YUM;
			}
		case CURSOR_INVENTORY:
			switch(direction) {
				case NUVIE_DIR_N:
				case NUVIE_DIR_NW:
				case NUVIE_DIR_NE: cursor_pos = CURSOR_COMBAT; return GUI_YUM;
				case NUVIE_DIR_W:
				case NUVIE_DIR_SW: cursor_pos = cursor_party; return GUI_YUM;
				default: return GUI_YUM;
			}
		default: return GUI_YUM;
	}
}

GUI_status DollViewGump::KeyDown(SDL_keysym key)
{
	bool numlock = (key.mod & KMOD_NUM); // SDL doesn't get the proper num lock state in Windows
	switch(key.sym) {
		case SDLK_KP1: if(numlock) break;
			return moveCursorRelative( NUVIE_DIR_SW);
		case SDLK_KP3: if(numlock) break;
			return moveCursorRelative( NUVIE_DIR_SE);
		case SDLK_KP7: if(numlock) break;
			return moveCursorRelative( NUVIE_DIR_NW);
		case SDLK_KP9: if(numlock) break;
			return moveCursorRelative( NUVIE_DIR_NE);
		case SDLK_KP8: if(numlock) break;
		case SDLK_UP:
			return moveCursorRelative( NUVIE_DIR_N);
		case SDLK_KP2: if(numlock) break;
		case SDLK_DOWN:
			return moveCursorRelative( NUVIE_DIR_S);
		case SDLK_KP4: if(numlock) break;
		case SDLK_LEFT:
			return moveCursorRelative( NUVIE_DIR_W);
		case SDLK_KP6: if(numlock) break;
		case SDLK_RIGHT:
			return moveCursorRelative( NUVIE_DIR_E);
		case SDLK_RETURN:
		case SDLK_KP_ENTER: {
			Event *event = Game::get_game()->get_event();
			bool in_party = party->get_member_num(actor) >= 0;
			if(event->get_mode() == ATTACK_MODE || cursor_pos == CURSOR_CHECK) {
				Game::get_game()->get_view_manager()->close_gump(this);
			} else if(cursor_pos == CURSOR_LEFT) {
				if(in_party)
					left_arrow();
			} else if(cursor_pos == CURSOR_RIGHT) {
				if(in_party)
					right_arrow();
			} else if(cursor_pos == CURSOR_COMBAT) {
				activate_combat_button();
			} else if(cursor_pos == CURSOR_HEART) {
				if(in_party || event->using_control_cheat())
					Game::get_game()->get_view_manager()->open_portrait_gump(actor);
			} else if(cursor_pos == CURSOR_PARTY) {
				if(in_party)
					{}
			} else if(cursor_pos == CURSOR_INVENTORY) {
				Game::get_game()->get_view_manager()->open_container_view(actor);
			} else {
				Obj *obj = actor->inventory_get_readied_object((uint8)cursor_pos);
				if(event->get_mode() == MOVE_MODE || event->get_mode() == EQUIP_MODE) {
					if(obj)
						event->unready(obj);
				} else
					event->select_view_obj(obj, actor);
			}
			return GUI_YUM;
		}
		default: break;
	}
	return GUI_PASS;
}

void DollViewGump::activate_combat_button()
{
	Event *event = Game::get_game()->get_event();
	if(actor->is_in_party() && party->get_member_num(actor) != 0) {
		set_combat_mode(actor);
		update_display = true;
	} else if(event->get_mode() != INPUT_MODE && event->get_mode() != CAST_MODE
	          && event->get_mode() != ATTACK_MODE )
		event->newAction(COMBAT_MODE);
}

GUI_status DollViewGump::MouseDown(int x, int y, int button)
{
 if(party->get_member_num(actor) >= 0)
 {
	 if(button == SDL_BUTTON_WHEELDOWN)
	 {
		right_arrow();
		return GUI_YUM;
	 }
	 else if(button == SDL_BUTTON_WHEELUP)
	 {
		 left_arrow();
		 return GUI_YUM;
	 }
 }

	return DraggableView::MouseDown(x, y, button);
}

GUI_status DollViewGump::MouseUp(int x, int y, int button)
{
	return DraggableView::MouseUp(x, y, button);
}
