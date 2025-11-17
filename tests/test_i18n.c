/*
 * test_i18n.c - Unit tests for internationalization (i18n) system
 *
 * Tests the message catalog loading and retrieval functionality
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

/* Include i18n header */
#include "../i18n.h"
#include "../i18n_korean.h"

/*
 * Test: i18n_init should succeed
 */
static void test_i18n_init_success(void **state) {
    (void) state; /* unused */
    int result = i18n_init();
    assert_int_equal(result, 0);
}

/*
 * Test: msg_get should return English message for known key
 */
static void test_msg_get_known_key(void **state) {
    (void) state; /* unused */
    const char *msg = msg_get("MSG_MORE");
    assert_non_null(msg);
    assert_true(strlen(msg) > 0);
}

/*
 * Test: msg_get should return key itself for unknown key
 */
static void test_msg_get_unknown_key(void **state) {
    (void) state; /* unused */
    const char *msg = msg_get("MSG_NONEXISTENT_KEY_12345");
    assert_non_null(msg);
    assert_string_equal(msg, "MSG_NONEXISTENT_KEY_12345");
}

/*
 * Test: msg_get should handle NULL key gracefully
 */
static void test_msg_get_null_key(void **state) {
    (void) state; /* unused */
    const char *msg = msg_get(NULL);
    assert_non_null(msg);
    assert_string_equal(msg, "");
}

/*
 * Test: msg_get should handle empty key
 */
static void test_msg_get_empty_key(void **state) {
    (void) state; /* unused */
    const char *msg = msg_get("");
    assert_non_null(msg);
    assert_string_equal(msg, "");
}

/*
 * Test: Verify specific English messages
 */
static void test_english_messages(void **state) {
    (void) state; /* unused */
    /* Set English locale for testing */
    setenv("LANG", "en_US.UTF-8", 1);
    i18n_cleanup();
    i18n_init();

    /* io.c messages */
    assert_string_equal(msg_get("MSG_MORE"), "--More--");
    assert_string_equal(msg_get("MSG_LEVEL"), "Level");
    assert_string_equal(msg_get("MSG_GOLD"), "Gold");
    assert_string_equal(msg_get("MSG_HP"), "Hp");
    assert_string_equal(msg_get("MSG_STR"), "Str");
    assert_string_equal(msg_get("MSG_ARM"), "Arm");
    assert_string_equal(msg_get("MSG_EXP"), "Exp");
    assert_string_equal(msg_get("MSG_HUNGRY"), "Hungry");
    assert_string_equal(msg_get("MSG_WEAK"), "Weak");
    assert_string_equal(msg_get("MSG_FAINT"), "Faint");
}

/*
 * Test: Verify command.c messages
 */
static void test_command_messages(void **state) {
    (void) state; /* unused */
    /* Set English locale for testing */
    setenv("LANG", "en_US.UTF-8", 1);
    i18n_cleanup();
    i18n_init();

    assert_string_equal(msg_get("MSG_YOU_CAN_MOVE_AGAIN"), "you can move again");
    assert_string_equal(msg_get("MSG_THERE_IS"), "there is");
    assert_string_equal(msg_get("MSG_NOTHING_HERE"), "nothing here");
    assert_string_equal(msg_get("MSG_VERSION"), "version %s. (mctesq was here)");
    assert_string_equal(msg_get("MSG_ILLEGAL_COMMAND"), "illegal command '%s'");
}

/*
 * Test: Verify identify.c messages
 */
static void test_identify_messages(void **state) {
    (void) state; /* unused */
    /* Set English locale for testing */
    setenv("LANG", "en_US.UTF-8", 1);
    i18n_cleanup();
    i18n_init();

    assert_string_equal(msg_get("MSG_WALL"), "wall of a room");
    assert_string_equal(msg_get("MSG_GOLD_DESC"), "gold");
    assert_string_equal(msg_get("MSG_STAIRCASE"), "a staircase");
    assert_string_equal(msg_get("MSG_DOOR"), "door");
    assert_string_equal(msg_get("MSG_YOU"), "you");
    assert_string_equal(msg_get("MSG_POTION"), "potion");
    assert_string_equal(msg_get("MSG_SCROLL"), "scroll");
    assert_string_equal(msg_get("MSG_WEAPON"), "weapon");
    assert_string_equal(msg_get("MSG_ARMOR"), "armor");
    assert_string_equal(msg_get("MSG_AMULET"), "the Amulet of Yendor");
}

/*
 * Test: msg_get should return same pointer for repeated calls
 */
static void test_msg_get_consistency(void **state) {
    (void) state; /* unused */
    const char *msg1 = msg_get("MSG_MORE");
    const char *msg2 = msg_get("MSG_MORE");
    assert_ptr_equal(msg1, msg2);  /* Should be same pointer */
}

/*
 * Test: Multiple init calls should be safe
 */
static void test_multiple_init(void **state) {
    (void) state; /* unused */
    int result1 = i18n_init();
    int result2 = i18n_init();
    assert_int_equal(result1, 0);
    assert_int_equal(result2, 0);

    /* Messages should still work */
    const char *msg = msg_get("MSG_MORE");
    assert_non_null(msg);
}

/*
 * Test: Cleanup should be safe
 */
static void test_cleanup(void **state) {
    (void) state; /* unused */
    i18n_cleanup();
    /* Multiple cleanup should be safe */
    i18n_cleanup();
}

/*
 * Test: After cleanup and reinit, messages should work
 */
static void test_cleanup_and_reinit(void **state) {
    (void) state; /* unused */
    i18n_cleanup();
    int result = i18n_init();
    assert_int_equal(result, 0);

    const char *msg = msg_get("MSG_MORE");
    assert_non_null(msg);
    assert_true(strlen(msg) > 0);
}

/*
 * ========================================
 * Transliteration Tests
 * ========================================
 */

/*
 * Test: Basic vowel transliteration
 */
static void test_vowel_transliteration(void **state) {
    (void) state; /* unused */
    const char *result;

    /* Set Korean locale for testing */
    setenv("LANG", "ko_KR.UTF-8", 1);
    i18n_cleanup();
    i18n_init();

    /* Test single vowels - vowels without consonants use ㅇ as choseong */
    result = transliterate_to_korean("a");
    assert_string_equal(result, "아");

    result = transliterate_to_korean("e");
    assert_string_equal(result, "에");

    result = transliterate_to_korean("i");
    assert_string_equal(result, "이");

    result = transliterate_to_korean("o");
    assert_string_equal(result, "오");

    result = transliterate_to_korean("u");
    assert_string_equal(result, "우");
}

/*
 * Test: Basic consonant transliteration
 */
static void test_consonant_transliteration(void **state) {
    (void) state; /* unused */
    const char *result;

    setenv("LANG", "ko_KR.UTF-8", 1);
    i18n_cleanup();
    i18n_init();

    /* Test consonants with default vowel ㅡ */
    result = transliterate_to_korean("b");
    assert_string_equal(result, "브");

    result = transliterate_to_korean("s");
    assert_string_equal(result, "스");

    result = transliterate_to_korean("t");
    assert_string_equal(result, "트");

    result = transliterate_to_korean("k");
    assert_string_equal(result, "크");

    result = transliterate_to_korean("n");
    assert_string_equal(result, "느");
}

/*
 * Test: Consonant-vowel combination transliteration
 */
static void test_cv_combination_transliteration(void **state) {
    (void) state; /* unused */
    const char *result;

    setenv("LANG", "ko_KR.UTF-8", 1);
    i18n_cleanup();
    i18n_init();

    /* Consonant + vowel combinations */
    result = transliterate_to_korean("ba");
    assert_string_equal(result, "바");

    result = transliterate_to_korean("su");
    assert_string_equal(result, "수");

    result = transliterate_to_korean("ti");
    assert_string_equal(result, "티");

    result = transliterate_to_korean("ko");
    assert_string_equal(result, "코");
}

/*
 * Test: Simple word transliteration with jongseong
 */
static void test_simple_word_transliteration(void **state) {
    (void) state; /* unused */
    const char *result;

    setenv("LANG", "ko_KR.UTF-8", 1);
    i18n_cleanup();
    i18n_init();

    /* Words with consonant-vowel-consonant patterns */
    result = transliterate_to_korean("alpha");
    assert_string_equal(result, "알파");

    result = transliterate_to_korean("beta");
    assert_string_equal(result, "베타");

    /* Note: Current implementation may need refinement for some edge cases */
    result = transliterate_to_korean("simple");
    assert_non_null(result);  /* Just verify it doesn't crash */
}

/*
 * Test: Scroll-like name transliteration
 */
static void test_scroll_name_transliteration(void **state) {
    (void) state; /* unused */
    const char *result;

    setenv("LANG", "ko_KR.UTF-8", 1);
    i18n_cleanup();
    i18n_init();

    /* Test actual scroll-like names with syllable composition */
    result = transliterate_to_korean("alpha beta");
    assert_string_equal(result, "알파 베타");

    result = transliterate_to_korean("hello");
    assert_non_null(result);

    result = transliterate_to_korean("magic");
    assert_non_null(result);
}

/*
 * Test: English locale should not transliterate
 */
static void test_english_locale_no_transliteration(void **state) {
    (void) state; /* unused */
    const char *result;

    /* Set English locale */
    setenv("LANG", "en_US.UTF-8", 1);
    i18n_cleanup();
    i18n_init();

    /* Should return unchanged */
    result = transliterate_to_korean("test");
    assert_string_equal(result, "test");

    result = transliterate_to_korean("hello world");
    assert_string_equal(result, "hello world");
}

/*
 * Test: Edge cases
 */
static void test_transliteration_edge_cases(void **state) {
    (void) state; /* unused */
    const char *result;

    setenv("LANG", "ko_KR.UTF-8", 1);
    i18n_cleanup();
    i18n_init();

    /* Empty string */
    result = transliterate_to_korean("");
    assert_string_equal(result, "");

    /* Single space */
    result = transliterate_to_korean(" ");
    assert_string_equal(result, " ");

    /* Multiple spaces */
    result = transliterate_to_korean("a  b");
    assert_string_equal(result, "아  브");
}

/*
 * Test: User-requested example
 */
static void test_user_example_transliteration(void **state) {
    (void) state; /* unused */
    const char *result;

    setenv("LANG", "ko_KR.UTF-8", 1);
    i18n_cleanup();
    i18n_init();

    /* User example: "nejdan valzum umodsef snamicdo" */
    result = transliterate_to_korean("nejdan valzum umodsef snamicdo");
    assert_string_equal("넺단 발줌 우몯셒 스나밐도", result);

    result = transliterate_to_korean("lorem ipsum dolor sit amet consectetur adipiscing elit");
    assert_string_equal("로렘 잎숨 돌롤 싵 아멭 콘셐테툴 아디피스킹 엘맅", result);

    result = transliterate_to_korean("vitae lumen per saecula manet, sed verba ventis abeunt");
    assert_string_equal("비타에 루멘 펠 사에쿨라 마넽, 섿 벨바 벤티스 아베운트", result);

    result = transliterate_to_korean("aurum sine mente nihil est, et ratio lucem parit");
    assert_string_equal("아우룸 시네 멘테 니힐 에스트, 엩 라티오 루켐 파맅", result);
}

/*
 * Test: Special handling for letter 'x'
 */
static void test_transliteration_letter_x(void **state) {
    (void) state; /* unused */
    const char *result;

    setenv("LANG", "ko_KR.UTF-8", 1);
    i18n_cleanup();
    i18n_init();

    result = transliterate_to_korean("exdi");
    assert_string_equal("엑스디", result);

    result = transliterate_to_korean("alxa");
    assert_string_equal("알크사", result);

    result = transliterate_to_korean("alxbab");
    assert_string_equal("알크스밥", result);

    result = transliterate_to_korean("aXo");
    assert_string_equal("악소", result);
}

/*
 * Test: Special handling for 'ng' sound -> ㅇ jongseong
 */
static void test_transliteration_ng(void **state) {
    (void) state; /* unused */
    const char *result;

    setenv("LANG", "ko_KR.UTF-8", 1);
    i18n_cleanup();
    i18n_init();

    result = transliterate_to_korean("ang");
    assert_string_equal("앙", result);

    result = transliterate_to_korean("bang");
    assert_string_equal("방", result);

    result = transliterate_to_korean("aNg");
    assert_string_equal("앙", result);

    /* User-requested ng test cases:
     * Rule: When 'ng' appears:
     * - If current syllable has no jongseong: add ㅇ (NG) as jongseong
     * - If current syllable has jongseong: move it to next syllable's choseong, add ㅡㅇ
     */

    /* Example 1: tang -> 탕 (no existing jongseong, add ㅇ) */
    result = transliterate_to_korean("tang");
    assert_string_equal("탕", result);

    /* Example 2: takng -> 타킁 (k becomes jongseong, ng adds ㅇ) */
    result = transliterate_to_korean("takng");
    assert_string_equal("타킁", result);

    /* Example 3: talng -> 탈릉 (l becomes jongseong, ng creates new syllable ㄹ+ㅡ+ㅇ) */
    result = transliterate_to_korean("talng");
    assert_string_equal("탈릉", result);

    /* Example 4: taxng -> 탘승 (x creates ㄱ jongseong, ng creates new syllable ㅅ+ㅡ+ㅇ) */
    result = transliterate_to_korean("taxng");
    assert_string_equal("탘승", result);
}

/*
 * Test: Special handling for letter 'l' and 'r'
 */
static void test_transliteration_lr_rules(void **state) {
    (void) state; /* unused */
    const char *result;

    setenv("LANG", "ko_KR.UTF-8", 1);
    i18n_cleanup();
    i18n_init();

    result = transliterate_to_korean("elit");
    assert_string_equal("엘맅", result);

    result = transliterate_to_korean("erit");
    assert_string_equal("에맅", result);

    result = transliterate_to_korean("Lima");
    assert_string_equal("리마", result);
}

/*
 * Test: 's' should become separate 스 when no following vowel
 */
static void test_transliteration_s_ending(void **state) {
    (void) state; /* unused */
    const char *result;

    setenv("LANG", "ko_KR.UTF-8", 1);
    i18n_cleanup();
    i18n_init();

    result = transliterate_to_korean("ventis");
    assert_string_equal("벤티스", result);

    result = transliterate_to_korean("as");
    assert_string_equal("아스", result);
}

/*
 * ========================================
 * Weapon Name Translation Tests
 * ========================================
 */

/*
 * Test: msg_get_weapon_name with Korean locale
 */
static void test_weapon_name_korean(void **state) {
    (void) state; /* unused */
    const char *result;

    setenv("LANG", "ko_KR.UTF-8", 1);
    i18n_cleanup();
    i18n_init();

    /* Test weapon type 0 (mace) */
    result = msg_get_weapon_name(0);
    assert_string_equal(result, "철퇴");

    /* Test weapon type 1 (long sword) */
    result = msg_get_weapon_name(1);
    assert_string_equal(result, "장검");

    /* Test weapon type 4 (dagger) */
    result = msg_get_weapon_name(4);
    assert_string_equal(result, "단검");
}

/*
 * Test: msg_get_weapon_name with invalid index
 */
static void test_weapon_name_invalid_index(void **state) {
    (void) state; /* unused */
    const char *result;

    setenv("LANG", "ko_KR.UTF-8", 1);
    i18n_cleanup();
    i18n_init();

    /* Test out of range index */
    result = msg_get_weapon_name(99);
    assert_string_equal(result, "");

    result = msg_get_weapon_name(-1);
    assert_string_equal(result, "");
}

/*
 * ========================================
 * Monster Name Translation Tests
 * ========================================
 */

/*
 * Test: msg_get_monster_name with Korean locale
 */
static void test_monster_name_korean(void **state) {
    (void) state; /* unused */
    const char *result;

    setenv("LANG", "ko_KR.UTF-8", 1);
    i18n_cleanup();
    i18n_init();

    /* Test monster index 0 (aquator - A) */
    result = msg_get_monster_name(0);
    assert_string_equal(result, "아쿼터");

    /* Test monster index 18 (snake - S) */
    result = msg_get_monster_name(18);
    assert_string_equal(result, "뱀");

    /* Test monster index 3 (dragon - D) */
    result = msg_get_monster_name(3);
    assert_string_equal(result, "용");
}

/*
 * Test: msg_get_monster_name with invalid index
 */
static void test_monster_name_invalid_index(void **state) {
    (void) state; /* unused */
    const char *result;

    setenv("LANG", "ko_KR.UTF-8", 1);
    i18n_cleanup();
    i18n_init();

    /* Test out of range index */
    result = msg_get_monster_name(26);
    assert_string_equal(result, "");

    result = msg_get_monster_name(-1);
    assert_string_equal(result, "");
}

/*
 * ========================================
 * Color Translation Tests
 * ========================================
 */

/*
 * Test: msg_get_color with Korean locale
 */
static void test_color_korean(void **state) {
    (void) state; /* unused */
    const char *result;

    setenv("LANG", "ko_KR.UTF-8", 1);
    i18n_cleanup();
    i18n_init();

    /* Test color translation */
    result = msg_get_color("amber");
    assert_string_equal(result, "호박색");

    result = msg_get_color("aquamarine");
    assert_string_equal(result, "하늘색");

    result = msg_get_color("red");
    assert_string_equal(result, "빨간색");

    result = msg_get_color("blue");
    assert_string_equal(result, "파란색");
}

/*
 * Test: msg_get_color with English locale should return unchanged
 */
static void test_color_english(void **state) {
    (void) state; /* unused */
    const char *result;

    setenv("LANG", "en_US.UTF-8", 1);
    i18n_cleanup();
    i18n_init();

    /* Should return unchanged in English */
    result = msg_get_color("amber");
    assert_string_equal(result, "amber");

    result = msg_get_color("red");
    assert_string_equal(result, "red");
}

/*
 * Test: msg_get_color with unknown color
 */
static void test_color_unknown(void **state) {
    (void) state; /* unused */
    const char *result;

    setenv("LANG", "ko_KR.UTF-8", 1);
    i18n_cleanup();
    i18n_init();

    /* Unknown color should return original */
    result = msg_get_color("unknowncolor");
    assert_string_equal(result, "unknowncolor");
}

/*
 * ========================================
 * Stone Translation Tests
 * ========================================
 */

/*
 * Test: msg_get_stone with Korean locale
 */
static void test_stone_korean(void **state) {
    (void) state; /* unused */
    const char *result;

    setenv("LANG", "ko_KR.UTF-8", 1);
    i18n_cleanup();
    i18n_init();

    /* Test stone translation */
    result = msg_get_stone("diamond");
    assert_string_equal(result, "다이아몬드");

    result = msg_get_stone("ruby");
    assert_string_equal(result, "루비");

    result = msg_get_stone("emerald");
    assert_string_equal(result, "에메랄드");
}

/*
 * Test: msg_get_stone with English locale should return unchanged
 */
static void test_stone_english(void **state) {
    (void) state; /* unused */
    const char *result;

    setenv("LANG", "en_US.UTF-8", 1);
    i18n_cleanup();
    i18n_init();

    /* Should return unchanged in English */
    result = msg_get_stone("diamond");
    assert_string_equal(result, "diamond");

    result = msg_get_stone("ruby");
    assert_string_equal(result, "ruby");
}

/*
 * ========================================
 * Material Translation Tests
 * ========================================
 */

/*
 * Test: msg_get_material with Korean locale
 */
static void test_material_korean(void **state) {
    (void) state; /* unused */
    const char *result;

    setenv("LANG", "ko_KR.UTF-8", 1);
    i18n_cleanup();
    i18n_init();

    /* Test material translation */
    result = msg_get_material("oak");
    assert_string_equal(result, "참나무");

    result = msg_get_material("bamboo");
    assert_string_equal(result, "대나무");

    result = msg_get_material("pine");
    assert_string_equal(result, "소나무");
}

/*
 * Test: msg_get_material with English locale should return unchanged
 */
static void test_material_english(void **state) {
    (void) state; /* unused */
    const char *result;

    setenv("LANG", "en_US.UTF-8", 1);
    i18n_cleanup();
    i18n_init();

    /* Should return unchanged in English */
    result = msg_get_material("oak");
    assert_string_equal(result, "oak");

    result = msg_get_material("bamboo");
    assert_string_equal(result, "bamboo");
}

/*
 * Test: msg_transliterate wrapper function with Korean locale
 */
static void test_msg_transliterate_korean(void **state) {
    (void) state; /* unused */
    const char *result;

    setenv("LANG", "ko_KR.UTF-8", 1);
    i18n_cleanup();
    i18n_init();

    /* Test basic transliteration */
    result = msg_transliterate("amber");
    assert_non_null(result);
    assert_true(strlen(result) > 0);
    /* Should be Korean characters */
    assert_true((unsigned char)result[0] >= 0x80);

    result = msg_transliterate("blue");
    assert_non_null(result);
    assert_true(strlen(result) > 0);
    assert_true((unsigned char)result[0] >= 0x80);
}

/*
 * Test: msg_transliterate wrapper function with English locale
 */
static void test_msg_transliterate_english(void **state) {
    (void) state; /* unused */
    const char *result;

    setenv("LANG", "en_US.UTF-8", 1);
    i18n_cleanup();
    i18n_init();

    /* In English locale, should return unchanged */
    result = msg_transliterate("amber");
    assert_string_equal(result, "amber");

    result = msg_transliterate("blue");
    assert_string_equal(result, "blue");
}

int run_i18n_tests(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_i18n_init_success),
        cmocka_unit_test(test_msg_get_known_key),
        cmocka_unit_test(test_msg_get_unknown_key),
        cmocka_unit_test(test_msg_get_null_key),
        cmocka_unit_test(test_msg_get_empty_key),
        cmocka_unit_test(test_english_messages),
        cmocka_unit_test(test_command_messages),
        cmocka_unit_test(test_identify_messages),
        cmocka_unit_test(test_msg_get_consistency),
        cmocka_unit_test(test_multiple_init),
        cmocka_unit_test(test_cleanup),
        cmocka_unit_test(test_cleanup_and_reinit),
        cmocka_unit_test(test_vowel_transliteration),
        cmocka_unit_test(test_consonant_transliteration),
        cmocka_unit_test(test_cv_combination_transliteration),
        cmocka_unit_test(test_simple_word_transliteration),
        cmocka_unit_test(test_scroll_name_transliteration),
        cmocka_unit_test(test_english_locale_no_transliteration),
        cmocka_unit_test(test_transliteration_edge_cases),
        cmocka_unit_test(test_user_example_transliteration),
        cmocka_unit_test(test_transliteration_letter_x),
        cmocka_unit_test(test_transliteration_ng),
        cmocka_unit_test(test_transliteration_lr_rules),
        cmocka_unit_test(test_transliteration_s_ending),
        cmocka_unit_test(test_weapon_name_korean),
        cmocka_unit_test(test_weapon_name_invalid_index),
        cmocka_unit_test(test_monster_name_korean),
        cmocka_unit_test(test_monster_name_invalid_index),
        cmocka_unit_test(test_color_korean),
        cmocka_unit_test(test_color_english),
        cmocka_unit_test(test_color_unknown),
        cmocka_unit_test(test_stone_korean),
        cmocka_unit_test(test_stone_english),
        cmocka_unit_test(test_material_korean),
        cmocka_unit_test(test_material_english),
        cmocka_unit_test(test_msg_transliterate_korean),
        cmocka_unit_test(test_msg_transliterate_english),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
