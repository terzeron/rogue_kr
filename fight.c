/*
 * All the fighting gets done here
 *
 * @(#)fight.c	4.67 (Berkeley) 09/06/83
 *
 * Rogue: Exploring the Dungeons of Doom
 * Copyright (C) 1980-1983, 1985, 1999 Michael Toy, Ken Arnold and Glenn Wichman
 * All rights reserved.
 *
 * See the file LICENSE.TXT for full copyright and licensing information.
 */

#include <stdlib.h>
#include <curses.h>
#include <string.h>
#include <ctype.h>
#include "rogue.h"
#include "i18n.h"

#define	EQSTR(a, b)	(strcmp(a, b) == 0)

char *h_names[] = {		/* strings for hitting */
	"MSG_HIT_MONSTER_1",
	"MSG_HIT_MONSTER_2",
	"MSG_HIT_MONSTER_3",
	"MSG_HIT_MONSTER_4",
	"MSG_MONSTER_HIT_1",
	"MSG_MONSTER_HIT_2",
	"MSG_MONSTER_HIT_3",
	"MSG_MONSTER_HIT_4"
};

char *m_names[] = {		/* strings for missing */
	"MSG_MISS_MONSTER_1",
	"MSG_MISS_MONSTER_2",
	"MSG_MISS_MONSTER_3",
	"MSG_MISS_MONSTER_4",
	"MSG_MONSTER_MISS_1",
	"MSG_MONSTER_MISS_2",
	"MSG_MONSTER_MISS_3",
	"MSG_MONSTER_MISS_4"
};

/*
 * adjustments to hit probabilities due to strength
 */
static int str_plus[] = {
    -7, -6, -5, -4, -3, -2, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1,
    1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3,
};

/*
 * adjustments to damage done due to strength
 */
static int add_dam[] = {
    -7, -6, -5, -4, -3, -2, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 2, 3,
    3, 4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 6
};

/*
 * fight:
 *	The player attacks the monster.
 */
int
fight(coord *mp, THING *weap, bool thrown)
{
    register THING *tp;
    register bool did_hit = TRUE;
    register const char *mname;
    register char ch;

    /*
     * Find the monster we want to fight
     */
#ifdef MASTER
    if ((tp = moat(mp->y, mp->x)) == NULL)
	debug("Fight what @ %d,%d", mp->y, mp->x);
#else
    tp = moat(mp->y, mp->x);
#endif
    /*
     * Since we are fighting, things are not quiet so no healing takes
     * place.
     */
    count = 0;
    quiet = 0;
    runto(mp);
    /*
     * Let him know it was really a xeroc (if it was one).
     */
    ch = '\0';
    if (tp->t_type == 'X' && tp->t_disguise != 'X' && !on(player, ISBLIND))
    {
	tp->t_disguise = 'X';
	if (on(player, ISHALU)) {
	    ch = (char)(rnd(26) + 'A');
	    mvaddch(tp->t_pos.y, tp->t_pos.x, ch);
	}
	msg(choose_str(msg_get("MSG_FIGHT_HEAVY"),
		       msg_get("MSG_FIGHT_WAIT")));
	if (!thrown)
	    return FALSE;
    }
    mname = set_mname(tp);
    did_hit = FALSE;
    has_hit = (terse && !to_death);
    if (roll_em(&player, tp, weap, thrown))
    {
	did_hit = FALSE;
	if (thrown)
	    thunk(weap, mname, terse);
	else
	    hit((char *) NULL, mname, terse);
	if (on(player, CANHUH))
	{
	    did_hit = TRUE;
	    tp->t_flags |= ISHUH;
	    player.t_flags &= ~CANHUH;
	    endmsg();
	    has_hit = FALSE;
	    msg(msg_get("MSG_HANDS_STOP_GLOWING"), pick_color("red"));
	}
	if (tp->t_stats.s_hpt <= 0)
	    killed(tp, TRUE);
	else if (did_hit && !on(player, ISBLIND))
	    msg(msg_get("MSG_APPEARS_CONFUSED"), mname);
	did_hit = TRUE;
    }
    else
	if (thrown)
	    bounce(weap, mname, terse);
	else
	    miss((char *) NULL, mname, terse);
    return did_hit;
}

/*
 * attack:
 *	The monster attacks the player
 */
int
attack(THING *mp)
{
    register const char *mname;
    register int oldhp;

    /*
     * Since this is an attack, stop running and any healing that was
     * going on at the time.
     */
    running = FALSE;
    count = 0;
    quiet = 0;
    if (to_death && !on(*mp, ISTARGET))
    {
	to_death = FALSE;
	kamikaze = FALSE;
    }
    if (mp->t_type == 'X' && mp->t_disguise != 'X' && !on(player, ISBLIND))
    {
	mp->t_disguise = 'X';
	if (on(player, ISHALU))
	    mvaddch(mp->t_pos.y, mp->t_pos.x, rnd(26) + 'A');
    }
    mname = set_mname(mp);
    oldhp = pstats.s_hpt;
    if (roll_em(mp, &player, (THING *) NULL, FALSE))
    {
	if (mp->t_type != 'I')
	{
	    if (has_hit)
		addmsg(msg_get("MSG_PERIOD"));
	    hit(mname, (char *) NULL, FALSE);
	}
	else
	    if (has_hit)
		endmsg();
	has_hit = FALSE;
	if (pstats.s_hpt <= 0)
	    death(mp->t_type);	/* Bye bye life ... */
	else if (!kamikaze)
	{
	    oldhp -= pstats.s_hpt;
	    if (oldhp > max_hit)
		max_hit = oldhp;
	    if (pstats.s_hpt <= max_hit)
		to_death = FALSE;
	}
	if (!on(*mp, ISCANC))
	    switch (mp->t_type)
	    {
		case 'A':
		    /*
		     * If an aquator hits, you can lose armor class.
		     */
		    rust_armor(cur_armor);
		when 'I':
		    /*
		     * The ice monster freezes you
		     */
		    player.t_flags &= ~ISRUN;
		    if (!no_command)
		    {
			addmsg(msg_get("MSG_YOU_ARE_FROZEN"));
			if (!terse)
			    addmsg(msg_get("MSG_BY_THE"), mname);
			endmsg();
		    }
		    no_command += rnd(2) + 2;
		    if (no_command > BORE_LEVEL)
			death('h');
		when 'R':
		    /*
		     * Rattlesnakes have poisonous bites
		     */
		    if (!save(VS_POISON))
		    {
			if (!ISWEARING(R_SUSTSTR))
			{
			    chg_str(-1);
			    if (!terse)
				msg(msg_get("MSG_BITE_LEG_WEAKER"));
			    else
				msg(msg_get("MSG_BITE_WEAKENED"));
			}
			else if (!to_death)
			{
			    if (!terse)
				msg(msg_get("MSG_BITE_MOMENTARILY"));
			    else
				msg(msg_get("MSG_BITE_NO_EFFECT"));
			}
		    }
		when 'W':
		case 'V':
		    /*
		     * Wraiths might drain energy levels, and Vampires
		     * can steal max_hp
		     */
		    if (rnd(100) < (mp->t_type == 'W' ? 15 : 30))
		    {
			register int fewer;

			if (mp->t_type == 'W')
			{
			    if (pstats.s_exp == 0)
				death('W');		/* All levels gone */
			    if (--pstats.s_lvl == 0)
			    {
				pstats.s_exp = 0;
				pstats.s_lvl = 1;
			    }
			    else
				pstats.s_exp = e_levels[pstats.s_lvl-1]+1;
			    fewer = roll(1, 10);
			}
			else
			    fewer = roll(1, 3);
			pstats.s_hpt -= fewer;
			max_hp -= fewer;
			if (pstats.s_hpt <= 0)
			    pstats.s_hpt = 1;
			if (max_hp <= 0)
			    death(mp->t_type);
			msg(msg_get("MSG_SUDDENLY_WEAKER"));
		    }
		when 'F':
		    /*
		     * Venus Flytrap stops the poor guy from moving
		     */
		    player.t_flags |= ISHELD;
            char *dst = monsters['F'-'A'].m_stats.s_dmg;
            if (vf_hit < 0)
                vf_hit = 0;
            snprintf(dst, sizeof monsters['F'-'A'].m_stats.s_dmg, "%dx1", vf_hit);
		    if (--pstats.s_hpt <= 0)
			death('F');
		when 'L':
		{
		    /*
		     * Leperachaun steals some gold
		     */
		    register int lastpurse;

		    lastpurse = purse;
		    purse -= GOLDCALC;
		    if (!save(VS_MAGIC))
			purse -= GOLDCALC + GOLDCALC + GOLDCALC + GOLDCALC;
		    if (purse < 0)
			purse = 0;
		    remove_mon(&mp->t_pos, mp, FALSE);
                    mp=NULL;
		    if (purse != lastpurse)
			msg(msg_get("MSG_PURSE_LIGHTER"));
		}
		when 'N':
		{
		    register THING *obj, *steal;
		    register int nobj;

		    /*
		     * Nymph's steal a magic item, look through the pack
		     * and pick out one we like.
		     */
		    steal = NULL;
		    for (nobj = 0, obj = pack; obj != NULL; obj = next(obj))
			if (obj != cur_armor && obj != cur_weapon
			    && obj != cur_ring[LEFT] && obj != cur_ring[RIGHT]
			    && is_magic(obj) && rnd(++nobj) == 0)
				steal = obj;
		    if (steal != NULL)
		    {
			remove_mon(&mp->t_pos, moat(mp->t_pos.y, mp->t_pos.x), FALSE);
                        mp=NULL;
			leave_pack(steal, FALSE, FALSE);
			msg(msg_get("MSG_SHE_STOLE"), inv_name(steal, TRUE));
			discard(steal);
		    }
		}
		otherwise:
		    break;
	    }
    }
    else if (mp->t_type != 'I')
    {
	if (has_hit)
	{
	    addmsg(msg_get("MSG_PERIOD"));
	    has_hit = FALSE;
	}
	if (mp->t_type == 'F')
	{
	    pstats.s_hpt -= vf_hit;
	    if (pstats.s_hpt <= 0)
		death(mp->t_type);	/* Bye bye life ... */
	}
	miss(mname, (char *) NULL, FALSE);
    }
    if (fight_flush && !to_death)
	flush_type();
    count = 0;
    status();
    if (mp == NULL)
        return(-1);
    else
        return(0);
}

/*
 * set_mname:
 *	return the monster name for the given monster
 */
const char *
set_mname(THING *tp)
{
    int ch;
    const char *mname;
    static char tbuf[MAXSTR];
    static const char *monster_msg_keys[] = {
	"MSG_MONSTER_AQUATOR", "MSG_MONSTER_BAT", "MSG_MONSTER_CENTAUR",
	"MSG_MONSTER_DRAGON", "MSG_MONSTER_EMU", "MSG_MONSTER_VENUS_FLYTRAP",
	"MSG_MONSTER_GRIFFIN", "MSG_MONSTER_HOBGOBLIN", "MSG_MONSTER_ICE_MONSTER",
	"MSG_MONSTER_JABBERWOCK", "MSG_MONSTER_KESTREL", "MSG_MONSTER_LEPRECHAUN",
	"MSG_MONSTER_MEDUSA", "MSG_MONSTER_NYMPH", "MSG_MONSTER_ORC",
	"MSG_MONSTER_PHANTOM", "MSG_MONSTER_QUAGGA", "MSG_MONSTER_RATTLESNAKE",
	"MSG_MONSTER_SNAKE", "MSG_MONSTER_TROLL", "MSG_MONSTER_BLACK_UNICORN",
	"MSG_MONSTER_VAMPIRE", "MSG_MONSTER_WRAITH", "MSG_MONSTER_XEROC",
	"MSG_MONSTER_YETI", "MSG_MONSTER_ZOMBIE"
    };

    if (!see_monst(tp) && !on(player, SEEMONST))
	return (terse ? msg_get("MSG_IT") : msg_get("MSG_SOMETHING"));
    else if (on(player, ISHALU))
    {
	move(tp->t_pos.y, tp->t_pos.x);
	ch = toascii(inch());
	if (!isupper(ch))
	    ch = rnd(26);
	else
	    ch -= 'A';
	mname = msg_get(monster_msg_keys[ch]);
    }
    else
	mname = msg_get(monster_msg_keys[tp->t_type - 'A']);
    strcpy(tbuf, mname);
    return tbuf;
}

/*
 * swing:
 *	Returns true if the swing hits
 */
int
swing(int at_lvl, int op_arm, int wplus)
{
    int res = rnd(20);
    int need = (20 - at_lvl) - op_arm;

    return (res + wplus >= need);
}

/*
 * roll_em:
 *	Roll several attacks
 */
bool
roll_em(THING *thatt, THING *thdef, THING *weap, bool hurl)
{
    register struct stats *att, *def;
    register char *cp;
    register int ndice, nsides, def_arm;
    register bool did_hit = FALSE;
    register int hplus;
    register int dplus;
    register int damage;

    att = &thatt->t_stats;
    def = &thdef->t_stats;
    if (weap == NULL)
    {
	cp = att->s_dmg;
	dplus = 0;
	hplus = 0;
    }
    else
    {
	hplus = (weap == NULL ? 0 : weap->o_hplus);
	dplus = (weap == NULL ? 0 : weap->o_dplus);
	if (weap == cur_weapon)
	{
	    if (ISRING(LEFT, R_ADDDAM))
		dplus += cur_ring[LEFT]->o_arm;
	    else if (ISRING(LEFT, R_ADDHIT))
		hplus += cur_ring[LEFT]->o_arm;
	    if (ISRING(RIGHT, R_ADDDAM))
		dplus += cur_ring[RIGHT]->o_arm;
	    else if (ISRING(RIGHT, R_ADDHIT))
		hplus += cur_ring[RIGHT]->o_arm;
	}
	cp = weap->o_damage;
	if (hurl)
	{
	    if ((weap->o_flags&ISMISL) && cur_weapon != NULL &&
	      cur_weapon->o_which == weap->o_launch)
	    {
		cp = weap->o_hurldmg;
		hplus += cur_weapon->o_hplus;
		dplus += cur_weapon->o_dplus;
	    }
	    else if (weap->o_launch < 0)
		cp = weap->o_hurldmg;
	}
    }
    /*
     * If the creature being attacked is not running (alseep or held)
     * then the attacker gets a plus four bonus to hit.
     */
    if (!on(*thdef, ISRUN))
	hplus += 4;
    def_arm = def->s_arm;
    if (def == &pstats)
    {
	if (cur_armor != NULL)
	    def_arm = cur_armor->o_arm;
	if (ISRING(LEFT, R_PROTECT))
	    def_arm -= cur_ring[LEFT]->o_arm;
	if (ISRING(RIGHT, R_PROTECT))
	    def_arm -= cur_ring[RIGHT]->o_arm;
    }
    while(cp != NULL && *cp != '\0')
    {
	ndice = atoi(cp);
	if ((cp = strchr(cp, 'x')) == NULL)
	    break;
	nsides = atoi(++cp);
	if (swing(att->s_lvl, def_arm, hplus + str_plus[att->s_str]))
	{
	    int proll;

	    proll = roll(ndice, nsides);
#ifdef MASTER
	    if (ndice + nsides > 0 && proll <= 0)
		debug("Damage for %dx%d came out %d, dplus = %d, add_dam = %d, def_arm = %d", ndice, nsides, proll, dplus, add_dam[att->s_str], def_arm);
#endif
	    damage = dplus + proll + add_dam[att->s_str];
	    def->s_hpt -= max(0, damage);
	    did_hit = TRUE;
	}
	if ((cp = strchr(cp, '/')) == NULL)
	    break;
	cp++;
    }
    return did_hit;
}

/*
 * prname:
 *	The print name of a combatant
 */
char *
prname(const char *mname, bool upper)
{
    static char tbuf[MAXSTR];

    *tbuf = '\0';
    if (mname == 0)
	strcpy(tbuf, msg_get("MSG_YOU"));
    else
	strcpy(tbuf, mname);
    if (upper)
	*tbuf = (char) toupper(*tbuf);
    return tbuf;
}

/*
 * thunk:
 *	A missile hits a monster
 */
void
thunk(THING *weap, const char *mname, bool noend)
{
    if (to_death)
	return;
    if (weap->o_type == WEAPON)
	addmsg(msg_get("MSG_THE_HITS"), msg_get_weapon_name(weap->o_which));
    else
	addmsg(msg_get("MSG_YOU_HIT"));
    addmsg(msg_get("MSG_FORMAT_SPACE_S"), mname);
    if (!noend)
	endmsg();
}

/*
 * hit:
 *	Print a message to indicate a succesful hit
 */

void
hit(const char *er, const char *ee, bool noend)
{
    int i;
    const char *s;
    extern char *h_names[];
    char buf[MAXSTR];

    if (to_death)
	return;

    if (terse)
	i = 0;
    else
    {
	i = rnd(4);
    }
    if (er != NULL)
	i += 4;

    s = msg_get(h_names[i]);

    /* For Korean, the message format includes both attacker and defender */
    if (er == NULL && ee != NULL)
	sprintf(buf, s, prname(ee, FALSE));
    else if (er != NULL && ee == NULL)
	sprintf(buf, s, prname(er, TRUE));
    else
	sprintf(buf, s, "");

    addmsg(buf);
    if (!noend)
	endmsg();
}

/*
 * miss:
 *	Print a message to indicate a poor swing
 */
void
miss(const char *er, const char *ee, bool noend)
{
    int i;
    extern char *m_names[];
    const char *s;
    char buf[MAXSTR];

    if (to_death)
	return;

    if (terse)
	i = 0;
    else
	i = rnd(4);
    if (er != NULL)
	i += 4;

    s = msg_get(m_names[i]);

    /* For Korean, the message format includes both attacker and defender */
    if (er == NULL && ee != NULL)
	sprintf(buf, s, prname(ee, FALSE));
    else if (er != NULL && ee == NULL)
	sprintf(buf, s, prname(er, TRUE));
    else
	sprintf(buf, s, "");

    addmsg(buf);
    if (!noend)
	endmsg();
}

/*
 * bounce:
 *	A missile misses a monster
 */
void
bounce(THING *weap, const char *mname, bool noend)
{
    if (to_death)
	return;
    if (weap->o_type == WEAPON)
	addmsg(msg_get("MSG_THE_MISSES"), msg_get_weapon_name(weap->o_which));
    else
	addmsg(msg_get("MSG_YOU_MISSED"));
    addmsg(msg_get("MSG_FORMAT_SPACE_S"), mname);
    if (!noend)
	endmsg();
}

/*
 * remove_mon:
 *	Remove a monster from the screen
 */
void
remove_mon(coord *mp, THING *tp, bool waskill)
{
    register THING *obj, *nexti;

    for (obj = tp->t_pack; obj != NULL; obj = nexti)
    {
	nexti = next(obj);
	obj->o_pos = tp->t_pos;
	detach(tp->t_pack, obj);
	if (waskill)
	    fall(obj, FALSE);
	else
	    discard(obj);
    }
    moat(mp->y, mp->x) = NULL;
    mvaddch(mp->y, mp->x, tp->t_oldch);
    detach(mlist, tp);
    if (on(*tp, ISTARGET))
    {
	kamikaze = FALSE;
	to_death = FALSE;
	if (fight_flush)
	    flush_type();
    }
    discard(tp);
}

/*
 * killed:
 *	Called to put a monster to death
 */
void
killed(THING *tp, bool pr)
{
    const char *mname;

    pstats.s_exp += tp->t_stats.s_exp;

    /*
     * If the monster was a venus flytrap, un-hold him
     */
    switch (tp->t_type)
    {
	case 'F':
	    player.t_flags &= ~ISHELD;
	    vf_hit = 0;
	    strcpy(monsters['F'-'A'].m_stats.s_dmg, "000x0");
	when 'L':
	{
	    THING *gold;

	    if (fallpos(&tp->t_pos, &tp->t_room->r_gold) && level >= max_level)
	    {
		gold = new_item();
		gold->o_type = GOLD;
		gold->o_goldval = GOLDCALC;
		if (save(VS_MAGIC))
		    gold->o_goldval += GOLDCALC + GOLDCALC
				     + GOLDCALC + GOLDCALC;
		attach(tp->t_pack, gold);
	    }
	}
    }
    /*
     * Get rid of the monster.
     */
    mname = set_mname(tp);
    remove_mon(&tp->t_pos, tp, TRUE);
    if (pr)
    {
	if (has_hit)
	{
	    msg(msg_get("MSG_DEFEATED_MONSTER"), mname);
	    has_hit = FALSE;
	}
	else
	{
	    msg(msg_get("MSG_YOU_DEFEATED_MONSTER"), mname);
	}
    }
    /*
     * Do adjustments if he went up a level
     */
    check_level();
    if (fight_flush)
	flush_type();
}
