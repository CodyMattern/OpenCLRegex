#pragma OPENCL EXTENSION cl_khr_global_int32_base_atomics: enable

bool push(global uint * stack, uint value) {
	// pushing from bot, so you can pop it from top later (FIFO)
	// circular buffer for top performance
	uint bufLen = 64;
	// zeroth element is counter for newest added element
	// first element is oldest element
	// circular buffer
	uint nextIndex = (stack[0] % bufLen + 2); // +2 because of top-bot headers
											  // if overflows, it overwrites oldest elements one by one
	stack[nextIndex] = value;
	// if overflows, it still increments
	stack[0]++;
	// simple and fast
	return true;
}
bool isEmpty(global uint * stack) {
	// tricky if you overflow both
	return (stack[0] == stack[1]);
}
uint peek(global uint * stack) {
	uint bufLen = 64;
	// oldest element value (top)
	uint ptr = stack[1] % bufLen + 2; // circular adr + 2 header
	return stack[ptr];
}
uint pop(global uint * stack) {
	uint bufLen = 64;
	uint ptr = stack[1] % bufLen + 2;
	// pop from top (oldest)
	uint returnValue = stack[ptr];
	stack[ptr] = 0;
	// this will be new top ctr for ptr
	stack[1]++;
	// if underflows, gets garbage, don't underflow
	return returnValue;
}
// A utility function to check if the given character is operand
int isOperand(char ch) {
	return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z');
}
// A utility function to return precedence of a given operator
// Higher returned value means higher precedence
int Prec(char ch) {
	switch (ch) {
	case '|':
		return 1;
	case '.':
		return 2;
	case '*':
	case '+':
	case '?':
		return 3;
	case '^':
		return 4;
	}
	return -1;
}

kernel void PostRegex(global char * exp, global uint * stack, global char * post) {
	int id = get_global_id(0);
	if (id == 0) {
		char c;
		char cPeek;
		int k = -1;
		int cPrec;
		int peekPrec;
		int stackSize = 0;
		for (int i = 0; exp[i]; i++) {
			c = exp[i];
			if (isOperand(c)) {
				post[++k] = c;
			}
			else if (Prec(c) > -1) {
				while (stackSize > 0 && Prec(peek(stack)) > -1) {
					cPeek = peek(stack);
					cPrec = Prec(c);
					peekPrec = Prec(cPeek);
					if (((cPrec == 2 || cPrec == 1) && peekPrec >= cPrec) || (cPrec >= 3 && peekPrec > cPrec)) {
						post[++k] = pop(stack);
						stackSize--;
					}
					else {
						break;
					}
				}
				push(stack, c);
				stackSize++;
			}
			else if (c == '(') {
				push(stack, c);
				stackSize++;
			}
			else if (c == ')') {
				cPeek = pop(stack);
				stackSize--;
				while (cPeek != '(') {
					post[++k] = cPeek;
					cPeek = pop(stack);
					stackSize--;
				}
			}
		}
		while (stackSize > 0) {
			post[++k] = pop(stack);
			stackSize--;
		}


		for (int i = 0; post[i]; i++) {
		}

	}
	barrier(CLK_LOCAL_MEM_FENCE | CLK_GLOBAL_MEM_FENCE);









}
