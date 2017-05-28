#define NULL 0
#define stack_peek(stack) ((stack)->top < 0 ? NULL: (stack)->array[(stack)->top])
#define stack_size(stack) ((stack)->top + 1)

//Stack//
typedef struct Stack {
	int top;
	void ** array;
} Stack;
int stack_init(Stack *stack);
int stack_destroy(Stack *stack);
int stack_push(Stack *stack, void * ptr);
int stack_pop(Stack *stack, void ** ptr);



kernel void PostRegex(global char * exp, volatile __global int * counterPtr, global uint * stack, global char * post,
  const int patLen) {
  int id = get_global_id(0);
  if (id == 0) {
      stack_init(stack);

  }
  barrier( CLK_LOCAL_MEM_FENCE | CLK_GLOBAL_MEM_FENCE );
}

int stack_init(Stack *stack)
{
	stack->top = -1;
	stack->array = NULL;
	return 0;
}

int stack_destroy(Stack *stack)
{
	stack->top = -1;
	if (!stack->array) {
		stack->array = NULL;
	}
	return 0;
}

int stack_push(Stack *stack, void * ptr)
{
	stack->top++;
	//stack->array = (void **) realloc(stack->array, (stack->top + 1) * sizeof(void *));
	stack->array[stack->top] = ptr;
	return 0;
}

int stack_pop(Stack *stack, void ** ptr)
{
	if (stack->top > -1) {
		*ptr = stack->array[stack->top];
		//stack->array = (void **) realloc(stack->array, stack->top * sizeof(void *));
		stack->top--;
	} else {
		*ptr = NULL;
	}
	return 0;
}
