#ifndef SIMPLE_STACK_QUEUE_H
#define SIMPLE_STACK_QUEUE_H

#include <stdlib.h>

#define node_decl(type) \
typedef struct __node_##type { type e; struct __node_##type *next; } node_##type; \
node_##type *create_node_##type(type element, node_##type *next = nullptr) {\
	node_##type *r = (node_##type *)malloc(sizeof(node_##type));\
	if (!r) return nullptr;\
	r->e = element;\
	r->next = next;\
	return r;\
}\
void stream_node_##type##_tackle(node_##type *r, void (*act)(void *)) {\
	while (r) {\
		node_##type *n = r->next;\
		act(r);\
		r = n;\
	}\
}\
void free_node_##type(node_##type *f) {\
	stream_node_##type##_tackle(f, &free);\
}\

#define node(type) node_##type


#define stack_decl(type) \
typedef struct { node(type) *root; } stack_##type;\
stack_##type *push_##type(stack_##type *s, type element) {\
	s->root = create_node_##type(element, s->root);\
	return s;\
}\
int empty_stack_##type(stack_##type *s) { \
	return !s->root; \
}\
type pop_##type(stack_##type *s) {\
	type r = s->root->e;\
	node(type) *k = s->root;\
	s->root = s->root->next;\
	free(k);\
	return r;\
}\
void free_stack_##type(stack_##type *s) {\
	if (!s->root) return;\
	free_node_##type(s->root);\
	s->root = nullptr;\
}
#define stack(type) stack_##type
#define create_stack(type, name) stack_##type name; name.root = nullptr;

#define queue_decl(type) \
typedef struct { node(type) *head; node(type) *tail; } queue_##type;\
queue_##type *enquque_##type(queue_##type *q, type element) {\
	if (!q->tail) {\
		q->head = create_node_##type(element);\
		q->tail = q->head;\
		return q;\
	}\
	q->tail->next = create_node_##type(element);\
	q->tail = q->tail->next;\
	return q;\
}\
int empty_queue_##type(queue_##type *q) { return !q->head; }\
type dequeue_##type(queue_##type *q) {\
	type r = q->head->e;\
	node(type) *k;\
	if (q->head == q->tail) {\
		free(q->head);\
		q->head = nullptr;\
		q->tail = nullptr;\
		goto ret;\
	}\
	k = q->head->next;\
	free(q->head);\
	q->head = k;\
ret:\
	return r;\
}\
void free_queue_##type(queue_##type *q) {\
	free_node_##type(q->head);\
	q->head = nullptr;\
	q->tail = nullptr;\
}
#define create_queue(type, name) queue_##type name; name.head = nullptr; name.tail = nullptr;

// here to DECLARE the types
// NOTE that: type here must be a continuous string
// which means char* is not acceptable
// if you need to create the type as char*, you'll like to:
//		1. typedef char* pchar;
//		2. <any kind>_decl(pchar)
node_decl(char)
stack_decl(char)
queue_decl(char)

#include <stdio.h>
void print_char_node_stream(void *r) {
	if (!r) return;
	node_char *s = (node_char *)r;
	printf("%c", s->e);
}

void run() {
	const char buffer[] = "I love program";
	// DO NOT create stack/queue by: stack_<type> <name>; create_{stack | queue} is the initializer of these two
	// create_{stack | queue}(<type>, <name>) means to create a variable with the name as <name>
	create_stack(char, sc);
	create_queue(char, qc);
	// because sizeof("<anything>") will be the visible length + 1, here needs to minus 1
	for (size_t i = 0; i < sizeof(buffer) - 1; ++i) {
		push_char(&sc, buffer[i]);
		enquque_char(&qc, buffer[i]);
	}
	stream_node_char_tackle(sc.root, &print_char_node_stream);
	printf("\n");
	stream_node_char_tackle(qc.head, &print_char_node_stream);
	printf("\n");
	// to FREE the resource, here may be an optional operation
	// because the program is going to leave here
	// but note that it's certainly a good habit as to free the resource right after use
	free_stack_char(&sc);
	free_queue_char(&qc);
}

#endif // !SIMPLE_STACK_QUEUE_H
