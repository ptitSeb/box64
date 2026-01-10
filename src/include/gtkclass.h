#ifndef __GTKCLASS_H__
#define __GTKCLASS_H__

#include <stdint.h>
#include <stddef.h>

#include "globalsymbols.h"

typedef struct box64context_s box64context_t;
typedef struct bridge_s bridge_t;

typedef struct my_GValue_s
{
  size_t      g_type;
  union {
    int        v_int;
    int64_t    v_int64;
    uint64_t   v_uint64;
    float      v_float;
    double     v_double;
    void*      v_pointer;
  } data[2];
} my_GValue_t;

typedef struct my_GTypeInstance_s
{
  void* g_class;
} my_GTypeInstance_t;

typedef struct my_GObject_s
{
  my_GTypeInstance_t  g_type_instance;
  uint32_t            ref_count;
  void*               qdata;
} my_GObject_t;

typedef struct my_GObjectClass_s
{
  size_t   g_type_class;
  void* construct_properties;
  void* (*constructor)     (int type, uint32_t n_construct_properties, void* construct_properties);
  void  (*set_property)    (void* object, uint32_t property_id, void* value, void* pspec);
  void  (*get_property)    (void* object, uint32_t property_id, void* value, void* pspec);
  void  (*dispose)         (void* object);
  void  (*finalize)        (void* object);
  void  (*dispatch_properties_changed) (void* object, uint32_t n_pspecs, void* pspecs);
  void  (*notify)          (void* object, void* pspec);
  void  (*constructed)     (void* object);
  uint32_t flags;
  void*  pdummy[6];
} my_GObjectClass_t;

typedef struct my_GTypeModule_s
{
  my_GObject_t parent_instance;
  uint32_t use_count;
  void* type_infos;      // GSList
  void* interface_infos; // GSList
  char *name;
} my_GTypeModule_t;

typedef struct my_GTypeModuleClass_s
{
  my_GObjectClass_t parent_class;
  int  (*load)   (my_GTypeModule_t* module);
  void (*unload) (my_GTypeModule_t* module);
  void (*reserved1) (void);
  void (*reserved2) (void);
  void (*reserved3) (void);
  void (*reserved4) (void);
} my_GTypeModuleClass_t;

typedef struct my_GInitiallyUnowned_s
{
  my_GTypeInstance_t  g_type_instance;
  uint32_t            ref_count;
  void*               qdata;
} my_GInitiallyUnowned_t;

typedef struct my_GInitiallyUnownedClass_s
{
  int   g_type_class;
  void* construct_properties;
  void* (*constructor)     (int type, uint32_t n_construct_properties, void* construct_properties);
  void  (*set_property)    (void* object, uint32_t property_id, void* value, void* pspec);
  void  (*get_property)    (void* object, uint32_t property_id, void* value, void* pspec);
  void  (*dispose)         (void* object);
  void  (*finalize)        (void* object);
  void  (*dispatch_properties_changed) (void* object, uint32_t n_pspecs, void* pspecs);
  void  (*notify)          (void* object, void* pspec);
  void  (*constructed)     (void* object);
  uint32_t flags;
  void*  pdummy[6];
} my_GInitiallyUnownedClass_t;

typedef struct my_GApplication_s
{
  my_GObject_t parent;
  void* priv;
} my_GApplication_t;

typedef struct my_GApplicationClass_s
{
    my_GObjectClass_t parent_class;
    void (* startup) (void* application);
    void (* activate) (void* application);
    void (* open) (void* application, void* files, int n_files, void* hint);
    int (* command_line) (void* application, void* command_line);
    int (* local_command_line) (void* application, void* arguments, void* exit_status);
    void (* before_emit) (void* application, void* platform_data);
    void (* after_emit) (void* application, void* platform_data);
    void (* add_platform_data) (void* application, void* builder);
    void (* quit_mainloop) (void* application);
    void (* run_mainloop) (void* application);
    void (* shutdown) (void* application);
    int (* dbus_register) (void* application, void* connection, void* object_path, void* error);
    void (* dbus_unregister) (void* appvoidlication, void* connection, void* object_path);
    int (* handle_local_options) (void* application, void* options);
    int (* name_lost) (void* application);
    void*  padding[7];
} my_GApplicationClass_t;

typedef struct my_GtkApplication_s
{
  my_GApplication_t parent;
  void* priv;
} my_GtkApplication_t;

typedef struct my_GtkApplicationClass_s
{
  my_GApplicationClass_t   parent_class;
  void (*window_added)     (void* application, void* window);
  void (*window_removed)   (void* application, void* window);
} my_GtkApplicationClass_t;

typedef struct my_GtkObject_s
{
  my_GInitiallyUnowned_t parent;
  uint32_t flags;
} my_GtkObject_t;

typedef struct my_GtkObjectClass_s
{
  my_GInitiallyUnownedClass_t parent_class;
  void (*set_arg) (void* object, void* arg, uint32_t arg_id);
  void (*get_arg) (void* object, void* arg, uint32_t arg_id);
  void (*destroy) (void* object);
} my_GtkObjectClass_t;

typedef struct my_GdkRectangle_s
{
  int x;
  int y;
  int width;
  int height;
} my_GdkRectangle_t;

typedef struct my_GtkRequisition_s
{
  int width;
  int height;
} my_GtkRequisition_t;

typedef struct my_GtkWidget2_s
{
  my_GtkObject_t  private;
  int16_t         private_flags;
  uint8_t         state;
  char*           name;
  void*           style;  //GtkStyle
  my_GtkRequisition_t requisition;
  my_GdkRectangle_t   allocation;
  void*           window; //GdkWindow
  void*           parent; //GtkWidget
} my_GtkWidget2_t;

typedef struct my_GtkWidget2Class_s
{
  my_GtkObjectClass_t parent_class;
  uint32_t activate_signal;
  uint32_t set_scroll_adjustments_signal;
  void (*dispatch_child_properties_changed) (void* widget, uint32_t n_pspecs, void* pspecs);
  void (* show)                (void* widget);
  void (* show_all)            (void* widget);
  void (* hide)                (void* widget);
  void (* hide_all)            (void* widget);
  void (* map)                 (void* widget);
  void (* unmap)               (void* widget);
  void (* realize)             (void* widget);
  void (* unrealize)           (void* widget);
  void (* size_request)        (void* widget, void* requisition);
  void (* size_allocate)       (void* widget, void* allocation);
  void (* state_changed)       (void* widget, int previous_state);
  void (* parent_set)          (void* widget, void* previous_parent);
  void (* hierarchy_changed)   (void* widget, void* previous_toplevel);
  void (* style_set)           (void* widget, void* previous_style);
  void (* direction_changed)   (void* widget, int previous_direction);
  void (* grab_notify)         (void* widget, int was_grabbed);
  void (* child_notify)        (void* widget, void* pspec);
  int  (* mnemonic_activate)   (void* widget, int group_cycling);
  void (* grab_focus)          (void* widget);
  int  (* focus)               (void* widget, int direction);
  int  (* event)                    (void* widget, void* event);
  int  (* button_press_event)       (void* widget, void* event);
  int  (* button_release_event)     (void* widget, void* event);
  int  (* scroll_event)             (void* widget, void* event);
  int  (* motion_notify_event)      (void* widget, void* event);
  int  (* delete_event)             (void* widget, void* event);
  int  (* destroy_event)            (void* widget, void* event);
  int  (* expose_event)             (void* widget, void* event);
  int  (* key_press_event)          (void* widget, void* event);
  int  (* key_release_event)        (void* widget, void* event);
  int  (* enter_notify_event)       (void* widget, void* event);
  int  (* leave_notify_event)       (void* widget, void* event);
  int  (* configure_event)          (void* widget, void* event);
  int  (* focus_in_event)           (void* widget, void* event);
  int  (* focus_out_event)          (void* widget, void* event);
  int  (* map_event)                (void* widget, void* event);
  int  (* unmap_event)              (void* widget, void* event);
  int  (* property_notify_event)    (void* widget, void* event);
  int  (* selection_clear_event)    (void* widget, void* event);
  int  (* selection_request_event)  (void* widget, void* event);
  int  (* selection_notify_event)   (void* widget, void* event);
  int  (* proximity_in_event)       (void* widget, void* event);
  int  (* proximity_out_event)      (void* widget, void* event);
  int  (* visibility_notify_event)  (void* widget, void* event);
  int  (* client_event)             (void* widget, void* event);
  int  (* no_expose_event)          (void* widget, void* event);
  int  (* window_state_event)       (void* widget, void* event);
  void (* selection_get)            (void* widget, void* selection_data, uint32_t info, uint32_t time_);
  void (* selection_received)       (void* widget, void* selection_data, uint32_t time_);
  void (* drag_begin)               (void* widget, void* context);
  void (* drag_end)                 (void* widget, void* context);
  void (* drag_data_get)            (void* widget, void* context, void* selection_data, uint32_t info, uint32_t time_);
  void (* drag_data_delete)         (void* widget, void* context);
  void (* drag_leave)               (void* widget, void* context,uint32_t time_);
  int  (* drag_motion)              (void* widget, void* context, int32_t x, int32_t y, uint32_t time_);
  int  (* drag_drop)                (void* widget, void* context, int32_t x, int32_t y, uint32_t time_);
  void (* drag_data_received)       (void* widget, void* context, int32_t x, int32_t y, void* selection_data, uint32_t info, uint32_t time_);
  int  (* popup_menu)               (void* widget);
  int  (* show_help)                (void* widget, int help_type);
  void*(*get_accessible)            (void* widget);
  void (*screen_changed)            (void* widget, void* previous_screen);
  int  (*can_activate_accel)        (void* widget, uint32_t signal_id);
  int  (*grab_broken_event)         (void* widget, void* event);
  void (* composited_changed)       (void* widget);
  int  (* query_tooltip)            (void* widget, int32_t x, int32_t y, int keyboard_tooltip, void* tooltip);
  void (*_gtk_reserved5) (void);
  void (*_gtk_reserved6) (void);
  void (*_gtk_reserved7) (void);
} my_GtkWidget2Class_t;

typedef struct my_GtkWidget3_s
{
  my_GInitiallyUnowned_t  parent;
  void*                   priv;
} my_GtkWidget3_t;

typedef struct my_GtkWidget3Class_s
{
  my_GInitiallyUnownedClass_t parent_class;
  uint32_t activate_signal;
  void (*dispatch_child_properties_changed) (void* widget, uint32_t n_pspecs, void* pspecs);
  void (* destroy)             (void* widget);
  void (* show)                (void* widget);
  void (* show_all)            (void* widget);
  void (* hide)                (void* widget);
  void (* map)                 (void* widget);
  void (* unmap)               (void* widget);
  void (* realize)             (void* widget);
  void (* unrealize)           (void* widget);
  void (* size_allocate)       (void* widget, void* allocation);
  void (* state_changed)       (void* widget, int previous_state);
  void (* state_flags_changed) (void* widget, int previous_state_flags);
  void (* parent_set)          (void* widget, void* previous_parent);
  void (* hierarchy_changed)   (void* widget, void* previous_toplevel);
  void (* style_set)           (void* widget, void* previous_style);
  void (* direction_changed)   (void* widget, int previous_direction);
  void (* grab_notify)         (void* widget, int was_grabbed);
  void (* child_notify)        (void* widget, void* child_property);
  int (* draw)                 (void* widget, void* cr);
  int (* get_request_mode)     (void* widget);
  void (* get_preferred_height) (void* widget, void* minimum_height, void* natural_height);
  void (* get_preferred_width_for_height) (void* widget, int height, void* minimum_width, void* natural_width);
  void (* get_preferred_width) (void* widget, void* minimum_width, void* natural_width);
  void (* get_preferred_height_for_width) (void* widget, int width, void* minimum_height, void* natural_height);
  int (* mnemonic_activate)    (void* widget, int group_cycling);
  void (* grab_focus)          (void* widget);
  int (* focus)                (void* widget, int direction);
  void (* move_focus)          (void* widget, int direction);
  int (* keynav_failed)        (void* widget, int direction);
  int (* event)                (void* widget, void* event);
  int (* button_press_event)   (void* widget, void* event);
  int (* button_release_event) (void* widget, void* event);
  int (* scroll_event)         (void* widget, void* event);
  int (* motion_notify_event)  (void* widget, void* event);
  int (* delete_event)         (void* widget, void* event);
  int (* destroy_event)        (void* widget, void* event);
  int (* key_press_event)      (void* widget, void* event);
  int (* key_release_event)    (void* widget, void* event);
  int (* enter_notify_event)   (void* widget, void* event);
  int (* leave_notify_event)   (void* widget, void* event);
  int (* configure_event)      (void* widget, void* event);
  int (* focus_in_event)       (void* widget, void* event);
  int (* focus_out_event)      (void* widget, void* event);
  int (* map_event)            (void* widget, void* event);
  int (* unmap_event)          (void* widget, void* event);
  int (* property_notify_event) (void* widget, void* event);
  int (* selection_clear_event) (void* widget, void* event);
  int (* selection_request_event) (void* widget, void* event);
  int (* selection_notify_event) (void* widget, void* event);
  int (* proximity_in_event)   (void* widget, void* event);
  int (* proximity_out_event)  (void* widget, void* event);
  int (* visibility_notify_event) (void* widget, void* event);
  int (* window_state_event)   (void* widget, void* event);
  int (* damage_event)         (void* widget, void* event);
  int (* grab_broken_event)    (void* widget, void* event);
  void (* selection_get)       (void* widget, void* selection_data, uint32_t info, uint32_t time_);
  void (* selection_received)  (void* widget, void* selection_data, uint32_t time_);
  void (* drag_begin)          (void* widget, void* context);
  void (* drag_end)            (void* widget, void* context);
  void (* drag_data_get)       (void* widget, void* context, void* selection_data, uint32_t info, uint32_t time_);
  void (* drag_data_delete)    (void* widget, void* context);
  void (* drag_leave)          (void* widget, void* context, uint32_t time_);
  int (* drag_motion)          (void* widget, void* context, int x, int y, uint32_t time_);
  int (* drag_drop)            (void* widget, void* context, int x, int y, uint32_t time_);
  void (* drag_data_received)  (void* widget, void* context, int x, int y, void* selection_data, uint32_t info, uint32_t time_);
  int (* drag_failed)          (void* widget, void* context, int result);
  int (* popup_menu)           (void* widget);
  int (* show_help)            (void* widget, int help_type);
  void* (* get_accessible)     (void *widget);
  void (* screen_changed)      (void* widget, void* previous_screen);
  int (* can_activate_accel)   (void* widget, uint32_t signal_id);
  void (* composited_changed)  (void* widget);
  int (* query_tooltip)        (void* widget, int x, int y, int keyboard_tooltip, void* tooltip);
  void (* compute_expand)      (void* widget, void*hexpand_p, void* vexpand_p);
  void (* adjust_size_request) (void* widget, int orientation, void* minimum_size, void* natural_size);
  void (* adjust_size_allocation) (void*widget, int orientation, void* minimum_size, void* natural_size, void* allocated_pos, void* allocated_size);
  void (* style_updated)       (void* widget);
  int (* touch_event)          (void* widget, void* event);
  void (* get_preferred_height_and_baseline_for_width) (void* widget, int width, void* minimum_height, void* natural_height, void* minimum_baseline, void* natural_baseline);
  void (* adjust_baseline_request) (void* widget, void* minimum_baseline, void* natural_baseline);
  void (* adjust_baseline_allocation) (void* widget, void* baseline);
  void (*queue_draw_region)    (void* widget, void* region);
  void* priv;
  void (*_gtk_reserved6) (void);
  void (*_gtk_reserved7) (void);
} my_GtkWidget3Class_t;

typedef struct my_GtkContainer2_s
{
  my_GtkWidget2_t parent;
  void*           focus_child;  //GtkWidget
  uint32_t        border_width : 16;
  uint32_t        need_resize : 1;
  uint32_t        resize_mode : 2;
  uint32_t        reallocate_redraws : 1;
  uint32_t        has_focus_chain : 1;
} my_GtkContainer2_t;

typedef struct my_GtkContainer2Class_s 
{
  my_GtkWidget2Class_t parent_class;
  void    (*add)                (void* container, void* widget);
  void    (*remove)             (void* container, void* widget);
  void    (*check_resize)       (void* container);
  void    (*forall)             (void* container, int include_internals, void* callback, void* callback_data);
  void    (*set_focus_child)    (void* container, void* widget);
  int     (*child_type)         (void* container);
  void*   (*composite_name)     (void* container, void* child);
  void    (*set_child_property) (void* container, void* child, uint32_t property_id, void* value, void* pspec);
  void    (*get_child_property) (void* container, void* child, uint32_t property_id, void* value, void* pspec);
  void (*_gtk_reserved1) (void);
  void (*_gtk_reserved2) (void);
  void (*_gtk_reserved3) (void);
  void (*_gtk_reserved4) (void);
} my_GtkContainer2Class_t;

typedef struct my_GtkContainer3_s
{
  my_GtkWidget3_t parent;
  void*           priv;
} my_GtkContainer3_t;

typedef struct my_GtkContainer3Class_s 
{
  my_GtkWidget3Class_t parent_class;
  void    (*add)                (void* container, void* widget);
  void    (*remove)             (void* container, void* widget);
  void    (*check_resize)       (void* container);
  void    (*forall)             (void* container, int include_internals, void* callback, void* callback_data);
  void    (*set_focus_child)    (void* container, void* widget);
  int     (*child_type)         (void* container);
  void*   (*composite_name)     (void* container, void* child);
  void    (*set_child_property) (void* container, void* child, uint32_t property_id, void* value, void* pspec);
  void    (*get_child_property) (void* container, void* child, uint32_t property_id, void* value, void* pspec);
  void*   (*get_path_for_child) (void* container, void* child);
  unsigned int _handle_border_width : 1;
  void (*_gtk_reserved1) (void);
  void (*_gtk_reserved2) (void);
  void (*_gtk_reserved3) (void);
  void (*_gtk_reserved4) (void);
  void (*_gtk_reserved5) (void);
  void (*_gtk_reserved6) (void);
  void (*_gtk_reserved7) (void);
  void (*_gtk_reserved8) (void);
} my_GtkContainer3Class_t;

typedef struct my_GtkAction_s
{
  my_GObject_t parent;
  void*        private_data;
} my_GtkAction_t;

typedef struct my_GtkActionClass_s
{
  my_GObjectClass_t   parent_class;
  void     (* activate)           (void* action);
  int      menu_item_type;
  int      toolbar_item_type;
  void*    (* create_menu_item)   (void* action);
  void*    (* create_tool_item)   (void* action);
  void     (* connect_proxy)      (void* action, void* proxy);
  void     (* disconnect_proxy)   (void* action, void* proxy);
  void*    (* create_menu)        (void* action);
  void (*_gtk_reserved1) (void);
  void (*_gtk_reserved2) (void);
  void (*_gtk_reserved3) (void);
  void (*_gtk_reserved4) (void);
} my_GtkActionClass_t;

typedef struct my_GtkDrawingArea3_s
{
  my_GtkWidget3_t  parent;
  void* draw_data;
} my_GtkDrawingArea3_t;

typedef struct my_GtkDrawingArea3Class_s
{
  my_GtkWidget3Class_t parent_class;

  /* Padding for future expansion */
  void (*_gtk_reserved1) (void);
  void (*_gtk_reserved2) (void);
  void (*_gtk_reserved3) (void);
  void (*_gtk_reserved4) (void);
} my_GtkDrawingArea3Class_t;

typedef struct my_GtkMisc2_s
{
  my_GtkWidget2_t  parent;
  float           xalign;
  float           yalign;
  uint16_t        xpad;
  uint16_t        ypad;
} my_GtkMisc2_t;

typedef struct my_GtkMisc2Class_s
{
  my_GtkWidget2Class_t parent_class;
} my_GtkMisc2Class_t;

typedef struct my_GtkMisc3_s
{
  my_GtkWidget3_t parent;
  void*           priv;
} my_GtkMisc3_t;

typedef struct my_GtkMisc3Class_s
{
  my_GtkWidget3Class_t parent_class;
  void (*_gtk_reserved1) (void);
  void (*_gtk_reserved2) (void);
  void (*_gtk_reserved3) (void);
  void (*_gtk_reserved4) (void);
} my_GtkMisc3Class_t;

typedef struct my_GtkImage3_s
{
  my_GtkMisc3_t parent;
  void*         priv;
} my_GtkImage3_t;

typedef struct my_GtkImageClass_s
{
  my_GtkMisc3Class_t parent_class;
  void (*_gtk_reserved1) (void);
  void (*_gtk_reserved2) (void);
  void (*_gtk_reserved3) (void);
  void (*_gtk_reserved4) (void);
} my_GtkImage3Class_t;

typedef struct my_GtkLabel2_s
{
  my_GtkMisc2_t misc;
  char*         label;
  uint32_t      jtype            : 2;
  uint32_t      wrap             : 1;
  uint32_t      use_underline    : 1;
  uint32_t      use_markup       : 1;
  uint32_t      ellipsize        : 3;
  uint32_t      single_line_mode : 1;
  uint32_t      have_transform   : 1;
  uint32_t      in_click         : 1;
  uint32_t      wrap_mode        : 3;
  uint32_t      pattern_set      : 1;
  uint32_t      track_links      : 1;
  uint32_t      mnemonic_keyval;
  char*         text;
  void*         attrs;  //PangoAttrList
  void*         effective_attrs;  //PangoAttrList
  void*         layout; //PangoLayout
  void*         mnemonic_widget;  //GtkWidget
  void*         mnemonic_window;  //GtkWindow
  void*         select_info;  //GtkLabelSelectionInfo
} my_GtkLabel2_t;

typedef struct my_GtkLabel2Class_s
{
  my_GtkMisc2Class_t parent_class;
  void (* move_cursor)     (void* label, int step, int count, int extend_selection);
  void (* copy_clipboard)  (void* label);
  void (* populate_popup)  (void* label, void* menu);
  int  (*activate_link)    (void* label, void* uri);
  void (*_gtk_reserved1) (void);
  void (*_gtk_reserved2) (void);
  void (*_gtk_reserved3) (void);
} my_GtkLabel2Class_t;

typedef struct my_GtkLabel3_s
{
  my_GtkMisc3_t misc;
  void*         priv;
} my_GtkLabel3_t;

typedef struct my_GtkLabel3Class_s
{
  my_GtkMisc3Class_t parent_class;
  void (* move_cursor)     (void* label, int step, int count, int extend_selection);
  void (* copy_clipboard)  (void* label);
  void (* populate_popup)  (void* label, void* menu);
  int  (*activate_link)    (void* label, void* uri);
  void (*_gtk_reserved1) (void);
  void (*_gtk_reserved2) (void);
  void (*_gtk_reserved3) (void);
  void (*_gtk_reserved4) (void);
  void (*_gtk_reserved5) (void);
  void (*_gtk_reserved6) (void);
  void (*_gtk_reserved7) (void);
  void (*_gtk_reserved8) (void);
} my_GtkLabel3Class_t;

typedef struct my_GtkTreeView2_s
{
  my_GtkContainer2_t  parent;
  void*               priv;
} my_GtkTreeView2_t;

typedef struct my_GtkTreeView2Class_s
{
  my_GtkContainer2Class_t parent_class;
  void (* set_scroll_adjustments)     (void* tree_view, void* hadjustment, void* vadjustment);
  void (* row_activated)              (void* tree_view, void* path, void* column);
  int  (* test_expand_row)            (void* tree_view, void* iter, void* path);
  int  (* test_collapse_row)          (void* tree_view, void* iter, void* path);
  void (* row_expanded)               (void* tree_view, void* iter, void* path);
  void (* row_collapsed)              (void* tree_view, void* iter, void* path);
  void (* columns_changed)            (void* tree_view);
  void (* cursor_changed)             (void* tree_view);
  int  (* move_cursor)                (void* tree_view, int step, int count);
  int  (* select_all)                 (void* tree_view);
  int  (* unselect_all)               (void* tree_view);
  int  (* select_cursor_row)          (void* tree_view, int start_editing);
  int  (* toggle_cursor_row)          (void* tree_view);
  int  (* expand_collapse_cursor_row) (void* tree_view, int logical, int expand, int open_all);
  int  (* select_cursor_parent)       (void* tree_view);
  int  (* start_interactive_search)   (void* tree_view);
  void (*_gtk_reserved0) (void);
  void (*_gtk_reserved1) (void);
  void (*_gtk_reserved2) (void);
  void (*_gtk_reserved3) (void);
  void (*_gtk_reserved4) (void);
} my_GtkTreeView2Class_t;

typedef struct my_GtkBin2_s
{
  my_GtkContainer2_t  container;
  void*               child;  //GtkWidget
} my_GtkBin2_t;

typedef struct my_GtkBin2Class_s
{
  my_GtkContainer2Class_t parent_class;
} my_GtkBin2Class_t;

typedef struct my_GtkBin3_s
{
  my_GtkContainer3_t  container;
  void*               priv;
} my_GtkBin3_t;

typedef struct my_GtkBin3Class_s
{
  my_GtkContainer3Class_t parent_class;
  void (*_gtk_reserved1) (void);
  void (*_gtk_reserved2) (void);
  void (*_gtk_reserved3) (void);
  void (*_gtk_reserved4) (void);
} my_GtkBin3Class_t;

typedef struct my_GtkWindow2_s
{
  my_GtkBin2_t  parent;
  char*         title;
  char*         wmclass_name;
  char*         wmclass_class;
  char*         wm_role;
  void*         focus_widget; // GtkWidget
  void*         default_widget; // GtkWidget
  void*         transient_parent; // GtkWindow
  void*         geometry_info;  // GtkWindowGeometryInfo
  void*         frame;  // GdkWindow
  void*         group;  // GtkWindowGroup
  uint16_t      configure_request_count;
  uint32_t      allow_shrink : 1;
  uint32_t      allow_grow : 1;
  uint32_t      configure_notify_received : 1;
  uint32_t      need_default_position : 1;
  uint32_t      need_default_size : 1;
  uint32_t      position : 3;
  uint32_t      type : 4;
  uint32_t      has_user_ref_count : 1;
  uint32_t      has_focus : 1;
  uint32_t      modal : 1;
  uint32_t      destroy_with_parent : 1;
  uint32_t      has_frame : 1;
  uint32_t      iconify_initially : 1;
  uint32_t      stick_initially : 1;
  uint32_t      maximize_initially : 1;
  uint32_t      decorated : 1;
  uint32_t      type_hint : 3;
  uint32_t      gravity : 5;
  uint32_t      is_active : 1;
  uint32_t      has_toplevel_focus : 1;
  uint32_t      frame_left;
  uint32_t      frame_top;
  uint32_t      frame_right;
  uint32_t      frame_bottom;
  uint32_t      keys_changed_handler;
  int           mnemonic_modifier;
  void*         screen; // GdkScreen
} my_GtkWindow2_t;

typedef struct my_GtkWindow2Class_s {
  my_GtkBin2Class_t parent_class;
  void (* set_focus)    (void* window, void* focus);
  int  (* frame_event)  (void* window, void* event);
  void (* activate_focus)          (void* window);
  void (* activate_default)        (void* window);
  void (* move_focus)              (void* window, int direction);
  void (*keys_changed)             (void* window);
  void (*_gtk_reserved1) (void);
  void (*_gtk_reserved2) (void);
  void (*_gtk_reserved3) (void);
  void (*_gtk_reserved4) (void);
} my_GtkWindow2Class_t;

typedef struct my_GtkWindow3_s
{
  my_GtkBin3_t  bin;
  void*         priv;
} my_GtkWindow3_t;

typedef struct my_GtkWindow3Class_s
{
  my_GtkBin3Class_t parent_class;
  void (* set_focus) (void* window, void* focus);
  void (* activate_focus) (void* window);
  void (* activate_default) (void* window);
  void (* keys_changed) (void* window);
  int (* enable_debugging) (void* window, int toggle);
  void (* _gtk_reserved1) (void);
  void (* _gtk_reserved2) (void);
  void (* _gtk_reserved3) (void);
}my_GtkWindow3Class_t;

typedef struct my_GtkApplicationWindow_s
{
  my_GtkWindow3_t parent;
  void*           priv;
} my_GtkApplicationWindow_t;

typedef struct my_GtkApplicationWindowClass_s
{
  my_GtkWindow3Class_t parent_class;
}my_GtkApplicationWindowClass_t;

typedef struct my_GtkListBox_s
{
  my_GtkContainer3_t parent;
} my_GtkListBox_t;

typedef struct my_GtkListBoxClass_s
{
  my_GtkContainer3Class_t parent_class;
  void (*row_selected)  (void *box, void *row);
  void (*row_activated) (void *box, void *row);
  void (*activate_cursor_row) (void *box);
  void (*toggle_cursor_row) (void *box);
  void (*move_cursor) (void *box, int step, int count);
  void (*selected_rows_changed) (void *box);
  void (*select_all)            (void *box);
  void (*unselect_all)          (void *box);
  void (* _gtk_reserved1) (void);
  void (* _gtk_reserved2) (void);
  void (* _gtk_reserved3) (void);
}my_GtkListBoxClass_t;

typedef struct my_GtkListBoxRow_s
{
  my_GtkBin3_t parent;
} my_GtkListBoxRow_t;

typedef struct my_GtkListBoxRowClass_s {
  my_GtkBin3Class_t parent_class;
  void (* activate) (void* row);
  void (* _gtk_reserved1) (void);
  void (* _gtk_reserved2) (void);
}my_GtkListBoxRowClass_t;

typedef struct my_GtkButton2_s
{
  my_GtkBin2_t  bin;
  void*         event_window; // GdkWindow
  char*         label_text;
  uint32_t      activate_timeout;
  uint32_t      constructed : 1;
  uint32_t      in_button : 1;
  uint32_t      button_down : 1;
  uint32_t      relief : 2;
  uint32_t      use_underline : 1;
  uint32_t      use_stock : 1;
  uint32_t      depressed : 1;
  uint32_t      depress_on_activate : 1;
  uint32_t      focus_on_click : 1;
} my_GtkButton2_t;

typedef struct my_GtkButton2Class_s {
  my_GtkBin2Class_t parent_class;
  void (* pressed)  (void *button);
  void (* released) (void *button);
  void (* clicked)  (void *button);
  void (* enter)    (void *button);
  void (* leave)    (void *button);
  void (* activate) (void *button);
  void (*_gtk_reserved1) (void);
  void (*_gtk_reserved2) (void);
  void (*_gtk_reserved3) (void);
  void (*_gtk_reserved4) (void);
} my_GtkButton2Class_t;

typedef struct my_GtkButton3_s
{
  my_GtkBin3_t  bin;
  void*         priv;
} my_GtkButton3_t;

typedef struct my_GtkButton3Class_s {
  my_GtkBin3Class_t parent_class;
  void (* pressed)  (void *button);
  void (* released) (void *button);
  void (* clicked)  (void *button);
  void (* enter)    (void *button);
  void (* leave)    (void *button);
  void (* activate) (void *button);
  void (*_gtk_reserved1) (void);
  void (*_gtk_reserved2) (void);
  void (*_gtk_reserved3) (void);
  void (*_gtk_reserved4) (void);
} my_GtkButton3Class_t;

typedef struct my_GtkToggleButton2_s
{
  my_GtkButton2_t button;
  uint32_t        active : 1;
  uint32_t        draw_indicator : 1;
  uint32_t        inconsistent : 1;
} my_GtkToggleButton2_t;

typedef struct my_GtkToggleButton2Class_s {
  my_GtkButton2Class_t parent_class;
  void (* toggled) (void* toggle_button);
  void (*_gtk_reserved1) (void);
  void (*_gtk_reserved2) (void);
  void (*_gtk_reserved3) (void);
  void (*_gtk_reserved4) (void);
} my_GtkToggleButton2Class_t;

typedef struct my_GtkToggleButton3_s
{
  my_GtkButton3_t parent;
  void*           priv;
} my_GtkToggleButton3_t;

typedef struct my_GtkToggleButton3Class_s
{
  my_GtkButton3Class_t parent_class;
  void (* toggled) (void* toggle_button);
  void (*_gtk_reserved1) (void);
  void (*_gtk_reserved2) (void);
  void (*_gtk_reserved3) (void);
  void (*_gtk_reserved4) (void);
} my_GtkToggleButton3Class_t;

typedef struct my_GtkCheckButton2_s
{
  my_GtkToggleButton2_t parent;
} my_GtkCheckButton2_t;

typedef struct my_GtkCheckButton2Class_s {
  my_GtkToggleButton2Class_t parent_class;
  void (* draw_indicator) (void* check_button, void* area);
  void (*_gtk_reserved0) (void);
  void (*_gtk_reserved1) (void);
  void (*_gtk_reserved2) (void);
} my_GtkCheckButton2Class_t;

typedef struct my_GtkCheckButton3_s
{
  my_GtkToggleButton3_t parent;
} my_GtkCheckButton3_t;

typedef struct my_GtkCheckButton3Class_s {
  my_GtkToggleButton3Class_t parent_class;
  void (* draw_indicator) (void* check_button, void* area);
  void (*_gtk_reserved0) (void);
  void (*_gtk_reserved1) (void);
  void (*_gtk_reserved2) (void);
} my_GtkCheckButton3Class_t;

typedef struct my_GtkMenuButton3_s
{
  my_GtkToggleButton3_t parent;
  void*                 priv;
} my_GtkMenuButton3_t;

typedef struct my_GtkMenuButton3Class_s
{
  my_GtkToggleButton3Class_t parent_class;
  void (*_gtk_reserved1) (void);
  void (*_gtk_reserved2) (void);
  void (*_gtk_reserved3) (void);
  void (*_gtk_reserved4) (void);
}my_GtkMenuButton3Class_t;

typedef struct my_GtkComboBox2_s
{
  my_GtkBin2_t    parent;
  void*           priv;
} my_GtkComboBox2_t;

typedef struct my_GtkComboBox2Class_s {
  my_GtkBin2Class_t parent_class;
  void (* changed)        (void* combo_box);
  void*(* get_active_text)(void* combo_box);
  void (*_gtk_reserved1) (void);
  void (*_gtk_reserved2) (void);
  void (*_gtk_reserved3) (void);
  void (*_gtk_reserved4) (void);
} my_GtkComboBox2Class_t;

typedef struct my_GtkEntry2_s
{
  my_GtkWidget2_t   parent;
  char*             text;
  uint32_t          editable : 1;
  uint32_t          visible  : 1;
  uint32_t          overwrite_mode : 1;
  uint32_t          in_drag : 1;
  uint16_t          text_length;
  uint16_t          text_max_length;
  void*             text_area;  // GdkWindow
  void*             im_context; // GtkIMContext
  void*             popup_menu; // GtkWidget
  int               current_pos;
  int               selection_bound;
  void*             cached_layout;  // PangoLayout
  uint32_t          cache_includes_preedit : 1;
  uint32_t          need_im_reset          : 1;
  uint32_t          has_frame              : 1;
  uint32_t          activates_default      : 1;
  uint32_t          cursor_visible         : 1;
  uint32_t          in_click               : 1;
  uint32_t          is_cell_renderer       : 1;
  uint32_t          editing_canceled       : 1;
  uint32_t          mouse_cursor_obscured  : 1;
  uint32_t          select_words           : 1;
  uint32_t          select_lines           : 1;
  uint32_t          resolved_dir           : 4;
  uint32_t          truncate_multiline     : 1;
  uint32_t          button;
  uint32_t          blink_timeout;
  uint32_t          recompute_idle;
  int               scroll_offset;
  int               ascent;
  int               descent;
  uint16_t          x_text_size;
  uint16_t          x_n_bytes;
  uint16_t          preedit_length;
  uint16_t          preedit_cursor;
  int               dnd_position;
  int               drag_start_x;
  int               drag_start_y;
  uint32_t          invisible_char;
  int               width_chars;
} my_GtkEntry2_t;

typedef struct my_GtkEntry2Class_s {
  my_GtkWidget2Class_t parent_class;
  void (* populate_popup)   (void* entry, void* menu);
  void (* activate)           (void* entry);
  void (* move_cursor)        (void* entry, int step, int count, int extend_selection);
  void (* insert_at_cursor)   (void* entry, void* str);
  void (* delete_from_cursor) (void* entry, int type, int count);
  void (* backspace)          (void* entry);
  void (* cut_clipboard)      (void* entry);
  void (* copy_clipboard)     (void* entry);
  void (* paste_clipboard)    (void* entry);
  void (* toggle_overwrite)   (void* entry);
  void (* get_text_area_size) (void* entry, void* x, void* y, void* width, void* height);
  void (*_gtk_reserved1) (void);
  void (*_gtk_reserved2) (void);
} my_GtkEntry2Class_t;

typedef struct my_GtkSpinButton2_s
{
  my_GtkEntry2_t  entry;
  void*           adjustment; // GtkAdjustment
  void*           panel;  // GdkWindow
  uint32_t        timer;
  double          climb_rate;
  double          timer_step;
  int             update_policy;
  uint32_t        in_child : 2;
  uint32_t        click_child : 2;
  uint32_t        button : 2;
  uint32_t        need_timer : 1;
  uint32_t        timer_calls : 3;
  uint32_t        digits : 10;
  uint32_t        numeric : 1;
  uint32_t        wrap : 1;
  uint32_t        snap_to_ticks : 1;
} my_GtkSpinButton2_t;

typedef struct my_GtkSpinButton2Class_s {
  my_GtkEntry2Class_t parent_class;
  int  (*input)  (void* spin_button, void* new_value);
  int  (*output) (void* spin_button);
  void (*value_changed) (void* spin_button);
  void (*change_value) (void* spin_button, int scroll);
  void (*wrapped) (void* spin_button);
  void (*_gtk_reserved1) (void);
  void (*_gtk_reserved2) (void);
  void (*_gtk_reserved3) (void);
} my_GtkSpinButton2Class_t;

typedef struct my_GtkProgress2_s
{
  my_GtkWidget2_t   widget;
  void*             adjustment; // GtkAdjustment
  void*             offscreen_pixmap; // GdkPixmap
  char*             format;
  float             x_align;
  float             y_align;
  uint32_t          show_text : 1;
  uint32_t          activity_mode : 1;
  uint32_t          use_text_format : 1;
} my_GtkProgress2_t;

typedef struct my_GtkProgress2Class_s {
  my_GtkWidget2Class_t parent;
  void (* paint)            (void* progress);
  void (* update)           (void* progress);
  void (* act_mode_enter)   (void* progress);
  void (*_gtk_reserved1) (void);
  void (*_gtk_reserved2) (void);
  void (*_gtk_reserved3) (void);
  void (*_gtk_reserved4) (void);
} my_GtkProgress2Class_t;

typedef struct my_GtkProgressBar2_s
{
  my_GtkProgress2_t parent;
  int               bar_style;
  int               orientation;
  uint32_t          blocks;
  int               in_block;
  int               activity_pos;
  uint32_t          activity_step;
  uint32_t          activity_blocks;
  double            pulse_fraction;
  uint32_t          activity_dir : 1;
  uint32_t          ellipsize : 3;
  uint32_t          dirty : 1;
} my_GtkProgressBar2_t;

typedef struct my_GtkProgressBar2Class_s {
  my_GtkProgress2Class_t parent_class;
  void (*_gtk_reserved1) (void);
  void (*_gtk_reserved2) (void);
  void (*_gtk_reserved3) (void);
  void (*_gtk_reserved4) (void);
} my_GtkProgressBar2Class_t;

typedef struct my_GtkFrame2_s
{
  my_GtkBin2_t      bin;
  void*             label_widget; // GtkWidget
  int16_t           shadow_type;
  float             label_xalign;
  float             label_yalign;
  my_GdkRectangle_t child_allocation;
} my_GtkFrame2_t;

typedef struct my_GtkFrame2Class_s {
  my_GtkBin2Class_t parent_class;
  void (*compute_child_allocation) (void* frame, void* allocation);
} my_GtkFrame2Class_t;

typedef struct my_GtkMenuShell2_s
{
  my_GtkContainer2_t  container;
  void*               children; // GList
  void*               active_menu_item; // GtkWidget
  void*               parent_menu_shell;  // GtkWidget
  uint32_t            button;
  uint32_t            activate_time;
  uint32_t            active : 1;
  uint32_t            have_grab : 1;
  uint32_t            have_xgrab : 1;
  uint32_t            ignore_leave : 1;
  uint32_t            menu_flag : 1;
  uint32_t            ignore_enter : 1;
  uint32_t            keyboard_mode : 1;
} my_GtkMenuShell2_t;

typedef struct my_GtkMenuShell2Class_s {
  my_GtkContainer2Class_t parent_class;
  void (*deactivate)       (void* menu_shell);
  void (*selection_done)   (void* menu_shell);
  void (*move_current)     (void* menu_shell, int direction);
  void (*activate_current) (void* menu_shell, int force_hide);
  void (*cancel)           (void* menu_shell);
  void (*select_item)      (void* menu_shell, void* menu_item);
  void (*insert)           (void* menu_shell, void* child, int position);
  int  (*get_popup_delay)  (void* menu_shell);
  int  (*move_selected)    (void* menu_shell, int distance);
  void (*_gtk_reserved1) (void);
  void (*_gtk_reserved2) (void);
} my_GtkMenuShell2Class_t;

typedef struct my_GtkMenuBar2_s
{
  my_GtkMenuShell2_t  parent;
} my_GtkMenuBar2_t;

typedef struct my_GtkMenuBar2Class_s {
  my_GtkMenuShell2Class_t parent_class;
  void (*_gtk_reserved1) (void);
  void (*_gtk_reserved2) (void);
  void (*_gtk_reserved3) (void);
  void (*_gtk_reserved4) (void);
} my_GtkMenuBar2Class_t;

typedef struct my_GtkTextView2_s
{
  my_GtkContainer2_t    parent;
  void*                 layout; // struct _GtkTextLayout
  void*                 buffer; // GtkTextBuffer
  uint32_t              selection_drag_handler;
  uint32_t              scroll_timeout;
  int                   pixels_above_lines;
  int                   pixels_below_lines;
  int                   pixels_inside_wrap;
  int                   wrap_mode;
  int                   justify;
  int                   left_margin;
  int                   right_margin;
  int                   indent;
  void*                 tabs; // PangoTabArray
  uint32_t              editable : 1;
  uint32_t              overwrite_mode : 1;
  uint32_t              cursor_visible : 1;
  uint32_t              need_im_reset : 1;
  uint32_t              accepts_tab : 1;
  uint32_t              width_changed : 1;
  uint32_t              onscreen_validated : 1;
  uint32_t              mouse_cursor_obscured : 1;
  void*                 text_window;  // GtkTextWindow
  void*                 left_window;  // GtkTextWindow
  void*                 right_window; // GtkTextWindow
  void*                 top_window; // GtkTextWindow
  void*                 bottom_window;  // GtkTextWindow
  void*                 hadjustment;  // GtkAdjustment
  void*                 vadjustment;  // GtkAdjustment
  int                   xoffset;
  int                   yoffset;
  int                   width;
  int                   height;
  int                   virtual_cursor_x;
  int                   virtual_cursor_y;
  void*                 first_para_mark;  // GtkTextMark
  int                   first_para_pixels;
  void*                 dnd_mark; // GtkTextMark
  uint32_t              blink_timeout;
  uint32_t              first_validate_idle;
  uint32_t              incremental_validate_idle;
  void*                 im_context; // GtkIMContext
  void*                 popup_menu; // GtkWidget
  int                   drag_start_x;
  int                   drag_start_y;
  void*                 children; // GSList
  void*                 pending_scroll; // GtkTextPendingScroll
  int                   pending_place_cursor_button;
} my_GtkTextView2_t;

typedef struct my_GtkTextView2Class_s {
  my_GtkContainer2Class_t parent_class;
  void (* set_scroll_adjustments)   (void* text_view, void* hadjustment, void* vadjustment);
  void (* populate_popup)           (void* text_view, void* menu);
  void (* move_cursor)              (void* text_view, int step, int count, int extend_selection);
  void (* page_horizontally)        (void* text_view, int count, int extend_selection);
  void (* set_anchor)               (void* text_view);
  void (* insert_at_cursor)         (void* text_view, void* str);
  void (* delete_from_cursor)       (void* text_view, int type, int count);
  void (* backspace)                (void* text_view);
  void (* cut_clipboard)            (void* text_view);
  void (* copy_clipboard)           (void* text_view);
  void (* paste_clipboard)          (void* text_view);
  void (* toggle_overwrite)         (void* text_view);
  void (* move_focus)               (void* text_view, int direction);
  void (*_gtk_reserved1) (void);
  void (*_gtk_reserved2) (void);
  void (*_gtk_reserved3) (void);
  void (*_gtk_reserved4) (void);
  void (*_gtk_reserved5) (void);
  void (*_gtk_reserved6) (void);
  void (*_gtk_reserved7) (void);
} my_GtkTextView2Class_t;

typedef struct my_GtkTextView3_s
{
  my_GtkContainer3_t  parent;
  void*               priv;
} my_GtkTextView3_t;

typedef struct my_GtkTextView3Class_s {
  my_GtkContainer3Class_t parent_class;
  void (* populate_popup)           (void* text_view, void* menu);
  void (* move_cursor)              (void* text_view, int step, int count, int extend_selection);
  void (* set_anchor)               (void* text_view);
  void (* insert_at_cursor)         (void* text_view, void* str);
  void (* delete_from_cursor)       (void* text_view, int type, int count);
  void (* backspace)                (void* text_view);
  void (* cut_clipboard)            (void* text_view);
  void (* copy_clipboard)           (void* text_view);
  void (* paste_clipboard)          (void* text_view);
  void (* toggle_overwrite)         (void* text_view);
  void*(* create_buffer)            (void* text_view);
  void (* draw_layer)               (void* text_view, int layer, void* cr);
  int  (* extend_selection)         (void* text_view, int granularity, void* location, void* start, void* end);
  void (* insert_emoji)             (void* text_view);
  void (*_gtk_reserved1) (void);
  void (*_gtk_reserved2) (void);
  void (*_gtk_reserved3) (void);
  void (*_gtk_reserved4) (void);
} my_GtkTextView3Class_t;

typedef struct my_GtkGrid3_s
{
  my_GtkContainer3_t  parent;
  void*               priv;
} my_GtkGrid3_t;

typedef struct my_GtkGrid3Class_s {
  my_GtkContainer3Class_t parent_class;
  void (* _gtk_reserved1) (void);
  void (* _gtk_reserved2) (void);
  void (* _gtk_reserved3) (void);
  void (* _gtk_reserved4) (void);
  void (* _gtk_reserved5) (void);
  void (* _gtk_reserved6) (void);
  void (* _gtk_reserved7) (void);
  void (* _gtk_reserved8) (void);
}my_GtkGrid3Class_t;

typedef struct my_GtkEventController_s
{
  my_GObject_t parent;
} my_GtkEventController_t;

typedef struct my_GtkEventControllerClass_s {
  my_GObjectClass_t parent_class;
  void (* set_widget)   (void *controller, void *widget);
  void (* unset_widget) (void *controller);
  int (* handle_event) (void *controller, void *event, double x, double y);
  void (* reset) (void *controller);
  void (* handle_crossing) (void *controller, void *crossing, double x, double y);
  int (* filter_event) (void *controller, void *event);
  void* padding[10];
}my_GtkEventControllerClass_t;

typedef struct my_GtkGesture_s
{
  my_GtkEventController_t parent;
} my_GtkGesture_t;

typedef struct my_GtkGestureClass_s {
  my_GtkEventControllerClass_t parent_class;
  int (* check)  (void *gesture);
  void (* begin)  (void *gesture, void *sequence);
  void (* update) (void *gesture, void *sequence);
  void (* end) (void *gesture, void *sequence);
  void (* cancel) (void *gesture, void *sequence);
  void (* sequence_state_changed) (void *gesture, void *sequence, int state);
  void* padding[10];
}my_GtkGestureClass_t;

typedef struct my_GtkGestureSingle_s
{
  my_GtkGesture_t parent;
} my_GtkGestureSingle_t;

typedef struct my_GtkGestureSingleClass_s {
  my_GtkGestureClass_t parent_class;
  void* padding[10];
}my_GtkGestureSingleClass_t;

typedef struct my_GtkGestureLongPress_s
{
  my_GtkGestureSingle_t parent;
} my_GtkGestureLongPress_t;

typedef struct my_GtkGestureLongPressClass_s {
  my_GtkGestureSingleClass_t parent_class;
  void (* pressed) (void *gesture, double x, double y);
  void (* cancelled) (void *gesture);
  /*< private >*/
  void* padding[10];
}my_GtkGestureLongPressClass_t;

typedef struct my_MetaFrames2_s
{
  my_GtkWindow2_t parent; // to be checked (is MetaFrame2 still usefull?)
} my_MetaFrames2_t;

typedef struct my_MetaFrames2Class_s
{
  my_GtkWindow2Class_t parent_class;

} my_MetaFrames2Class_t;

typedef struct my_GtkTable2_s
{
  my_GtkContainer2_t  container;
  void*               children; // GList
  void*               rows; // GtkTableRowCol
  void*               cols; // GtkTableRowCol
  uint16_t            nrows;
  uint16_t            ncols;
  uint16_t            column_spacing;
  uint16_t            row_spacing;
  uint32_t            homogeneous : 1;
} my_GtkTable2_t;

typedef struct my_GtkTable2Class_s
{
  my_GtkContainer2Class_t parent_class;
} my_GtkTable2Class_t;

typedef struct my_GtkFixed2_s
{
  my_GtkContainer2_t  parent;
  void*               children; // GList
} my_GtkFixed2_t;

typedef struct my_GtkFixed2Class_s
{
  my_GtkContainer2Class_t parent_class;
} my_GtkFixed2Class_t;

typedef struct my_GtkFixed3_s
{
  my_GtkContainer3_t      parent;
} my_GtkFixed3_t;

typedef struct my_GtkFixed3Class_s
{
  my_GtkContainer3Class_t parent_class;
  void*                   padding[8];
} my_GtkFixed3Class_t;

typedef struct my_GtkNotebook2_s
{
  my_GtkContainer2_t parent;
  void* cur_page;
  void* children;
  void* first_tab;
  void* focus_tab;
  void* menu;
  void* event_window;
  uint32_t timer;
  uint16_t tab_hborder;
  uint16_t tab_vborder;
  uint32_t show_tabs          : 1;
  uint32_t homogeneous        : 1;
  uint32_t show_border        : 1;
  uint32_t tab_pos            : 2;
  uint32_t scrollable         : 1;
  uint32_t in_child           : 3;
  uint32_t click_child        : 3;
  uint32_t button             : 2;
  uint32_t need_timer         : 1;
  uint32_t child_has_focus    : 1;
  uint32_t have_visible_child : 1;
  uint32_t focus_out          : 1;
  uint32_t has_before_previous: 1;
  uint32_t has_before_next    : 1;
  uint32_t has_after_previous : 1;
  uint32_t has_after_next     : 1;
} my_GtkNotebook2_t;

typedef struct my_GtkNotebook2Class_s
{
  my_GtkContainer2Class_t     parent_class;
  void (* switch_page)        (void* notebook, void* page, uint32_t page_num);
  int  (* select_page)        (void* notebook, int move_focus);
  int  (* focus_tab)          (void* notebook, int type);
  int  (* change_current_page)(void* notebook, int offset);
  void (* move_focus_out)     (void* notebook, int direction);
  int  (* reorder_tab)        (void* notebook, int direction, int move_to_last);
  int  (* insert_page)        (void* notebook, void* child, void* tab_label, void* menu_label, int position);
  void*(* create_window)      (void* notebook, void* page, int x, int y);
  void (*_gtk_reserved1)      (void);
} my_GtkNotebook2Class_t;

typedef struct my_GtkCellRenderer2_s
{
  my_GtkObject_t parent;
  float xalign;
  float yalign;
  int width;
  int height;
  uint16_t xpad;
  uint16_t ypad;
  uint32_t mode : 2;
  uint32_t visible : 1;
  uint32_t is_expander : 1;
  uint32_t is_expanded : 1;
  uint32_t cell_background_set : 1;
  uint32_t sensitive : 1;
  uint32_t editing : 1;
} my_GtkCellRenderer2_t;

typedef struct my_GtkCellRenderer2Class_s
{
  my_GtkObjectClass_t parent_class;
  void   (* get_size)         (void* cell, void* widget, void* cell_area, int* x_offset, int* y_offset, int* width, int* height);
  void   (* render)           (void* cell, void* window, void* widget, void* background_area, void* cell_area, void* expose_area, int flags);
  int    (* activate)         (void* cell, void* event, void* widget, void* path, void* background_area, void* cell_area, int flags);
  void*  (* start_editing)    (void* cell, void* event, void* widget, void* path, void* background_area, void* cell_area, int flags);
  void   (* editing_canceled) (void* cell);
  void   (* editing_started)  (void* cell, void* editable, void* path);
  void (*_gtk_reserved1) (void);
  void (*_gtk_reserved2) (void);
} my_GtkCellRenderer2Class_t;

typedef struct my_PangoColor_s
{
  uint16_t red;
  uint16_t green;
  uint16_t blue;
} my_PangoColor_t;

typedef struct my_GtkCellRendererText2_s
{
  my_GtkCellRenderer2_t parent;
  void* text;
  void* font;
  double font_scale;
  my_PangoColor_t foreground;
  my_PangoColor_t background;
  void* extra_attrs;
  int underline_style;
  int rise;
  int fixed_height_rows;
  uint32_t strikethrough : 1;
  uint32_t editable  : 1;
  uint32_t scale_set : 1;
  uint32_t foreground_set : 1;
  uint32_t background_set : 1;
  uint32_t underline_set : 1;
  uint32_t rise_set : 1;
  uint32_t strikethrough_set : 1;
  uint32_t editable_set : 1;
  uint32_t calc_fixed_height : 1;
} my_GtkCellRendererText2_t;

typedef struct my_GtkCellRendererText2Class_s
{
  my_GtkCellRenderer2Class_t parent_class;
  void (* edited) (void* cell_renderer_text, void* path, void* new_text);
  void (*_gtk_reserved1) (void);
  void (*_gtk_reserved2) (void);
  void (*_gtk_reserved3) (void);
  void (*_gtk_reserved4) (void);
} my_GtkCellRendererText2Class_t;

typedef struct my_GDBusObjectManagerClient_s
{
  my_GObject_t  parent;
  void*         priv;
} my_GDBusObjectManagerClient_t;

typedef struct my_GDBusObjectManagerClientClass_s
{
  my_GObjectClass_t parent_class;
  void    (*interface_proxy_signal)             (void* manager, void* object_proxy, void* interface_proxy, void* sender_name, void* signal_name, void* parameters);
  void    (*interface_proxy_properties_changed) (void* manager, void* object_proxy, void* interface_proxy, void* changed_properties, void* invalidated_properties);
  void* padding[8];
} my_GDBusObjectManagerClientClass_t;

typedef struct my_AtkObject_s
{
  my_GObject_t  parent;
  char*         description;
  char*         name;
  void*         accessible_parent;  // AtkObject
  int           role;
  void*         relation_set; // AtkRelationSet
  int           layer;
} my_AtkObject_t;

typedef struct my_AtkObjectClass_s
{
  my_GObjectClass_t parent;
  void* (* get_name)            (void* accessible);
  void* (* get_description)     (void* accessible);
  void* (*get_parent)           (void* accessible);
  int   (* get_n_children)      (void* accessible);
  void* (* ref_child)           (void* accessible, int i);
  int   (* get_index_in_parent) (void* accessible);
  void* (* ref_relation_set)    (void* accessible);
  int   (* get_role)            (void* accessible);
  int   (* get_layer)           (void* accessible);
  int   (* get_mdi_zorder)      (void* accessible);
  void* (* ref_state_set)       (void* accessible);
  void  (* set_name)            (void* accessible, void* name);
  void  (* set_description)     (void* accessible, void* description);
  void  (* set_parent)          (void* accessible, void* parent);
  void  (* set_role)            (void* accessible, int role);
  uint32_t (* connect_property_change_handler)    (void* accessible, void* handler);
  void  (* remove_property_change_handler)     (void* accessible, uint32_t handler_id);
  void  (* initialize)          (void* accessible, void* data);
  void  (* children_changed)    (void* accessible, uint32_t change_index, void* changed_child);
  void  (* focus_event)         (void* accessible, int focus_in);
  void  (* property_change)     (void* accessible, void* values);
  void  (* state_change)        (void* accessible, void* name, int state_set);
  void  (*visible_data_changed) (void* accessible);
  void  (*active_descendant_changed) (void* accessible, void* child);
  void* (*get_attributes)       (void* accessible);
  void* (*get_object_locale)    (void* accessible);
  void*  pad1;
} my_AtkObjectClass_t;

typedef struct my_AtkUtil_s
{
  my_GObject_t parent;
} my_AtkUtil_t;

typedef struct my_AtkUtilClass_s
{
   my_GObjectClass_t parent;
   uint32_t (* add_global_event_listener)    (void* listener, void* event_type);
   void     (* remove_global_event_listener) (uint32_t listener_id);
   uint32_t (* add_key_event_listener)       (void* listener, void* data);
   void     (* remove_key_event_listener)    (uint32_t listener_id);
   void*    (* get_root)                     (void);
   void*    (* get_toolkit_name)             (void);
   void*    (* get_toolkit_version)          (void);
} my_AtkUtilClass_t;

typedef union my_GMutex_s
{
  void*     p;
  uint32_t  i[2];
} my_GMutex_t;

typedef struct my_GstObject_s
{
  my_GInitiallyUnowned_t  parent;
  my_GMutex_t             lock;
  char*                   name;
  void*                   _parent;  // GstObject
  uint32_t                flags;
  void*                   control_bindings; // GList
  uint64_t                control_rate;
  uint64_t                last_sync;
  void*                   _gst_reserved;
} my_GstObject_t;

typedef struct my_GstObjectClass_s {
  my_GInitiallyUnownedClass_t parent;
  const char*   path_string_separator;
  void          (*deep_notify)      (void* object, void* orig, void* pspec);
  void*        _gst_reserved[4];
} my_GstObjectClass_t;

typedef struct my_GstAllocator_s
{
  my_GstObject_t        parent;
  const char*           mem_type;
  void*(*mem_map)       (void* mem, size_t maxsize, int flags);
  void (*mem_unmap)     (void* mem);
  void*(*mem_copy)      (void* mem, ssize_t offset, ssize_t size);
  void*(*mem_share)     (void* mem, ssize_t offset, ssize_t size);
  int  (*mem_is_span)   (void* mem1, void* mem2, void* offset);
  void*(*mem_map_full)  (void* mem, void* info, size_t maxsize);
  void (*mem_unmap_full)(void* mem, void* info);
  void*                 _gst_reserved[4 - 2];
  void*                 priv;
} my_GstAllocator_t;

typedef struct my_GstAllocatorClass_s
{
  my_GstObjectClass_t parent;
  void*     (*alloc)      (void *allocator, size_t size, void *params);
  void      (*free)       (void *allocator, void *memory);
  void*    _gst_reserved[4];
} my_GstAllocatorClass_t;

typedef struct my_GstTaskPool_s
{
  my_GstObject_t      parent;
  void*               pool; // GThreadPool
  void*               _gst_reserved[4];
} my_GstTaskPool_t;

typedef struct my_GstTaskPoolClass_s {
  my_GstObjectClass_t parent_class;
  void      (*prepare)  (void* pool, void* error);
  void      (*cleanup)  (void* pool);
  void*     (*push)     (void* pool, void* func, void* user_data, void* error);
  void      (*join)     (void* pool, void* id);
  void      (*dispose_handle) (void* pool, void* id);
  void*     _gst_reserved[4-1];
} my_GstTaskPoolClass_t;

typedef struct my_GCond_s
{
  void*     p;
  uint32_t  i[2];
} my_GCond_t;

typedef struct my_GRecMutex_s
{
  void*     p;
  uint32_t  i[2];
} my_GRecMutex_t;

typedef struct my_GstElement_s
{
  my_GstObject_t        parent;
  my_GRecMutex_t        state_lock;
  my_GCond_t            state_cond;
  uint32_t              state_cookie;
  int                   target_state;
  int                   current_state;
  int                   next_state;
  int                   pending_state;
  int                   last_return;
  void*                 bus;  // GstBus
  void*                 clock;  // GstClock
  int64_t               base_time;
  uint64_t              start_time;
  uint16_t              numpads;
  void*                 pads; //GstClock
  uint16_t              numsrcpads;
  void*                 srcpads;  //GstClock
  uint16_t              numsinkpads;
  void*                 sinkpads; //GstClock
  uint32_t              pads_cookie;
  void*                 contexts; //GstClock
  void*                 _gst_reserved[4-1];
} my_GstElement_t;

typedef struct my_GstElementClass_s {
  my_GstObjectClass_t   parent_class;
  void*                 metadata;
  void*                 elementfactory;
  void*                 padtemplates;
  int                   numpadtemplates;
  uint32_t              pad_templ_cookie;
  void                  (*pad_added)     (void* element, void* pad);
  void                  (*pad_removed)   (void* element, void* pad);
  void                  (*no_more_pads)  (void* element);
  void*                 (*request_new_pad)      (void* element, void* templ, void* name, void* caps);
  void                  (*release_pad)          (void* element, void* pad);
  int                   (*get_state)            (void*  element, void* state, void* pending, uint64_t timeout);
  int                   (*set_state)            (void* element, int state);
  int                   (*change_state)         (void* element, int transition);
  void                  (*state_changed)        (void* element, int oldstate, int newstate, int pending);
  void                  (*set_bus)              (void*  element, void* bus);
  void*                 (*provide_clock)        (void* element);
  int                   (*set_clock)            (void* element, void* clock);
  int                   (*send_event)           (void* element, void* event);
  int                   (*query)                (void* element, void* query);
  int                   (*post_message)         (void* element, void* message);
  void                  (*set_context)          (void* element, void* context);
  void* _gst_reserved[20-2];
} my_GstElementClass_t;

typedef struct my_GstBin_s {
  my_GstElement_t     parent;
  int                 numchildren;
  void*               children; // GList
  uint32_t            children_cookie;
  void*               child_bus;  // GstBus
  void*               messages; // GList
  int                 polling;
  int                 state_dirty;
  int                 clock_dirty;
  void*               provided_clock; // GstClock
  void*               clock_provider; // GstElement
  void*               priv;
  void*               _gst_reserved[4];
} my_GstBin_t;

typedef struct my_GstBinClass_s {
  my_GstElementClass_t parent_class;
  void* pool;
  void  (*element_added)        (void* bin, void* child);
  void  (*element_removed)      (void* bin, void* child);
  int   (*add_element)          (void* bin, void* element);
  int   (*remove_element)       (void* bin, void* element);
  void  (*handle_message)       (void* bin, void* message);
  int   (*do_latency)           (void* bin);
  void  (*deep_element_added)   (void* bin, void* sub_bin, void* child);
  void  (*deep_element_removed) (void* bin, void* sub_bin, void* child);
  void* _gst_reserved[4-2];
} my_GstBinClass_t;

typedef struct my_GstSegment_s
{
  int       flags;
  double    rate;
  double    applied_rate;
  int       format;
  uint64_t  base;
  uint64_t  offset;
  uint64_t  start;
  uint64_t  stop;
  uint64_t  time;
  uint64_t  position;
  uint64_t  duration;
  void*     _gst_reserved[4];
} my_GstSegment_t;

typedef struct my_GstBaseTransform_s
{
  my_GstElement_t     parent;
  void*               sinkpad;  // GstPad
  void*               srcpad; // GstPad
  int                 have_segment;
  my_GstSegment_t     segment;
  void*               queued_buf; // GstBuffer
  void*               priv;
  void*               _gst_reserved[20-1];
} my_GstBaseTransform_t;

typedef struct my_GstBaseTransformClass_s {
  my_GstElementClass_t parent_class;
  int passthrough_on_same_caps;
  int transform_ip_on_passthrough;
  void*  (*transform_caps) (void* trans, int direction, void* caps, void* filter);
  void*  (*fixate_caps)    (void* trans, int direction, void* caps, void* othercaps);
  int    (*accept_caps)    (void* trans, int direction, void* caps);
  int    (*set_caps)       (void* trans, void* incaps, void* outcaps);
  int    (*query)          (void* trans, int direction, void* query);
  int    (*decide_allocation)  (void* trans, void* query);
  int    (*filter_meta)        (void* trans, void* query, size_t api, void* params);
  int    (*propose_allocation) (void* trans, void* decide_query, void* query);
  int    (*transform_size) (void* trans, int direction, void* caps, size_t size, void* othercaps, void* othersize);
  int    (*get_unit_size)  (void* trans, void* caps, void* size);
  int    (*start)        (void* trans);
  int    (*stop)         (void* trans);
  int    (*sink_event)   (void* trans, void* event);
  int    (*src_event)    (void* trans, void* event);
  int    (*prepare_output_buffer) (void*  trans, void* input, void* outbuf);
  int    (*copy_metadata)     (void* trans, void* input, void* outbuf);
  int    (*transform_meta)    (void* trans, void* outbuf, void* meta, void* inbuf);
  void   (*before_transform)  (void* trans, void* buffer);
  int    (*transform)    (void* trans, void* inbuf, void* outbuf);
  int    (*transform_ip) (void* trans, void* buf);
  int    (*submit_input_buffer) (void* trans, int is_discont, void* input);
  int    (*generate_output) (void* trans, void*   outbuf);
  void* _gst_reserved[20-2];
} my_GstBaseTransformClass_t;

typedef struct my_GstVideoDecoder_s
{
  my_GstElement_t parent;
  void*           sinkpad;  // GstPad
  void*           srcpad; // GstPad
  my_GRecMutex_t  stream_lock;
  my_GstSegment_t input_segment;
  my_GstSegment_t output_segment;
  void*           priv;
  void*           padding[20];
} my_GstVideoDecoder_t;

typedef struct my_GstVideoDecoderClass_s {
  my_GstElementClass_t parent_class;
  int      (*open)           (void* decoder);
  int      (*close)          (void* decoder);
  int      (*start)          (void* decoder);
  int      (*stop)           (void* decoder);
  int      (*parse)          (void* decoder, void* frame, void* adapter, int at_eos);
  int      (*set_format)     (void* decoder, void* state);
  int      (*reset)          (void* decoder, int hard);
  int      (*finish)         (void* decoder);
  int      (*handle_frame)   (void* decoder, void* frame);
  int      (*sink_event)     (void* decoder, void* event);
  int      (*src_event)      (void* decoder, void* event);
  int      (*negotiate)      (void* decoder);
  int      (*decide_allocation)  (void* decoder, void* query);
  int      (*propose_allocation) (void* decoder, void*  query);
  int      (*flush)              (void* decoder);
  int      (*sink_query)     (void* decoder, void* query);
  int      (*src_query)      (void* decoder, void* query);
  void*    (*getcaps)        (void* decoder, void* filter);
  int      (*drain)          (void* decoder);
  int      (*transform_meta) (void* decoder, void* frame, void* meta);
  int      (*handle_missing_data) (void* decoder, uint64_t timestamp, uint64_t duration);
  void* padding[20-7];
} my_GstVideoDecoderClass_t;

typedef struct my_GstVideoEncoder_s
{
  my_GstElement_t   parent;
  void*             sinkpad;  // GstPad
  void*             srcpad; // GstPad
  my_GRecMutex_t    stream_lock;
  my_GstSegment_t   input_segment;
  my_GstSegment_t   output_segment;
  void*             priv;
  void*             padding[20];
} my_GstVideoEncoder_t;

typedef struct my_GstVideoEncoderClass_s {
  my_GstElementClass_t  parent_class;
  int      (*open)         (void* encoder);
  int      (*close)        (void* encoder);
  int      (*start)        (void* encoder);
  int      (*stop)         (void* encoder);
  int      (*set_format)   (void* encoder, void* state);
  int      (*handle_frame) (void* encoder, void* frame);
  int      (*reset)        (void* encoder, int hard);
  int      (*finish)       (void* encoder);
  int      (*pre_push)     (void* encoder, void* frame);
  void*    (*getcaps)      (void* encoder, void* filter);
  int      (*sink_event)   (void* encoder, void* event);
  int      (*src_event)    (void* encoder, void* event);
  int      (*negotiate)    (void* encoder);
  int      (*decide_allocation)  (void* encoder, void* query);
  int      (*propose_allocation) (void*  encoder, void*  query);
  int      (*flush)              (void* encoder);
  int      (*sink_query)     (void* encoder, void* query);
  int      (*src_query)      (void* encoder, void* query);
  int      (*transform_meta) (void* encoder, void* frame, void* meta);
  void*  _gst_reserved[20-4];
} my_GstVideoEncoderClass_t;

typedef struct my_GstBaseSink_s
{
  my_GstElement_t     parent;
  void*               sinkpad;  // GstPad
  int                 pad_mode;
  uint64_t            offset;
  int                 can_activate_pull;
  int                 can_activate_push;
  my_GMutex_t         preroll_lock;
  my_GCond_t          preroll_cond;
  int                 eos;
  int                 need_preroll;
  int                 have_preroll;
  int                 playing_async;
  int                 have_newsegment;
  my_GstSegment_t     segment;
  int                 clock_id;
  int                 sync;
  int                 flushing;
  int                 running;
  int64_t             max_lateness;
  void*               priv;
  void*               _gst_reserved[20];
} my_GstBaseSink_t;

typedef struct my_GstBaseSinkClass_s {
  my_GstElementClass_t parent_class;
  void*         (*get_caps)             (void* sink, void* filter);
  int           (*set_caps)             (void* sink, void* caps);
  void*         (*fixate)               (void* sink, void* caps);
  int           (*activate_pull)        (void* sink, int active);
  void          (*get_times)            (void* sink, void* buffer, void* start, void* end);
  int           (*propose_allocation)   (void* sink, void* query);
  int           (*start)                (void* sink);
  int           (*stop)                 (void* sink);
  int           (*unlock)               (void* sink);
  int           (*unlock_stop)          (void* sink);
  int           (*query)                (void* sink, void* query);
  int           (*event)                (void* sink, void* event);
  int           (*wait_event)           (void* sink, void* event);
  int           (*prepare)              (void* sink, void* buffer);
  int           (*prepare_list)         (void* sink, void* buffer_list);
  int           (*preroll)              (void* sink, void* buffer);
  int           (*render)               (void* sink, void* buffer);
  int           (*render_list)          (void* sink, void* buffer_list);
  void*       _gst_reserved[20];
} my_GstBaseSinkClass_t;

typedef struct my_GstVideoSink_s
{
  my_GstBaseSink_t  parent;
  int               width, height;
  void*             priv;
  void*             _gst_reserved[4];
} my_GstVideoSink_t;

typedef struct my_GstVideoSinkClass_s
{
  my_GstBaseSinkClass_t parent_class;
  int       (*show_frame) (void* video_sink, void* buf);
  int       (*set_info)   (void* video_sink, void* caps, void* info);
  void* _gst_reserved[4-1];
} my_GstVideoSinkClass_t;

typedef struct my_GstGLBaseFilter_s
{
  my_GstBaseTransform_t   parent;
  void*                   display;  // GstGLDisplay
  void*                   context;  // GstGLContext
  void*                   in_caps;  // GstCaps
  void*                   out_caps; // GstCaps
  void*                   _padding[4];
  void*                   priv;
} my_GstGLBaseFilter_t;

typedef struct my_GstGLBaseFilterClass_s
{
  my_GstBaseTransformClass_t parent_class;
  int supported_gl_api;
  int      (*gl_start)          (void* filter);
  void     (*gl_stop)           (void* filter);
  int      (*gl_set_caps)       (void* filter, void* incaps, void* outcaps);
  void* _padding[4];
} my_GstGLBaseFilterClass_t;

typedef struct my_GstVideoColorimetry_t
{
  int range;
  int matrix;
  int transfer;
  int primaries;
} my_GstVideoColorimetry_t;

typedef struct my_GstVideoInfo_s
{
  void*                     finfo;  // const GstVideoFormatInfo
  int                       interlace_mode;
  int                       flags;
  int                       width;
  int                       height;
  size_t                    size;
  int                       views;
  int                       chroma_site;
  my_GstVideoColorimetry_t  colorimetry;
  int                       par_n;
  int                       par_d;
  int                       fps_n;
  int                       fps_d;
  size_t                    offset[4];
  int                       stride[4];
  union {
    struct {
      int   multiview_mode;
      int   multiview_flags;
      int   field_order;
    } abi;
    void* _gst_reserved[4];
  } ABI;
} my_GstVideoInfo_t;

typedef struct my_GstGLFilter_s
{
  my_GstGLBaseFilter_t    parent;
  my_GstVideoInfo_t       in_info;
  my_GstVideoInfo_t       out_info;
  int                     in_texture_target;
  int                     out_texture_target;
  void*                   out_caps; // GstCaps
  void*                   fbo;  // GstGLFramebuffer
  int                     gl_result;
  void*                   inbuf;  // GstBuffer
  void*                   outbuf; // GstBuffer
  void*                   default_shader; // GstGLShader
  int                     valid_attributes;
  uint32_t                vao;
  uint32_t                vbo_indices;
  uint32_t                vertex_buffer;
  int                     draw_attr_position_loc;
  int                     draw_attr_texture_loc;
  void*                   _padding[4];
} my_GstGLFilter_t;

typedef struct my_GstGLFilterClass_s
{
  my_GstGLBaseFilterClass_t parent_class;
  int      (*set_caps)                (void* filter, void* incaps, void* outcaps);
  int      (*filter)                  (void* filter, void* inbuf, void* outbuf);
  int      (*filter_texture)          (void* filter, void* input, void* output);
  int      (*init_fbo)                (void* filter);
  void*    (*transform_internal_caps) (void* filter, int direction, void* caps, void* filter_caps);
  void*    _padding[4];
} my_GstGLFilterClass_t;

typedef struct my_GstAggregator_s
{
  my_GstElement_t   parent;
  void*             srcpad; // GstPad
  void*             priv;
  void*             _gst_reserved[20];
} my_GstAggregator_t;

typedef struct my_GstAggregatorClass_s {
  my_GstElementClass_t   parent_class;
  int       (*flush)                (void* self);
  void*     (*clip)                 (void* self, void* aggregator_pad, void* buf);
  int       (*finish_buffer)        (void* self, void* buffer);
  int       (*sink_event)           (void* self, void* aggregator_pad, void* event);
  int       (*sink_query)           (void* self, void* aggregator_pad, void* query);
  int       (*src_event)            (void* self, void* event);
  int       (*src_query)            (void* self, void* query);
  int       (*src_activate)         (void* self, int mode, int active);
  int       (*aggregate)            (void* self, int timeout);
  int       (*stop)                 (void* self);
  int       (*start)                (void* self);
  uint64_t  (*get_next_time)        (void* self);
  void*     (*create_new_pad)       (void* self, void* templ, void* req_name, void* caps);
  int       (*update_src_caps)      (void* self, void* caps, void* ret);
  void*     (*fixate_src_caps)      (void* self, void* caps);
  int       (*negotiated_src_caps)  (void* self, void* caps);
  int       (*decide_allocation)    (void* self, void* query);
  int       (*propose_allocation)   (void* self, void*pad, void* decide_query, void* query);
  int       (*negotiate)            (void* self);
  int       (*sink_event_pre_queue) (void* self, void* aggregator_pad, void* event);
  int       (*sink_query_pre_queue) (void* self, void* aggregator_pad, void* query);
  int       (*finish_buffer_list)   (void* self, void* bufferlist);
  void      (*peek_next_sample)     (void* self, void* aggregator_pad);
  void*      _gst_reserved[20-5];
} my_GstAggregatorClass_t;

typedef struct my_GstVideoAggregator_s
{
  my_GstAggregator_t  aggregator;
  my_GstVideoInfo_t   info;
  void*               priv;
  void*               _gst_reserved[20];
} my_GstVideoAggregator_t;

typedef struct my_GstVideoAggregatorClass_s
{
  my_GstAggregatorClass_t parent_class;
  void*     (*update_caps)               (void* vagg, void* caps);
  int       (*aggregate_frames)          (void* vagg, void* outbuffer);
  int       (*create_output_buffer)      (void* vagg, void* outbuffer);
  void      (*find_best_format)          (void* vagg, void* downstream_caps, void* best_info, void* at_least_one_alpha);
  void*      _gst_reserved[20];
} my_GstVideoAggregatorClass_t;

typedef struct my_GHookList_s
{
  unsigned long     seq_id;
  uint32_t          hook_size : 16;
  uint32_t          is_setup : 1;
  void*             hooks;  // GHook
  void*             dummy3;
  void (*finalize_hook) (void* hook_list, void* hook);
  void*             dummy[2];
} my_GHookList_t;

typedef struct my_GstPad_s
{
  my_GstObject_t      parent;
  void*               element_private;
  void*               padtemplate;  // GstPadTemplate
  int                 direction;
  my_GRecMutex_t      stream_rec_lock;
  void*               task; // GstTask
  my_GCond_t          block_cond;
  my_GHookList_t      probes;
  int                 mode;
  int     (*activatefunc)       (void* pad, void* parent);
  void*               activatedata;
  void    (*activatenotify)     (void* a);
  int     (*activatemodefunc)   (void* pad, void* parent, int mode, int active);
  void*               activatemodedata;
  void    (*activatemodenotify) (void* a);
  void*                peer;  // GstPad
  int     (*linkfunc)           (void* pad, void* parent, void* peer);
  void*               linkdata;
  void    (*linknotify)         (void* a);
  void    (*unlinkfunc)         (void* pad, void* parent);
  void*               unlinkdata;
  void    (*unlinknotify)       (void* a);
  int     (*chainfunc)          (void* pad, void* parent, void* buffer);
  void*               chaindata;
  void    (*chainnotify)        (void* a);
  int     (*chainlistfunc)      (void* pad, void* parent, void* list);
  void*               chainlistdata;
  void    (*chainlistnotify)    (void* a);
  int     (*getrangefunc)       (void* pad, void* parent, uint64_t offset, uint32_t length, void* buffer);
  void*               getrangedata;
  void    (*getrangenotify)     (void* a);
  int     (*eventfunc)          (void* pad, void* parent, void* event);
  void*               eventdata;
  void    (*eventnotify)        (void* a);
  int64_t             offset;
  int     (*queryfunc)          (void* pad, void* parent, void* query);
  void*               querydata;
  void    (*querynotify)        (void* a);
  void*   (*iterintlinkfunc)    (void* pad, void* parent);
  void*               iterintlinkdata;
  void    (*iterintlinknotify)  (void* a);
  int                 num_probes;
  int                 num_blocked;
  void*               priv;
  union {
    void* _gst_reserved[4];
    struct {
      int    last_flowret;
      int (*eventfullfunc) (void* pad, void* parent, void* event);
    } abi;
  } ABI;
} my_GstPad_t;

typedef struct my_GstPadClass_s {
  my_GstObjectClass_t        parent_class;
  void       (*linked)       (void* pad, void* peer);
  void       (*unlinked)     (void* pad, void* peer);
  void* _gst_reserved[4];
} my_GstPadClass_t;

typedef struct my_GstAggregatorPad_s
{
  my_GstPad_t     parent;
  my_GstSegment_t segment;
  void*           priv;
  void*           _gst_reserved[4];
} my_GstAggregatorPad_t;

typedef struct my_GstAggregatorPadClass_s
{
  my_GstPadClass_t   parent_class;
  int      (*flush)       (void* aggpad, void* aggregator);
  int      (*skip_buffer) (void* aggpad, void* aggregator, void* buffer);
  void*      _gst_reserved[20];
} my_GstAggregatorPadClass_t;

typedef struct my_GstVideoAggregatorPad_s
{
  my_GstAggregatorPad_t parent;
  my_GstVideoInfo_t     info;
  void*                 priv;
  void*                 _gst_reserved[4];
} my_GstVideoAggregatorPad_t;

typedef struct my_GstVideoAggregatorPadClass_s
{
  my_GstAggregatorPadClass_t parent_class;
  void    (*update_conversion_info) (void* pad);
  int     (*prepare_frame)          (void* pad, void* vagg, void* buffer, void* prepared_frame);
  void    (*clean_frame)            (void* pad, void* vagg, void* prepared_frame);
  void    (*prepare_frame_start)    (void* pad, void* vagg, void* buffer, void* prepared_frame);
  void    (*prepare_frame_finish)   (void* pad, void* vagg, void* prepared_frame);
  void*   _gst_reserved[20-2];
} my_GstVideoAggregatorPadClass_t;

typedef struct my_GstBaseSrc_s
{
  my_GstElement_t parent;
  void*           srcpad; // GstPad
  my_GMutex_t     live_lock;
  my_GCond_t      live_cond;
  int             is_live;
  int             live_running;
  uint32_t        blocksize;
  int             can_activate_push;
  int             random_access;
  int             clock_id;
  my_GstSegment_t segment;
  int             need_newsegment;
  int             num_buffers;
  int             num_buffers_left;
  int             typefind;
  int             running;
  void*           pending_seek; // GstEvent
  void*           priv;
  void*           _gst_reserved[20];
} my_GstBaseSrc_t;

typedef struct my_GstBaseSrcClass_s {
  my_GstElementClass_t parent_class;
  void*      (*get_caps)              (void* src, void* filter);
  int        (*negotiate)             (void* src);
  void*      (*fixate)                (void* src, void* caps);
  int        (*set_caps)              (void* src, void* caps);
  int        (*decide_allocation)     (void* src, void* query);
  int        (*start)                 (void* src);
  int        (*stop)                  (void* src);
  void       (*get_times)             (void* src, void* buffer, void* start, void* end);
  int        (*get_size)              (void* src, void* size);
  int        (*is_seekable)           (void* src);
  int        (*prepare_seek_segment)  (void* src, void* seek, void* segment);
  int        (*do_seek)               (void* src, void* segment);
  int        (*unlock)                (void* src);
  int        (*unlock_stop)           (void* src);
  int        (*query)                 (void* src, void* query);
  int        (*event)                 (void* src, void* event);
  int        (*create)                (void* src, uint64_t offset, uint32_t size, void* buf);
  int        (*alloc)                 (void* src, uint64_t offset, uint32_t size, void* buf);
  int        (*fill)                  (void* src, uint64_t offset, uint32_t size, void* buf);
  void*       _gst_reserved[20];
} my_GstBaseSrcClass_t;

typedef struct my_GstPushSrc_s
{
  my_GstBaseSrc_t parent;
  void*           _gst_reserved[4];
} my_GstPushSrc_t;

typedef struct my_GstPushSrcClass_s {
  my_GstBaseSrcClass_t parent_class;
  int (*create) (void* src, void* buf);
  int (*alloc)  (void* src, void* buf);
  int (*fill)   (void* src, void* buf);
  void* _gst_reserved[4];
} my_GstPushSrcClass_t;

typedef struct my_GstGLBaseSrc_s
{
  my_GstPushSrc_t   parent;
  void*             display;  // GstGLDisplay
  void*             context;  // GstGLContext
  my_GstVideoInfo_t out_info;
  void*             out_caps; // GstCaps
  uint64_t          running_time;
  void*             _padding[4];
  void*             priv;
} my_GstGLBaseSrc_t;

typedef struct my_GstGLBaseSrcClass_s {
  my_GstPushSrcClass_t parent_class;
  int supported_gl_api;
  int      (*gl_start)          (void* src);
  void     (*gl_stop)           (void* src);
  int      (*fill_gl_memory)    (void* src, void* mem);
  void*     _padding[4];
} my_GstGLBaseSrcClass_t;

typedef struct my_GstAudioDecoder_s
{
  my_GstElement_t     parent;
  void*               sinkpad;  // GstPad
  void*               srcpad; // GstPad
  my_GRecMutex_t      stream_lock;
  my_GstSegment_t     input_segment;
  my_GstSegment_t     output_segment;
  void*               priv;
  void*               _gst_reserved[20];
} my_GstAudioDecoder_t;

typedef struct my_GstAudioDecoderClass_s
{
  my_GstElementClass_t parent_class;
  int      (*start)              (void* dec);
  int      (*stop)               (void* dec);
  int      (*set_format)         (void* dec, void* caps);
  int      (*parse)              (void* dec, void* adapter, void* offset, void* length);
  int      (*handle_frame)       (void* dec, void* buffer);
  void     (*flush)              (void* dec, int hard);
  int      (*pre_push)           (void* dec, void* buffer);
  int      (*sink_event)         (void* dec, void* event);
  int      (*src_event)          (void* dec, void* event);
  int      (*open)               (void* dec);
  int      (*close)              (void* dec);
  int      (*negotiate)          (void* dec);
  int      (*decide_allocation)  (void* dec, void* query);
  int      (*propose_allocation) (void* dec, void* query);
  int      (*sink_query)         (void* dec, void* query);
  int      (*src_query)          (void* dec, void* query);
  void*    (*getcaps)            (void* dec, void*  filter);
  int      (*transform_meta)     (void* enc, void* outbuf, void* meta, void* inbuf);
  void*   _gst_reserved[20 - 4];
} my_GstAudioDecoderClass_t;

typedef struct my_GstVideoFilter_s {
  my_GstBaseTransform_t parent;
  int                   negotiated;
  my_GstVideoInfo_t     in_info;
  my_GstVideoInfo_t     out_info;
  void*                 _gst_reserved[4];
} my_GstVideoFilter_t;

typedef struct my_GstVideoFilterClass_s {
  my_GstBaseTransformClass_t parent_class;
  int      (*set_info)           (void* filter, void* incaps, void* in_info, void* outcaps, void* out_info);
  int      (*transform_frame)    (void* filter, void* inframe, void* outframe);
  int      (*transform_frame_ip) (void* filter, void* frame);
  void* _gst_reserved[4];
} my_GstVideoFilterClass_t;

typedef struct my_GstAudioInfo_s {
  void*     finfo;
  int       flags;
  int       layout;
  int       rate;
  int       channels;
  int       bpf;
  int       position[64];
  void*     _gst_reserved[20];
} my_GstAudioInfo_t;

typedef struct my_GstAudioFilter_s {
  my_GstBaseTransform_t parent;
  my_GstAudioInfo_t     info;
  void*                 _gst_reserved[20];
} my_GstAudioFilter_t;

typedef struct my_GstAudioFilterClass_s {
  my_GstBaseTransformClass_t  parent_class;
  int   (*setup) (void * filter, void* info);
  void* _gst_reserved[20];
} my_GstAudioFilterClass_t;

typedef struct my_GstBufferPool_s {
  my_GstObject_t      object;
  int                 flushing;
  void*               priv; //GstBufferPoolPrivate
  void*               _gst_reserved[4];
} my_GstBufferPool_t;

typedef struct my_GstBufferPoolClass_s {
  my_GstObjectClass_t object_class;
  void* (*get_options)    (void* pool);
  int   (*set_config)     (void* pool, void* config);
  int   (*start)          (void* pool);
  int   (*stop)           (void* pool);
  int   (*acquire_buffer) (void* pool, void* buffer, void* params);
  int   (*alloc_buffer)   (void* pool, void* buffer, void* params);
  void  (*reset_buffer)   (void* pool, void* buffer);
  void  (*release_buffer) (void* pool, void* buffer);
  void  (*free_buffer)    (void* pool, void* buffer);
  void  (*flush_start)    (void* pool);
  void  (*flush_stop)     (void* pool);
  void*  _gst_reserved[4 - 2];
} my_GstBufferPoolClass_t;

typedef struct my_GstVideoBufferPool_s
{
  my_GstBufferPool_t bufferpool;
  void* priv; //GstVideoBufferPoolPrivate
} my_GstVideoBufferPool_t;

typedef struct my_GstVideoBufferPoolClass_s
{
  my_GstBufferPoolClass_t parent;
} my_GstVideoBufferPoolClass_t;

typedef struct my_GDBusProxy_s
{
  my_GObject_t  parent;
  void*         priv;
} my_GDBusProxy_t;

typedef struct my_GDBusProxyClass_s {
  my_GObjectClass_t parent_class;
  void (*g_properties_changed) (void* proxy, void* changed_properties, const char* const* invalidated_properties);
  void (*g_signal)             (void* proxy, const char* sender_name, const char* signal_name, void* parameters);
  void* padding[32];
} my_GDBusProxyClass_t;

typedef struct my_GTypeInterface_s {
  size_t g_type;
  size_t g_instance_type;
} my_GTypeInterface_t;

typedef struct my_GstURIHandlerInterface_s {
  my_GTypeInterface_t parent;
  int    (* get_type)           (size_t type);
  void*  (* get_protocols)      (size_t type);
  void*  (* get_uri)            (void* handler);
  int    (* set_uri)            (void* handler, void* uri, void* error);
} my_GstURIHandlerInterface_t;


// GTypeValueTable
typedef struct my_GTypeValueTable_s {
  void     (*value_init)         (void* value);
  void     (*value_free)         (void* value);
  void     (*value_copy)         (void* src_value, void* dest_value);
  void*    (*value_peek_pointer) (void* value);
  void*    collect_format;
  void*    (*collect_value)      (void* value, uint32_t n_collect_values, void* collect_values, uint32_t collect_flags);
  void*    lcopy_format;
  void*    (*lcopy_value)        (void* value, uint32_t n_collect_values, void* collect_values, uint32_t collect_flags);
} my_GTypeValueTable_t;

// GTypeInfo
typedef void (*GBaseInitFunc)           (void* g_class);
typedef void (*GBaseFinalizeFunc)       (void* g_class);
typedef void (*GClassInitFunc)          (void* g_class, void* class_data);
typedef void (*GClassFinalizeFunc)      (void* g_class, void* class_data);
typedef void (*GInstanceInitFunc)       (void* instance, void* g_class);

typedef struct my_GTypeInfo_s {
  uint16_t               class_size;
  GBaseInitFunc          base_init;
  GBaseFinalizeFunc      base_finalize;
  GClassInitFunc         class_init;
  GClassFinalizeFunc     class_finalize;
  void*                  class_data;
  uint16_t               instance_size;
  uint16_t               n_preallocs;
  GInstanceInitFunc      instance_init;
  my_GTypeValueTable_t*  value_table;
} my_GTypeInfo_t;

// GtkTypeInfo 
typedef GBaseInitFunc           GtkClassInitFunc;
typedef GInstanceInitFunc       GtkObjectInitFunc;

typedef struct my_GtkTypeInfo_s {
  void*              type_name;
  uint32_t    			 object_size;
  uint32_t    			 class_size;
  GtkClassInitFunc	 class_init_func;
  GtkObjectInitFunc	 object_init_func;
  void*         		 reserved_1;
  void*         		 reserved_2;
  GtkClassInitFunc	 base_class_init_func;
} my_GtkTypeInfo_t ;

my_GTypeValueTable_t* findFreeGTypeValueTable(my_GTypeValueTable_t* fcts);
my_GTypeInfo_t* findFreeGTypeInfo(my_GTypeInfo_t* fcts, size_t parent);
my_GtkTypeInfo_t* findFreeGtkTypeInfo(my_GtkTypeInfo_t* fcts, size_t parent);
void* find_class_init_Fct(void* fct, size_t parent);

void InitGTKClass(bridge_t *bridge);
void FiniGTKClass(void);

#define GTKCLASSES()                \
GTKCLASS(GObject)                   \
GTKCLASS(GInitiallyUnowned)         \
GTKCLASS(GApplication)              \
GTKCLASS(GtkApplication)            \
GTKCLASS(GtkDrawingArea3)           \
GTKCLASS(GtkObject)                 \
GTKCLASS(GtkWidget2)                \
GTKCLASS(GtkWidget3)                \
GTKCLASS(GtkContainer2)             \
GTKCLASS(GtkContainer3)             \
GTKCLASS(GtkAction)                 \
GTKCLASS(GtkLabel2)                 \
GTKCLASS(GtkLabel3)                 \
GTKCLASS(GtkMisc2)                  \
GTKCLASS(GtkMisc3)                  \
GTKCLASS(GtkImage3)                 \
GTKCLASS(GtkTreeView2)              \
GTKCLASS(GtkBin2)                   \
GTKCLASS(GtkBin3)                   \
GTKCLASS(GtkWindow2)                \
GTKCLASS(GtkWindow3)                \
GTKCLASS(GtkTable2)                 \
GTKCLASS(GtkFixed2)                 \
GTKCLASS(GtkFixed3)                 \
GTKCLASS(GtkApplicationWindow)      \
GTKCLASS(GtkListBox)                \
GTKCLASS(GtkListBoxRow)             \
GTKCLASS(GtkButton2)                \
GTKCLASS(GtkButton3)                \
GTKCLASS(GtkComboBox2)              \
GTKCLASS(GtkToggleButton2)          \
GTKCLASS(GtkToggleButton3)          \
GTKCLASS(GtkMenuButton3)            \
GTKCLASS(GtkCheckButton2)           \
GTKCLASS(GtkCheckButton3)           \
GTKCLASS(GtkEntry2)                 \
GTKCLASS(GtkSpinButton2)            \
GTKCLASS(GtkProgress2)              \
GTKCLASS(GtkProgressBar2)           \
GTKCLASS(GtkFrame2)                 \
GTKCLASS(GtkMenuShell2)             \
GTKCLASS(GtkMenuBar2)               \
GTKCLASS(GtkTextView2)              \
GTKCLASS(GtkTextView3)              \
GTKCLASS(GtkGrid3)                  \
GTKCLASS(GtkEventController)        \
GTKCLASS(GtkGesture)                \
GTKCLASS(GtkGestureSingle)          \
GTKCLASS(GtkGestureLongPress)       \
GTKCLASS(GtkNotebook2)              \
GTKCLASS(GtkCellRenderer2)          \
GTKCLASS(GtkCellRendererText2)      \
GTKCLASS(MetaFrames2)               \
GTKCLASS(GDBusObjectManagerClient)  \
GTKCLASS(AtkObject)                 \
GTKCLASS(AtkUtil)                   \
GTKCLASS(GstObject)                 \
GTKCLASS(GstAllocator)              \
GTKCLASS(GstTaskPool)               \
GTKCLASS(GDBusProxy)                \
GTKCLASS(GstElement)                \
GTKCLASS(GstBin)                    \
GTKCLASS(GstBaseTransform)          \
GTKCLASS(GstVideoDecoder)           \
GTKCLASS(GstVideoEncoder)           \
GTKCLASS(GstBaseSink)               \
GTKCLASS(GstVideoSink)              \
GTKCLASS(GstGLBaseFilter)           \
GTKCLASS(GstGLFilter)               \
GTKCLASS(GstAggregator)             \
GTKCLASS(GstVideoAggregator)        \
GTKCLASS(GstPad)                    \
GTKCLASS(GstAggregatorPad)          \
GTKCLASS(GstVideoAggregatorPad)     \
GTKCLASS(GstBaseSrc)                \
GTKCLASS(GstPushSrc)                \
GTKCLASS(GstGLBaseSrc)              \
GTKCLASS(GstAudioDecoder)           \
GTKCLASS(GstVideoFilter)            \
GTKCLASS(GstAudioFilter)            \
GTKCLASS(GstBufferPool)             \
GTKCLASS(GstVideoBufferPool)        \
GTKIFACE(GstURIHandler)             \

#define GTKCLASS(A) void Set##A##ID(size_t id);
#define GTKIFACE(A) GTKCLASS(A)
GTKCLASSES()
#undef GTKIFACE
#undef GTKCLASS

void SetGTypeName(void* f);
void SetGTypeParent(void* f);
void SetGClassPeek(void* f);
void AutoBridgeGtk(void*(*ref)(size_t), void(*unref)(void*));

void* wrapCopyGTKClass(void* cl, size_t type);
void* unwrapCopyGTKClass(void* klass, size_t type);
void wrapGTKClass(void* cl, size_t type);
void unwrapGTKClass(void* klass, size_t type);

void unwrapGTKInterface(void* cl, size_t type);
void* wrapCopyGTKInterface(void* cl, size_t type);
void* unwrapCopyGTKInterface(void* iface, size_t type);

void unwrapGTKInstance(void* cl, size_t type);
void bridgeGTKInstance(void* cl, size_t type);

void addRegisteredClass(size_t klass, char* name);

void my_add_signal_offset(size_t klass, uint32_t offset, int n);

#endif //__GTKCLASS_H__
