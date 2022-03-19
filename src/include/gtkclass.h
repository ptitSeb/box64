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


typedef struct my_GtkObjectClass_s
{
  my_GObjectClass_t parent_class;
  void (*set_arg) (void* object, void* arg, uint32_t arg_id);
  void (*get_arg) (void* object, void* arg, uint32_t arg_id);
  void (*destroy) (void* object);
} my_GtkObjectClass_t;

typedef struct my_GtkWidgetClass_s
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
} my_GtkWidgetClass_t;

typedef struct my_GtkContainerClass_s 
{
  my_GtkWidgetClass_t parent_class;
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
} my_GtkContainerClass_t;

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
  void (*_gtk_reserved2) (void);
  void (*_gtk_reserved3) (void);
  void (*_gtk_reserved4) (void);
} my_GtkActionClass_t;

typedef struct my_GtkMiscClass_s
{
  my_GtkWidgetClass_t parent_class;
} my_GtkMiscClass_t;

typedef struct my_GtkLabelClass_s
{
  my_GtkMiscClass_t parent_class;
  void (* move_cursor)     (void* label, int step, int count, int extend_selection);
  void (* copy_clipboard)  (void* label);
  void (* populate_popup)  (void* label, void* menu);
  int  (*activate_link)    (void* label, void* uri);
  void (*_gtk_reserved1) (void);
  void (*_gtk_reserved2) (void);
  void (*_gtk_reserved3) (void);
} my_GtkLabelClass_t;

typedef struct my_GtkTreeViewClass_s
{
  my_GtkContainerClass_t parent_class;
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
} my_GtkTreeViewClass_t;

typedef struct my_GtkBinClass_s
{
  my_GtkContainerClass_t parent_class;
  void (*_gtk_reserved1) (void);
  void (*_gtk_reserved2) (void);
  void (*_gtk_reserved3) (void);
  void (*_gtk_reserved4) (void);
} my_GtkBinClass_t;

typedef struct my_GtkWindowClass_s {
  my_GtkBinClass_t parent_class;
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
} my_GtkWindowClass_t;

typedef struct my_GtkButtonClass_s {
  my_GtkBinClass_t parent_class;
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
} my_GtkButtonClass_t;

typedef struct my_GtkToggleButtonClass_s {
  my_GtkButtonClass_t parent_class;
  void (* toggled) (void* toggle_button);
  void (*_gtk_reserved1) (void);
  void (*_gtk_reserved2) (void);
  void (*_gtk_reserved3) (void);
  void (*_gtk_reserved4) (void);
} my_GtkToggleButtonClass_t;

typedef struct my_GtkCheckButtonClass_s {
  my_GtkToggleButtonClass_t parent_class;
  void (* draw_indicator) (void* check_button, void* area);
  void (*_gtk_reserved0) (void);
  void (*_gtk_reserved1) (void);
  void (*_gtk_reserved2) (void);
} my_GtkCheckButtonClass_t;

typedef struct my_GtkComboBoxClass_s {
  my_GtkBinClass_t parent_class;
  void (* changed)        (void* combo_box);
  void*(* get_active_text)(void* combo_box);
  void (*_gtk_reserved1) (void);
  void (*_gtk_reserved2) (void);
  void (*_gtk_reserved3) (void);
  void (*_gtk_reserved4) (void);
} my_GtkComboBoxClass_t;

typedef struct my_GtkEntryClass_s {
  my_GtkWidgetClass_t parent_class;
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
} my_GtkEntryClass_t;

typedef struct my_GtkSpinButtonClass_s {
  my_GtkEntryClass_t parent_class;
  int  (*input)  (void* spin_button, void* new_value);
  int  (*output) (void* spin_button);
  void (*value_changed) (void* spin_button);
  void (*change_value) (void* spin_button, int scroll);
  void (*wrapped) (void* spin_button);
  void (*_gtk_reserved1) (void);
  void (*_gtk_reserved2) (void);
  void (*_gtk_reserved3) (void);
} my_GtkSpinButtonClass_t;

typedef struct my_GtkProgressClass_s {
  my_GtkWidgetClass_t parent_class;
  void (* paint)            (void* progress);
  void (* update)           (void* progress);
  void (* act_mode_enter)   (void* progress);
  void (*_gtk_reserved1) (void);
  void (*_gtk_reserved2) (void);
  void (*_gtk_reserved3) (void);
  void (*_gtk_reserved4) (void);
} my_GtkProgressClass_t;

typedef struct my_GtkProgressBarClass_s {
  my_GtkProgressClass_t parent_class;
  void (*_gtk_reserved1) (void);
  void (*_gtk_reserved2) (void);
  void (*_gtk_reserved3) (void);
  void (*_gtk_reserved4) (void);
} my_GtkProgressBarClass_t;

typedef struct my_GtkFrameClass_s {
  my_GtkBinClass_t parent_class;
  void (*compute_child_allocation) (void* frame, void* allocation);
} my_GtkFrameClass_t;

typedef struct my_MetaFramesClass_s
{
  my_GtkWindowClass_t parent_class;

} my_MetaFramesClass_t;

typedef struct my_GtkTableClass_s
{
  my_GtkContainerClass_t parent_class;
} my_GtkTableClass_t;

typedef struct my_GtkFixedClass_s
{
  my_GtkContainerClass_t parent_class;
} my_GtkFixedClass_t;

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

void InitGTKClass(bridge_t *bridge);
void FiniGTKClass();

#define GTKCLASSES()    \
GTKCLASS(GObject)       \
GTKCLASS(GtkObject)     \
GTKCLASS(GtkWidget)     \
GTKCLASS(GtkContainer)  \
GTKCLASS(GtkAction)     \
GTKCLASS(GtkLabel)      \
GTKCLASS(GtkMisc)       \
GTKCLASS(GtkTreeView)   \
GTKCLASS(GtkBin)        \
GTKCLASS(GtkWindow)     \
GTKCLASS(GtkTable)      \
GTKCLASS(GtkFixed)      \
GTKCLASS(GtkButton)     \
GTKCLASS(GtkComboBox)   \
GTKCLASS(GtkToggleButton)\
GTKCLASS(GtkCheckButton)\
GTKCLASS(GtkEntry)      \
GTKCLASS(GtkSpinButton) \
GTKCLASS(GtkProgress)   \
GTKCLASS(GtkProgressBar)\
GTKCLASS(GtkFrame)      \
GTKCLASS(MetaFrames)    \

#define GTKCLASS(A) void Set##A##ID(size_t id);
GTKCLASSES()
#undef GTKCLASS

void SetGTypeName(void* f);
void AutoBridgeGtk(void*(*ref)(size_t), void(*unref)(void*));

void* wrapCopyGTKClass(void* cl, size_t type);
void* unwrapCopyGTKClass(void* klass, size_t type);

typedef struct my_signal_s {
    uint64_t sign;  // signature
    void* data;
    uintptr_t c_handler;
    uintptr_t destroy;
} my_signal_t;
// some random sign to identify a my_signal_t
#define SIGN 0xFB3405EB4510AA00LL

my_signal_t* new_mysignal(void* f, void* data, void* destroy);
void my_signal_delete(my_signal_t* sig);
int my_signal_is_valid(void* sig);
int my_signal_cb(void* a, void* b, void* c, void* d);

void my_add_signal_offset(size_t klass, uint32_t offset, int n);

#endif //__GTKCLASS_H__