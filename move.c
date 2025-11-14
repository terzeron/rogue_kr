/*
 * hero movement commands
 *
 * @(#)move.c	4.49 (Berkeley) 02/05/99
 *
 * Rogue: Exploring the Dungeons of Doom
 * Copyright (C) 1980-1983, 1985, 1999 Michael Toy, Ken Arnold and Glenn Wichman
 * All rights reserved.
 *
 * See the file LICENSE.TXT for full copyright and licensing information.
 */

#include <curses.h>
#include <ctype.h>
#include "rogue.h"
#include "i18n.h"

/*
 * used to hold the new hero position
 */

coord nh;

/*
 * do_run:
 *	Start the hero running
 */

void
do_run(char ch)
{
    running = TRUE;
    after = FALSE;
    runch = ch;
}

/*
 * do_move:
 *	Check to see that a move is legal.  If it is handle the
 * consequences (fighting, picking up, etc.)
 */

void
do_move(int dy, int dx)
{
    char ch, fl;

    firstmove = FALSE;
    if (no_move)
    {
	no_move--;
	msg(msg_get("MSG_MOVE_STUCK_BEAR_TRAP"));
	return;
    }
    /*
     * Do a confused move (maybe)
     */
    if (on(player, ISHUH) && rnd(5) != 0)
    {
	nh = *rndmove(&player);
	if (ce(nh, hero))
	{
	    after = FALSE;
	    running = FALSE;
	    to_death = FALSE;
	    return;
	}
    }
    else
    {
over:
	nh.y = hero.y + dy;
	nh.x = hero.x + dx;
    }

    /*
     * Check if he tried to move off the screen or make an illegal
     * diagonal move, and stop him if he did.
     */
    if (nh.x < 0 || nh.x >= NUMCOLS || nh.y <= 0 || nh.y >= NUMLINES - 1)
	goto hit_bound;
    if (!diag_ok(&hero, &nh))
    {
	after = FALSE;
	running = FALSE;
	return;
    }
    if (running && ce(hero, nh))
	after = running = FALSE;
    fl = flat(nh.y, nh.x);
    ch = winat(nh.y, nh.x);
    if (!(fl & F_REAL) && ch == FLOOR)
    {
	if (!on(player, ISLEVIT))
	{
	    chat(nh.y, nh.x) = ch = TRAP;
	    flat(nh.y, nh.x) |= F_REAL;
	}
    }
    else if (on(player, ISHELD) && ch != 'F')
    {
	msg(msg_get("MSG_MOVE_BEING_HELD"));
	return;
    }
    switch (ch)
    {
	case ' ':
	case '|':
	case '-':
hit_bound:
	    if (passgo && running && (proom->r_flags & ISGONE)
		&& !on(player, ISBLIND))
	    {
		bool	b1, b2;

		switch (runch)
		{
		    case 'h':
		    case 'l':
			b1 = (bool)(hero.y != 1 && turn_ok(hero.y - 1, hero.x));
			b2 = (bool)(hero.y != NUMLINES - 2 && turn_ok(hero.y + 1, hero.x));
			if (!(b1 ^ b2))
			    break;
			if (b1)
			{
			    runch = 'k';
			    dy = -1;
			}
			else
			{
			    runch = 'j';
			    dy = 1;
			}
			dx = 0;
			turnref();
			goto over;
		    case 'j':
		    case 'k':
			b1 = (bool)(hero.x != 0 && turn_ok(hero.y, hero.x - 1));
			b2 = (bool)(hero.x != NUMCOLS - 1 && turn_ok(hero.y, hero.x + 1));
			if (!(b1 ^ b2))
			    break;
			if (b1)
			{
			    runch = 'h';
			    dx = -1;
			}
			else
			{
			    runch = 'l';
			    dx = 1;
			}
			dy = 0;
			turnref();
			goto over;
		}
	    }
	    running = FALSE;
	    after = FALSE;
	    break;
	case DOOR:
	    running = FALSE;
	    if (flat(hero.y, hero.x) & F_PASS)
		enter_room(&nh);
	    goto move_stuff;
	case TRAP:
	    ch = be_trapped(&nh);
	    if (ch == T_DOOR || ch == T_TELEP)
		return;
	    goto move_stuff;
	case PASSAGE:
	    /*
	     * when you're in a corridor, you don't know if you're in
	     * a maze room or not, and there ain't no way to find out
	     * if you're leaving a maze room, so it is necessary to
	     * always recalculate proom.
	     */
	    proom = roomin(&hero);
	    goto move_stuff;
	case FLOOR:
	    if (!(fl & F_REAL))
		be_trapped(&hero);
	    goto move_stuff;
	case STAIRS:
	    seenstairs = TRUE;
	    /* FALLTHROUGH */
	default:
	    running = FALSE;
	    if (isupper(ch) || moat(nh.y, nh.x))
		fight(&nh, cur_weapon, FALSE);
	    else
	    {
		if (ch != STAIRS)
		    take = ch;
move_stuff:
		mvaddch(hero.y, hero.x, floor_at());
		if ((fl & F_PASS) && chat(oldpos.y, oldpos.x) == DOOR)
		    leave_room(&nh);
		hero = nh;
	    }
    }
}

/*
 * turn_ok:
 *	Decide whether it is legal to turn onto the given space
 */
bool
turn_ok(int y, int x)
{
    PLACE *pp;

    pp = INDEX(y, x);
    return (pp->p_ch == DOOR
	|| (pp->p_flags & (F_REAL|F_PASS)) == (F_REAL|F_PASS));
}

/*
 * turnref:
 *	Decide whether to refresh at a passage turning or not
 */

void
turnref()
{
    PLACE *pp;

    pp = INDEX(hero.y, hero.x);
    if (!(pp->p_flags & F_SEEN))
    {
	if (jump)
	{
	    leaveok(stdscr, TRUE);
	    refresh();
	    leaveok(stdscr, FALSE);
	}
	pp->p_flags |= F_SEEN;
    }
}

/*
 * door_open:
 *	Called to illuminate a room.  If it is dark, remove anything
 *	that might move.
 */

void
door_open(struct room *rp)
{
    int y, x;

    if (!(rp->r_flags & ISGONE))
	for (y = rp->r_pos.y; y < rp->r_pos.y + rp->r_max.y; y++)
	    for (x = rp->r_pos.x; x < rp->r_pos.x + rp->r_max.x; x++)
		if (isupper(winat(y, x)))
		    wake_monster(y, x);
}

/*
 * be_trapped:
 *	The guy stepped on a trap.... Make him pay.
 */
char
be_trapped(coord *tc)
{
    PLACE *pp;
    THING *arrow;
    char tr;

    if (on(player, ISLEVIT))
	return T_RUST;	/* anything that's not a door or teleport */
    running = FALSE;
    count = FALSE;
    pp = INDEX(tc->y, tc->x);
    pp->p_ch = TRAP;
    tr = pp->p_flags & F_TMASK;
    pp->p_flags |= F_SEEN;
    switch (tr)
    {
	case T_DOOR:
	    level++;
	    new_level();
	    msg(msg_get("MSG_MOVE_FELL_TRAP"));
	when T_BEAR:
	    no_move += BEARTIME;
	    msg(msg_get("MSG_MOVE_CAUGHT_BEAR_TRAP"));
        when T_MYST:
            switch(rnd(11))
            {
                case 0: msg(msg_get("MSG_MOVE_PARALLEL_DIMENSION"));
                when 1: msg(msg_get("MSG_MOVE_LIGHT_SEEMS"), rainbow[rnd(cNCOLORS)]);
                when 2: msg(msg_get("MSG_MOVE_STING_NECK"));
                when 3: msg(msg_get("MSG_MOVE_MULTICOLOR_SWIRL"));
                when 4: msg(msg_get("MSG_MOVE_LIGHT_FLASH"), rainbow[rnd(cNCOLORS)]);
                when 5: msg(msg_get("MSG_MOVE_SPIKE_SHOOTS"));
                when 6: msg(msg_get("MSG_MOVE_SPARKS_DANCE"), rainbow[rnd(cNCOLORS)]);
                when 7: msg(msg_get("MSG_MOVE_FEEL_THIRSTY"));
                when 8: msg(msg_get("MSG_MOVE_TIME_SPEED_UP"));
                when 9: msg(msg_get("MSG_MOVE_TIME_SLOWER"));
                when 10: msg(msg_get("MSG_MOVE_PACK_TURNS"), rainbow[rnd(cNCOLORS)]);
            }
	when T_SLEEP:
	    no_command += SLEEPTIME;
	    player.t_flags &= ~ISRUN;
	    msg(msg_get("MSG_MOVE_WHITE_MIST"));
	when T_ARROW:
	    if (swing(pstats.s_lvl - 1, pstats.s_arm, 1))
	    {
		pstats.s_hpt -= roll(1, 6);
		if (pstats.s_hpt <= 0)
		{
		    msg(msg_get("MSG_MOVE_ARROW_KILLED"));
		    death('a');
		}
		else
		    msg(msg_get("MSG_MOVE_ARROW_SHOT"));
	    }
	    else
	    {
		arrow = new_item();
		init_weapon(arrow, ARROW);
		arrow->o_count = 1;
		arrow->o_pos = hero;
		fall(arrow, FALSE);
		msg(msg_get("MSG_MOVE_ARROW_PAST"));
	    }
	when T_TELEP:
	    /*
	     * since the hero's leaving, look() won't put a TRAP
	     * down for us, so we have to do it ourself
	     */
	    teleport();
	    mvaddch(tc->y, tc->x, TRAP);
	when T_DART:
	    if (!swing(pstats.s_lvl+1, pstats.s_arm, 1))
		msg(msg_get("MSG_MOVE_DART_WHIZZES"));
	    else
	    {
		pstats.s_hpt -= roll(1, 4);
		if (pstats.s_hpt <= 0)
		{
		    msg(msg_get("MSG_MOVE_DART_KILLED"));
		    death('d');
		}
		if (!ISWEARING(R_SUSTSTR) && !save(VS_POISON))
		    chg_str(-1);
		msg(msg_get("MSG_MOVE_DART_HIT"));
	    }
	when T_RUST:
	    msg(msg_get("MSG_MOVE_WATER_GUSH"));
	    rust_armor(cur_armor);
    }
    flush_type();
    return tr;
}

/*
 * rndmove:
 *	Move in a random direction if the monster/person is confused
 */
coord *
rndmove(THING *who)
{
    THING *obj;
    int x, y;
    char ch;
    static coord ret;  /* what we will be returning */

    y = ret.y = who->t_pos.y + rnd(3) - 1;
    x = ret.x = who->t_pos.x + rnd(3) - 1;
    /*
     * Now check to see if that's a legal move.  If not, don't move.
     * (I.e., bump into the wall or whatever)
     */
    if (y == who->t_pos.y && x == who->t_pos.x)
	return &ret;
    if (!diag_ok(&who->t_pos, &ret))
	goto bad;
    else
    {
	ch = winat(y, x);
	if (!step_ok(ch))
	    goto bad;
	if (ch == SCROLL)
	{
	    for (obj = lvl_obj; obj != NULL; obj = next(obj))
		if (y == obj->o_pos.y && x == obj->o_pos.x)
		    break;
	    if (obj != NULL && obj->o_which == S_SCARE)
		goto bad;
	}
    }
    return &ret;

bad:
    ret = who->t_pos;
    return &ret;
}

/*
 * rust_armor:
 *	Rust the given armor, if it is a legal kind to rust, and we
 *	aren't wearing a magic ring.
 */

void
rust_armor(THING *arm)
{
    if (arm == NULL || arm->o_type != ARMOR || arm->o_which == LEATHER ||
	arm->o_arm >= 9)
	    return;

    if ((arm->o_flags & ISPROT) || ISWEARING(R_SUSTARM))
    {
	if (!to_death)
	    msg(msg_get("MSG_MOVE_RUST_VANISHES"));
    }
    else
    {
	arm->o_arm++;
	if (!terse)
	    msg(msg_get("MSG_MOVE_ARMOR_WEAKER_OH"));
	else
	    msg(msg_get("MSG_MOVE_ARMOR_WEAKENS"));
    }
}
