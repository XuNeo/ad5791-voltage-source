#ifndef _MENU_H_
#define _MENU_H_

typedef enum{
  menu_error_ok = 0,
}menu_error_def;

typedef struct _menu_node{
  struct _menu_node *parent;
  struct _menu_node *brother;
  void (*update)(menu_event_def event, void *para);
  char *name;
}menu_node_def;

typedef enum{
  menu_event_key = 0, /**< key value has changed */
  menu_event_encoder, /**< encoder value has changed. */

}menu_event_def;

#endif
