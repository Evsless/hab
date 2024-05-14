#ifndef __LLIST_H__
#define __LLIST_H__

typedef struct xml_line_t
{
    char val[128];
    struct xml_line_t *next;
} xml_line_t;

xml_line_t *llist_init(void);
void llist_push(xml_line_t *head, char *val);
int llist_pop(xml_line_t **head);
void llist_free(xml_line_t *head);

#endif /* __LLIST_H__ */