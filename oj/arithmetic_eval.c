#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <ctype.h>
#include <stdarg.h>

enum expr_op_type {
	TYPE_OPERATOR = 0,  // operator
	TYPE_OPERAND,   // operand
};

typedef struct expr_list {
	union {
		float vf; /* float value */
		int vi;   /* numeric value */
	} data;
	int type;
	struct expr_list *prev;
	struct expr_list *next;
} expr_elem_t;

struct stack {
	void **array;
	int index;
	int size;
	int expand;
};

void adjust_stack(struct stack *sp)
{
	if (sp->index >= sp->size) {
		if (sp->size)
			sp->array = realloc(sp->array, (sp->size + sp->expand) * sizeof(void *));
		else
			sp->array = malloc((sp->size + sp->expand) * sizeof(void *));
		assert(sp->array != NULL);
		sp->size += sp->expand;
	}
}

struct stack *create_stack(void)
{
	struct stack *sp = calloc(1, sizeof(struct stack));
	sp->index = -1;
	sp->expand = 16;
	return sp;
}

void push_stack(struct stack *sp, void *ptr)
{
	assert(ptr != NULL);
	++sp->index;
	adjust_stack(sp);
	sp->array[sp->index] = ptr;
}

void *pop_stack(struct stack *sp)
{
	if (sp->index < 0)
		return NULL;
	return sp->array[sp->index--];
}

void destroy_stack(struct stack *sp)
{
	if (sp->array)
		free(sp->array);
	free(sp);
}

void free_expr_list(struct expr_list *list_head)
{
	expr_elem_t *elem = list_head, *temp;
	while (elem) {
		temp = elem->next;
		free(elem);
		elem = temp;
	}
}

#define link_doubly(prev, elem) { \
	(elem)->next = NULL; \
	(elem)->prev = (prev); \
	(prev)->next = (elem); \
	(prev) = elem; \
}

void
combine_expr_list (struct expr_list **plist_head)
{
	expr_elem_t *elem, *left_operand, *right_operand;
	/* first handle '*' and '/' */
	elem = *plist_head;
	while (elem) {
		if (elem->type == TYPE_OPERATOR &&
				(elem->data.vi == '*' || elem->data.vi == '/')) {
			left_operand = elem->prev;
			right_operand = elem->next;
			/* left and right must both exist and be operands */
			assert(left_operand != NULL && left_operand->type == TYPE_OPERAND);
			assert(right_operand != NULL && right_operand->type == TYPE_OPERAND);
			if (elem->data.vi == '*') /* multiply */
				right_operand->data.vf = left_operand->data.vf * right_operand->data.vf;
			else /* divide */
				right_operand->data.vf = left_operand->data.vf / right_operand->data.vf;
			/* free nodes after combined */
			if (left_operand->prev)
				left_operand->prev->next = right_operand;
			else
				*plist_head = right_operand;
			right_operand->prev = left_operand->prev;
			free(left_operand);
			free(elem);
			elem = right_operand->next;
		} else
			elem = elem->next;
	}
	/* then handle '+' and '-' */
	elem = *plist_head;
	while (elem) {
		if (elem->type == TYPE_OPERATOR &&
				(elem->data.vi == '+' || elem->data.vi == '-')) {
			left_operand = elem->prev;
			right_operand = elem->next;
			/* left and right must both exist and be operands */
			assert(left_operand != NULL && left_operand->type == TYPE_OPERAND);
			assert(right_operand != NULL && right_operand->type == TYPE_OPERAND);
			if (elem->data.vi == '+')
				right_operand->data.vf = left_operand->data.vf + right_operand->data.vf;
			else
				right_operand->data.vf = left_operand->data.vf - right_operand->data.vf;
			if (left_operand->prev)
				left_operand->prev->next = right_operand;
			else
				*plist_head = right_operand;
			right_operand->prev = left_operand->prev;
			free(left_operand);
			free(elem);
			elem = right_operand->next;
		} else
			elem = elem->next;
	}
}

#define insert_operand() { \
	if (bp > digits) { \
		elem = malloc(sizeof(expr_elem_t)); \
		elem->data.vf = (float)strtol(digits, &eop, 10); \
		if (!isspace(*eop) && *eop != '\0') { \
			fprintf(stderr, "bad number: %s!\n", digits); \
			free_expr_list(dummy.next); \
			return -EINVAL; \
		} \
		elem->type = TYPE_OPERAND; \
		link_doubly(prev, elem); \
		memset(digits, 0, sizeof(digits)); \
		bp = digits; \
		while (brace_level > 0) { \
			push_stack(stack, elem); \
			brace_level --; \
		} \
	} \
}

int
eval_expr(const char *expr, float *result)
{
	char digits[16] = {0}, *bp = digits, *eop;
	const char *cp = expr;
	struct expr_list dummy = {0}, *prev = &dummy;
	expr_elem_t *elem;
	struct stack *stack = create_stack();
	int brace_level = 0;

	while (*cp) {
		switch (*cp) {
			case '+':
			case '-':
			case '*':
			case '/':
				insert_operand();
				if (*cp == '-' && prev->type == TYPE_OPERATOR) { /* negative operand */
					*bp++ = *cp;
					break;
				}
				elem = malloc(sizeof(expr_elem_t));
				elem->data.vi = *cp;
				elem->type = TYPE_OPERATOR;
				link_doubly(prev, elem);
				break;
			/* previous can only be a operator */
			case '(':
			case '[':
			case '{':
				brace_level ++;
				break;
			/* previous can only be a operand */
			case ')':
			case ']':
			case '}':
				insert_operand();
				elem = pop_stack(stack);
				if (elem == NULL) {
					fprintf(stderr, "non-enclosed expression!\n");
					free_expr_list(dummy.next);
					return -EINVAL;
				}
				combine_expr_list(&elem);
				break;

			default:
				*bp++ = *cp;
				break;
		}
		cp++;
	}
	insert_operand();
	combine_expr_list(&dummy.next);
	*result = dummy.next->data.vf;
	free_expr_list(dummy.next);
	destroy_stack(stack);
	return 0;
}

int main (int argc, char *argv[])
{
	assert(argc == 2);
	const char *expr = argv[1];
	float result;
	printf("expression: %s, length: %lu\n", expr, strlen(expr));
	if (eval_expr(expr, &result) != 0) {
		fprintf(stderr, "conversion failed due to bad expression: %s\n", expr);
		exit(EXIT_FAILURE);
	}
	printf("eval result: %f\n", result);
	return 0;
}
