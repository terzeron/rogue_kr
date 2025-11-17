/*
 * i18n.c - Internationalization support implementation
 *
 * Simple message catalog system for translating game messages
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "i18n.h"
#include "i18n_korean.h"

/* Message entry structure */
typedef struct {
    char key[MAX_MSG_KEY];
    char value[MAX_MSG_VALUE];
} MsgEntry;

/* Message catalog storage */
static MsgEntry msg_catalog[MAX_MESSAGES];
static int msg_count = 0;
static int initialized = 0;

/*
 * Load messages from a file
 * File format: KEY=VALUE (one per line)
 * Lines starting with # are comments
 */
static int load_messages(const char *filename)
{
    FILE *fp;
    char line[MAX_MSG_KEY + MAX_MSG_VALUE + 2];
    char *equals;

    fp = fopen(filename, "r");
    if (fp == NULL)
        return -1;

    msg_count = 0;

    while (fgets(line, sizeof(line), fp) != NULL && msg_count < MAX_MESSAGES)
    {
        /* Remove trailing newline */
        line[strcspn(line, "\n")] = 0;

        /* Skip empty lines and comments */
        if (line[0] == '\0' || line[0] == '#')
            continue;

        /* Find the '=' separator */
        equals = strchr(line, '=');
        if (equals == NULL)
            continue;

        /* Split into key and value */
        *equals = '\0';

        /* Copy key and value to catalog */
        strncpy(msg_catalog[msg_count].key, line, MAX_MSG_KEY - 1);
        msg_catalog[msg_count].key[MAX_MSG_KEY - 1] = '\0';

        strncpy(msg_catalog[msg_count].value, equals + 1, MAX_MSG_VALUE - 1);
        msg_catalog[msg_count].value[MAX_MSG_VALUE - 1] = '\0';

        msg_count++;
    }

    fclose(fp);
    return 0;
}

/*
 * Initialize the message catalog
 */
int i18n_init(void)
{
    char *lang;
    char filename[256];

    if (initialized)
        return 0;

    /* Get language from environment */
    lang = getenv("LANG");

    /* Determine message file to load */
    if (lang != NULL && strncmp(lang, "ko", 2) == 0)
    {
        /* Korean */
        strcpy(filename, "ko.msg");
    }
    else
    {
        /* Default to English */
        strcpy(filename, "en.msg");
    }

    /* Try to load the message file */
    if (load_messages(filename) == -1)
    {
        /* If file not found, try loading from default location */
        char alt_filename[512];
        snprintf(alt_filename, sizeof(alt_filename), "/usr/local/share/rogue/%s", filename);
        if (load_messages(alt_filename) == -1)
        {
            /* Still failed - just continue with English hardcoded strings */
            /* This is not an error - the game will still work */
            msg_count = 0;
        }
    }

    initialized = 1;
    return 0;
}

/*
 * Get a translated message by key
 */
const char *msg_get(const char *key)
{
    int i;

    /* Handle NULL or empty key */
    if (key == NULL || key[0] == '\0')
        return "";

    /* Make sure we're initialized */
    if (!initialized)
        i18n_init();

    /* Search for the key in catalog */
    for (i = 0; i < msg_count; i++)
    {
        if (strcmp(msg_catalog[i].key, key) == 0)
            return msg_catalog[i].value;
    }

    /* Key not found - return the key itself as fallback */
    return key;
}

/*
 * Get translated weapon name
 */
const char *msg_get_weapon_name(int weapon_type)
{
    static const char *weapon_msg_keys[] = {
	"MSG_WEAPON_MACE",          /* 0 - mace */
	"MSG_WEAPON_LONG_SWORD",    /* 1 - long sword */
	"MSG_WEAPON_SHORT_BOW",     /* 2 - short bow */
	"MSG_WEAPON_ARROW",         /* 3 - arrow */
	"MSG_WEAPON_DAGGER",        /* 4 - dagger */
	"MSG_WEAPON_TWO_HANDED_SWORD", /* 5 - two handed sword */
	"MSG_WEAPON_DART",          /* 6 - dart */
	"MSG_WEAPON_SHURIKEN",      /* 7 - shuriken */
	"MSG_WEAPON_SPEAR"          /* 8 - spear */
    };

    if (weapon_type >= 0 && weapon_type < 9)
	return msg_get(weapon_msg_keys[weapon_type]);
    return "";
}

/*
 * Get translated armor name
 */
const char *msg_get_armor_name(int armor_type)
{
    static const char *armor_msg_keys[] = {
	"MSG_ARMOR_LEATHER_ARMOR",        /* 0 - leather armor */
	"MSG_ARMOR_RING_MAIL",            /* 1 - ring mail */
	"MSG_ARMOR_STUDDED_LEATHER_ARMOR",/* 2 - studded leather armor */
	"MSG_ARMOR_SCALE_MAIL",           /* 3 - scale mail */
	"MSG_ARMOR_CHAIN_MAIL",           /* 4 - chain mail */
	"MSG_ARMOR_SPLINT_MAIL",          /* 5 - splint mail */
	"MSG_ARMOR_BANDED_MAIL",          /* 6 - banded mail */
	"MSG_ARMOR_PLATE_MAIL"            /* 7 - plate mail */
    };

    if (armor_type >= 0 && armor_type < 8)
	return msg_get(armor_msg_keys[armor_type]);
    return "";
}

/*
 * Get translated monster name
 */
const char *msg_get_monster_name(int monster_index)
{
    static const char *monster_msg_keys[] = {
	"MSG_MONSTER_AQUATOR",        /* A - aquator */
	"MSG_MONSTER_BAT",            /* B - bat */
	"MSG_MONSTER_CENTAUR",        /* C - centaur */
	"MSG_MONSTER_DRAGON",         /* D - dragon */
	"MSG_MONSTER_EMU",            /* E - emu */
	"MSG_MONSTER_VENUS_FLYTRAP",  /* F - venus flytrap */
	"MSG_MONSTER_GRIFFIN",        /* G - griffin */
	"MSG_MONSTER_HOBGOBLIN",      /* H - hobgoblin */
	"MSG_MONSTER_ICE_MONSTER",    /* I - ice monster */
	"MSG_MONSTER_JABBERWOCK",     /* J - jabberwock */
	"MSG_MONSTER_KESTREL",        /* K - kestrel */
	"MSG_MONSTER_LEPRECHAUN",     /* L - leprechaun */
	"MSG_MONSTER_MEDUSA",         /* M - medusa */
	"MSG_MONSTER_NYMPH",          /* N - nymph */
	"MSG_MONSTER_ORC",            /* O - orc */
	"MSG_MONSTER_PHANTOM",        /* P - phantom */
	"MSG_MONSTER_QUAGGA",         /* Q - quagga */
	"MSG_MONSTER_RATTLESNAKE",    /* R - rattlesnake */
	"MSG_MONSTER_SNAKE",          /* S - snake */
	"MSG_MONSTER_TROLL",          /* T - troll */
	"MSG_MONSTER_BLACK_UNICORN",  /* U - black unicorn */
	"MSG_MONSTER_VAMPIRE",        /* V - vampire */
	"MSG_MONSTER_WRAITH",         /* W - wraith */
	"MSG_MONSTER_XEROC",          /* X - xeroc */
	"MSG_MONSTER_YETI",           /* Y - yeti */
	"MSG_MONSTER_ZOMBIE"          /* Z - zombie */
    };

    if (monster_index >= 0 && monster_index < 26)
	return msg_get(monster_msg_keys[monster_index]);
    return "";
}

/*
 * Get translated color name
 */
const char *msg_get_color(const char *color)
{
    static char msg_key[64];
    static const char *color_names[] = {
	"amber", "aquamarine", "black", "blue", "brown", "clear", "crimson",
	"cyan", "ecru", "gold", "green", "grey", "magenta", "orange", "pink",
	"plaid", "purple", "red", "silver", "tan", "tangerine", "topaz",
	"turquoise", "vermilion", "violet", "white", "yellow", NULL
    };
    int i;

    if (!initialized)
	i18n_init();

    /* Check if we need to translate (Korean locale) */
    if (!is_korean_locale())
	return color;

    /* Find color in list and create message key */
    for (i = 0; color_names[i] != NULL; i++)
    {
	if (strcmp(color, color_names[i]) == 0)
	{
	    /* Convert color name to uppercase for message key */
	    char upper_color[32];
	    int j;
	    for (j = 0; color[j] && j < 31; j++)
	    {
		if (color[j] >= 'a' && color[j] <= 'z')
		    upper_color[j] = color[j] - 'a' + 'A';
		else if (color[j] == ' ')
		    upper_color[j] = '_';
		else
		    upper_color[j] = color[j];
	    }
	    upper_color[j] = '\0';

	    snprintf(msg_key, sizeof(msg_key), "MSG_COLOR_%s", upper_color);
	    return msg_get(msg_key);
	}
    }

    /* Not found, return original */
    return color;
}

/*
 * Get translated stone name (for rings)
 */
const char *msg_get_stone(const char *stone)
{
    static char msg_key[64];
    char upper_stone[64];
    int i;

    if (!initialized)
	i18n_init();

    /* Check if we need to translate (Korean locale) */
    if (!is_korean_locale())
	return stone;

    /* Convert stone name to uppercase for message key */
    for (i = 0; stone[i] && i < 63; i++)
    {
	if (stone[i] >= 'a' && stone[i] <= 'z')
	    upper_stone[i] = stone[i] - 'a' + 'A';
	else if (stone[i] == ' ')
	    upper_stone[i] = '_';
	else
	    upper_stone[i] = stone[i];
    }
    upper_stone[i] = '\0';

    snprintf(msg_key, sizeof(msg_key), "MSG_STONE_%.50s", upper_stone);
    return msg_get(msg_key);
}

/*
 * Get translated material name (for wands/staves)
 */
const char *msg_get_material(const char *material)
{
    static char msg_key[64];
    char upper_material[64];
    int i;

    if (!initialized)
	i18n_init();

    /* Check if we need to translate (Korean locale) */
    if (!is_korean_locale())
	return material;

    /* Convert material name to uppercase for message key */
    for (i = 0; material[i] && i < 63; i++)
    {
	if (material[i] >= 'a' && material[i] <= 'z')
	    upper_material[i] = material[i] - 'a' + 'A';
	else if (material[i] == ' ')
	    upper_material[i] = '_';
	else
	    upper_material[i] = material[i];
    }
    upper_material[i] = '\0';

    int maxlen = sizeof(msg_key) - strlen("MSG_MATERIAL_") - 1;
    snprintf(msg_key, sizeof(msg_key), "MSG_MATERIAL_%.*s", maxlen, upper_material);
    return msg_get(msg_key);
}

/*
 * Transliterate English text to target language
 * For Korean locale: uses syllable-based transliteration
 * For other locales: returns original text
 */
const char *msg_transliterate(const char *english)
{
    if (!initialized)
	i18n_init();

    /* Only transliterate for Korean locale */
    if (!is_korean_locale())
	return english;

    /* Use Korean-specific transliteration */
    return transliterate_to_korean(english);
}

/*
 * Clean up resources
 */
void i18n_cleanup(void)
{
    msg_count = 0;
    initialized = 0;
}
