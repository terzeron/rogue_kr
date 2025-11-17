/*
 * i18n_korean.h - Korean-specific internationalization support
 *
 * Korean language specific transliteration and processing
 */

#ifndef I18N_KOREAN_H
#define I18N_KOREAN_H

#include <stdbool.h>

/*
 * Check if current locale is Korean
 * Returns true if LANG environment variable starts with "ko"
 */
bool is_korean_locale(void);

/*
 * Transliterate English text to Korean using syllable composition
 * This is for scroll titles and similar pseudo-words
 * Returns pointer to static buffer containing Korean text
 * If not Korean locale, returns original English text
 */
const char *transliterate_to_korean(const char *english);

#endif /* I18N_KOREAN_H */
