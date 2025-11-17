/*
 * i18n.h - Internationalization support for Rogue
 *
 * Simple message catalog system for translating game messages
 */

#ifndef I18N_H
#define I18N_H

/* Maximum message key and value lengths */
#define MAX_MSG_KEY 64
#define MAX_MSG_VALUE 256
#define MAX_MESSAGES 512

/*
 * Initialize the message catalog system
 * Loads messages from appropriate language file based on LANG environment variable
 * Returns 0 on success, -1 on failure
 */
int i18n_init(void);

/*
 * Get a translated message by key
 * Returns the translated message, or the key itself if not found
 */
const char *msg_get(const char *key);

/*
 * Get translated weapon name by weapon type index
 * Returns the translated weapon name
 */
const char *msg_get_weapon_name(int weapon_type);

/*
 * Get translated armor name by armor type index
 * Returns the translated armor name
 */
const char *msg_get_armor_name(int armor_type);

/*
 * Get translated monster name by monster index (0-25 for A-Z)
 * Returns the translated monster name
 */
const char *msg_get_monster_name(int monster_index);

/*
 * Get translated color name
 * Returns the translated color name
 */
const char *msg_get_color(const char *color);

/*
 * Get translated stone name (for rings)
 * Returns the translated stone name
 */
const char *msg_get_stone(const char *stone);

/*
 * Get translated material name (for wands/staves)
 * Returns the translated material name
 */
const char *msg_get_material(const char *material);

/*
 * Transliterate English text to target language
 * For Korean locale: converts English to Korean phonetic approximation
 * For other locales: returns original English text
 * Returns pointer to transliterated text (may be static buffer)
 */
const char *msg_transliterate(const char *english);

/*
 * Clean up message catalog resources
 */
void i18n_cleanup(void);

#endif /* I18N_H */
