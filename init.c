/*
 * global variable initializaton
 *
 * @(#)init.c	4.31 (Berkeley) 02/05/99
 *
 * Rogue: Exploring the Dungeons of Doom
 * Copyright (C) 1980-1983, 1985, 1999 Michael Toy, Ken Arnold and Glenn Wichman
 * All rights reserved.
 *
 * See the file LICENSE.TXT for full copyright and licensing information.
 */

#include <stdlib.h>
#include <curses.h>
#include <ctype.h>
#include <string.h>
#include "rogue.h"
#include "i18n.h"

/*
 * init_player:
 *	Roll her up
 */
void
init_player()
{
    register THING *obj;

    pstats = max_stats;
    food_left = HUNGERTIME;
    /*
     * Give him some food
     */
    obj = new_item();
    obj->o_type = FOOD;
    obj->o_count = 1;
    add_pack(obj, TRUE);
    /*
     * And his suit of armor
     */
    obj = new_item();
    obj->o_type = ARMOR;
    obj->o_which = RING_MAIL;
    obj->o_arm = a_class[RING_MAIL] - 1;
    obj->o_flags |= ISKNOW;
    obj->o_count = 1;
    cur_armor = obj;
    add_pack(obj, TRUE);
    /*
     * Give him his weaponry.  First a mace.
     */
    obj = new_item();
    init_weapon(obj, MACE);
    obj->o_hplus = 1;
    obj->o_dplus = 1;
    obj->o_flags |= ISKNOW;
    add_pack(obj, TRUE);
    cur_weapon = obj;
    /*
     * Now a +1 bow
     */
    obj = new_item();
    init_weapon(obj, BOW);
    obj->o_hplus = 1;
    obj->o_flags |= ISKNOW;
    add_pack(obj, TRUE);
    /*
     * Now some arrows
     */
    obj = new_item();
    init_weapon(obj, ARROW);
    obj->o_count = rnd(15) + 25;
    obj->o_flags |= ISKNOW;
    add_pack(obj, TRUE);
}

/*
 * Contains defintions and functions for dealing with things like
 * potions and scrolls
 */

/* Potion color names - will be initialized with translated strings */
char *rainbow[27];

/* English color names for initialization */
static const char *rainbow_keys[] = {
    "amber",
    "aquamarine",
    "black",
    "blue",
    "brown",
    "clear",
    "crimson",
    "cyan",
    "ecru",
    "gold",
    "green",
    "grey",
    "magenta",
    "orange",
    "pink",
    "plaid",
    "purple",
    "red",
    "silver",
    "tan",
    "tangerine",
    "topaz",
    "turquoise",
    "vermilion",
    "violet",
    "white",
    "yellow",
};

#define NCOLORS (sizeof rainbow / sizeof (char *))
int cNCOLORS = NCOLORS;

static char *sylls[] = {
    "a", "ab", "ag", "aks", "ala", "an", "app", "arg", "arze", "ash",
    "bek", "bie", "bit", "bjor", "blu", "bot", "bu", "byt", "comp",
    "con", "cos", "cre", "dalf", "dan", "den", "do", "e", "eep", "el",
    "eng", "er", "ere", "erk", "esh", "evs", "fa", "fid", "fri", "fu",
    "gan", "gar", "glen", "gop", "gre", "ha", "hyd", "i", "ing", "ip",
    "ish", "it", "ite", "iv", "jo", "kho", "kli", "klis", "la", "lech",
    "mar", "me", "mi", "mic", "mik", "mon", "mung", "mur", "nej",
    "nelg", "nep", "ner", "nes", "nes", "nih", "nin", "o", "od", "ood",
    "org", "orn", "ox", "oxy", "pay", "ple", "plu", "po", "pot",
    "prok", "re", "rea", "rhov", "ri", "ro", "rog", "rok", "rol", "sa",
    "san", "sat", "sef", "seh", "shu", "ski", "sna", "sne", "snik",
    "sno", "so", "sol", "sri", "sta", "sun", "ta", "tab", "tem",
    "ther", "ti", "tox", "trol", "tue", "turs", "u", "ulk", "um", "un",
    "uni", "ur", "val", "viv", "vly", "vom", "wah", "wed", "werg",
    "wex", "whon", "wun", "xo", "y", "yot", "yu", "zant", "zeb", "zim",
    "zok", "zon", "zum",
};

STONE stones[26];

/* Stone data for dynamic initialization */
static const struct {
    const char *key;
    int value;
} stone_data[] = {
    { "agate",		 25},
    { "alexandrite",	 40},
    { "amethyst",	 50},
    { "carnelian",	 40},
    { "diamond",	300},
    { "emerald",	300},
    { "germanium",	225},
    { "granite",	  5},
    { "garnet",		 50},
    { "jade",		150},
    { "kryptonite",	300},
    { "lapis_lazuli",	 50},
    { "moonstone",	 50},
    { "obsidian",	 15},
    { "onyx",		 60},
    { "opal",		200},
    { "pearl",		220},
    { "peridot",	 63},
    { "ruby",		350},
    { "sapphire",	285},
    { "stibotantalite",	200},
    { "tiger_eye",	 50},
    { "topaz",		 60},
    { "turquoise",	 70},
    { "taaffeite",	300},
    { "zircon",	 	 80},
};

#define NSTONES (sizeof stone_data / sizeof (stone_data[0]))
int cNSTONES = NSTONES;

char *wood[33];

/* Wood names for dynamic initialization */
static const char *wood_keys[] = {
    "avocado_wood",
    "balsa",
    "bamboo",
    "banyan",
    "birch",
    "cedar",
    "cherry",
    "cinnibar",
    "cypress",
    "dogwood",
    "driftwood",
    "ebony",
    "elm",
    "eucalyptus",
    "fall",
    "hemlock",
    "holly",
    "ironwood",
    "kukui_wood",
    "mahogany",
    "manzanita",
    "maple",
    "oaken",
    "persimmon_wood",
    "pecan",
    "pine",
    "poplar",
    "redwood",
    "rosewood",
    "spruce",
    "teak",
    "walnut",
    "zebrawood",
};

#define NWOOD (sizeof wood_keys / sizeof (char *))
int cNWOOD = NWOOD;

char *metal[22];

/* Metal names for dynamic initialization */
static const char *metal_keys[] = {
    "aluminum",
    "beryllium",
    "bone",
    "brass",
    "bronze",
    "copper",
    "electrum",
    "gold",
    "iron",
    "lead",
    "magnesium",
    "mercury",
    "nickel",
    "pewter",
    "platinum",
    "steel",
    "silver",
    "silicon",
    "tin",
    "titanium",
    "tungsten",
    "zinc",
};

#define NMETAL (sizeof metal_keys / sizeof (char *))
int cNMETAL = NMETAL;
#define MAX3(a,b,c)	(a > b ? (a > c ? a : c) : (b > c ? b : c))

static bool used[MAX3(NCOLORS, NSTONES, NWOOD)];

/*
 * init_rainbow:
 *	Initialize rainbow array with translated color names
 */
void
init_rainbow()
{
    int i;
    char msg_key[50];
    char *p;

    for (i = 0; i < 27; i++)
    {
	/* Convert color name to uppercase message key */
	snprintf(msg_key, sizeof(msg_key), "MSG_COLOR_%s", rainbow_keys[i]);
	/* Convert to uppercase */
	for (p = msg_key; *p; p++)
	    *p = toupper((unsigned char)*p);

	rainbow[i] = (char *)msg_get(msg_key);
    }
}

/*
 * init_stones_array:
 *	Initialize stones array with translated stone names
 */
void
init_stones_array()
{
    int i;
    char msg_key[50];
    char *p;

    for (i = 0; i < 26; i++)
    {
	/* Convert stone name to uppercase message key */
	snprintf(msg_key, sizeof(msg_key), "MSG_STONE_%s", stone_data[i].key);
	/* Convert to uppercase */
	for (p = msg_key; *p; p++)
	    *p = toupper((unsigned char)*p);

	stones[i].st_name = (char *)msg_get(msg_key);
	stones[i].st_value = stone_data[i].value;
    }
}

/*
 * init_wood_array:
 *	Initialize wood array with translated wood names
 */
void
init_wood_array()
{
    int i;
    char msg_key[50];
    char *p;

    for (i = 0; i < 33; i++)
    {
	/* Convert wood name to uppercase message key */
	snprintf(msg_key, sizeof(msg_key), "MSG_WOOD_%s", wood_keys[i]);
	/* Convert to uppercase */
	for (p = msg_key; *p; p++)
	    *p = toupper((unsigned char)*p);

	wood[i] = (char *)msg_get(msg_key);
    }
}

/*
 * init_metal_array:
 *	Initialize metal array with translated metal names
 */
void
init_metal_array()
{
    int i;
    char msg_key[50];
    char *p;

    for (i = 0; i < 22; i++)
    {
	/* Convert metal name to uppercase message key */
	snprintf(msg_key, sizeof(msg_key), "MSG_METAL_%s", metal_keys[i]);
	/* Convert to uppercase */
	for (p = msg_key; *p; p++)
	    *p = toupper((unsigned char)*p);

	metal[i] = (char *)msg_get(msg_key);
    }
}

/*
 * init_colors:
 *	Initialize the potion color scheme for this time
 */
void
init_colors()
{
    register int i, j;

    for (i = 0; i < NCOLORS; i++)
	used[i] = FALSE;
    for (i = 0; i < MAXPOTIONS; i++)
    {
	do
	    j = rnd(NCOLORS);
	until (!used[j]);
	used[j] = TRUE;
	p_colors[i] = rainbow[j];
    }
}

/*
 * init_names:
 *	Generate the names of the various scrolls
 */
#define MAXNAME	40	/* Max number of characters in a name */

void
init_names()
{
    register int nsyl;
    register char *cp, *sp;
    register int i, nwords;

    for (i = 0; i < MAXSCROLLS; i++)
    {
	cp = prbuf;
	nwords = rnd(3) + 2;
	while (nwords--)
	{
	    nsyl = rnd(3) + 1;
	    while (nsyl--)
	    {
		sp = sylls[rnd((sizeof sylls) / (sizeof (char *)))];
		if (&cp[strlen(sp)] > &prbuf[MAXNAME])
			break;
		while (*sp)
		    *cp++ = *sp++;
	    }
	    *cp++ = ' ';
	}
	*--cp = '\0';
	/* Transliterate scroll title for target language */
	{
	    const char *transliterated = msg_transliterate(prbuf);
	    s_names[i] = (char *) malloc((unsigned) strlen(transliterated)+1);
	    strcpy(s_names[i], transliterated);
	}
    }
}

/*
 * init_stones:
 *	Initialize the ring stone setting scheme for this time
 */
void
init_stones()
{
    register int i, j;

    for (i = 0; i < NSTONES; i++)
	used[i] = FALSE;
    for (i = 0; i < MAXRINGS; i++)
    {
	do
	    j = rnd(NSTONES);
	until (!used[j]);
	used[j] = TRUE;
	r_stones[i] = stones[j].st_name;
	ring_info[i].oi_worth += stones[j].st_value;
    }
}

/*
 * init_materials:
 *	Initialize the construction materials for wands and staffs
 */
void
init_materials()
{
    register int i, j;
    register char *str;
    static bool metused[NMETAL];

    for (i = 0; i < NWOOD; i++)
	used[i] = FALSE;
    for (i = 0; i < NMETAL; i++)
	metused[i] = FALSE;
    for (i = 0; i < MAXSTICKS; i++)
    {
		for (;;)
		    if (rnd(2) == 0)
		    {
			j = rnd(NMETAL);
			if (!metused[j])
			{
			    ws_type[i] = "wand";
			    str = metal[j];
			    metused[j] = TRUE;
			    break;
			}
		    }
		    else
		    {
			j = rnd(NWOOD);
			if (!used[j])
			{
			    ws_type[i] = "staff";
			    str = wood[j];
			    used[j] = TRUE;
			    break;
			}
		    }
	ws_made[i] = str;
    }
}

#ifdef MASTER
# define	NT	NUMTHINGS, "things"
# define	MP	MAXPOTIONS, "potions"
# define	MS	MAXSCROLLS, "scrolls"
# define	MR	MAXRINGS, "rings"
# define	MWS	MAXSTICKS, "sticks"
# define	MW	MAXWEAPONS, "weapons"
# define	MA	MAXARMORS, "armor"
#else
# define	NT	NUMTHINGS
# define	MP	MAXPOTIONS
# define	MS	MAXSCROLLS
# define	MR	MAXRINGS
# define	MWS	MAXSTICKS
# define	MW	MAXWEAPONS
# define	MA	MAXARMORS
#endif

/*
 * sumprobs:
 *	Sum up the probabilities for items appearing
 */
void
sumprobs(struct obj_info *info, int bound
#ifdef MASTER
	, char *name
#endif
)
{
#ifdef MASTER
	struct obj_info *start = info;
#endif
    struct obj_info *endp;

    endp = info + bound;
    while (++info < endp)
	info->oi_prob += (info - 1)->oi_prob;
#ifdef MASTER
    badcheck(name, start, bound);
#endif
}

/*
 * init_probs:
 *	Initialize the probabilities for the various items
 */
void
init_probs()
{
    sumprobs(things, NT);
    sumprobs(pot_info, MP);
    sumprobs(scr_info, MS);
    sumprobs(ring_info, MR);
    sumprobs(ws_info, MWS);
    sumprobs(weap_info, MW);
    sumprobs(arm_info, MA);
}

#ifdef MASTER
/*
 * badcheck:
 *	Check to see if a series of probabilities sums to 100
 */
void
badcheck(char *name, struct obj_info *info, int bound)
{
    register struct obj_info *end;

    if (info[bound - 1].oi_prob == 100)
	return;
    printf("\nBad percentages for %s (bound = %d):\n", name, bound);
    for (end = &info[bound]; info < end; info++)
	printf("%3d%% %s\n", info->oi_prob, info->oi_name);
    printf("[hit RETURN to continue]");
    fflush(stdout);
    while (getchar() != '\n')
	continue;
}
#endif

/*
 * pick_color:
 *	If he is halucinating, pick a random color name and return it,
 *	otherwise return the translated color name.
 */
char *
pick_color(char *col)
{
    if (on(player, ISHALU))
	return rainbow[rnd(NCOLORS)];

    /* Translate common color names */
    if (strcmp(col, "red") == 0)
	return msg_get("MSG_COLOR_RED");
    else if (strcmp(col, "blue") == 0)
	return msg_get("MSG_COLOR_BLUE");
    else if (strcmp(col, "silver") == 0)
	return msg_get("MSG_COLOR_SILVER");
    else if (strcmp(col, "gold") == 0)
	return msg_get("MSG_COLOR_GOLD");
    else
	return col;  /* Unknown color, return as-is */
}

/*
 * init_traps:
 *	Initialize the trap names array using internationalized messages
 */
void
init_traps()
{
    tr_name[0] = (char *)msg_get("MSG_TRAP_TRAPDOOR");
    tr_name[1] = (char *)msg_get("MSG_TRAP_ARROW");
    tr_name[2] = (char *)msg_get("MSG_TRAP_SLEEPING_GAS");
    tr_name[3] = (char *)msg_get("MSG_TRAP_BEARTRAP");
    tr_name[4] = (char *)msg_get("MSG_TRAP_TELEPORT");
    tr_name[5] = (char *)msg_get("MSG_TRAP_POISON_DART");
    tr_name[6] = (char *)msg_get("MSG_TRAP_RUST");
    tr_name[7] = (char *)msg_get("MSG_TRAP_MYSTERIOUS");
}

/*
 * init_help:
 *	Initialize the help strings array using internationalized messages
 */
void
init_help()
{
    helpstr[0].h_desc = msg_get("MSG_HELP_PRINTS_HELP");
    helpstr[1].h_desc = msg_get("MSG_HELP_IDENTIFY_OBJECT");
    helpstr[2].h_desc = msg_get("MSG_HELP_LEFT");
    helpstr[3].h_desc = msg_get("MSG_HELP_DOWN");
    helpstr[4].h_desc = msg_get("MSG_HELP_UP");
    helpstr[5].h_desc = msg_get("MSG_HELP_RIGHT");
    helpstr[6].h_desc = msg_get("MSG_HELP_UP_LEFT");
    helpstr[7].h_desc = msg_get("MSG_HELP_UP_RIGHT");
    helpstr[8].h_desc = msg_get("MSG_HELP_DOWN_LEFT");
    helpstr[9].h_desc = msg_get("MSG_HELP_DOWN_RIGHT");
    helpstr[10].h_desc = msg_get("MSG_HELP_RUN_LEFT");
    helpstr[11].h_desc = msg_get("MSG_HELP_RUN_DOWN");
    helpstr[12].h_desc = msg_get("MSG_HELP_RUN_UP");
    helpstr[13].h_desc = msg_get("MSG_HELP_RUN_RIGHT");
    helpstr[14].h_desc = msg_get("MSG_HELP_RUN_UP_LEFT");
    helpstr[15].h_desc = msg_get("MSG_HELP_RUN_UP_RIGHT");
    helpstr[16].h_desc = msg_get("MSG_HELP_RUN_DOWN_LEFT");
    helpstr[17].h_desc = msg_get("MSG_HELP_RUN_DOWN_RIGHT");
    helpstr[18].h_desc = msg_get("MSG_HELP_RUN_LEFT_ADJ");
    helpstr[19].h_desc = msg_get("MSG_HELP_RUN_DOWN_ADJ");
    helpstr[20].h_desc = msg_get("MSG_HELP_RUN_UP_ADJ");
    helpstr[21].h_desc = msg_get("MSG_HELP_RUN_RIGHT_ADJ");
    helpstr[22].h_desc = msg_get("MSG_HELP_RUN_UP_LEFT_ADJ");
    helpstr[23].h_desc = msg_get("MSG_HELP_RUN_UP_RIGHT_ADJ");
    helpstr[24].h_desc = msg_get("MSG_HELP_RUN_DOWN_LEFT_ADJ");
    helpstr[25].h_desc = msg_get("MSG_HELP_RUN_DOWN_RIGHT_ADJ");
    helpstr[26].h_desc = msg_get("MSG_HELP_SHIFT_DIR");
    helpstr[27].h_desc = msg_get("MSG_HELP_CTRL_DIR");
    helpstr[28].h_desc = msg_get("MSG_HELP_FIGHT");
    helpstr[29].h_desc = msg_get("MSG_HELP_THROW");
    helpstr[30].h_desc = msg_get("MSG_HELP_MOVE");
    helpstr[31].h_desc = msg_get("MSG_HELP_ZAP");
    helpstr[32].h_desc = msg_get("MSG_HELP_IDENTIFY_TRAP");
    helpstr[33].h_desc = msg_get("MSG_HELP_SEARCH");
    helpstr[34].h_desc = msg_get("MSG_HELP_DOWN_STAIR");
    helpstr[35].h_desc = msg_get("MSG_HELP_UP_STAIR");
    helpstr[36].h_desc = msg_get("MSG_HELP_REST");
    helpstr[37].h_desc = msg_get("MSG_HELP_PICK_UP");
    helpstr[38].h_desc = msg_get("MSG_HELP_INVENTORY");
    helpstr[39].h_desc = msg_get("MSG_HELP_INVENTORY_SINGLE");
    helpstr[40].h_desc = msg_get("MSG_HELP_QUAFF");
    helpstr[41].h_desc = msg_get("MSG_HELP_READ");
    helpstr[42].h_desc = msg_get("MSG_HELP_EAT");
    helpstr[43].h_desc = msg_get("MSG_HELP_WIELD");
    helpstr[44].h_desc = msg_get("MSG_HELP_WEAR");
    helpstr[45].h_desc = msg_get("MSG_HELP_TAKE_OFF");
    helpstr[46].h_desc = msg_get("MSG_HELP_PUT_ON");
    helpstr[47].h_desc = msg_get("MSG_HELP_REMOVE");
    helpstr[48].h_desc = msg_get("MSG_HELP_DROP");
    helpstr[49].h_desc = msg_get("MSG_HELP_CALL");
    helpstr[50].h_desc = msg_get("MSG_HELP_REPEAT");
    helpstr[51].h_desc = msg_get("MSG_HELP_PRINT_WEAPON");
    helpstr[52].h_desc = msg_get("MSG_HELP_PRINT_ARMOR");
    helpstr[53].h_desc = msg_get("MSG_HELP_PRINT_RINGS");
    helpstr[54].h_desc = msg_get("MSG_HELP_PRINT_STATS");
    helpstr[55].h_desc = msg_get("MSG_HELP_DISCOVERED");
    helpstr[56].h_desc = msg_get("MSG_HELP_OPTIONS");
    helpstr[57].h_desc = msg_get("MSG_HELP_REDRAW");
    helpstr[58].h_desc = msg_get("MSG_HELP_REPEAT_MSG");
    helpstr[59].h_desc = msg_get("MSG_HELP_CANCEL");
    helpstr[60].h_desc = msg_get("MSG_HELP_SAVE");
    helpstr[61].h_desc = msg_get("MSG_HELP_QUIT");
    helpstr[62].h_desc = msg_get("MSG_HELP_SHELL");
    helpstr[63].h_desc = msg_get("MSG_HELP_FIGHT_DEATH");
    helpstr[64].h_desc = msg_get("MSG_HELP_VERSION");
}

/*
 * init_monsters:
 *	Initialize monster names with translated strings
 */
void
init_monsters()
{
    monsters['A'-'A'].m_name = (char *)msg_get("MSG_MONSTER_AQUATOR");
    monsters['B'-'A'].m_name = (char *)msg_get("MSG_MONSTER_BAT");
    monsters['C'-'A'].m_name = (char *)msg_get("MSG_MONSTER_CENTAUR");
    monsters['D'-'A'].m_name = (char *)msg_get("MSG_MONSTER_DRAGON");
    monsters['E'-'A'].m_name = (char *)msg_get("MSG_MONSTER_EMU");
    monsters['F'-'A'].m_name = (char *)msg_get("MSG_MONSTER_VENUS_FLYTRAP");
    monsters['G'-'A'].m_name = (char *)msg_get("MSG_MONSTER_GRIFFIN");
    monsters['H'-'A'].m_name = (char *)msg_get("MSG_MONSTER_HOBGOBLIN");
    monsters['I'-'A'].m_name = (char *)msg_get("MSG_MONSTER_ICE_MONSTER");
    monsters['J'-'A'].m_name = (char *)msg_get("MSG_MONSTER_JABBERWOCK");
    monsters['K'-'A'].m_name = (char *)msg_get("MSG_MONSTER_KESTREL");
    monsters['L'-'A'].m_name = (char *)msg_get("MSG_MONSTER_LEPRECHAUN");
    monsters['M'-'A'].m_name = (char *)msg_get("MSG_MONSTER_MEDUSA");
    monsters['N'-'A'].m_name = (char *)msg_get("MSG_MONSTER_NYMPH");
    monsters['O'-'A'].m_name = (char *)msg_get("MSG_MONSTER_ORC");
    monsters['P'-'A'].m_name = (char *)msg_get("MSG_MONSTER_PHANTOM");
    monsters['Q'-'A'].m_name = (char *)msg_get("MSG_MONSTER_QUAGGA");
    monsters['R'-'A'].m_name = (char *)msg_get("MSG_MONSTER_RATTLESNAKE");
    monsters['S'-'A'].m_name = (char *)msg_get("MSG_MONSTER_SNAKE");
    monsters['T'-'A'].m_name = (char *)msg_get("MSG_MONSTER_TROLL");
    monsters['U'-'A'].m_name = (char *)msg_get("MSG_MONSTER_BLACK_UNICORN");
    monsters['V'-'A'].m_name = (char *)msg_get("MSG_MONSTER_VAMPIRE");
    monsters['W'-'A'].m_name = (char *)msg_get("MSG_MONSTER_WRAITH");
    monsters['X'-'A'].m_name = (char *)msg_get("MSG_MONSTER_XEROC");
    monsters['Y'-'A'].m_name = (char *)msg_get("MSG_MONSTER_YETI");
    monsters['Z'-'A'].m_name = (char *)msg_get("MSG_MONSTER_ZOMBIE");
}

/*
 * init_potions:
 *	Initialize potion effect names with translated strings
 */
void
init_potions()
{
    pot_info[0].oi_name = (char *)msg_get("MSG_POTION_CONFUSION");
    pot_info[1].oi_name = (char *)msg_get("MSG_POTION_HALLUCINATION");
    pot_info[2].oi_name = (char *)msg_get("MSG_POTION_POISON");
    pot_info[3].oi_name = (char *)msg_get("MSG_POTION_GAIN_STRENGTH");
    pot_info[4].oi_name = (char *)msg_get("MSG_POTION_SEE_INVISIBLE");
    pot_info[5].oi_name = (char *)msg_get("MSG_POTION_HEALING");
    pot_info[6].oi_name = (char *)msg_get("MSG_POTION_MONSTER_DETECTION");
    pot_info[7].oi_name = (char *)msg_get("MSG_POTION_MAGIC_DETECTION");
    pot_info[8].oi_name = (char *)msg_get("MSG_POTION_RAISE_LEVEL");
    pot_info[9].oi_name = (char *)msg_get("MSG_POTION_EXTRA_HEALING");
    pot_info[10].oi_name = (char *)msg_get("MSG_POTION_HASTE_SELF");
    pot_info[11].oi_name = (char *)msg_get("MSG_POTION_RESTORE_STRENGTH");
    pot_info[12].oi_name = (char *)msg_get("MSG_POTION_BLINDNESS");
    pot_info[13].oi_name = (char *)msg_get("MSG_POTION_LEVITATION");
}

/*
 * init_scrolls:
 *	Initialize scroll effect names with translated strings
 */
void
init_scrolls()
{
    scr_info[0].oi_name = (char *)msg_get("MSG_SCROLL_MONSTER_CONFUSION");
    scr_info[1].oi_name = (char *)msg_get("MSG_SCROLL_MAGIC_MAPPING");
    scr_info[2].oi_name = (char *)msg_get("MSG_SCROLL_HOLD_MONSTER");
    scr_info[3].oi_name = (char *)msg_get("MSG_SCROLL_SLEEP");
    scr_info[4].oi_name = (char *)msg_get("MSG_SCROLL_ENCHANT_ARMOR");
    scr_info[5].oi_name = (char *)msg_get("MSG_SCROLL_IDENTIFY_POTION");
    scr_info[6].oi_name = (char *)msg_get("MSG_SCROLL_IDENTIFY_SCROLL");
    scr_info[7].oi_name = (char *)msg_get("MSG_SCROLL_IDENTIFY_WEAPON");
    scr_info[8].oi_name = (char *)msg_get("MSG_SCROLL_IDENTIFY_ARMOR");
    scr_info[9].oi_name = (char *)msg_get("MSG_SCROLL_IDENTIFY_RING_WAND_STAFF");
    scr_info[10].oi_name = (char *)msg_get("MSG_SCROLL_SCARE_MONSTER");
    scr_info[11].oi_name = (char *)msg_get("MSG_SCROLL_FOOD_DETECTION");
    scr_info[12].oi_name = (char *)msg_get("MSG_SCROLL_TELEPORTATION");
    scr_info[13].oi_name = (char *)msg_get("MSG_SCROLL_ENCHANT_WEAPON");
    scr_info[14].oi_name = (char *)msg_get("MSG_SCROLL_CREATE_MONSTER");
    scr_info[15].oi_name = (char *)msg_get("MSG_SCROLL_REMOVE_CURSE");
    scr_info[16].oi_name = (char *)msg_get("MSG_SCROLL_AGGRAVATE_MONSTERS");
    scr_info[17].oi_name = (char *)msg_get("MSG_SCROLL_PROTECT_ARMOR");
}

/*
 * init_rings:
 *	Initialize ring effect names with translated strings
 */
void
init_rings()
{
    ring_info[0].oi_name = (char *)msg_get("MSG_RING_PROTECTION");
    ring_info[1].oi_name = (char *)msg_get("MSG_RING_ADD_STRENGTH");
    ring_info[2].oi_name = (char *)msg_get("MSG_RING_SUSTAIN_STRENGTH");
    ring_info[3].oi_name = (char *)msg_get("MSG_RING_SEARCHING");
    ring_info[4].oi_name = (char *)msg_get("MSG_RING_SEE_INVISIBLE");
    ring_info[5].oi_name = (char *)msg_get("MSG_RING_ADORNMENT");
    ring_info[6].oi_name = (char *)msg_get("MSG_RING_AGGRAVATE_MONSTER");
    ring_info[7].oi_name = (char *)msg_get("MSG_RING_DEXTERITY");
    ring_info[8].oi_name = (char *)msg_get("MSG_RING_INCREASE_DAMAGE");
    ring_info[9].oi_name = (char *)msg_get("MSG_RING_REGENERATION");
    ring_info[10].oi_name = (char *)msg_get("MSG_RING_SLOW_DIGESTION");
    ring_info[11].oi_name = (char *)msg_get("MSG_RING_TELEPORTATION");
    ring_info[12].oi_name = (char *)msg_get("MSG_RING_STEALTH");
    ring_info[13].oi_name = (char *)msg_get("MSG_RING_MAINTAIN_ARMOR");
}

/*
 * init_sticks:
 *	Initialize wand/staff effect names with translated strings
 */
void
init_sticks()
{
    ws_info[0].oi_name = (char *)msg_get("MSG_STICK_LIGHT");
    ws_info[1].oi_name = (char *)msg_get("MSG_STICK_INVISIBILITY");
    ws_info[2].oi_name = (char *)msg_get("MSG_STICK_LIGHTNING");
    ws_info[3].oi_name = (char *)msg_get("MSG_STICK_FIRE");
    ws_info[4].oi_name = (char *)msg_get("MSG_STICK_COLD");
    ws_info[5].oi_name = (char *)msg_get("MSG_STICK_POLYMORPH");
    ws_info[6].oi_name = (char *)msg_get("MSG_STICK_MAGIC_MISSILE");
    ws_info[7].oi_name = (char *)msg_get("MSG_STICK_HASTE_MONSTER");
    ws_info[8].oi_name = (char *)msg_get("MSG_STICK_SLOW_MONSTER");
    ws_info[9].oi_name = (char *)msg_get("MSG_STICK_DRAIN_LIFE");
    ws_info[10].oi_name = (char *)msg_get("MSG_STICK_NOTHING");
    ws_info[11].oi_name = (char *)msg_get("MSG_STICK_TELEPORT_AWAY");
    ws_info[12].oi_name = (char *)msg_get("MSG_STICK_TELEPORT_TO");
    ws_info[13].oi_name = (char *)msg_get("MSG_STICK_CANCELLATION");
}

/*
 * init_inv_t_name:
 *	Initialize inventory type names with translated strings
 */
void
init_inv_t_name()
{
    inv_t_name[0] = (char *)msg_get("MSG_INV_TYPE_OVERWRITE");
    inv_t_name[1] = (char *)msg_get("MSG_INV_TYPE_SLOW");
    inv_t_name[2] = (char *)msg_get("MSG_INV_TYPE_CLEAR");
}
