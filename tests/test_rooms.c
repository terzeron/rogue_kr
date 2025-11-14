/*
 * Tests for rooms.c helper functions
 */

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <string.h>
#include <cmocka.h>
#include <curses.h>
#include "rogue.h"

extern void rnd_pos(struct room *rp, coord *cp);

/* Ensure each test starts with a blank dungeon layout */
static void reset_places_grid(void) {
    for (int i = 0; i < MAXLINES * MAXCOLS; i++) {
        places[i].p_ch = ' ';
        places[i].p_flags = 0;
        places[i].p_monst = NULL;
    }
}

static void reset_rooms_state(void) {
    memset(rooms, 0, sizeof(struct room) * MAXROOMS);
}

static int setup_rooms_suite(void **state) {
    (void) state;
    reset_places_grid();
    reset_rooms_state();
    seed = 0;
    level = 1;
    return 0;
}

static int teardown_rooms_suite(void **state) {
    (void) state;
    reset_places_grid();
    reset_rooms_state();
    return 0;
}

static void fill_room_interior(struct room *rp, char tile) {
    for (int y = rp->r_pos.y + 1; y < rp->r_pos.y + rp->r_max.y - 1; y++) {
        for (int x = rp->r_pos.x + 1; x < rp->r_pos.x + rp->r_max.x - 1; x++) {
            chat(y, x) = tile;
        }
    }
}

static void set_room_monsters(struct room *rp, THING *monst) {
    for (int y = rp->r_pos.y + 1; y < rp->r_pos.y + rp->r_max.y - 1; y++) {
        for (int x = rp->r_pos.x + 1; x < rp->r_pos.x + rp->r_max.x - 1; x++) {
            INDEX(y, x)->p_monst = monst;
        }
    }
}

/* Test: rnd_pos picks coordinates within room bounds */
static void test_rnd_pos_within_bounds(void **state) {
    (void) state;

    struct room room = { .r_pos = {2, 3}, .r_max = {6, 6} };
    coord c;

    for (int i = 0; i < 10; i++) {
        rnd_pos(&room, &c);
        assert_in_range(c.x, room.r_pos.x + 1, room.r_pos.x + room.r_max.x - 2);
        assert_in_range(c.y, room.r_pos.y + 1, room.r_pos.y + room.r_max.y - 2);
    }
}

/* Test: vert() draws a vertical line of walls */
static void test_vert_draws_walls(void **state) {
    (void) state;

    struct room room = { .r_pos = {5, 4}, .r_max = {6, 6} };
    vert(&room, room.r_pos.x);

    for (int y = room.r_pos.y + 1; y <= room.r_pos.y + room.r_max.y - 1; y++) {
        assert_int_equal('|', chat(y, room.r_pos.x));
    }
}

/* Test: horiz() draws a horizontal line of walls */
static void test_horiz_draws_walls(void **state) {
    (void) state;

    struct room room = { .r_pos = {4, 6}, .r_max = {7, 5} };
    horiz(&room, room.r_pos.y);

    for (int x = room.r_pos.x; x <= room.r_pos.x + room.r_max.x - 1; x++) {
        assert_int_equal('-', chat(room.r_pos.y, x));
    }
}

/* Test: draw_room() places the border and fills floors */
static void test_draw_room_builds_room(void **state) {
    (void) state;

    struct room room = { .r_pos = {3, 3}, .r_max = {6, 5} };
    draw_room(&room);

    /* Check horizontal borders */
    for (int x = room.r_pos.x; x < room.r_pos.x + room.r_max.x; x++) {
        assert_int_equal('-', chat(room.r_pos.y, x));
        assert_int_equal('-', chat(room.r_pos.y + room.r_max.y - 1, x));
    }
    /* Check vertical borders */
    for (int y = room.r_pos.y + 1; y < room.r_pos.y + room.r_max.y - 1; y++) {
        assert_int_equal('|', chat(y, room.r_pos.x));
        assert_int_equal('|', chat(y, room.r_pos.x + room.r_max.x - 1));
    }
    /* Interior must be floor tiles */
    for (int y = room.r_pos.y + 1; y < room.r_pos.y + room.r_max.y - 1; y++) {
        for (int x = room.r_pos.x + 1; x < room.r_pos.x + room.r_max.x - 1; x++) {
            assert_int_equal(FLOOR, chat(y, x));
        }
    }
}

/* Test: find_floor() locates a valid floor tile in a normal room */
static void test_find_floor_regular_room(void **state) {
    (void) state;

    struct room room = { .r_pos = {1, 1}, .r_max = {6, 6}, .r_flags = 0 };
    draw_room(&room);

    coord where;
    assert_true(find_floor(&room, &where, 50, FALSE));
    assert_int_equal(FLOOR, chat(where.y, where.x));
    assert_in_range(where.x, room.r_pos.x + 1, room.r_pos.x + room.r_max.x - 2);
    assert_in_range(where.y, room.r_pos.y + 1, room.r_pos.y + room.r_max.y - 2);
}

/* Test: find_floor() honors maze rooms by looking for PASSAGE tiles */
static void test_find_floor_maze_room(void **state) {
    (void) state;

    struct room room = { .r_pos = {2, 2}, .r_max = {7, 7}, .r_flags = ISMAZE };
    fill_room_interior(&room, PASSAGE);

    coord where;
    assert_true(find_floor(&room, &where, 50, FALSE));
    assert_int_equal(PASSAGE, chat(where.y, where.x));
}

/* Test: find_floor() fails when the iteration limit is exhausted */
static void test_find_floor_respects_limit(void **state) {
    (void) state;

    struct room room = { .r_pos = {1, 1}, .r_max = {6, 6}, .r_flags = 0 };
    fill_room_interior(&room, ' '); /* No floor tiles available */

    coord where;
    assert_false(find_floor(&room, &where, 3, FALSE));
}

/* Test: find_floor() with monst=true requires walkable, unoccupied tiles */
static void test_find_floor_monster_spot(void **state) {
    (void) state;

    struct room room = { .r_pos = {1, 1}, .r_max = {6, 6}, .r_flags = 0 };
    fill_room_interior(&room, FLOOR);

    coord where;
    assert_true(find_floor(&room, &where, 20, TRUE));
    assert_int_equal(FLOOR, chat(where.y, where.x));
}

/* Test: find_floor() with monst=true fails if everything is occupied */
static void test_find_floor_monster_blocked(void **state) {
    (void) state;

    struct room room = { .r_pos = {1, 1}, .r_max = {6, 6}, .r_flags = 0 };
    fill_room_interior(&room, FLOOR);
    set_room_monsters(&room, &player);

    coord where;
    assert_false(find_floor(&room, &where, 5, TRUE));
}

/* Test: find_floor() can select a room when rp is NULL */
static void test_find_floor_with_null_room(void **state) {
    (void) state;

    struct room *first = &rooms[0];
    first->r_pos.x = 1;
    first->r_pos.y = 1;
    first->r_max.x = 6;
    first->r_max.y = 6;
    first->r_flags = 0;
    draw_room(first);

    for (int i = 1; i < MAXROOMS; i++) {
        rooms[i].r_flags = ISGONE;
    }

    coord where;
    assert_true(find_floor(NULL, &where, 20, FALSE));
    assert_in_range(where.x, first->r_pos.x + 1, first->r_pos.x + first->r_max.x - 2);
    assert_in_range(where.y, first->r_pos.y + 1, first->r_pos.y + first->r_max.y - 2);
}

int run_rooms_tests(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(test_rnd_pos_within_bounds, setup_rooms_suite, teardown_rooms_suite),
        cmocka_unit_test_setup_teardown(test_vert_draws_walls, setup_rooms_suite, teardown_rooms_suite),
        cmocka_unit_test_setup_teardown(test_horiz_draws_walls, setup_rooms_suite, teardown_rooms_suite),
        cmocka_unit_test_setup_teardown(test_draw_room_builds_room, setup_rooms_suite, teardown_rooms_suite),
        cmocka_unit_test_setup_teardown(test_find_floor_regular_room, setup_rooms_suite, teardown_rooms_suite),
        cmocka_unit_test_setup_teardown(test_find_floor_maze_room, setup_rooms_suite, teardown_rooms_suite),
        cmocka_unit_test_setup_teardown(test_find_floor_respects_limit, setup_rooms_suite, teardown_rooms_suite),
        cmocka_unit_test_setup_teardown(test_find_floor_monster_spot, setup_rooms_suite, teardown_rooms_suite),
        cmocka_unit_test_setup_teardown(test_find_floor_monster_blocked, setup_rooms_suite, teardown_rooms_suite),
        cmocka_unit_test_setup_teardown(test_find_floor_with_null_room, setup_rooms_suite, teardown_rooms_suite),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
