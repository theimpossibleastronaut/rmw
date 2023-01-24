/*
 * Darwin comes with an outdated version of sys/queue.h
 * however since Darwin is BSD based we can fix this by
 * using a fake queue, and redefinging a lot of the structures
 */

#undef SIMPLEQ_HEAD
#undef SIMPLEQ_ENTRY
#undef SIMPLEQ_FOREACH
#undef SIMPLEQ_INSERT_TAIL
#undef SIMPLEQ_FIRST
#undef SIMPLEQ_END
#undef SIMPLEQ_NEXT

#define SIMPLEQ_FIRST(head) ((head)->sqh_first)
#define SIMPLEQ_EMPTY(head) (SIMPLEQ_FIRST (head) == SIMPLEQ_END (head))
#define SIMPLEQ_END(head) NULL
#define SIMPLEQ_NEXT(elm, field) ((elm)->field.sqe_next)

#define SIMPLEQ_HEAD(name, type)                                              \
  struct name                                                                 \
  {                                                                           \
    struct type *sqh_first; /* first element */                               \
    struct type **sqh_last; /* addr of last next element */                   \
  }

#define SIMPLEQ_ENTRY(type)                                                   \
  struct                                                                      \
  {                                                                           \
    struct type *sqe_next; /* next element */                                 \
  }

#define SIMPLEQ_INSERT_TAIL(head, elm, field)                                 \
  do                                                                          \
    {                                                                         \
      (elm)->field.sqe_next = NULL;                                           \
      *(head)->sqh_last = (elm);                                              \
      (head)->sqh_last = &(elm)->field.sqe_next;                              \
    }                                                                         \
  while (0)

#define SIMPLEQ_HEAD_INITIALIZER(head)                                        \
  {                                                                           \
    NULL, &(head).sqh_first                                                   \
  }

#define SIMPLEQ_FOREACH(var, head, field)                                     \
  for ((var) = SIMPLEQ_FIRST (head); (var) != SIMPLEQ_END (head);             \
       (var) = SIMPLEQ_NEXT (var, field))