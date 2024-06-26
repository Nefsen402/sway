#ifndef _SWAY_INPUT_CURSOR_H
#define _SWAY_INPUT_CURSOR_H
#include <stdbool.h>
#include <stdint.h>
#include <wlr/types/wlr_scene.h>
#include <wlr/types/wlr_pointer_constraints_v1.h>
#include <wlr/types/wlr_pointer_gestures_v1.h>
#include <wlr/types/wlr_compositor.h>
#include "sway/input/seat.h"
#include "config.h"

#define SWAY_CURSOR_PRESSED_BUTTONS_CAP 32

#define SWAY_SCROLL_UP KEY_MAX + 1
#define SWAY_SCROLL_DOWN KEY_MAX + 2
#define SWAY_SCROLL_LEFT KEY_MAX + 3
#define SWAY_SCROLL_RIGHT KEY_MAX + 4

struct sway_cursor {
	struct sway_seat *seat;
	struct {
		double x, y;
		struct sway_node *node;
	} previous;
	struct wlr_xcursor_manager *xcursor_manager;
	struct wl_list tablets;
	struct wl_list tablet_pads;

	const char *image;

	struct wlr_pointer_constraint_v1 *active_constraint;
	pixman_region32_t confine; // invalid if active_constraint == NULL
	bool active_confine_requires_warp;
	
	bool simulating_pointer_from_touch;
	bool pointer_touch_up;
	int32_t pointer_touch_id;

	bool simulating_pointer_from_tool_tip;
	bool simulating_pointer_from_tool_button;
	uint32_t tool_buttons;

	struct wl_listener request_set_cursor;

	struct wl_listener constraint_commit;

	struct wl_event_source *hide_source;
	// This field is just a cache of the field in seat_config in order to avoid
	// costly seat_config lookups on every keypress. HIDE_WHEN_TYPING_DEFAULT
	// indicates that there is no cached value.
	enum seat_config_hide_cursor_when_typing hide_when_typing;

	size_t pressed_button_count;

	struct wlr_scene_tree *scene;
	double x, y;
};

struct sway_cursor_constraints {
	struct wlr_box box;
	struct sway_output *output;
};

struct sway_cursor_pointer {
	struct sway_cursor *cursor;
	struct wlr_pointer *wlr_pointer;

	struct wl_listener hold_begin;
	struct wl_listener hold_end;
	struct wl_listener pinch_begin;
	struct wl_listener pinch_update;
	struct wl_listener pinch_end;
	struct wl_listener swipe_begin;
	struct wl_listener swipe_update;
	struct wl_listener swipe_end;
	struct wl_listener motion;
	struct wl_listener motion_absolute;
	struct wl_listener button;
	struct wl_listener axis;
	struct wl_listener frame;

	struct wl_listener destroy;

	struct sway_cursor_constraints constraints;
};

struct sway_cursor_touch {
	struct sway_cursor *cursor;
	struct wlr_touch *wlr_touch;

	struct wl_listener down;
	struct wl_listener up;
	struct wl_listener motion;
	struct wl_listener frame;
	struct wl_listener cancel;

	struct wl_listener destroy;

	struct sway_cursor_constraints constraints;
};

struct sway_cursor_tablet {
	struct sway_cursor *cursor;
	struct wlr_tablet *wlr_tablet;

	struct wl_listener tool_axis;
	struct wl_listener tool_tip;
	struct wl_listener tool_proximity;
	struct wl_listener tool_button;

	struct wl_listener destroy;

	struct sway_cursor_constraints constraints;
};

struct sway_node;

struct sway_node *node_at_coords(
		struct sway_seat *seat, double lx, double ly,
		struct wlr_surface **surface, double *sx, double *sy);

void sway_cursor_destroy(struct sway_cursor *cursor);
struct sway_cursor *sway_cursor_create(struct sway_seat *seat);

void sway_cursor_move(struct sway_cursor *cursor, double delta_x, double delta_y);
void sway_cursor_warp(struct sway_cursor *cursor, double x, double y);

struct sway_cursor_pointer *sway_cursor_pointer_create(struct sway_cursor *cursor, struct wlr_pointer *pointer);
struct sway_cursor_tablet *sway_cursor_tablet_create(struct sway_cursor *cursor, struct wlr_tablet *tablet);
struct sway_cursor_touch *sway_cursor_touch_create(struct sway_cursor *cursor, struct wlr_touch *touch);

/**
 * "Rebase" a cursor on top of whatever view is underneath it.
 *
 * This chooses a cursor icon and sends a motion event to the surface.
 */
void cursor_rebase(struct sway_cursor *cursor);
void cursor_rebase_all(void);
void cursor_update_image(struct sway_cursor *cursor, struct sway_node *node);

void cursor_handle_activity_from_idle_source(struct sway_cursor *cursor,
		enum sway_input_idle_source idle_source);
void cursor_handle_activity_from_device(struct sway_cursor *cursor,
		struct wlr_input_device *device);
void cursor_unhide(struct sway_cursor *cursor);
int cursor_get_timeout(struct sway_cursor *cursor);
void cursor_notify_key_press(struct sway_cursor *cursor);

void pointer_motion(struct sway_cursor *cursor, uint32_t time_msec,
		struct wlr_input_device *device, double dx, double dy,
		double dx_unaccel, double dy_unaccel);

void dispatch_cursor_button(struct sway_cursor *cursor,
	struct wlr_input_device *device, uint32_t time_msec, uint32_t button,
	enum wl_pointer_button_state state);

void dispatch_cursor_axis(struct sway_cursor *cursor,
		struct wlr_pointer_axis_event *event);

void cursor_set_image(struct sway_cursor *cursor, const char *image,
	struct wl_client *client);

void cursor_set_image_surface(struct sway_cursor *cursor,
		struct wlr_surface *surface, int32_t hotspot_x, int32_t hotspot_y,
		struct wl_client *client);

void cursor_warp_to_container(struct sway_cursor *cursor,
	struct sway_container *container, bool force);

void cursor_warp_to_workspace(struct sway_cursor *cursor,
		struct sway_workspace *workspace);


void sway_cursor_constrain(struct sway_cursor *cursor,
	struct wlr_pointer_constraint_v1 *constraint);

uint32_t get_mouse_bindsym(const char *name, char **error);

uint32_t get_mouse_bindcode(const char *name, char **error);

// Considers both bindsym and bindcode
uint32_t get_mouse_button(const char *name, char **error);

const char *get_mouse_button_name(uint32_t button);

void handle_request_set_cursor_shape(struct wl_listener *listener, void *data);

#endif
