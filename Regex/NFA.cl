#define NULL 0

#define stack_size(stack) ((stack)->top + 1)

//Heap Definitions
typedef struct HeapStruct {
	uint next;
	uchar heap;
} HeapStruct;
global void* malloc(size_t size, __global HeapStruct *heap); //TODO: Right

															 //Stack Definitions
typedef struct {
	int top;
	global void ** array;
} Stack;
void InitializeStack(global Stack * stack, global HeapStruct * heap);
int stack_init(global Stack *stack);
int stack_destroy(global Stack *stack);
int stack_push(global Stack *stack, global void * ptr);
int stack_pop(global Stack *stack, global void ** ptr);
int stack_peek(global Stack *stack);

//Regex 2 Post Definitions
typedef struct {
	int id;
	char c;
} InputSymbol;

typedef struct {
	int id;
	int * transitions;
} State;

typedef struct {
	global State * start;
	global State * final;
} NFA;

int checkPrecedence(global Stack * opStack, global char *ptr);
int evaluate(global Stack * symbolStack, global Stack * opStack, global State *** states, int * numberStates, int numberInputSymbols, global HeapStruct * heap);
int ifInputSymbol(char c);
int prepareInputSymbols(global char * str, InputSymbol * symbols);
int pushInputSymbol(char c, global Stack * symbolStack, InputSymbol * symbols, State *** states, int * numberStates, int numberInputSymbols, global HeapStruct * heap);
kernel void Regex1(__constant char * exp, __global HeapStruct *heap) {
	int id = get_global_id(0);
	printf("%s\n", exp[id]);
}

kernel void Regex(global char * exp, __global HeapStruct *heap) {
	int id = get_global_id(0);
	if (id == 0) {
		//Declare Variables
		InputSymbol symbols[27];
		int numberInputSymbols;
		global char * str;
		int i, j;
		//Create Stacks
		global Stack* symbolStack = (global Stack*)malloc(sizeof(Stack), heap);
		//printf("%p \n", symbolStack);
		global Stack* opStack = (global Stack*)malloc(sizeof(Stack), heap);
		//printf("%p \n", opStack);
		InitializeStack(symbolStack, heap);
		InitializeStack(opStack, heap);

		//Initialize
		numberInputSymbols = prepareInputSymbols(exp, symbols);
		str = exp;
		global State ** states = NULL;
		int numberStates = 0;

		for (i = 0; str[i]; i++) {
			printf("%s \n", str[i]);
			if (ifInputSymbol(str[i])) {
				pushInputSymbol(str[i], symbolStack, symbols, &states, &numberStates, numberInputSymbols, heap);
			}
			else if (opStack->top == -1) {
				stack_push(opStack, str[i]);
				evaluate(symbolStack, opStack, &states, &numberStates, numberInputSymbols, heap);
			}
			else if (str[i] == '(') {
				stack_push(opStack, &str[i]);
			}
			else if (str[i] == ')') {
				global char * ptr = (global char *)malloc(sizeof(char), heap);
				char peek = stack_peek(opStack);
				printf("%s \n", peek);
				/*while (*((char *)stack_peek(opStack)) != '(') {
					printf("here12");
					evaluate(symbolStack, opStack, &states, &numberStates, numberInputSymbols, heap);
				}
				printf("here13");
				stack_pop(opStack, (global void **)&ptr);
				printf("here14");*/
			}
		//	else {
		//		global char * ptr = (global char *)malloc(sizeof(char), heap);
		//		*ptr = str[i];
		//		if (*((char *)stack_peek(opStack)) != '(') {
		//			//check precedence
		//			if (checkPrecedence(opStack, ptr)) {
		//				stack_push(opStack, ptr);
		//				evaluate(symbolStack, opStack, &states, &numberStates, numberInputSymbols, heap);
		//			}
		//			else {
		//				evaluate(symbolStack, opStack, &states, &numberStates, numberInputSymbols, heap);
		//				stack_push(opStack, ptr);
		//			}
		//		}
		//		else {
		//			stack_push(opStack, ptr);
		//		}
		//	}
		//}
		//printf("We made it here");
		//for (i = 0; i < numberStates; i++) {
		//	printf("%2d", states[i]->id);
		//	for (j = 0; j < numberInputSymbols; j++) {
		//		printf("\t%2d", states[i]->transitions[j]);
		//	}
		//	printf("\n");
		//}
		//if (symbolStack->top == 0) {
		//	NFA * nfa = NULL;
		//	stack_pop(symbolStack, (void **)&nfa);
		//	printf("Start State: %d\n", nfa->start->id);
		//	printf("Final State: %d\n", nfa->final->id);
		}
	}
	barrier(CLK_LOCAL_MEM_FENCE | CLK_GLOBAL_MEM_FENCE);
}

int checkPrecedence(global Stack * opStack, global char *ptr) {
	if (*ptr == '$') {
		return 0;
	}
	else if (*((global char *)stack_peek(opStack)) == *ptr) {
		return 0;
	}
	else if (*((global char *)stack_peek(opStack)) == '*') {
		return 0;
	}
	else if (*ptr == '*') {
		return 1;
	}
	else if (*((global char *)stack_peek(opStack)) == '|') {
		return 1;
	}
	else if (*ptr == '|') {
		return 0;
	}
}

global State * createState(global State *** states, int * numberStates, int numberInputSymbols, global HeapStruct * heap)
{
	int i;
	//create new state
	global State * s1 = (global State *)malloc(sizeof(State), heap);
	*numberStates = *numberStates + 1;
	s1->id = *numberStates;
	//s1->transitions = (global int *)malloc(numberInputSymbols * sizeof(int), heap);
	for (i = 0; i < numberInputSymbols; i++) {
		s1->transitions[i] = 0;
	}

	//push to list of states
	//*states = (State **)realloc(*states, sizeof(State *)*(*numberStates));
	//Once finish dynamic memory managemen

	//*states = (State **)

	(*states)[*numberStates - 1] = s1;

	return s1;
}

int evaluate(global Stack * symbolStack, global Stack * opStack, global State *** states, int * numberStates, int numberInputSymbols, global HeapStruct * heap)
{
	char * op = NULL;
	stack_pop(opStack, (void **)&op);
	//printf("%p\n", op);
	if (*op == '*') {
		global NFA * new = (global NFA *)malloc(sizeof(NFA), heap);
		NFA * n = NULL;
		stack_pop(symbolStack, (void **)&n);
		global State * s1;
		global State * s2;
		int i;

		//create state 1
		s1 = createState(states, numberStates, numberInputSymbols, heap);

		//create state 2
		s2 = createState(states, numberStates, numberInputSymbols, heap);

		n->final->transitions[0] = (n->start->id) * 100 + s2->id;
		s1->transitions[0] = (n->start->id) * 100 + s2->id;
		//free memory to avoid leak
		//free(n);
		n = NULL;
		//push NFA later
		new->start = s1;
		new->final = s2;
		stack_push(symbolStack, (global void *) new);
	}
	else if (*op == '.') {
		global NFA * n2 = NULL;
		global NFA * n1 = NULL;
		stack_pop(symbolStack, (global void **)&n2);
		stack_pop(symbolStack, (global void **)&n1);
		n1->final->transitions[0] = n2->start->id;
		//push NFA later
		n1->final = n2->final;
		//free(n2);
		n2 = NULL;
		stack_push(symbolStack, (global void *)n1);
	}
	else if (*op == '|') {
		global NFA * new = (global NFA *)malloc(sizeof(NFA), heap);
		global NFA * n2 = NULL;
		global NFA * n1 = NULL;
		global State * s1;
		global State * s2;
		int i;

		stack_pop(symbolStack, (global void **)&n2);
		stack_pop(symbolStack, (global void **)&n1);
		//create state 1
		s1 = createState(states, numberStates, numberInputSymbols, heap);

		//create state 2
		s2 = createState(states, numberStates, numberInputSymbols, heap);

		n1->final->transitions[0] = s2->id;
		n2->final->transitions[0] = s2->id;
		s1->transitions[0] = (n1->start->id) * 100 + n2->start->id;
		//push NFA later
		new->start = s1;
		new->final = s2;
		stack_push(symbolStack, (global void *) new);
	}


	return 0;
}

int pushInputSymbol(char c, global Stack * symbolStack, InputSymbol * symbols, State *** states, int * numberStates, int numberInputSymbols, global HeapStruct *heap)
{
	global State * s1;
	global State * s2;
	global NFA * n;
	int i;

	//create state 1
	s1 = createState(states, numberStates, numberInputSymbols, heap);

	//create state 2
	s2 = createState(states, numberStates, numberInputSymbols, heap);

	//create transition
	s1->transitions[symbols[c - 96].id] = s2->id;

	//create NFA
	n = (global NFA *)malloc(sizeof(NFA), heap);
	n->start = s1;
	n->final = s2;

	//push NFA
	stack_push(symbolStack, n);

	return 0;
}

int ifInputSymbol(char c)
{
	if ((c > 96) && (c < 123)) {
		return 1;
	}
	else {
		return 0;
	}
}

int ifOperationSymbol(char c)
{
	if ((c == '|') || (c == '.') || (c == '|')) {
		return 1;
	}
	else {
		return 0;
	}
}
int prepareInputSymbols(global char * str, InputSymbol * symbols)
{
	int id = 1;
	int i;
	for (i = 0; i < 27; i++) {
		symbols[i].id = -1;
	}
	i = 0;
	symbols[0].id = 0;
	symbols[0].c = '0';
	//printf("%2d", 00);
	//printf("\tepsilon");
	while (str[i] != '\0') {
		if (ifInputSymbol(str[i])) {
			if (symbols[str[i] - 96].id == -1) {
				symbols[str[i] - 96].id = id;
				id++;
				symbols[str[i] - 96].c = str[i];
				//printf("\t%2c", symbols[str[i] - 96].c);
			}
		}
		i++;
	}

	printf("Input Symbols: %d\n", id);
	printf("\n");
	return id;
}



void InitializeStack(global Stack * stack, global HeapStruct * heap) {
	stack->top = -1;
	stack->array = NULL;
	void * test = (void *)malloc(sizeof(void *) * 100, heap);
	stack->array = &test;
	printf("Init: %p \n", test);
	printf("Init2: %p \n", &stack->array);
}

int stack_destroy(global Stack *stack)
{
	stack->top = -1;
	if (!stack->array) {
		//free(stack->array);
		stack->array = NULL;
	}
	return 0;
}

int stack_push(global Stack *stack, global void * ptr)
{
	printf("ptr val %s\n", *(char *)ptr);
	stack->top++;

	//stack->array = (void **)realloc(stack->array, (stack->top + 1) * sizeof(void *));
	//Return
	printf("top stack %p \n", &stack->array[stack->top]);
	stack->array[stack->top] = ptr;
	printf("%p \n", &stack->array[stack->top]);
	printf("a%s \n", (char *)stack->array[stack->top]);
	return 0;
}

int stack_pop(global Stack *stack, global void ** ptr)
{
	if (stack->top > -1) {
		*ptr = stack->array[stack->top];
		stack->array[stack->top] = NULL;
		//stack->array = (void **)realloc(stack->array, stack->top * sizeof(void *));
		//Will need to reutnr



		stack->top--;
	}
	else {
		*ptr = NULL;
	}
	return 0;
}

int stack_peek(global Stack *stack) {
	if (stack->top == -1)
		return 0;
	int peek = stack->array[stack->top];
	return peek;
}

global void* malloc(size_t size, __global HeapStruct *heap)
{
	uint index = atomic_add(&heap->next, size);
	return &heap->heap + index;
}