/*
 * i18n_korean.c - Korean-specific internationalization implementation
 *
 * Korean language specific transliteration and processing
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "i18n.h"
#include "i18n_korean.h"

/*
 * Check if current locale is Korean
 */
bool is_korean_locale(void)
{
    char *lang = getenv("LANG");
    return (lang != NULL && strncmp(lang, "ko", 2) == 0);
}

/*
 * Map English consonant to Korean jamo index
 * Returns -1 for vowels, 0-18 for consonants (choseong index)
 * Choseong: ㄱ ㄲ ㄴ ㄷ ㄸ ㄹ ㅁ ㅂ ㅃ ㅅ ㅆ ㅇ ㅈ ㅉ ㅊ ㅋ ㅌ ㅍ ㅎ
 */
static int get_choseong_index(char c)
{
    switch(c) {
	case 'b': return 7;  /* ㅂ */
	case 'c': return 15; /* ㅋ */
	case 'd': return 3;  /* ㄷ */
	case 'f': return 17; /* ㅍ (f->p) */
	case 'g': return 0;  /* ㄱ */
	case 'h': return 18; /* ㅎ */
	case 'j': return 12; /* ㅈ */
	case 'k': return 15; /* ㅋ */
	case 'l': return 5;  /* ㄹ */
	case 'm': return 6;  /* ㅁ */
	case 'n': return 2;  /* ㄴ */
	case 'p': return 17; /* ㅍ */
	case 'q': return 15; /* ㅋ (q->k) */
	case 'r': return 5;  /* ㄹ */
	case 's': return 9;  /* ㅅ */
	case 't': return 16; /* ㅌ */
	case 'v': return 7;  /* ㅂ (v->b) */
	case 'w': return -1; /* vowel-like */
	case 'x': return -1; /* special handling */
	case 'y': return -1; /* vowel-like */
	case 'z': return 12; /* ㅈ (z->j) */
	default: return -1;  /* vowel or unknown */
    }
}

/*
 * Map English consonant to Korean jongseong (final consonant) index
 * Returns 0 for none, 1-27 for consonants
 * Jongseong: (없음) ㄱ ㄲ ㄱㅅ ㄴ ㄴㅈ ㄴㅎ ㄷ ㄹ ㄹㄱ ㄹㅁ ㄹㅂ ㄹㅅ ㄹㅌ ㄹㅍ ㄹㅎ ㅁ ㅂ ㅂㅅ ㅅ ㅆ ㅇ ㅈ ㅊ ㅋ ㅌ ㅍ ㅎ
 */
static int get_jongseong_index(char c)
{
    switch(c) {
	case 'b': return 17; /* ㅂ */
	case 'c': return 24; /* ㅋ */
	case 'd': return 7;  /* ㄷ */
	case 'f': return 26; /* ㅍ */
	case 'g': return 1;  /* ㄱ */
	case 'h': return 27; /* ㅎ */
	case 'j': return 22; /* ㅈ */
	case 'k': return 24; /* ㅋ */
	case 'l': return 8;  /* ㄹ */
	case 'm': return 16; /* ㅁ */
	case 'n': return 4;  /* ㄴ */
	case 'p': return 26; /* ㅍ */
	case 'q': return 24; /* ㅋ */
	case 'r': return 8;  /* ㄹ */
	case 's': return 19; /* ㅅ */
	case 't': return 25; /* ㅌ */
	case 'v': return 17; /* ㅂ */
	case 'w': return 0;  /* no final consonant */
	case 'x': return 19; /* ㅅ (x->s in final position) */
	case 'y': return 0;  /* no final consonant */
	case 'z': return 22; /* ㅈ */
	default: return 0;   /* no final consonant */
    }
}

/*
 * Map English vowel to Korean jungseong (vowel) index
 * Returns 0-20 for vowels, -1 for consonants
 * Jungseong: ㅏ ㅐ ㅑ ㅒ ㅓ ㅔ ㅕ ㅖ ㅗ ㅘ ㅙ ㅚ ㅛ ㅜ ㅝ ㅞ ㅟ ㅠ ㅡ ㅢ ㅣ
 */
static int get_jungseong_index(char c)
{
    switch(c) {
	case 'a': return 0;  /* ㅏ */
	case 'e': return 5;  /* ㅔ */
	case 'i': return 20; /* ㅣ */
	case 'o': return 8;  /* ㅗ */
	case 'u': return 13; /* ㅜ */
	case 'y': return 20; /* ㅣ (y as vowel) */
	case 'w': return 13; /* ㅜ (w as vowel) */
	default: return -1;  /* consonant */
    }
}

/*
 * Compose a Korean syllable from jamo indices
 * Returns UTF-8 bytes written to out
 */
static int compose_syllable(char *out, int cho, int jung, int jong)
{
    int unicode = 0xAC00 + (cho * 21 * 28) + (jung * 28) + jong;

    /* Convert Unicode codepoint to UTF-8 */
    out[0] = 0xE0 | ((unicode >> 12) & 0x0F);
    out[1] = 0x80 | ((unicode >> 6) & 0x3F);
    out[2] = 0x80 | (unicode & 0x3F);
    return 3;
}

static void
append_syllable(char **outp, char **last_pos, int *last_cho,
    int *last_jung, int *last_jong, int cho, int jung, int jong)
{
    char *pos = *outp;
    *last_pos = pos;
    *last_cho = cho;
    *last_jung = jung;
    *last_jong = jong;
    *outp += compose_syllable(pos, cho, jung, jong);
}

static void
flush_pending(char **outp, int *pending_consonant, char **last_pos,
    int *last_cho, int *last_jung, int *last_jong)
{
    if (*pending_consonant >= 0)
    {
	append_syllable(outp, last_pos, last_cho, last_jung, last_jong,
	    *pending_consonant, 18, 0);
	*pending_consonant = -1;
    }
}

static int
determine_jongseong(const char *in, int *consumed_chars)
{
    if (consumed_chars != NULL)
	*consumed_chars = 0;

    if (in[1] == '\0' || in[1] == ' ')
	return 0;

    char next = (char) tolower((unsigned char) in[1]);
    if (next == 'x' || next == 's')
	return 0;

    if (get_jungseong_index(next) >= 0)
	return 0;

    /* Check for consonant + 'ng' pattern (e.g., kng, lng, xng) */
    if (in[2] && tolower((unsigned char)in[2]) == 'n' &&
	in[3] && tolower((unsigned char)in[3]) == 'g')
    {
	bool fourth_is_vowel = (in[4] &&
	    get_jungseong_index(
		(char) tolower((unsigned char)in[4])) >= 0);

	if (!fourth_is_vowel)
	{
	    /* Pattern: vowel + consonant + ng (no vowel after)
	     * Don't add jongseong here - let the consonant be handled
	     * by its special case handler (l, x, etc.) which will add
	     * jongseong and prepare for ng */
	    if (consumed_chars != NULL)
		*consumed_chars = 0;
	    return 0;
	}
    }

    /* Check for 'ng' pattern */
    if (next == 'n')
    {
	if (in[2] && tolower((unsigned char)in[2]) == 'g')
	{
	    bool third_is_vowel = (in[3] &&
		get_jungseong_index(
		    (char) tolower((unsigned char)in[3])) >= 0);
	    if (!third_is_vowel)
	    {
		if (consumed_chars != NULL)
		    *consumed_chars = 2;
		return 21; /* ㅇ jongseong */
	    }
	}
    }

    bool next_next_is_vowel = (in[2] &&
	get_jungseong_index(
	    (char) tolower((unsigned char)in[2])) >= 0);
    if (!next_next_is_vowel)
    {
	if (consumed_chars != NULL)
	    *consumed_chars = 1;
	return get_jongseong_index(next);
    }

    return 0;
}

/*
 * Transliterate English text to Korean using syllable composition
 * This is for scroll titles and similar pseudo-words
 */
const char *transliterate_to_korean(const char *english)
{
    static char korean_buf[MAX_MSG_VALUE];
    char *out = korean_buf;
    const char *in = english;
    int pending_consonant = -1; /* Store consonant waiting for vowel */
    char *last_syllable_pos = NULL;
    int last_cho = -1, last_jung = -1, last_jong = 0;

    /* Only transliterate if Korean locale */
    if (!is_korean_locale())
    {
	strncpy(korean_buf, english, MAX_MSG_VALUE - 1);
	korean_buf[MAX_MSG_VALUE - 1] = '\0';
	return korean_buf;
    }

    korean_buf[0] = '\0';
    out = korean_buf;

    while (*in && (out - korean_buf) < MAX_MSG_VALUE - 10)
    {
	/* Skip spaces and copy as-is */
	if (*in == ' ')
	{
	    /* Flush pending consonant with default vowel */
	    flush_pending(&out, &pending_consonant, &last_syllable_pos,
		&last_cho, &last_jung, &last_jong);
	    *out++ = *in++;
	    last_syllable_pos = NULL;
	    continue;
	}

	unsigned char raw = (unsigned char)*in;
	char c = (char) tolower(raw);

	/* Check for digraphs (special two-letter combinations) */
	if (in[0] && in[1])
	{
	    /* Handle 'ph' as 'f' sound */
	    if (tolower((unsigned char)in[0]) == 'p' &&
		tolower((unsigned char)in[1]) == 'h')
	    {
		flush_pending(&out, &pending_consonant, &last_syllable_pos,
		    &last_cho, &last_jung, &last_jong);
		pending_consonant = 17; /* ㅍ */
		in += 2;
		continue;
	    }
	    /* Handle 'th' as 's' sound */
	    if (tolower((unsigned char)in[0]) == 't' &&
		tolower((unsigned char)in[1]) == 'h')
	    {
		flush_pending(&out, &pending_consonant, &last_syllable_pos,
		    &last_cho, &last_jung, &last_jong);
		pending_consonant = 9; /* ㅅ */
		in += 2;
		continue;
	    }
	}

	if (c == 'x')
	{
	    flush_pending(&out, &pending_consonant, &last_syllable_pos,
		&last_cho, &last_jung, &last_jong);

	    /* Check if next is 'ng' */
	    char next = in[1];
	    bool followed_by_ng = (next && tolower((unsigned char)next) == 'n' &&
		in[2] && tolower((unsigned char)in[2]) == 'g');

	    if (last_syllable_pos != NULL && last_jong == 0)
	    {
		/* Add jongseong to previous syllable */
		if (followed_by_ng)
		    last_jong = 24; /* ㅋ jongseong when followed by ng */
		else
		    last_jong = 1; /* ㄱ jongseong otherwise */
		compose_syllable(last_syllable_pos, last_cho, last_jung, last_jong);

		if (followed_by_ng)
		{
		    /* x followed by ng - set pending ㅅ and let ng be handled later */
		    pending_consonant = 9; /* ㅅ */
		    in++;
		    continue;
		}
	    }
	    else
	    {
		append_syllable(&out, &last_syllable_pos, &last_cho, &last_jung,
		    &last_jong, 15, 18, 0); /* 크 */
	    }

	    int next_vowel = -1;
	    if (next != '\0' && next != ' ')
		next_vowel = get_jungseong_index(
		    (char) tolower((unsigned char) next));

	    if (next_vowel >= 0)
	    {
		pending_consonant = 9; /* ㅅ leading next vowel */
	    }
	    else
	    {
		append_syllable(&out, &last_syllable_pos, &last_cho, &last_jung,
		    &last_jong, 9, 18, 0); /* 스 */
	    }
	    in++;
	    continue;
	}

	if (c == 'l')
	{
	    flush_pending(&out, &pending_consonant, &last_syllable_pos,
		&last_cho, &last_jung, &last_jong);

	    /* Check if followed by 'ng' */
	    bool followed_by_ng = (in[1] && tolower((unsigned char)in[1]) == 'n' &&
		in[2] && tolower((unsigned char)in[2]) == 'g');

	    bool attached = false;
	    if (last_syllable_pos != NULL && last_jong == 0)
	    {
		last_jong = 8; /* ㄹ jongseong */
		compose_syllable(last_syllable_pos, last_cho, last_jung, last_jong);
		attached = true;

		/* If followed by ng, we're done - ng will be handled separately */
		if (followed_by_ng)
		{
		    in++;
		    continue;
		}
	    }

	    bool next_is_vowel = (in[1] &&
		get_jungseong_index(
		    (char) tolower((unsigned char) in[1])) >= 0);

	    if (!attached && !next_is_vowel)
	    {
		/* No place to attach and no vowel ahead: emit standalone 을 */
		append_syllable(&out, &last_syllable_pos, &last_cho, &last_jung,
		    &last_jong, 11, 18, 8);
	    }

	    pending_consonant = next_is_vowel ? 5 : -1;
	    in++;
	    continue;
	}

	if (c == 'r')
	{
	    flush_pending(&out, &pending_consonant, &last_syllable_pos,
		&last_cho, &last_jung, &last_jong);

	    bool next_is_vowel = (in[1] &&
		get_jungseong_index(
		    (char) tolower((unsigned char) in[1])) >= 0);

	    if (next_is_vowel)
		pending_consonant = 5;
	    else
	    {
		append_syllable(&out, &last_syllable_pos, &last_cho, &last_jung,
		    &last_jong, 5, 18, 0); /* 르 */
		pending_consonant = -1;
	    }
	    in++;
	    continue;
	}

	/* Handle 'ng' specially */
	if (c == 'n' && in[1] && tolower((unsigned char)in[1]) == 'g')
	{
	    bool next_is_vowel = (in[2] &&
		get_jungseong_index(
		    (char) tolower((unsigned char) in[2])) >= 0);

	    if (next_is_vowel)
	    {
		/* ng before vowel - use ㄴ as choseong, g will be handled separately */
		flush_pending(&out, &pending_consonant, &last_syllable_pos,
		    &last_cho, &last_jung, &last_jong);
		pending_consonant = 2; /* ㄴ */
		in++;
		continue;
	    }
	    else if (pending_consonant >= 0)
	    {
		/* Pending consonant + ng - combine into syllable with ㅡ + ㅇ */
		append_syllable(&out, &last_syllable_pos, &last_cho, &last_jung,
		    &last_jong, pending_consonant, 18, 21);
		pending_consonant = -1;
		in += 2; /* consume 'ng' */
		continue;
	    }
	    else
	    {
		/* ng not before vowel and no pending consonant */
		if (last_syllable_pos != NULL && last_jong == 0)
		{
		    /* Previous syllable has no jongseong - add ㅇ jongseong */
		    last_jong = 21; /* ㅇ */
		    compose_syllable(last_syllable_pos, last_cho, last_jung, last_jong);
		    in += 2; /* consume 'ng' */
		    continue;
		}
		else if (last_syllable_pos != NULL && last_jong > 0)
		{
		    /* Previous syllable has jongseong - copy it to new syllable's choseong */
		    int copied_cho = -1;
		    /* Map jongseong to choseong */
		    switch (last_jong)
		    {
			case 1:  copied_cho = 0;  break;  /* ㄱ */
			case 4:  copied_cho = 2;  break;  /* ㄴ */
			case 7:  copied_cho = 3;  break;  /* ㄷ */
			case 8:  copied_cho = 5;  break;  /* ㄹ */
			case 16: copied_cho = 6;  break;  /* ㅁ */
			case 17: copied_cho = 7;  break;  /* ㅂ */
			case 19: copied_cho = 9;  break;  /* ㅅ */
			case 21: copied_cho = 11; break;  /* ㅇ */
			case 22: copied_cho = 12; break;  /* ㅈ */
			case 24: copied_cho = 15; break;  /* ㅋ */
			case 25: copied_cho = 16; break;  /* ㅌ */
			case 26: copied_cho = 17; break;  /* ㅍ */
			case 27: copied_cho = 18; break;  /* ㅎ */
			default: copied_cho = 11; break;  /* ㅇ fallback */
		    }

		    /* Keep jongseong in previous syllable (don't remove it) */
		    /* Create new syllable with copied consonant + ㅡ + ㅇ */
		    append_syllable(&out, &last_syllable_pos, &last_cho, &last_jung,
			&last_jong, copied_cho, 18, 21);
		    in += 2; /* consume 'ng' */
		    continue;
		}
		else
		{
		    /* Standalone ng - create syllable with ㅇ choseong */
		    append_syllable(&out, &last_syllable_pos, &last_cho, &last_jung,
			&last_jong, 11, 18, 21); /* ㅇ+ㅡ+ㅇ */
		    in += 2; /* consume 'ng' */
		    continue;
		}
	    }
	}

	int vowel_idx = get_jungseong_index(c);
	int consonant_idx = get_choseong_index(c);

	if (vowel_idx >= 0)
	{
	    /* It's a vowel */
	    int consumed = 0;
	    int jong = determine_jongseong(in, &consumed);
	    if (pending_consonant >= 0)
	    {
		/* Combine pending consonant with this vowel */
		append_syllable(&out, &last_syllable_pos, &last_cho, &last_jung,
		    &last_jong, pending_consonant, vowel_idx, jong);
		pending_consonant = -1;
	    }
	    else
	    {
		/* Vowel without initial consonant - use ㅇ (index 11) */
		append_syllable(&out, &last_syllable_pos, &last_cho, &last_jung,
		    &last_jong, 11, vowel_idx, jong);
	    }
	    in += consumed;
	    in++;
	}
	else if (consonant_idx >= 0)
	{
	    /* It's a consonant */
	    flush_pending(&out, &pending_consonant, &last_syllable_pos,
		&last_cho, &last_jung, &last_jong);
	    pending_consonant = consonant_idx;
	    in++;
	}
	else
	{
	    /* Unknown character, copy as-is */
	    flush_pending(&out, &pending_consonant, &last_syllable_pos,
		&last_cho, &last_jung, &last_jong);
	    *out++ = *in++;
	    last_syllable_pos = NULL;
	}
    }

    /* Flush any remaining pending consonant */
    if (pending_consonant >= 0)
    {
	append_syllable(&out, &last_syllable_pos, &last_cho, &last_jung,
	    &last_jong, pending_consonant, 18, 0); /* ㅡ */
    }

    *out = '\0';
    return korean_buf;
}
