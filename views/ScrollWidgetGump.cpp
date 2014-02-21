/*
 *  ScrollWidgetGump.cpp
 *  Nuvie
 *
 *  Created by Eric Fry on Wed Nov 06 2013.
 *  Copyright (c) 2013. All rights reserved.
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
#include <string>
#include <cctype>
#include <iostream>

#include "nuvieDefs.h"
#include "Configuration.h"
#include "U6misc.h"
#include "FontManager.h"
#include "Font.h"
#include "GamePalette.h"
#include "GUI.h"
#include "MsgScroll.h"
#include "Portrait.h"
#include "Player.h"
#include "ConvFont.h"
#include "ScrollWidgetGump.h"
#include "ActorManager.h"
#include "TimedEvent.h"
#include "Keys.h"

// ScrollWidgetGump Class

ScrollWidgetGump::ScrollWidgetGump(Configuration *cfg, Screen *s)
{
 drop_target = false; //we don't participate in drag and drop.

 font_normal = Game::get_game()->get_font_manager()->get_conv_font();

 font_garg = Game::get_game()->get_font_manager()->get_conv_garg_font();

 init(cfg, font_normal);

 font_color = 0; // black
 font_highlight = FONT_COLOR_U6_HIGHLIGHT;

 scroll_width = 40;
 scroll_height = 10;

 show_up_arrow = false;
 show_down_arrow = false;

 GUI_Widget::Init(NULL, 0, 0, SCROLLWIDGETGUMP_W, SCROLLWIDGETGUMP_H);

 add_new_line(); //MsgScroll requires a line to start.

 position = 0;
// ignore_page_breaks = true;
}

ScrollWidgetGump::~ScrollWidgetGump()
{
// ignore_page_breaks = false;
}

void ScrollWidgetGump::set_font(uint8 font_type)
{
	if(font_type == NUVIE_FONT_NORMAL)
	{
		font = font_normal;
	}
	else
	{
		font = font_garg;
	}
}

bool ScrollWidgetGump::is_garg_font()
{
	return (font==font_garg);
}

bool ScrollWidgetGump::can_fit_token_on_msgline(MsgLine *msg_line, MsgText *token)
{
  if(msg_line->get_display_width() + token->getDisplayWidth() > SCROLLWIDGETGUMP_W - 8 - 8)
  {
    return false; //token doesn't fit on the current line.
  }

  return true;
}

void ScrollWidgetGump::display_string(std::string s)
{
  MsgScroll::display_string(s);
  update_arrows();
}

void ScrollWidgetGump::Display(bool full_redraw)
{
	MsgText *token;

	uint16 y = area.y + 4;
	std::list<MsgLine *>::iterator iter;

	if(show_up_arrow)
	{
	  font_normal->drawChar(screen, FONT_UP_ARROW_CHAR, area.x + SCROLLWIDGETGUMP_W - 8, area.y + 4);
	}

  if(show_down_arrow)
  {
    font_normal->drawChar(screen, FONT_DOWN_ARROW_CHAR, area.x + SCROLLWIDGETGUMP_W - 8, area.y + SCROLLWIDGETGUMP_H - 8);
  }

	iter=msg_buf.begin();
	for(uint16 i=0;i < position && iter != msg_buf.end(); i++)
		iter++;

	for(uint16 i=0;i< scroll_height && iter != msg_buf.end();i++,iter++)
	{
		MsgLine *msg_line = *iter;
		std::list<MsgText *>::iterator iter1;

		iter1=msg_line->text.begin();

		//if not last record or if last record is not an empty line.
		if(i + position < (msg_buf.size()-1) || (iter1 != msg_line->text.end() && ((*iter)->total_length != 0)))
		{
		  //screen->fill(26, area.x, y + (i==0?-4:4), scroll_width * 7 + 8, (i==0?18:10));


			for(uint16 total_length = 0;iter1 != msg_line->text.end() ; iter1++)
			{
				token = *iter1;

				total_length += token->font->drawString(screen, token->s.c_str(), area.x + 4 + 4 + total_length, y + 4, font_color, font_highlight); //FIX for hardcoded font height
			}
			y+=10;
		}

	}

	screen->update(area.x,area.y, area.w, area.h);
}

GUI_status ScrollWidgetGump::KeyDown(SDL_keysym key)
{
	ScrollEventType event = SCROLL_ESCAPE;

	KeyBinder *keybinder = Game::get_game()->get_keybinder();
	ActionType a = keybinder->get_ActionType(key);

	switch(keybinder->GetActionKeyType(a))
	{
	case MSGSCROLL_DOWN_KEY: event = SCROLL_PAGE_DOWN; break;
	case SOUTH_KEY: event = SCROLL_DOWN; break;
	case MSGSCROLL_UP_KEY: event = SCROLL_PAGE_UP; break;
	case NORTH_KEY: event = SCROLL_UP; break;
	case HOME_KEY: event = SCROLL_TO_BEGINNING; break;
	case END_KEY: event = SCROLL_TO_END; break;
	default : break;
	}

	if(scroll_movement_event(event) == GUI_YUM)
		return GUI_YUM;

    return MsgScroll::KeyDown(key);
}

static SDL_Rect arrow_up_rect[1] = {{SCROLLWIDGETGUMP_W - 8 - 1, 4 + 1, 7, 5}};
static SDL_Rect arrow_down_rect[1] = {{SCROLLWIDGETGUMP_W - 8 - 1, SCROLLWIDGETGUMP_H - 8 + 3, 7 , 5}};

GUI_status ScrollWidgetGump::MouseDown(int x, int y, int button)
{
	ScrollEventType event = SCROLL_ESCAPE;

	switch(button)
	{
	case SDL_BUTTON_WHEELDOWN : event = SCROLL_DOWN; break;
	case SDL_BUTTON_WHEELUP : event = SCROLL_UP; break;
	case SDL_BUTTON_LEFT : {
	                       x -= area.x;
	                       y -= area.y;
	                       if(HitRect(x,y, arrow_up_rect[0]))
	                           event = SCROLL_UP;
	                       else if(HitRect(x,y, arrow_down_rect[0]))
	                           event = SCROLL_DOWN;
// FIXME - uncomment when we get a checkmark
//	                       else if(show_down_arrow || show_up_arrow) // don't close if scrollable
//	                           return GUI_YUM;
	                       break;
	}
	default : break;
	}

	return scroll_movement_event(event);
}

GUI_status ScrollWidgetGump::scroll_movement_event(ScrollEventType event)
{
	switch(event)
	{
	case SCROLL_UP :
		if(position > 0)
		{
			//timer = new TimedCallback(this, NULL, 2000);
			position--;
			update_arrows();
			//grab_focus();
		}
		return GUI_YUM;

	case SCROLL_DOWN :
		//timer = new TimedCallback(this, NULL, 2000);
		if(page_break && position + scroll_height == msg_buf.size())
		{
			process_page_break();
			update_arrows(); // just in case noting is added to the buffer
		}
		if(position + scroll_height < msg_buf.size())
		{
			position++;
			update_arrows();
		}
		return (GUI_YUM);
	case SCROLL_PAGE_UP:
		if(position > 0)
		{
			position = position > scroll_height ? position - scroll_height : 0;
			update_arrows();
		}
		return GUI_YUM;
	case SCROLL_PAGE_DOWN:
		if(position + scroll_height < msg_buf.size() || page_break)
		{
			for(int i=0; i < scroll_height; i++)
			{
				if(position + scroll_height < msg_buf.size())
					position++;
				else if(page_break) // we didn't have enough text showing to fill out the current page.
				{
					process_page_break();
					for(; i < scroll_height; i++) { // Stop advancing text position
						if(page_break && position + size_t(scroll_height*2) <= msg_buf.size()) // make sure we have enough text to fill the page
							process_page_break();
						else
							i = scroll_height; // break out of both loops
					}
				}
			}
			update_arrows();
		}
		return GUI_YUM;
	case SCROLL_TO_BEGINNING :
		if(position > 0)
		{
			position = 0;
			update_arrows();
		}
		return GUI_YUM;
	case SCROLL_TO_END :
		if(position + scroll_height < msg_buf.size() || page_break)
		{
			while(position + scroll_height < msg_buf.size() || page_break)
			{
				if(page_break)
					process_page_break();
				else // added else just in case noting is added to the buffer
					position++;
			}
			update_arrows();
		}
		return GUI_YUM;
	default :
		//release_focus();
		//new TimedCallback(this, NULL, 50);
		break;
	}

	return GUI_PASS;
}

void ScrollWidgetGump::update_arrows()
{
  if(position == 0)
  {
    show_up_arrow = false;
  }
  else
  {
    show_up_arrow = true;
  }

  if(position + scroll_height < msg_buf.size() || page_break)
  {
    show_down_arrow = true;
  }
  else
  {
    show_down_arrow = false;
  }
}
