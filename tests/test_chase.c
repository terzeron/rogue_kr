/*
 * Tests for chase.c functions
 */

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "../tests/test_stubs.h"

/* External function declarations from chase.c */
extern void runners(void);
extern int move_monst(THING *tp);
extern void relocate(THING *th, coord *new_loc);
extern int do_chase(THING *th);
extern void set_oldch(THING *tp, coord *cp);
extern bool see_monst(THING *mp);
extern void runto(coord *runner);
extern bool chase(THING *tp, coord *ee);
extern struct room *roomin(coord *cp);
extern bool diag_ok(coord *sp, coord *ep);
extern bool cansee(int y, int x);
extern coord *find_dest(THING *tp);

/* Helper to clear the map for tests */
static void clear_map(void) {
    for (int y = 0; y < NUMLINES; y++) {
        for (int x = 0; x < NUMCOLS; x++) {
            PLACE *pp = INDEX(y, x);
            pp->p_ch = ' ';
            pp->p_flags = 0;
            pp->p_monst = NULL;
        }
    }
}

/* Helper to create a simple room for testing */
static void create_test_room(void) {
    for (int y = 1; y < 10; y++) {
        for (int x = 1; x < 20; x++) {
            chat(y, x) = FLOOR;
        }
    }
}

/* Setup function for chase tests */
static int setup_chase_tests(void **state) {
    (void) state;
    clear_map();
    create_test_room();

    // Initialize a room in the global rooms array for roomin() to find
    memset(&rooms[0], 0, sizeof(struct room));
    rooms[0].r_pos.y = 1;
    rooms[0].r_pos.x = 1; 
    rooms[0].r_max.y = 9;  // Corresponds to create_test_room's y range (1 to 9, so max is 9)
    rooms[0].r_max.x = 19; // Corresponds to create_test_room's x range (1 to 19, so max is 19)
    rooms[0].r_flags = 0; // Default to not dark

    mlist = NULL;
    player.t_flags = 0;
    hero.y = 5;
    hero.x = 10;

    proom = roomin(&hero); // Initialize proom
    return 0;
}

/* Teardown function */
static int teardown_chase_tests(void **state) {
    (void) state;
    clear_map();
    mlist = NULL;
    return 0;
}

/* Test: chase() moves monster one step closer to the hero */
static void test_chase_moves_closer(void **state) {
    (void) state;
    THING monster;
    memset(&monster, 0, sizeof(THING));
    monster.t_pos.y = 5;
    monster.t_pos.x = 5;

    int before = dist_cp(&monster.t_pos, &hero);

    bool keep_chasing = chase(&monster, &hero);

    assert_true(keep_chasing);
    assert_true(abs(ch_ret.y - monster.t_pos.y) <= 1);
    assert_true(abs(ch_ret.x - monster.t_pos.x) <= 1);
    assert_int_equal(dist_cp(&ch_ret, &hero), before - 1);
}

/* Test: chase() still finds a valid closer square when off-axis */
static void test_chase_moves_off_axis(void **state) {
    (void) state;
    THING monster;
    memset(&monster, 0, sizeof(THING));
    monster.t_pos.y = 3;
    monster.t_pos.x = 3;

    int before = dist_cp(&monster.t_pos, &hero);

    bool keep_chasing = chase(&monster, &hero);

    assert_true(keep_chasing);
    assert_true(!ce(ch_ret, monster.t_pos));
    assert_true(abs(ch_ret.y - monster.t_pos.y) <= 1);
    assert_true(abs(ch_ret.x - monster.t_pos.x) <= 1);
    assert_int_equal(dist_cp(&ch_ret, &hero), before - 1);
}

/* Test: chase() targets the hero's square when already adjacent */
static void test_chase_targets_hero_when_adjacent(void **state) {
    (void) state;
    THING monster;
    memset(&monster, 0, sizeof(THING));
    monster.t_pos.y = 5;
    monster.t_pos.x = 9; /* Directly left of the hero */

    bool keep_chasing = chase(&monster, &hero);

    assert_true(keep_chasing);
    assert_int_equal(ch_ret.y, hero.y);
    assert_int_equal(ch_ret.x, hero.x);
}

/* Test: chase() stays put when every closer tile is blocked */
static void test_chase_stays_put_when_blocked(void **state) {
    (void) state;
    THING monster;
    memset(&monster, 0, sizeof(THING));
    monster.t_pos.y = 5;
    monster.t_pos.x = 8;

    /* Block the direct path; diagonal routes require this tile too. */
    chat(5, 9) = '|';

    bool keep_chasing = chase(&monster, &hero);

    assert_true(keep_chasing);
    assert_true(ce(ch_ret, monster.t_pos));
}

/* Test: see_monst() returns false when player is blind */
static void test_see_monst_player_blind(void **state) {
    (void) state;
    THING monster;
    memset(&monster, 0, sizeof(THING));
    monster.t_pos.y = 5;
    monster.t_pos.x = 5;
    monster.t_room = proom; /* Assume same room */

    /* Player is blind */
    player.t_flags = ISBLIND;
    assert_false(see_monst(&monster));
}

/* Test: see_monst() returns false for invisible monster without CANSEE */
static void test_see_monst_invisible(void **state) {
    (void) state;
    THING monster;
    memset(&monster, 0, sizeof(THING));
    monster.t_pos.y = 5;
    monster.t_pos.x = 5;
    monster.t_flags = ISINVIS;
    monster.t_room = proom;

    player.t_flags = 0;
    assert_false(see_monst(&monster));
}

/* Test: see_monst() returns true for visible monster in lit room */
static void test_see_monst_visible_in_room(void **state) {
    (void) state;
    THING monster;
    memset(&monster, 0, sizeof(THING));
    monster.t_pos.y = 5;
    monster.t_pos.x = 5;
    monster.t_room = proom;
    proom->r_flags &= ~ISDARK; /* Room is lit */

    player.t_flags = 0;
    assert_true(see_monst(&monster));
}

/* Test: diag_ok() returns true for valid diagonal move */
static void test_diag_ok_is_valid(void **state) {
    (void) state;
    coord start, end;
    start.y = 5;
    start.x = 5;
    end.y = 6;
    end.x = 6;

    /* Both adjacent tiles are floor, so it's ok */
    chat(6, 5) = FLOOR; // Tile at (end.y, start.x)
    chat(5, 6) = FLOOR; // Tile at (start.y, end.x)

    // diag_ok checks if both adjacent tiles are step_ok. (5,6) and (6,5)
    assert_true(diag_ok(&start, &end)); // Should be true if both (5,6) and (6,5) are step_ok
}

/* Test: diag_ok() returns false when blocked by a wall */
static void test_diag_ok_is_blocked(void **state) {
    (void) state;
    coord start, end;
    start.y = 5;
    start.x = 5;
    end.y = 6;
    end.x = 6;

    /* One adjacent tile is a wall, so it's not ok. (6,5) is a wall, (5,6) is floor */
    chat(6, 5) = '|';
    chat(5, 6) = FLOOR;

    // (6,5) is a wall, so diag_ok should be false
    assert_false(diag_ok(&start, &end));
}

/* Test suite runner */
int run_chase_tests(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(test_chase_moves_closer, setup_chase_tests, teardown_chase_tests),
        cmocka_unit_test_setup_teardown(test_chase_moves_off_axis, setup_chase_tests, teardown_chase_tests),
        cmocka_unit_test_setup_teardown(test_chase_targets_hero_when_adjacent, setup_chase_tests, teardown_chase_tests),
        cmocka_unit_test_setup_teardown(test_chase_stays_put_when_blocked, setup_chase_tests, teardown_chase_tests),
        cmocka_unit_test_setup_teardown(test_see_monst_player_blind, setup_chase_tests, teardown_chase_tests),
        cmocka_unit_test_setup_teardown(test_see_monst_invisible, setup_chase_tests, teardown_chase_tests),
        cmocka_unit_test_setup_teardown(test_see_monst_visible_in_room, setup_chase_tests, teardown_chase_tests),
        cmocka_unit_test_setup_teardown(test_diag_ok_is_valid, setup_chase_tests, teardown_chase_tests),
        cmocka_unit_test_setup_teardown(test_diag_ok_is_blocked, setup_chase_tests, teardown_chase_tests),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
