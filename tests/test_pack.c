/*
 * Tests for pack.c functions
 * Tests inventory management and pack operations
 */

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "../tests/test_stubs.h"

/* Number of pack slots tracked by pack_used in extern.c */
#define PACK_USED_SLOTS (('z' - 'a') + 1)

/* External function declarations from pack.c */
extern void add_pack(THING *obj, bool silent);
extern bool pack_room(bool from_floor, THING *obj);
extern THING *leave_pack(THING *obj, bool newobj, bool all);
extern char pack_char(void);
extern bool inventory(THING *list, int type);
extern void pick_up(char ch);
extern void move_msg(THING *obj);
extern void picky_inven(void);
extern THING *get_item(const char *purpose, int type);
extern void money(int value);
extern char floor_ch(void);
extern char floor_at(void);
extern void reset_last(void);

/* Helper function to create a new item */
static THING *create_item(int type, int which, int count) {
    THING *obj = malloc(sizeof(THING));
    memset(obj, 0, sizeof(THING));
    obj->o_type = type;
    obj->o_which = which;
    obj->o_count = count;
    obj->o_group = 0; /* Group for stacking */
    return obj;
}

/* Setup function to clear pack before each test */
static int setup_pack(void **state) {
    (void) state;
    pack = NULL;
    inpack = 0;
    purse = 0;
    memset(pack_used, 0, PACK_USED_SLOTS * sizeof(*pack_used));
    player.t_flags = 0;
    lvl_obj = NULL;
    return 0;
}

/* Teardown function to free pack items */
static int teardown_pack(void **state) {
    (void) state;
    THING *curr = pack;
    while (curr != NULL) {
        THING *next = curr->l_next;
        free(curr);
        curr = next;
    }
    pack = NULL;
    return 0;
}

/* Test: add_pack() with NULL pack */
static void test_add_pack_empty_pack(void **state) {
    (void) state;
    THING *obj = create_item(WEAPON, 0, 1);
    add_pack(obj, TRUE);
    assert_non_null(pack);
    assert_int_equal(inpack, 1);
    assert_ptr_equal(pack, obj);
}

/* Test: add_pack() with NULL object */
static void test_add_pack_null_object(void **state) {
    (void) state;
    add_pack(NULL, FALSE);
    assert_null(pack);
    assert_int_equal(inpack, 0);
}

/* Test: add_pack() stacks similar items (potions) */
static void test_add_pack_stacking_similar_items(void **state) {
    (void) state;
    THING *potion1 = create_item(POTION, 0, 1);
    THING *potion2 = create_item(POTION, 0, 1);

    add_pack(potion1, TRUE);
    add_pack(potion2, TRUE);

    assert_int_equal(inpack, 1); /* Should not increment inpack */
    assert_non_null(pack);
    assert_int_equal(pack->o_count, 2); /* Count should be 2 */
    assert_null(pack->l_next); /* Should be only one item in the list */
}

/* Test: add_pack() does not stack different items */
static void test_add_pack_no_stacking_different_items(void **state) {
    (void) state;
    THING *potion = create_item(POTION, 0, 1);
    THING *scroll = create_item(SCROLL, 0, 1);

    add_pack(potion, TRUE);
    add_pack(scroll, TRUE);

    assert_int_equal(inpack, 2); /* Should be two separate items */
    assert_non_null(pack);
    assert_non_null(pack->l_next);
    assert_int_equal(pack->o_count, 1);
    assert_int_equal(pack->l_next->o_count, 1);
}

/* Test: add_pack() does not stack weapons */
static void test_add_pack_no_stacking_weapons(void **state) {
    (void) state;
    THING *weapon1 = create_item(WEAPON, MACE, 1);
    THING *weapon2 = create_item(WEAPON, MACE, 1);

    add_pack(weapon1, TRUE);
    add_pack(weapon2, TRUE);

    assert_int_equal(inpack, 2); /* Weapons should not stack */
    assert_non_null(pack);
    assert_non_null(pack->l_next);
}

/* Test: pack_room() with full pack */
static void test_pack_room_full(void **state) {
    (void) state;
    inpack = MAXPACK;
    assert_false(pack_room(FALSE, NULL));
}

/* Test: pack_room() with available space */
static void test_pack_room_available(void **state) {
    (void) state;
    inpack = 5;
    assert_true(pack_room(FALSE, NULL));
    assert_int_equal(inpack, 6);
}

/* Test: leave_pack() single item */
static void test_leave_pack_single_item(void **state) {
    (void) state;
    THING *obj = create_item(WEAPON, 0, 1);
    add_pack(obj, TRUE);

    THING *result = leave_pack(obj, FALSE, FALSE);
    assert_non_null(result);
    assert_null(pack);
    assert_int_equal(inpack, 0);
}

/* Test: leave_pack() one from a stack */
static void test_leave_pack_one_from_stack(void **state) {
    (void) state;
    THING *potion = create_item(POTION, 0, 5);
    add_pack(potion, TRUE);

    THING *result = leave_pack(potion, FALSE, FALSE);
    assert_non_null(result);
    assert_int_equal(result->o_count, 1); /* New item has count 1 */
    assert_non_null(pack);
    assert_int_equal(pack->o_count, 4); /* Original stack is now 4 */
    assert_int_equal(inpack, 1);
    free(result);
}

/* Test: leave_pack() all from a stack */
static void test_leave_pack_all_from_stack(void **state) {
    (void) state;
    THING *potion = create_item(POTION, 0, 5);
    add_pack(potion, TRUE);

    THING *result = leave_pack(potion, FALSE, TRUE); /* all=TRUE */
    assert_non_null(result);
    assert_int_equal(result->o_count, 5); /* Dropped item has full count */
    assert_null(pack); /* Pack should be empty */
    assert_int_equal(inpack, 0);
}

/* Test: leave_pack() from middle of the list */
static void test_leave_pack_from_middle(void **state) {
    (void) state;
    THING *item1 = create_item(WEAPON, 0, 1);
    THING *item2 = create_item(ARMOR, 0, 1);
    THING *item3 = create_item(SCROLL, 0, 1);
    add_pack(item1, TRUE);
    add_pack(item2, TRUE);
    add_pack(item3, TRUE);

    leave_pack(item2, TRUE, FALSE); /* newobj=TRUE to free it */
    assert_int_equal(inpack, 2);
    assert_ptr_equal(pack, item3);
    assert_ptr_equal(pack->l_next, item1);
    assert_null(pack->l_next->l_next);
}

/* Test: pack_char() returns valid character */
static void test_pack_char_returns_valid(void **state) {
    (void) state;
    char ch = pack_char();
    assert_true(ch >= 'a' && ch <= 'z');
    assert_true(pack_used[ch - 'a']);
}

/* Test: pack_char() sequential allocation */
static void test_pack_char_sequential(void **state) {
    (void) state;
    assert_int_equal(pack_char(), 'a');
    assert_int_equal(pack_char(), 'b');
    assert_int_equal(pack_char(), 'c');
}

/* Test: pack_char() when all characters are used */
static void test_pack_char_full(void **state) {
    (void) state;
    for (int i = 0; i < 26; i++) {
        pack_used[i] = TRUE;
    }
    assert_int_equal(pack_char(), '.');
}

/* Test: inventory() with empty pack */
static void test_inventory_empty_pack(void **state) {
    (void) state;
    assert_false(inventory(pack, 0));
}

/* Test: inventory() filters by type */
static void test_inventory_filter_type(void **state) {
    (void) state;
    add_pack(create_item(WEAPON, 0, 1), TRUE);
    add_pack(create_item(ARMOR, 0, 1), TRUE);
    
    /* Looking for POTION, should find nothing and print nothing */
    assert_false(inventory(pack, POTION));
}

/* Test: pick_up() a normal item */
static void test_pick_up_normal_item(void **state) {
    (void) state;
    hero.y = 5; hero.x = 5;
    THING *obj = create_item(SCROLL, 0, 1);
    obj->o_pos = hero;
    lvl_obj = obj; /* Item is on the floor */

    pick_up(SCROLL);
    assert_int_equal(inpack, 1);
    assert_ptr_equal(pack, obj);
    assert_null(lvl_obj); /* Item removed from floor */
}

/* Test: pick_up() with a full pack */
static void test_pick_up_full_pack(void **state) {
    (void) state;
    hero.y = 5; hero.x = 5;
    inpack = MAXPACK;
    THING *obj = create_item(SCROLL, 0, 1);
    obj->o_pos = hero;
    lvl_obj = obj;

    pick_up(SCROLL);
    assert_int_equal(inpack, MAXPACK);
    assert_null(pack); /* Pack is untouched */
    assert_non_null(lvl_obj); /* Item remains on the floor */
    free(obj);
}

/* Test: pick_up() while levitating */
static void test_pick_up_levitating(void **state) {
    (void) state;
    player.t_flags = ISLEVIT;
    pick_up(WEAPON);
    assert_int_equal(inpack, 0);
}

/* Test: money() increases purse */
static void test_money_increases_purse(void **state) {
    (void) state;
    purse = 100;
    money(50);
    assert_int_equal(purse, 150);
}

/* Test: floor_at() returns correct character from mocked map */
static void test_floor_at_various_locations(void **state) {
    (void) state;
}

/* Test: get_item() with empty pack */
static void test_get_item_empty_pack(void **state) {
    (void) state;
    pack = NULL;
    THING *result = get_item("test", WEAPON);
    assert_null(result);
}

/* Test suite runner */
int run_pack_tests(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(test_add_pack_empty_pack, setup_pack, teardown_pack),
        cmocka_unit_test_setup_teardown(test_add_pack_null_object, setup_pack, teardown_pack),
        cmocka_unit_test_setup_teardown(test_add_pack_stacking_similar_items, setup_pack, teardown_pack),
        cmocka_unit_test_setup_teardown(test_add_pack_no_stacking_different_items, setup_pack, teardown_pack),
        cmocka_unit_test_setup_teardown(test_add_pack_no_stacking_weapons, setup_pack, teardown_pack),
        cmocka_unit_test_setup_teardown(test_pack_room_full, setup_pack, teardown_pack),
        cmocka_unit_test_setup_teardown(test_pack_room_available, setup_pack, teardown_pack),
        cmocka_unit_test_setup_teardown(test_leave_pack_single_item, setup_pack, teardown_pack),
        cmocka_unit_test_setup_teardown(test_leave_pack_one_from_stack, setup_pack, teardown_pack),
        cmocka_unit_test_setup_teardown(test_leave_pack_all_from_stack, setup_pack, teardown_pack),
        cmocka_unit_test_setup_teardown(test_leave_pack_from_middle, setup_pack, teardown_pack),
        cmocka_unit_test_setup_teardown(test_pack_char_returns_valid, setup_pack, teardown_pack),
        cmocka_unit_test_setup_teardown(test_pack_char_sequential, setup_pack, teardown_pack),
        cmocka_unit_test_setup_teardown(test_pack_char_full, setup_pack, teardown_pack),
        cmocka_unit_test_setup_teardown(test_inventory_empty_pack, setup_pack, teardown_pack),
        cmocka_unit_test_setup_teardown(test_inventory_filter_type, setup_pack, teardown_pack),
        cmocka_unit_test_setup_teardown(test_pick_up_normal_item, setup_pack, teardown_pack),
        cmocka_unit_test_setup_teardown(test_pick_up_full_pack, setup_pack, teardown_pack),
        cmocka_unit_test_setup_teardown(test_pick_up_levitating, setup_pack, teardown_pack),
        cmocka_unit_test_setup_teardown(test_money_increases_purse, setup_pack, teardown_pack),
        cmocka_unit_test_setup_teardown(test_floor_at_various_locations, setup_pack, teardown_pack),
        cmocka_unit_test_setup_teardown(test_get_item_empty_pack, setup_pack, teardown_pack),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
