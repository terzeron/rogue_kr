/*
 * Contains functions for dealing with things like potions, scrolls,
 * and other items.
 *
 * @(#)things.c	4.53 (Berkeley) 02/05/99
 *
 * Rogue: Exploring the Dungeons of Doom
 * Copyright (C) 1980-1983, 1985, 1999 Michael Toy, Ken Arnold and Glenn Wichman
 * All rights reserved.
 *
 * See the file LICENSE.TXT for full copyright and licensing information.
 */

#include <curses.h>
#include <string.h>
#include <ctype.h>
#include "rogue.h"
#include "i18n.h"
#include "i18n_korean.h"

/*
 * inv_name:
 *	Return the name of something as it would appear in an
 *	inventory.
 */
char *
inv_name(THING *obj, bool drop)
{
    char *pb;
    struct obj_info *op;
    const char *sp;
    int which;

    pb = prbuf;
    which = obj->o_which;
    switch (obj->o_type)
    {
        case POTION:
	    nameit(obj, POTION, p_colors[which], &pot_info[which], nullstr);
	when RING:
	    nameit(obj, RING, r_stones[which], &ring_info[which], ring_num);
	when STICK:
	    nameit(obj, STICK, ws_made[which], &ws_info[which], charge_str);
	when SCROLL:
	    {
		char *lang = getenv("LANG");
		int is_korean = (lang != NULL && strncmp(lang, "ko", 2) == 0);

		op = &scr_info[which];

		if (is_korean)
		{
		    /* Korean word order */
		    if (op->oi_know)
		    {
			/* Known: "name의 두루마리" */
			sprintf(pb, "%s의 %s", op->oi_name, msg_get("MSG_SCROLL"));
		    }
		    else if (op->oi_guess)
		    {
			/* Guessed: "guess(이)라고 불리는 두루마리" */
			sprintf(pb, "%s(이)라고 불리는 %s", op->oi_guess, msg_get("MSG_SCROLL"));
		    }
		    else
		    {
			/* Unknown: "'title'이라는 이름의 두루마리" */
			const char *transliterated = transliterate_to_korean(s_names[which]);
			if (obj->o_count == 1)
			    sprintf(pb, "'%s'이라는 이름의 %s", transliterated, msg_get("MSG_SCROLL"));
			else
			    sprintf(pb, "%d개의 '%s'이라는 이름의 %s", obj->o_count, transliterated, msg_get("MSG_SCROLL"));
		    }
		}
		else
		{
		    /* English word order */
		    if (obj->o_count == 1)
		    {
			strcpy(pb, msg_get("MSG_THING_SCROLL_A"));
			strcat(pb, " ");
			pb = &prbuf[strlen(prbuf)];
		    }
		    else
		    {
			sprintf(pb, msg_get("MSG_THING_SCROLLS"), obj->o_count);
			strcat(pb, " ");
			pb = &prbuf[strlen(prbuf)];
		    }
		    if (op->oi_know)
			sprintf(pb, msg_get("MSG_THING_OF"), op->oi_name);
		    else if (op->oi_guess)
			sprintf(pb, msg_get("MSG_THING_CALLED"), op->oi_guess);
		    else
			sprintf(pb, msg_get("MSG_THING_TITLED"), s_names[which]);
		}
	    }
	when FOOD:
	    {
		char *lang = getenv("LANG");
		int is_korean = (lang != NULL && strncmp(lang, "ko", 2) == 0);

		if (which == 1)
		{
		    if (obj->o_count == 1)
			sprintf(pb, msg_get("MSG_FORMAT_A_FRUIT"), vowelstr(fruit), fruit);
		    else
			sprintf(pb, msg_get("MSG_FORMAT_N_FRUITS"), obj->o_count, fruit);
		}
		else
		{
		    if (obj->o_count == 1)
		    {
			strcpy(pb, msg_get("MSG_THING_SOME_FOOD"));
		    }
		    else
		    {
			sprintf(pb, msg_get("MSG_THING_RATIONS"), obj->o_count);
		    }
		}
	    }
	when WEAPON:
	    {
		char *lang = getenv("LANG");
		int is_korean = (lang != NULL && strncmp(lang, "ko", 2) == 0);

		sp = is_korean ? msg_get_weapon_name(which) : weap_info[which].oi_name;

		if (is_korean)
		{
		    /* Korean word order */
		    if (obj->o_count > 1)
			sprintf(pb, msg_get("MSG_FORMAT_WEAPON_N_KO"), sp, obj->o_count);
		    else if (obj->o_flags & ISKNOW)
			sprintf(pb, msg_get("MSG_FORMAT_WEAPON_KNOWN_KO"), num(obj->o_hplus,obj->o_dplus,WEAPON), sp);
		    else
			sprintf(pb, "%s", sp);
		}
		else
		{
		    /* English word order */
		    if (obj->o_count > 1)
			sprintf(pb, "%d ", obj->o_count);
		    else
			sprintf(pb, msg_get("MSG_FORMAT_A_WEAPON"), vowelstr(sp));
		    pb = &prbuf[strlen(prbuf)];
		    if (obj->o_flags & ISKNOW)
			sprintf(pb, msg_get("MSG_FORMAT_WEAPON_KNOWN_EN"), num(obj->o_hplus,obj->o_dplus,WEAPON), sp);
		    else
			sprintf(pb, "%s", sp);
		    if (obj->o_count > 1)
			strcat(pb, msg_get("MSG_FORMAT_PLURAL_S"));
		}

		if (obj->o_label != NULL)
		{
		    pb = &prbuf[strlen(prbuf)];
		    sprintf(pb, msg_get("MSG_FORMAT_LABELED"), obj->o_label);
		}
	    }
	when ARMOR:
	    {
		char *lang = getenv("LANG");
		int is_korean = (lang != NULL && strncmp(lang, "ko", 2) == 0);

		sp = is_korean ? msg_get_armor_name(which) : arm_info[which].oi_name;

		if (obj->o_flags & ISKNOW)
		{
		    if (is_korean)
		    {
			/* Korean: "+2 가죽 갑옷 [방어력 8]" */
			sprintf(pb, msg_get("MSG_FORMAT_ARMOR_KNOWN_KO"),
			    num(a_class[which] - obj->o_arm, 0, ARMOR), sp,
			    terse ? "" : msg_get("MSG_ARMOR_PROTECTION_LABEL"), 10 - obj->o_arm);
		    }
		    else
		    {
			/* English: "+2 leather armor [protection 8]" */
			sprintf(pb, msg_get("MSG_FORMAT_ARMOR_KNOWN_EN_START"),
			    num(a_class[which] - obj->o_arm, 0, ARMOR), sp);
			if (!terse)
			    strcat(pb, msg_get("MSG_ARMOR_PROTECTION_WORD"));
			pb = &prbuf[strlen(prbuf)];
			sprintf(pb, msg_get("MSG_FORMAT_ARMOR_KNOWN_EN_END"), 10 - obj->o_arm);
		    }
		}
		else
		    sprintf(pb, "%s", sp);

		if (obj->o_label != NULL)
		{
		    pb = &prbuf[strlen(prbuf)];
		    sprintf(pb, msg_get("MSG_FORMAT_LABELED"), obj->o_label);
		}
	    }
	when AMULET:
	    strcpy(pb, msg_get("MSG_THING_AMULET"));
	when GOLD:
	    sprintf(prbuf, msg_get("MSG_THING_GOLD_PIECES"), obj->o_goldval);
#ifdef MASTER
	otherwise:
	    debug("Picked up something funny %s", unctrl(obj->o_type));
	    sprintf(pb, msg_get("MSG_THING_BIZARRE"), unctrl(obj->o_type));
#endif
    }
    if (inv_describe)
    {
	if (obj == cur_armor)
	    strcat(pb, msg_get("MSG_THING_BEING_WORN"));
	if (obj == cur_weapon)
	    strcat(pb, msg_get("MSG_THING_WEAPON_IN_HAND"));
	if (obj == cur_ring[LEFT])
	    strcat(pb, msg_get("MSG_THING_ON_LEFT_HAND"));
	else if (obj == cur_ring[RIGHT])
	    strcat(pb, msg_get("MSG_THING_ON_RIGHT_HAND"));
    }
    if (drop && isupper(prbuf[0]))
	prbuf[0] = (char) tolower(prbuf[0]);
    else if (!drop && islower(*prbuf))
	*prbuf = (char) toupper(*prbuf);
    prbuf[MAXSTR-1] = '\0';
    return prbuf;
}

/*
 * drop:
 *	Put something down
 */

void
drop()
{
    char ch;
    THING *obj;

    ch = chat(hero.y, hero.x);
    if (ch != FLOOR && ch != PASSAGE)
    {
	after = FALSE;
	msg(msg_get("MSG_THING_SOMETHING_THERE"));
	return;
    }
    if ((obj = get_item(msg_get("MSG_PURPOSE_DROP"), 0)) == NULL)
	return;
    if (!dropcheck(obj))
	return;
    obj = leave_pack(obj, TRUE, (bool)!ISMULT(obj->o_type));
    /*
     * Link it into the level object list
     */
    attach(lvl_obj, obj);
    chat(hero.y, hero.x) = (char) obj->o_type;
    flat(hero.y, hero.x) |= F_DROPPED;
    obj->o_pos = hero;
    if (obj->o_type == AMULET)
	amulet = FALSE;
    msg(msg_get("MSG_THING_DROPPED"), inv_name(obj, TRUE));
}

/*
 * dropcheck:
 *	Do special checks for dropping or unweilding|unwearing|unringing
 */
bool
dropcheck(THING *obj)
{
    if (obj == NULL)
	return TRUE;
    if (obj != cur_armor && obj != cur_weapon
	&& obj != cur_ring[LEFT] && obj != cur_ring[RIGHT])
	    return TRUE;
    if (obj->o_flags & ISCURSED)
    {
	msg(msg_get("MSG_THING_CANT_CURSED"));
	return FALSE;
    }
    if (obj == cur_weapon)
	cur_weapon = NULL;
    else if (obj == cur_armor)
    {
	waste_time();
	cur_armor = NULL;
    }
    else
    {
	cur_ring[obj == cur_ring[LEFT] ? LEFT : RIGHT] = NULL;
	switch (obj->o_which)
	{
	    case R_ADDSTR:
		chg_str(-obj->o_arm);
		break;
	    case R_SEEINVIS:
		unsee();
		extinguish(unsee);
		break;
	}
    }
    return TRUE;
}

/*
 * new_thing:
 *	Return a new thing
 */
THING *
new_thing()
{
    THING *cur;
    int r;

    cur = new_item();
    cur->o_hplus = 0;
    cur->o_dplus = 0;
    strncpy(cur->o_damage, "0x0", sizeof(cur->o_damage));
    strncpy(cur->o_hurldmg, "0x0", sizeof(cur->o_hurldmg));
    cur->o_arm = 11;
    cur->o_count = 1;
    cur->o_group = 0;
    cur->o_flags = 0;
    /*
     * Decide what kind of object it will be
     * If we haven't had food for a while, let it be food.
     */
    switch (no_food > 3 ? 2 : pick_one(things, NUMTHINGS))
    {
	case 0:
	    cur->o_type = POTION;
	    cur->o_which = pick_one(pot_info, MAXPOTIONS);
	when 1:
	    cur->o_type = SCROLL;
	    cur->o_which = pick_one(scr_info, MAXSCROLLS);
	when 2:
	    cur->o_type = FOOD;
	    no_food = 0;
	    if (rnd(10) != 0)
		cur->o_which = 0;
	    else
		cur->o_which = 1;
	when 3:
	    init_weapon(cur, pick_one(weap_info, MAXWEAPONS));
	    if ((r = rnd(100)) < 10)
	    {
		cur->o_flags |= ISCURSED;
		cur->o_hplus -= rnd(3) + 1;
	    }
	    else if (r < 15)
		cur->o_hplus += rnd(3) + 1;
	when 4:
	    cur->o_type = ARMOR;
	    cur->o_which = pick_one(arm_info, MAXARMORS);
	    cur->o_arm = a_class[cur->o_which];
	    if ((r = rnd(100)) < 20)
	    {
		cur->o_flags |= ISCURSED;
		cur->o_arm += rnd(3) + 1;
	    }
	    else if (r < 28)
		cur->o_arm -= rnd(3) + 1;
	when 5:
	    cur->o_type = RING;
	    cur->o_which = pick_one(ring_info, MAXRINGS);
	    switch (cur->o_which)
	    {
		case R_ADDSTR:
		case R_PROTECT:
		case R_ADDHIT:
		case R_ADDDAM:
		    if ((cur->o_arm = rnd(3)) == 0)
		    {
			cur->o_arm = -1;
			cur->o_flags |= ISCURSED;
		    }
		when R_AGGR:
		case R_TELEPORT:
		    cur->o_flags |= ISCURSED;
	    }
	when 6:
	    cur->o_type = STICK;
	    cur->o_which = pick_one(ws_info, MAXSTICKS);
	    fix_stick(cur);
#ifdef MASTER
	otherwise:
	    debug("Picked a bad kind of object");
	    wait_for(' ');
#endif
    }
    return cur;
}

/*
 * pick_one:
 *	Pick an item out of a list of nitems possible objects
 */
int
pick_one(struct obj_info *info, int nitems)
{
    struct obj_info *end;
    struct obj_info *start;
    int i;

    start = info;
    for (end = &info[nitems], i = rnd(100); info < end; info++)
	if (i < info->oi_prob)
	    break;
    if (info == end)
    {
#ifdef MASTER
	if (wizard)
	{
	    msg(msg_get("MSG_THING_BAD_PICK_ONE"), i, nitems);
	    for (info = start; info < end; info++)
		msg(msg_get("MSG_THING_ITEM_PROB"), info->oi_name, info->oi_prob);
	}
#endif
	info = start;
    }
    return (int)(info - start);
}

/*
 * discovered:
 *	list what the player has discovered in this game of a certain type
 */
static int line_cnt = 0;

static bool newpage = FALSE;

static char *lastfmt, *lastarg;


void
discovered()
{
    char ch;
    bool disc_list;

    do {
	disc_list = FALSE;
	if (!terse)
	    addmsg(msg_get("MSG_THING_FOR"));
	addmsg(msg_get("MSG_THING_WHAT_TYPE"));
	if (!terse)
	    addmsg(msg_get("MSG_THING_WANT_LIST"));
	msg(msg_get("MSG_THING_PROMPT_ALL"));
	ch = readchar();
	switch (ch)
	{
	    case ESCAPE:
		msg("");
		return;
	    case POTION:
	    case SCROLL:
	    case RING:
	    case STICK:
	    case '*':
		disc_list = TRUE;
		break;
	    default:
		if (terse)
		    msg(msg_get("MSG_THING_NOT_A_TYPE"));
		else
		    msg(msg_get("MSG_THING_PLEASE_TYPE"), POTION, SCROLL, RING, STICK);
	}
    } while (!disc_list);
    if (ch == '*')
    {
	print_disc(POTION);
	add_line("", NULL);
	print_disc(SCROLL);
	add_line("", NULL);
	print_disc(RING);
	add_line("", NULL);
	print_disc(STICK);
	end_line();
    }
    else
    {
	print_disc(ch);
	end_line();
    }
}

/*
 * print_disc:
 *	Print what we've discovered of type 'type'
 */

#define MAX4(a,b,c,d)	(a > b ? (a > c ? (a > d ? a : d) : (c > d ? c : d)) : (b > c ? (b > d ? b : d) : (c > d ? c : d)))


void
print_disc(char type)
{
    struct obj_info *info = NULL;
    int i, maxnum = 0, num_found;
    static THING obj;
    static int order[MAX4(MAXSCROLLS, MAXPOTIONS, MAXRINGS, MAXSTICKS)];

    switch (type)
    {
	case SCROLL:
	    maxnum = MAXSCROLLS;
	    info = scr_info;
	    break;
	case POTION:
	    maxnum = MAXPOTIONS;
	    info = pot_info;
	    break;
	case RING:
	    maxnum = MAXRINGS;
	    info = ring_info;
	    break;
	case STICK:
	    maxnum = MAXSTICKS;
	    info = ws_info;
	    break;
    }
    set_order(order, maxnum);
    obj.o_count = 1;
    obj.o_flags = 0;
    num_found = 0;
    for (i = 0; i < maxnum; i++)
	if (info[order[i]].oi_know || info[order[i]].oi_guess)
	{
	    obj.o_type = type;
	    obj.o_which = order[i];
	    add_line("%s", inv_name(&obj, FALSE));
	    num_found++;
	}
    if (num_found == 0)
	add_line(nothing(type), NULL);
}

/*
 * set_order:
 *	Set up order for list
 */

void
set_order(int *order, int numthings)
{
    int i, r, t;

    for (i = 0; i< numthings; i++)
	order[i] = i;

    for (i = numthings; i > 0; i--)
    {
	r = rnd(i);
	t = order[i - 1];
	order[i - 1] = order[r];
	order[r] = t;
    }
}

/*
 * add_line:
 *	Add a line to the list of discoveries
 */
/* VARARGS1 */
char
add_line(char *fmt, char *arg)
{
    WINDOW *tw, *sw;
    int x, y;
    const char *prompt = msg_get("MSG_PRESS_SPACE");
    static int maxlen = -1;

    if (line_cnt == 0)
    {
	    wclear(hw);
	    if (inv_type == INV_SLOW)
		mpos = 0;
    }
    if (inv_type == INV_SLOW)
    {
	if (*fmt != '\0')
	    if (msg(fmt, arg) == ESCAPE)
		return ESCAPE;
	line_cnt++;
    }
    else
    {
	if (maxlen < 0)
	    maxlen = (int) strlen(prompt);
	if (line_cnt >= LINES - 1 || fmt == NULL)
	{
	    if (inv_type == INV_OVER && fmt == NULL && !newpage)
	    {
		msg("");
		refresh();
		tw = newwin(line_cnt + 1, maxlen + 2, 0, COLS - maxlen - 3);
		sw = subwin(tw, line_cnt + 1, maxlen + 1, 0, COLS - maxlen - 2);
                for (y = 0; y <= line_cnt; y++)
                {
                    wmove(sw, y, 0);
                    for (x = 0; x <= maxlen; x++)
                        waddch(sw, mvwinch(hw, y, x));
                }
		wmove(tw, line_cnt, 1);
		waddstr(tw, prompt);
		/*
		 * if there are lines below, use 'em
		 */
		if (LINES > NUMLINES)
		{
		    if (NUMLINES + line_cnt > LINES)
			mvwin(tw, LINES - (line_cnt + 1), COLS - maxlen - 3);
		    else
			mvwin(tw, NUMLINES, 0);
		}
		touchwin(tw);
		wrefresh(tw);
		wait_for(' ');
                if (md_hasclreol())
		{
		    werase(tw);
		    leaveok(tw, TRUE);
		    wrefresh(tw);
		}
		delwin(tw);
		touchwin(stdscr);
	    }
	    else
	    {
		wmove(hw, LINES - 1, 0);
		waddstr(hw, prompt);
		wrefresh(hw);
		wait_for(' ');
		clearok(curscr, TRUE);
		wclear(hw);
		touchwin(stdscr);
	    }
	    newpage = TRUE;
	    line_cnt = 0;
	    maxlen = (int) strlen(prompt);
	}
	if (fmt != NULL && !(line_cnt == 0 && *fmt == '\0'))
	{
	    mvwprintw(hw, line_cnt++, 0, fmt, arg);
	    getyx(hw, y, x);
	    if (maxlen < x)
		maxlen = x;
	    lastfmt = fmt;
	    lastarg = arg;
	}
    }
    return ~ESCAPE;
}

/*
 * end_line:
 *	End the list of lines
 */

void
end_line()
{
    if (inv_type != INV_SLOW)
    {
	if (line_cnt == 1 && !newpage)
	{
	    mpos = 0;
	    msg(lastfmt, lastarg);
	}
	else
	    add_line((char *) NULL, NULL);
    }
    line_cnt = 0;
    newpage = FALSE;
}

/*
 * nothing:
 *	Set up prbuf so that message for "nothing found" is there
 */
char *
nothing(char type)
{
    char *sp, *tystr = NULL;

    if (terse)
	sprintf(prbuf, msg_get("MSG_THING_NOTHING"));
    else
	sprintf(prbuf, msg_get("MSG_THING_HAVENT_DISCOVERED"));
    if (type != '*')
    {
	sp = &prbuf[strlen(prbuf)];
	switch (type)
	{
	    case POTION: tystr = msg_get("MSG_POTION");
	    when SCROLL: tystr = msg_get("MSG_SCROLL");
	    when RING: tystr = msg_get("MSG_RING");
	    when STICK: tystr = msg_get("MSG_STICK");
	}
	sprintf(sp, msg_get("MSG_THING_ABOUT_ANY"), tystr);
    }
    return prbuf;
}

/*
 * nameit:
 *	Give the proper name to a potion, stick, or ring
 */

void
nameit(THING *obj, int type, char *which, struct obj_info *op,
    char *(*prfunc)(THING *))
{
    char *pb;
    char *lang = getenv("LANG");
    int is_korean = (lang != NULL && strncmp(lang, "ko", 2) == 0);
    const char *type_msg_id = NULL;
    const char *type_en = NULL;
    int is_wand = 0;

    /* Determine message ID and English name based on type constant */
    switch (type)
    {
	case POTION:
	    type_msg_id = "MSG_POTION";
	    type_en = "potion";
	    break;
	case RING:
	    type_msg_id = "MSG_RING";
	    type_en = "ring";
	    break;
	case STICK:
		/* For STICK, determine wand/staff based on ws_type entry */
		is_wand = (strcmp(ws_type[obj->o_which], "wand") == 0);
		type_msg_id = is_wand ? "MSG_WAND" : "MSG_STAFF";
		type_en = is_wand ? "wand" : "staff";
	    break;
	default:
	    type_msg_id = NULL;
	    type_en = "item";
	    break;
    }

    if (op->oi_know || op->oi_guess)
    {
	if (is_korean)
	{
	    /* Korean: name + type (no article) */
	    const char *translated_type = type_msg_id ? msg_get(type_msg_id) : type_en;

	    if (op->oi_know)
		sprintf(prbuf, "%s %s%s", op->oi_name, translated_type, (*prfunc)(obj));
	    else if (op->oi_guess)
		sprintf(prbuf, msg_get("MSG_FORMAT_CALLED_KOREAN"), op->oi_guess, translated_type, (*prfunc)(obj));
	}
	else
	{
	    /* English: article + type + of/called + name */
	    if (obj->o_count == 1)
		sprintf(prbuf, msg_get("MSG_FORMAT_A_TYPE"), type_en);
	    else
		sprintf(prbuf, msg_get("MSG_FORMAT_N_TYPES"), obj->o_count, type_en);
	    pb = &prbuf[strlen(prbuf)];
	    if (op->oi_know)
		sprintf(pb, msg_get("MSG_FORMAT_OF_NAME"), op->oi_name, (*prfunc)(obj), which);
	    else if (op->oi_guess)
		sprintf(pb, msg_get("MSG_FORMAT_CALLED_NAME"), op->oi_guess, (*prfunc)(obj), which);
	}
    }
    else
    {
	/* Unknown item - show color/material/stone */
	if (is_korean)
	{
	    /* Korean: adjective + type (no article) */
	    const char *translated_adjective;
	    const char *translated_type;

	    /* Determine adjective translation based on type */
	    switch (type)
	    {
		case POTION:
		    translated_adjective = msg_get_color(which);
		    translated_type = msg_get("MSG_POTION");
		    break;
		case RING:
		    translated_adjective = msg_get_stone(which);
		    translated_type = msg_get("MSG_RING");
		    break;
			case STICK:
			    translated_adjective = msg_get_material(which);
			    translated_type = msg_get(type_msg_id);
			    break;
			default:
			    translated_adjective = which;
			    translated_type = type_msg_id ? msg_get(type_msg_id) : type_en;
			    break;
	    }

	    if (obj->o_count == 1)
		sprintf(prbuf, "%s %s", translated_adjective, translated_type);
	    else
		sprintf(prbuf, msg_get("MSG_FORMAT_N_ADJECTIVE_TYPE"), obj->o_count, translated_adjective, translated_type);
	}
	else
	{
	    /* English: article + adjective + type */
	    if (obj->o_count == 1)
		sprintf(prbuf, msg_get("MSG_FORMAT_A_ADJ_TYPE"), vowelstr(which), which, type_en);
	    else
		sprintf(prbuf, msg_get("MSG_FORMAT_N_ADJ_TYPES"), obj->o_count, which, type_en);
	}
    }
}

/*
 * nullstr:
 *	Return a pointer to a null-length string
 */
char *
nullstr(THING *ignored)
{
    NOOP(ignored);
    return "";
}

# ifdef	MASTER
/*
 * pr_list:
 *	List possible potions, scrolls, etc. for wizard.
 */

void
pr_list()
{
    int ch;

    if (!terse)
	addmsg(msg_get("MSG_THING_FOR"));
    addmsg(msg_get("MSG_THING_WHAT_TYPE"));
    if (!terse)
	addmsg(msg_get("MSG_THING_WANT_LIST"));
    msg(msg_get("MSG_THING_PROMPT_WIZARD"));
    ch = readchar();
    switch (ch)
    {
	case POTION:
	    pr_spec(pot_info, MAXPOTIONS);
	when SCROLL:
	    pr_spec(scr_info, MAXSCROLLS);
	when RING:
	    pr_spec(ring_info, MAXRINGS);
	when STICK:
	    pr_spec(ws_info, MAXSTICKS);
	when ARMOR:
	    pr_spec(arm_info, MAXARMORS);
	when WEAPON:
	    pr_spec(weap_info, MAXWEAPONS);
	otherwise:
	    return;
    }
}

/*
 * pr_spec:
 *	Print specific list of possible items to choose from
 */

void
pr_spec(struct obj_info *info, int nitems)
{
    struct obj_info *endp;
    int i, lastprob;

    endp = &info[nitems];
    lastprob = 0;
    for (i = '0'; info < endp; i++)
    {
	if (i == '9' + 1)
	    i = 'a';
	sprintf(prbuf, "%c: %%s (%d%%%%)", i, info->oi_prob - lastprob);
	lastprob = info->oi_prob;
	add_line(prbuf, info->oi_name);
	info++;
    }
    end_line();
}
# endif	/* MASTER */
