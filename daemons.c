/*
 * All the daemon and fuse functions are in here
 *
 * @(#)daemons.c	4.24 (Berkeley) 02/05/99
 *
 * Rogue: Exploring the Dungeons of Doom
 * Copyright (C) 1980-1983, 1985, 1999 Michael Toy, Ken Arnold and Glenn Wichman
 * All rights reserved.
 *
 * See the file LICENSE.TXT for full copyright and licensing information.
 */

#include <curses.h>
#include "rogue.h"
#include "i18n.h"

/*
 * doctor:
 *	A healing daemon that restors hit points after rest
 */
void
doctor()
{
    register int lv, ohp;

    lv = pstats.s_lvl;
    ohp = pstats.s_hpt;
    quiet++;
    if (lv < 8)
    {
	if (quiet + (lv << 1) > 20)
	    pstats.s_hpt++;
    }
    else
	if (quiet >= 3)
	    pstats.s_hpt += rnd(lv - 7) + 1;
    if (ISRING(LEFT, R_REGEN))
	pstats.s_hpt++;
    if (ISRING(RIGHT, R_REGEN))
	pstats.s_hpt++;
    if (ohp != pstats.s_hpt)
    {
	if (pstats.s_hpt > max_hp)
	    pstats.s_hpt = max_hp;
	quiet = 0;
    }
}

/*
 * Swander:
 *	Called when it is time to start rolling for wandering monsters
 */
void
swander()
{
    start_daemon(rollwand, 0, BEFORE);
}

/*
 * rollwand:
 *	Called to roll to see if a wandering monster starts up
 */
int between = 0;
void
rollwand()
{

    if (++between >= 4)
    {
	if (roll(1, 6) == 4)
	{
	    wanderer();
	    kill_daemon(rollwand);
	    fuse(swander, 0, WANDERTIME, BEFORE);
	}
	between = 0;
    }
}

/*
 * unconfuse:
 *	Release the poor player from his confusion
 */
void
unconfuse()
{
    player.t_flags &= ~ISHUH;
    msg(msg_get("MSG_DAEMON_FEEL_LESS"), choose_str(msg_get("MSG_DAEMON_TRIPPY"), msg_get("MSG_DAEMON_CONFUSED")));
}

/*
 * unsee:
 *	Turn off the ability to see invisible
 */
void
unsee()
{
    register THING *th;

    for (th = mlist; th != NULL; th = next(th))
	if (on(*th, ISINVIS) && see_monst(th))
	    mvaddch(th->t_pos.y, th->t_pos.x, th->t_oldch);
    player.t_flags &= ~CANSEE;
}

/*
 * sight:
 *	He gets his sight back
 */
void
sight()
{
    if (on(player, ISBLIND))
    {
	extinguish(sight);
	player.t_flags &= ~ISBLIND;
	if (!(proom->r_flags & ISGONE))
	    enter_room(&hero);
	msg(choose_str(msg_get("MSG_DAEMON_FAR_OUT"),
		       msg_get("MSG_POTION_DARK_CLOAK")));
    }
}

/*
 * nohaste:
 *	End the hasting
 */
void
nohaste()
{
    player.t_flags &= ~ISHASTE;
    msg(msg_get("MSG_DAEMON_SLOWING_DOWN"));
}

/*
 * stomach:
 *	Digest the hero's food
 */
void
stomach()
{
    register int oldfood;
    int orig_hungry = hungry_state;

    if (food_left <= 0)
    {
	if (food_left-- < -STARVETIME)
	    death('s');
	/*
	 * the hero is fainting
	 */
	if (no_command || rnd(5) != 0)
	    return;
	no_command += rnd(8) + 4;
	hungry_state = 3;
	if (!terse)
	    addmsg(choose_str(msg_get("MSG_DAEMON_MUNCHIES_OVERPOWER"),
			      msg_get("MSG_DAEMON_TOO_WEAK")));
	msg(choose_str(msg_get("MSG_DAEMON_FREAK_OUT"), msg_get("MSG_DAEMON_FAINT")));
    }
    else
    {
	oldfood = food_left;
	food_left -= ring_eat(LEFT) + ring_eat(RIGHT) + 1 - amulet;

	if (food_left < MORETIME && oldfood >= MORETIME)
	{
	    hungry_state = 2;
	    msg(choose_str(msg_get("MSG_DAEMON_MUNCHIES_MOTOR"),
			   msg_get("MSG_MOVE_FEEL_WEAK")));
	}
	else if (food_left < 2 * MORETIME && oldfood >= 2 * MORETIME)
	{
	    hungry_state = 1;
	    if (terse)
		msg(choose_str(msg_get("MSG_DAEMON_GETTING_MUNCHIES"), msg_get("MSG_DAEMON_GETTING_HUNGRY")));
	    else
		msg(choose_str(msg_get("MSG_DAEMON_YOU_GETTING_MUNCHIES"),
			       msg_get("MSG_DAEMON_YOU_GETTING_HUNGRY")));
	}
    }
    if (hungry_state != orig_hungry) { 
        player.t_flags &= ~ISRUN; 
        running = FALSE; 
        to_death = FALSE; 
        count = 0; 
    } 
}

/*
 * come_down:
 *	Take the hero down off her acid trip.
 */
void
come_down()
{
    register THING *tp;
    register bool seemonst;

    if (!on(player, ISHALU))
	return;

    kill_daemon(visuals);
    player.t_flags &= ~ISHALU;

    if (on(player, ISBLIND))
	return;

    /*
     * undo the things
     */
    for (tp = lvl_obj; tp != NULL; tp = next(tp))
	if (cansee(tp->o_pos.y, tp->o_pos.x))
	    mvaddch(tp->o_pos.y, tp->o_pos.x, tp->o_type);

    /*
     * undo the monsters
     */
    seemonst = on(player, SEEMONST);
    for (tp = mlist; tp != NULL; tp = next(tp))
    {
	move(tp->t_pos.y, tp->t_pos.x);
	if (cansee(tp->t_pos.y, tp->t_pos.x))
	    if (!on(*tp, ISINVIS) || on(player, CANSEE))
		addch(tp->t_disguise);
	    else
		addch(chat(tp->t_pos.y, tp->t_pos.x));
	else if (seemonst)
	{
	    standout();
	    addch(tp->t_type);
	    standend();
	}
    }
    msg(msg_get("MSG_DAEMON_SO_BORING"));
}

/*
 * visuals:
 *	change the characters for the player
 */
void
visuals()
{
    register THING *tp;
    register bool seemonst;

    if (!after || (running && jump))
	return;
    /*
     * change the things
     */
    for (tp = lvl_obj; tp != NULL; tp = next(tp))
	if (cansee(tp->o_pos.y, tp->o_pos.x))
	    mvaddch(tp->o_pos.y, tp->o_pos.x, rnd_thing());

    /*
     * change the stairs
     */
    if (!seenstairs && cansee(stairs.y, stairs.x))
	mvaddch(stairs.y, stairs.x, rnd_thing());

    /*
     * change the monsters
     */
    seemonst = on(player, SEEMONST);
    for (tp = mlist; tp != NULL; tp = next(tp))
    {
	move(tp->t_pos.y, tp->t_pos.x);
	if (see_monst(tp))
	{
	    if (tp->t_type == 'X' && tp->t_disguise != 'X')
		addch(rnd_thing());
	    else
		addch(rnd(26) + 'A');
	}
	else if (seemonst)
	{
	    standout();
	    addch(rnd(26) + 'A');
	    standend();
	}
    }
}

/*
 * land:
 *	Land from a levitation potion
 */
void
land()
{
    player.t_flags &= ~ISLEVIT;
    msg(choose_str(msg_get("MSG_DAEMON_HIT_GROUND"),
		   msg_get("MSG_DAEMON_FEET_FLOOR")));
}
