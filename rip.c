/*
 * File for the fun ends
 * Death or a total win
 *
 * @(#)rip.c	4.57 (Berkeley) 02/05/99
 *
 * Rogue: Exploring the Dungeons of Doom
 * Copyright (C) 1980-1983, 1985, 1999 Michael Toy, Ken Arnold and Glenn Wichman
 * All rights reserved.
 *
 * See the file LICENSE.TXT for full copyright and licensing information.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <sys/types.h>
#include <ctype.h>
#include <fcntl.h>
#include <curses.h>
#include "rogue.h"
#include "score.h"
#include "i18n.h"

static char *rip[] = {
"                       __________\n",
"                      /          \\\n",
"                     /    REST    \\\n",
"                    /      IN      \\\n",
"                   /     PEACE      \\\n",
"                  /                  \\\n",
"                  |                  |\n",
"                  |                  |\n",
"                  |   killed by a    |\n",
"                  |                  |\n",
"                  |       1980       |\n",
"                 *|     *  *  *      | *\n",
"         ________)/\\\\_//(\\/(/\\)/\\//\\/|_)_______\n",
    0
};

/*
 * Calculate display width of UTF-8 string
 * Counts Korean/Chinese/Japanese characters as 2, ASCII as 1
 */
static int
display_width(const char *str)
{
    int width = 0;
    const unsigned char *s = (const unsigned char *)str;

    while (*s)
    {
	if (*s < 0x80)
	{
	    /* ASCII character - width 1 */
	    width++;
	    s++;
	}
	else if ((*s & 0xE0) == 0xC0)
	{
	    /* 2-byte UTF-8 - width 1 (most cases) */
	    width++;
	    s += 2;
	}
	else if ((*s & 0xF0) == 0xE0)
	{
	    /* 3-byte UTF-8 - width 2 (CJK characters) */
	    width += 2;
	    s += 3;
	}
	else if ((*s & 0xF8) == 0xF0)
	{
	    /* 4-byte UTF-8 - width 2 */
	    width += 2;
	    s += 4;
	}
	else
	{
	    /* Invalid UTF-8, skip */
	    s++;
	}
    }

    return width;
}

static void
format_tomb_line(char *out, size_t size,
    const char *prefix, const char *text, const char *suffix, int inner_width)
{
    const char *content = (text == NULL) ? "" : text;
    if (inner_width <= 0)
    {
	snprintf(out, size, "%s%s%s", prefix, content, suffix);
	return;
    }

    int len = display_width(content);
    if (len > inner_width)
    {
	snprintf(out, size, "%s%s%s", prefix, content, suffix);
	return;
    }

    int pad = inner_width - len;

    int left_pad = pad / 2;
    int right_pad = pad - left_pad;

    snprintf(out, size, "%s%*s%s%*s%s",
	     prefix, left_pad, "", content, right_pad, "", suffix);
}

/*
 * score:
 *	Figure score and post it.
 */
/* VARARGS2 */

void
score(int amount, int flags, char monst)
{
    SCORE *scp;
    int i;
    SCORE *sc2;
    SCORE *top_ten, *endp;
# ifdef MASTER
    int prflags = 0;
# endif
    void (*fp)(int);
    unsigned int uid;
    const char *reason[4];

    /* Initialize reason strings from i18n */
    reason[0] = msg_get("MSG_KILLED");
    reason[1] = msg_get("MSG_QUIT");
    reason[2] = msg_get("MSG_TOTAL_WINNER");
    reason[3] = msg_get("MSG_KILLED_WITH_AMULET");

    start_score();

 if (flags >= 0
#ifdef MASTER
            || wizard
#endif
        )
    {
	mvaddstr(LINES - 1, 0 , msg_get("MSG_PRESS_RETURN_TO_CONTINUE"));
        refresh();
        wgetnstr(stdscr,prbuf,80);
 	endwin();
        printf("\n");
        resetltchars();
	/*
	 * free up space to "guarantee" there is space for the top_ten
	 */
	delwin(stdscr);
	delwin(curscr);
	if (hw != NULL)
	    delwin(hw);
    }

    top_ten = (SCORE *) malloc(numscores * sizeof (SCORE));
    endp = &top_ten[numscores];
    for (scp = top_ten; scp < endp; scp++)
    {
	scp->sc_score = 0;
	for (i = 0; i < MAXSTR; i++)
	    scp->sc_name[i] = (unsigned char) rnd(255);
	scp->sc_flags = RN;
	scp->sc_level = RN;
	scp->sc_monster = (unsigned short) RN;
	scp->sc_uid = RN;
    }

    signal(SIGINT, SIG_DFL);

#ifdef MASTER
    if (wizard)
	if (strcmp(prbuf, "names") == 0)
	    prflags = 1;
	else if (strcmp(prbuf, "edit") == 0)
	    prflags = 2;
#endif
    rd_score(top_ten);
    /*
     * Insert her in list if need be
     */
    sc2 = NULL;
    if (!noscore)
    {
	uid = md_getuid();
	for (scp = top_ten; scp < endp; scp++)
	    if (amount > scp->sc_score)
		break;
	    else if (!allscore &&	/* only one score per nowin uid */
		flags != 2 && scp->sc_uid == uid && scp->sc_flags != 2)
		    scp = endp;
	if (scp < endp)
	{
	    if (flags != 2 && !allscore)
	    {
		for (sc2 = scp; sc2 < endp; sc2++)
		{
		    if (sc2->sc_uid == uid && sc2->sc_flags != 2)
			break;
		}
		if (sc2 >= endp)
		    sc2 = endp - 1;
	    }
	    else
		sc2 = endp - 1;
	    while (sc2 > scp)
	    {
		*sc2 = sc2[-1];
		sc2--;
	    }
	    scp->sc_score = amount;
	    strncpy(scp->sc_name, whoami, MAXSTR);
	    scp->sc_flags = flags;
	    if (flags == 2)
		scp->sc_level = max_level;
	    else
		scp->sc_level = level;
	    scp->sc_monster = monst;
	    scp->sc_uid = uid;
	    sc2 = scp;
	}
    }
    /*
     * Print the list
     */
    if (flags != -1)
	putchar('\n');
    printf(msg_get("MSG_TOP_SCORES"), msg_get("MSG_NUMNAME"), allscore ? msg_get("MSG_SCORES") : msg_get("MSG_ROGUEISTS"));
    putchar('\n');
    printf("%s", msg_get("MSG_SCORE_HEADER"));
    putchar('\n');
    for (scp = top_ten; scp < endp; scp++)
    {
	if (scp->sc_score) {
	    if (sc2 == scp)
            md_raw_standout();
	    printf("%2d %5d %s: %s ", (int) (scp - top_ten + 1),
		scp->sc_score, scp->sc_name, reason[scp->sc_flags]);
	    printf(msg_get("MSG_ON_LEVEL"), scp->sc_level);
	    if (scp->sc_flags == 0 || scp->sc_flags == 3) {
		printf(" ");
		printf(msg_get("MSG_BY"), killname((char) scp->sc_monster, TRUE));
	    }
#ifdef MASTER
	    if (prflags == 1)
	    {
	    printf(" (%s)", md_getrealname(scp->sc_uid));
	    }
	    else if (prflags == 2)
	    {
		fflush(stdout);
		(void) fgets(prbuf,10,stdin);
		if (prbuf[0] == 'd')
		{
		    for (sc2 = scp; sc2 < endp - 1; sc2++)
			*sc2 = *(sc2 + 1);
		    sc2 = endp - 1;
		    sc2->sc_score = 0;
		    for (i = 0; i < MAXSTR; i++)
			sc2->sc_name[i] = (char) rnd(255);
		    sc2->sc_flags = RN;
		    sc2->sc_level = RN;
		    sc2->sc_monster = (unsigned short) RN;
		    scp--;
		}
	    }
	    else
#endif /* MASTER */
                printf(".");
	    if (sc2 == scp)
		    md_raw_standend();
            putchar('\n');
	}
	else
	    break;
    }
    /*
     * Update the list file
     */
    if (sc2 != NULL)
    {
	if (lock_sc())
	{
	    fp = signal(SIGINT, SIG_IGN);
	    wr_score(top_ten);
	    unlock_sc();
	    signal(SIGINT, fp);
	}
    }
}

/*
 * death:
 *	Do something really fun when he dies
 */

void
death(char monst)
{
    char **dp, *killer;
    struct tm *lt;
    static time_t date;
    struct tm *localtime();

    signal(SIGINT, SIG_IGN);
    purse -= purse / 10;
    signal(SIGINT, leave);
    clear();
    killer = killname(monst, FALSE);
    if (!tombstone)
    {
        mvprintw(LINES - 2, 0, "%s", msg_get("MSG_KILLED_BY_PREFIX"));
	printw(" ");
	killer = killname(monst, FALSE);
	if (monst != 's' && monst != 'h')
	    printw("a%s ", vowelstr(killer));
	printw("%s ", killer);
	printw(msg_get("MSG_WITH_GOLD"), purse);
    }
    else
    {
	char rest_line[80], in_line[80], peace_line[80], killed_line[80];
	char killer_display_line[80];
	char killed_text[80], killer_line_text[80];
	const char *rest_txt = msg_get("MSG_TOMBSTONE_REST");
	const char *in_txt = msg_get("MSG_TOMBSTONE_IN");
	const char *peace_txt = msg_get("MSG_TOMBSTONE_PEACE");
	const char *killed_template = msg_get("MSG_TOMBSTONE_KILLED");
	const char *gold_label = msg_get("MSG_TOMBSTONE_GOLD");
	const char *article = (monst == 's' || monst == 'h') ? "" : vowelstr(killer);
	char *lang = getenv("LANG");
	int is_korean = (lang != NULL && strncmp(lang, "ko", 2) == 0);

	format_tomb_line(rest_line, sizeof(rest_line),
	                 "                     /", rest_txt, "\\\n", 12);
	format_tomb_line(in_line, sizeof(in_line),
	                 "                    /", in_txt, "\\\n", 14);
	format_tomb_line(peace_line, sizeof(peace_line),
	                 "                   /", peace_txt, "\\\n", 16);

	/* Korean: show monster name on line 8, cause on line 17 */
	/* English: show "killed by a" on line 8, monster name on line 17 */
	if (is_korean)
	{
	    /* Line 8: monster name */
	    strncpy(killed_text, killer, sizeof(killed_text) - 1);
	    killed_text[sizeof(killed_text) - 1] = '\0';
	    /* Line 17: "~(으)로 인해 살해됨" */
	    strncpy(killer_line_text, killed_template, sizeof(killer_line_text) - 1);
	    killer_line_text[sizeof(killer_line_text) - 1] = '\0';
	}
	else
	{
	    /* Line 8: "killed by a" */
	    if (strstr(killed_template, "%s") != NULL)
		snprintf(killed_text, sizeof(killed_text), killed_template, article);
	    else
		snprintf(killed_text, sizeof(killed_text), "%s%s", killed_template, article);
	    /* Line 17: monster name */
	    strncpy(killer_line_text, killer, sizeof(killer_line_text) - 1);
	    killer_line_text[sizeof(killer_line_text) - 1] = '\0';
	}

	format_tomb_line(killed_line, sizeof(killed_line),
	                 "                  |", killed_text, "|\n", 18);
	format_tomb_line(killer_display_line, sizeof(killer_display_line),
	                 "                  |", killer_line_text, "|\n", 18);
	time(&date);
	lt = localtime(&date);
	move(8, 0);
	for (int i = 0; rip[i] != NULL; i++)
	{
	    switch (i)
	    {
		case 2:
		    addstr(rest_line);
		    break;
		case 3:
		    addstr(in_line);
		    break;
		case 4:
		    addstr(peace_line);
		    break;
		case 8:
		    addstr(killed_line);
		    break;
		default:
		    addstr(rip[i]);
		    break;
	    }
	}
	mvaddstr(17, 0, killer_display_line);
	mvaddstr(14, center(whoami), whoami);
	snprintf(prbuf, MAXSTR, "%d %s", purse, gold_label);
	move(15, center(prbuf));
	addstr(prbuf);
	sprintf(prbuf, "%4d", 1900+lt->tm_year);
	mvaddstr(18, 26, prbuf);
    }
    move(LINES - 1, 0);
    refresh();
    score(purse, amulet ? 3 : 0, monst);
    printf("%s", msg_get("MSG_PRESS_RETURN_TO_CONTINUE"));
    fflush(stdout);
    if (fgets(prbuf,10,stdin) == NULL) {
        prbuf[0] = '\0';
    }
    my_exit(0);
}

/*
 * center:
 *	Return the index to center the given string
 */
int
center(char *str)
{
    return 28 - ((display_width(str) + 1) / 2);
}

/*
 * total_winner:
 *	Code for a winner
 */

void
total_winner()
{
    THING *obj;
    struct obj_info *op;
    int worth = 0;
    int oldpurse;

    clear();
    standout();
    addstr("                                                               \n");
    addstr("  @   @               @   @           @          @@@  @     @  \n");
    addstr("  @   @               @@ @@           @           @   @     @  \n");
    addstr("  @   @  @@@  @   @   @ @ @  @@@   @@@@  @@@      @  @@@    @  \n");
    addstr("   @@@@ @   @ @   @   @   @     @ @   @ @   @     @   @     @  \n");
    addstr("      @ @   @ @   @   @   @  @@@@ @   @ @@@@@     @   @     @  \n");
    addstr("  @   @ @   @ @  @@   @   @ @   @ @   @ @         @   @  @     \n");
    addstr("   @@@   @@@   @@ @   @   @  @@@@  @@@@  @@@     @@@   @@   @  \n");
    addstr("                                                               \n");
    addstr("     Congratulations, you have made it to the light of day!    \n");
    standend();
    addstr("\nYou have joined the elite ranks of those who have escaped the\n");
    addstr("Dungeons of Doom alive.  You journey home and sell all your loot at\n");
    addstr("a great profit and are admitted to the Fighters' Guild.\n");
    mvaddstr(LINES - 1, 0, msg_get("MSG_PRESS_SPACE"));
    refresh();
    wait_for(' ');
    clear();
    mvaddstr(0, 0, "   Worth  Item\n");
    oldpurse = purse;
    for (obj = pack; obj != NULL; obj = next(obj))
    {
	switch (obj->o_type)
	{
	    case FOOD:
		worth = 2 * obj->o_count;
	    when WEAPON:
		worth = weap_info[obj->o_which].oi_worth;
		worth *= 3 * (obj->o_hplus + obj->o_dplus) + obj->o_count;
		obj->o_flags |= ISKNOW;
	    when ARMOR:
		worth = arm_info[obj->o_which].oi_worth;
		worth += (9 - obj->o_arm) * 100;
		worth += (10 * (a_class[obj->o_which] - obj->o_arm));
		obj->o_flags |= ISKNOW;
	    when SCROLL:
		worth = scr_info[obj->o_which].oi_worth;
		worth *= obj->o_count;
		op = &scr_info[obj->o_which];
		if (!op->oi_know)
		    worth /= 2;
		op->oi_know = TRUE;
	    when POTION:
		worth = pot_info[obj->o_which].oi_worth;
		worth *= obj->o_count;
		op = &pot_info[obj->o_which];
		if (!op->oi_know)
		    worth /= 2;
		op->oi_know = TRUE;
	    when RING:
		op = &ring_info[obj->o_which];
		worth = op->oi_worth;
		if (obj->o_which == R_ADDSTR || obj->o_which == R_ADDDAM ||
		    obj->o_which == R_PROTECT || obj->o_which == R_ADDHIT)
		{
			if (obj->o_arm > 0)
			    worth += obj->o_arm * 100;
			else
			    worth = 10;
		}
		if (!(obj->o_flags & ISKNOW))
		    worth /= 2;
		obj->o_flags |= ISKNOW;
		op->oi_know = TRUE;
	    when STICK:
		op = &ws_info[obj->o_which];
		worth = op->oi_worth;
		worth += 20 * obj->o_charges;
		if (!(obj->o_flags & ISKNOW))
		    worth /= 2;
		obj->o_flags |= ISKNOW;
		op->oi_know = TRUE;
	    when AMULET:
		worth = 1000;
	}
	if (worth < 0)
	    worth = 0;
	printw("%c) %5d  %s\n", obj->o_packch, worth, inv_name(obj, FALSE));
	purse += worth;
    }
    printw("   %5d  Gold Pieces          ", oldpurse);
    refresh();
    score(purse, 2, ' ');
    my_exit(0);
}

/*
 * killname:
 *	Convert a code to a monster name
 */
char *
killname(char monst, bool doart)
{
    struct h_list *hp;
    const char *sp;
    bool article;
    static struct h_list nlist[] = {
	{'a',	"arrow",		TRUE},
	{'b',	"bolt",			TRUE},
	{'d',	"dart",			TRUE},
	{'h',	"hypothermia",		FALSE},
	{'s',	"starvation",		FALSE},
	{'\0'}
    };

    if (isupper(monst))
    {
	sp = monsters[monst-'A'].m_name;
	article = TRUE;
    }
    else
    {
	sp = "Wally the Wonder Badger";
	article = FALSE;
	for (hp = nlist; hp->h_ch; hp++)
	    if (hp->h_ch == monst)
	    {
		sp = hp->h_desc;
		article = hp->h_print;
		break;
	    }
    }

    /* Translate death causes and monster names */
    if (monst == 'a')
	sp = msg_get("MSG_DEATH_ARROW");
    else if (monst == 'b')
	sp = msg_get("MSG_DEATH_BOLT");
    else if (monst == 'd')
	sp = msg_get("MSG_DEATH_DART");
    else if (monst == 'h')
	sp = msg_get("MSG_DEATH_HYPOTHERMIA");
    else if (monst == 's')
	sp = msg_get("MSG_DEATH_STARVATION");
    else if (isupper(monst))
    {
	/* Translate monster names */
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
	sp = msg_get(monster_msg_keys[monst - 'A']);
    }

    /* For Korean, we don't use articles like "a" or "the" */
    strcpy(prbuf, sp);
    return prbuf;
}

/*
 * death_monst:
 *	Return a monster appropriate for a random death.
 */
char
death_monst()
{
    static char poss[] =
    {
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L',
	'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
	'Y', 'Z', 'a', 'b', 'h', 'd', 's',
	' '	/* This is provided to generate the "Wally the Wonder Badger"
		   message for killer */
    };

    return poss[rnd(sizeof poss / sizeof (char))];
}
